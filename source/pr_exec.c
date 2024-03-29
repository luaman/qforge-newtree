/*
	pr_exec.c

	(description)

	Copyright (C) 1996-1997  Id Software, Inc.

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

	$Id$
*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <stdarg.h>

#include "console.h"
#include "cvar.h"
#include "progs.h"
#include "server.h"
#include "sys.h"

char       *pr_opnames[] = {
	"DONE",

	"MUL_F",
	"MUL_V",
	"MUL_FV",
	"MUL_VF",

	"DIV",

	"ADD_F",
	"ADD_V",

	"SUB_F",
	"SUB_V",

	"EQ_F",
	"EQ_V",
	"EQ_S",
	"EQ_E",
	"EQ_FNC",

	"NE_F",
	"NE_V",
	"NE_S",
	"NE_E",
	"NE_FNC",

	"LE",
	"GE",
	"LT",
	"GT",

	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",
	"INDIRECT",

	"ADDRESS",

	"STORE_F",
	"STORE_V",
	"STORE_S",
	"STORE_ENT",
	"STORE_FLD",
	"STORE_FNC",

	"STOREP_F",
	"STOREP_V",
	"STOREP_S",
	"STOREP_ENT",
	"STOREP_FLD",
	"STOREP_FNC",

	"RETURN",

	"NOT_F",
	"NOT_V",
	"NOT_S",
	"NOT_ENT",
	"NOT_FNC",

	"IF",
	"IFNOT",

	"CALL0",
	"CALL1",
	"CALL2",
	"CALL3",
	"CALL4",
	"CALL5",
	"CALL6",
	"CALL7",
	"CALL8",

	"STATE",

	"GOTO",

	"AND",
	"OR",

	"BITAND",
	"BITOR"
};

//=============================================================================

/*
	PR_PrintStatement
*/
void
PR_PrintStatement (progs_t *pr, dstatement_t *s)
{
	int         i;

	if ((unsigned int) s->op < sizeof (pr_opnames) / sizeof (pr_opnames[0])) {
		Con_Printf ("%s ", pr_opnames[s->op]);
		i = strlen (pr_opnames[s->op]);
		for (; i < 10; i++)
			Con_Printf (" ");
	}

	if (s->op == OP_IF || s->op == OP_IFNOT)
		Con_Printf ("%sbranch %i", PR_GlobalString (pr, (unsigned short) s->a),
					s->b);
	else if (s->op == OP_GOTO) {
		Con_Printf ("branch %i", s->a);
	} else if ((unsigned int) (s->op - OP_STORE_F) < 6) {
		Con_Printf ("%s", PR_GlobalString (pr, (unsigned short) s->a));
		Con_Printf ("%s", PR_GlobalStringNoContents (pr, (unsigned short) s->b));
	} else {
		if (s->a)
			Con_Printf ("%s", PR_GlobalString (pr, (unsigned short) s->a));
		if (s->b)
			Con_Printf ("%s", PR_GlobalString (pr, (unsigned short) s->b));
		if (s->c)
			Con_Printf ("%s",
						PR_GlobalStringNoContents (pr, (unsigned short) s->c));
	}
	Con_Printf ("\n");
}

/*
	PR_StackTrace
*/
void
PR_StackTrace (progs_t *pr)
{
	dfunction_t *f;
	int         i;

	if (pr->pr_depth == 0) {
		Con_Printf ("<NO STACK>\n");
		return;
	}

	pr->pr_stack[pr->pr_depth].f = pr->pr_xfunction;
	for (i = pr->pr_depth; i >= 0; i--) {
		f = pr->pr_stack[i].f;

		if (!f) {
			Con_Printf ("<NO FUNCTION>\n");
		} else
			Con_Printf ("%12s : %s\n", PR_GetString (pr, f->s_file),
						PR_GetString (pr, f->s_name));
	}
}


/*
	PR_Profile
*/
void
PR_Profile (progs_t *pr)
{
	dfunction_t *f, *best;
	int         max;
	int         num;
	int         i;

	num = 0;
	do {
		max = 0;
		best = NULL;
		for (i = 0; i < pr->progs->numfunctions; i++) {
			f = &pr->pr_functions[i];
			if (f->profile > max) {
				max = f->profile;
				best = f;
			}
		}
		if (best) {
			if (num < 10)
				Con_Printf ("%7i %s\n", best->profile,
							PR_GetString (pr, best->s_name));
			num++;
			best->profile = 0;
		}
	} while (best);
}

/*
	PR_RunError

	Aborts the currently executing function
*/
void
PR_RunError (progs_t *pr, char *error, ...)
{
	va_list     argptr;
	char        string[1024];

	va_start (argptr, error);
	vsnprintf (string, sizeof (string), error, argptr);
	va_end (argptr);

	PR_PrintStatement (pr, pr->pr_statements + pr->pr_xstatement);
	PR_StackTrace (pr);
	Con_Printf ("%s\n", string);

	pr->pr_depth = 0;						// dump the stack so SV_Error can
	// shutdown functions

	SV_Error ("Program error");
}

/*
	PR_ExecuteProgram

	The interpretation main loop
*/

/*
	PR_EnterFunction

	Returns the new program statement counter
*/
int
PR_EnterFunction (progs_t *pr, dfunction_t *f)
{
	int         i, j, c, o;

	pr->pr_stack[pr->pr_depth].s = pr->pr_xstatement;
	pr->pr_stack[pr->pr_depth].f = pr->pr_xfunction;
	pr->pr_depth++;
	if (pr->pr_depth >= MAX_STACK_DEPTH)
		PR_RunError (pr, "stack overflow");

// save off any locals that the new function steps on
	c = f->locals;
	if (pr->localstack_used + c > LOCALSTACK_SIZE)
		PR_RunError (pr, "PR_ExecuteProgram: locals stack overflow\n");

	for (i = 0; i < c; i++)
		pr->localstack[pr->localstack_used + i] =
			((int *) pr->pr_globals)[f->parm_start + i];
	pr->localstack_used += c;

// copy parameters
	o = f->parm_start;
	for (i = 0; i < f->numparms; i++) {
		for (j = 0; j < f->parm_size[i]; j++) {
			((int *) pr->pr_globals)[o] = ((int *) pr->pr_globals)[OFS_PARM0 + i * 3 + j];
			o++;
		}
	}

	pr->pr_xfunction = f;
	return f->first_statement - 1;		// offset the s++
}

/*
	PR_LeaveFunction
*/
int
PR_LeaveFunction (progs_t *pr)
{
	int         i, c;

	if (pr->pr_depth <= 0)
		SV_Error ("prog stack underflow");

// restore locals from the stack
	c = pr->pr_xfunction->locals;
	pr->localstack_used -= c;
	if (pr->localstack_used < 0)
		PR_RunError (pr, "PR_ExecuteProgram: locals stack underflow\n");

	for (i = 0; i < c; i++)

		((int *) pr->pr_globals)[pr->pr_xfunction->parm_start + i] =
			pr->localstack[pr->localstack_used + i];

// up stack
	pr->pr_depth--;
	pr->pr_xfunction = pr->pr_stack[pr->pr_depth].f;
	return pr->pr_stack[pr->pr_depth].s;
}


/*
	PR_ExecuteProgram
*/
// LordHavoc: optimized
#define OPA ((eval_t *)&pr->pr_globals[(unsigned short) st->a])
#define OPB ((eval_t *)&pr->pr_globals[(unsigned short) st->b])
#define OPC ((eval_t *)&pr->pr_globals[(unsigned short) st->c])

extern cvar_t *pr_boundscheck;

void
PR_ExecuteProgram (progs_t *pr, func_t fnum)
{
	dstatement_t *st;
	dfunction_t *f, *newf;
	edict_t    *ed;
	int         exitdepth;
	eval_t     *ptr;
	int         profile, startprofile;

	if (!fnum || fnum >= pr->progs->numfunctions) {
		if (pr->pr_global_struct->self)
			ED_Print (pr, PROG_TO_EDICT (pr, pr->pr_global_struct->self));
		SV_Error ("PR_ExecuteProgram: NULL function");
	}

	f = &pr->pr_functions[fnum];

	pr->pr_trace = false;

// make a stack frame
	exitdepth = pr->pr_depth;

	st = &pr->pr_statements[PR_EnterFunction (pr, f)];
	startprofile = profile = 0;

	while (1) {
		st++;
		if (++profile > 1000000)		// LordHavoc: increased runaway loop
			// limit 10x
		{
			pr->pr_xstatement = st - pr->pr_statements;
			PR_RunError (pr, "runaway loop error");
		}

		if (pr->pr_trace)
			PR_PrintStatement (pr, st);

		switch (st->op) {
			case OP_ADD_F:
				OPC->_float = OPA->_float + OPB->_float;
				break;
			case OP_ADD_V:
				OPC->vector[0] = OPA->vector[0] + OPB->vector[0];
				OPC->vector[1] = OPA->vector[1] + OPB->vector[1];
				OPC->vector[2] = OPA->vector[2] + OPB->vector[2];
				break;
			case OP_SUB_F:
				OPC->_float = OPA->_float - OPB->_float;
				break;
			case OP_SUB_V:
				OPC->vector[0] = OPA->vector[0] - OPB->vector[0];
				OPC->vector[1] = OPA->vector[1] - OPB->vector[1];
				OPC->vector[2] = OPA->vector[2] - OPB->vector[2];
				break;
			case OP_MUL_F:
				OPC->_float = OPA->_float * OPB->_float;
				break;
			case OP_MUL_V:
				OPC->_float =
					OPA->vector[0] * OPB->vector[0] +
					OPA->vector[1] * OPB->vector[1] +
					OPA->vector[2] * OPB->vector[2];
				break;
			case OP_MUL_FV:
				OPC->vector[0] = OPA->_float * OPB->vector[0];
				OPC->vector[1] = OPA->_float * OPB->vector[1];
				OPC->vector[2] = OPA->_float * OPB->vector[2];
				break;
			case OP_MUL_VF:
				OPC->vector[0] = OPB->_float * OPA->vector[0];
				OPC->vector[1] = OPB->_float * OPA->vector[1];
				OPC->vector[2] = OPB->_float * OPA->vector[2];
				break;
			case OP_DIV_F:
				OPC->_float = OPA->_float / OPB->_float;
				break;
			case OP_BITAND:
				OPC->_float = (int) OPA->_float & (int) OPB->_float;
				break;
			case OP_BITOR:
				OPC->_float = (int) OPA->_float | (int) OPB->_float;
				break;
			case OP_GE:
				OPC->_float = OPA->_float >= OPB->_float;
				break;
			case OP_LE:
				OPC->_float = OPA->_float <= OPB->_float;
				break;
			case OP_GT:
				OPC->_float = OPA->_float > OPB->_float;
				break;
			case OP_LT:
				OPC->_float = OPA->_float < OPB->_float;
				break;
			case OP_AND:
				OPC->_float = OPA->_float && OPB->_float;
				break;
			case OP_OR:
				OPC->_float = OPA->_float || OPB->_float;
				break;
			case OP_NOT_F:
				OPC->_float = !OPA->_float;
				break;
			case OP_NOT_V:
				OPC->_float = VectorIsNull(OPA->vector);
				break;
			case OP_NOT_S:
				OPC->_float = !OPA->string || !*PR_GetString (pr, OPA->string);
				break;
			case OP_NOT_FNC:
				OPC->_float = !OPA->function;
				break;
			case OP_NOT_ENT:
				OPC->_float = (PROG_TO_EDICT (pr, OPA->edict) == *pr->edicts);
				break;
			case OP_EQ_F:
				OPC->_float = OPA->_float == OPB->_float;
				break;
			case OP_EQ_V:
				OPC->_float = (OPA->vector[0] == OPB->vector[0])
					&& (OPA->vector[1] == OPB->vector[1])
					&& (OPA->vector[2] == OPB->vector[2]);
				break;
			case OP_EQ_S:
				OPC->_float =
					!strcmp (PR_GetString (pr, OPA->string),
							 PR_GetString (pr, OPB->string));
				break;
			case OP_EQ_E:
				OPC->_float = OPA->_int == OPB->_int;
				break;
			case OP_EQ_FNC:
				OPC->_float = OPA->function == OPB->function;
				break;
			case OP_NE_F:
				OPC->_float = OPA->_float != OPB->_float;
				break;
			case OP_NE_V:
				OPC->_float = (OPA->vector[0] != OPB->vector[0])
					|| (OPA->vector[1] != OPB->vector[1])
					|| (OPA->vector[2] != OPB->vector[2]);
				break;
			case OP_NE_S:
				OPC->_float =
					strcmp (PR_GetString (pr, OPA->string),
							PR_GetString (pr, OPB->string));
				break;
			case OP_NE_E:
				OPC->_float = OPA->_int != OPB->_int;
				break;
			case OP_NE_FNC:
				OPC->_float = OPA->function != OPB->function;
				break;

				// ==================
			case OP_STORE_F:
			case OP_STORE_ENT:
			case OP_STORE_FLD:			// integers
			case OP_STORE_S:
			case OP_STORE_FNC:			// pointers
				OPB->_int = OPA->_int;
				break;
			case OP_STORE_V:
				OPB->vector[0] = OPA->vector[0];
				OPB->vector[1] = OPA->vector[1];
				OPB->vector[2] = OPA->vector[2];
				break;

			case OP_STOREP_F:
			case OP_STOREP_ENT:
			case OP_STOREP_FLD:		// integers
			case OP_STOREP_S:
			case OP_STOREP_FNC:		// pointers
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int + 4 > pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an out of bounds edict\n");
					return;
				}
				if (pr_boundscheck->int_val && (OPB->_int % pr->pr_edict_size <
												((byte *) & (*pr->edicts)->v -
												 (byte *) *pr->edicts))) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an engine edict field\n");
					return;
				}
				ptr = (eval_t *) ((byte *) *pr->edicts + OPB->_int);
				ptr->_int = OPA->_int;
				break;
			case OP_STOREP_V:
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int + 12 > pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an out of bounds edict\n");
					return;
				}
				ptr = (eval_t *) ((byte *) *pr->edicts + OPB->_int);
				ptr->vector[0] = OPA->vector[0];
				ptr->vector[1] = OPA->vector[1];
				ptr->vector[2] = OPA->vector[2];
				break;
			case OP_ADDRESS:
				if (pr_boundscheck->int_val
					&& (OPA->edict < 0 || OPA->edict >= pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to address an out of bounds edict\n");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPA->edict == 0 && pr->null_bad)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError (pr, "assignment to world entity");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int >= pr->progs->entityfields)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to address an invalid field in an edict\n");
					return;
				}
				ed = PROG_TO_EDICT (pr, OPA->edict);
				OPC->_int =
					(byte *) ((int *) &ed->v + OPB->_int) - (byte *) *pr->edicts;
				break;
			case OP_LOAD_F:
			case OP_LOAD_FLD:
			case OP_LOAD_ENT:
			case OP_LOAD_S:
			case OP_LOAD_FNC:
				if (pr_boundscheck->int_val
					&& (OPA->edict < 0 || OPA->edict >= pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an out of bounds edict number\n");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int >= pr->progs->entityfields)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an invalid field in an edict\n");
					return;
				}
				ed = PROG_TO_EDICT (pr, OPA->edict);
				OPC->_int = ((eval_t *) ((int *) &ed->v + OPB->_int))->_int;
				break;
			case OP_LOAD_V:
				if (pr_boundscheck->int_val
					&& (OPA->edict < 0 || OPA->edict >= pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an out of bounds edict number\n");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int + 2 >= pr->progs->entityfields)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an invalid field in an edict\n");
					return;
				}
				ed = PROG_TO_EDICT (pr, OPA->edict);
				OPC->vector[0] =
					((eval_t *) ((int *) &ed->v + OPB->_int))->vector[0];
				OPC->vector[1] =
					((eval_t *) ((int *) &ed->v + OPB->_int))->vector[1];
				OPC->vector[2] =
					((eval_t *) ((int *) &ed->v + OPB->_int))->vector[2];
				break;
				// ==================
			case OP_IFNOT:
				if (!OPA->_int)
					st += st->b - 1;	// offset the s++
				break;
			case OP_IF:
				if (OPA->_int)
					st += st->b - 1;	// offset the s++
				break;
			case OP_GOTO:
				st += st->a - 1;		// offset the s++
				break;
			case OP_CALL0:
			case OP_CALL1:
			case OP_CALL2:
			case OP_CALL3:
			case OP_CALL4:
			case OP_CALL5:
			case OP_CALL6:
			case OP_CALL7:
			case OP_CALL8:
				pr->pr_xfunction->profile += profile - startprofile;
				startprofile = profile;
				pr->pr_xstatement = st - pr->pr_statements;
				pr->pr_argc = st->op - OP_CALL0;
				if (!OPA->function)
					PR_RunError (pr, "NULL function");
				newf = &pr->pr_functions[OPA->function];
				if (newf->first_statement < 0) {	// negative
					// statements are
					// built in functions
					int         i = -newf->first_statement;

					if (i >= pr_numbuiltins)
						PR_RunError (pr, "Bad builtin call number");
					pr_builtins[i] (pr);
					break;
				}

				st = &pr->pr_statements[PR_EnterFunction (pr, newf)];
				break;
			case OP_DONE:
			case OP_RETURN:
				pr->pr_globals[OFS_RETURN] = pr->pr_globals[(unsigned short) st->a];
				pr->pr_globals[OFS_RETURN + 1] =
					pr->pr_globals[(unsigned short) st->a + 1];
				pr->pr_globals[OFS_RETURN + 2] =
					pr->pr_globals[(unsigned short) st->a + 2];
				st = &pr->pr_statements[PR_LeaveFunction (pr)];
				if (pr->pr_depth == exitdepth)
					return;				// all done
				break;
			case OP_STATE:
				ed = PROG_TO_EDICT (pr, pr->pr_global_struct->self);
				ed->v.v.nextthink = pr->pr_global_struct->time + 0.1;
				ed->v.v.frame = OPA->_float;
				ed->v.v.think = OPB->function;
				break;
// LordHavoc: to be enabled when Progs version 7 (or whatever it will be numbered) is finalized
/*
			case OP_ADD_I:
				OPC->_int = OPA->_int + OPB->_int;
				break;
			case OP_ADD_IF:
				OPC->_int = OPA->_int + (int) OPB->_float;
				break;
			case OP_ADD_FI:
				OPC->_float = OPA->_float + (float) OPB->_int;
				break;
			case OP_SUB_I:
				OPC->_int = OPA->_int - OPB->_int;
				break;
			case OP_SUB_IF:
				OPC->_int = OPA->_int - (int) OPB->_float;
				break;
			case OP_SUB_FI:
				OPC->_float = OPA->_float - (float) OPB->_int;
				break;
			case OP_MUL_I:
				OPC->_int = OPA->_int * OPB->_int;
				break;
			case OP_MUL_IF:
				OPC->_int = OPA->_int * (int) OPB->_float;
				break;
			case OP_MUL_FI:
				OPC->_float = OPA->_float * (float) OPB->_int;
				break;
			case OP_MUL_VI:
				OPC->vector[0] = (float) OPB->_int * OPA->vector[0];
				OPC->vector[1] = (float) OPB->_int * OPA->vector[1];
				OPC->vector[2] = (float) OPB->_int * OPA->vector[2];
				break;
			case OP_DIV_VF:
				{
					float       temp = 1.0f / OPB->_float;

					OPC->vector[0] = temp * OPA->vector[0];
					OPC->vector[1] = temp * OPA->vector[1];
					OPC->vector[2] = temp * OPA->vector[2];
				}
				break;
			case OP_DIV_I:
				OPC->_int = OPA->_int / OPB->_int;
				break;
			case OP_DIV_IF:
				OPC->_int = OPA->_int / (int) OPB->_float;
				break;
			case OP_DIV_FI:
				OPC->_float = OPA->_float / (float) OPB->_int;
				break;
			case OP_CONV_IF:
				OPC->_float = OPA->_int;
				break;
			case OP_CONV_FI:
				OPC->_int = OPA->_float;
				break;
			case OP_BITAND_I:
				OPC->_int = OPA->_int & OPB->_int;
				break;
			case OP_BITOR_I:
				OPC->_int = OPA->_int | OPB->_int;
				break;
			case OP_BITAND_IF:
				OPC->_int = OPA->_int & (int) OPB->_float;
				break;
			case OP_BITOR_IF:
				OPC->_int = OPA->_int | (int) OPB->_float;
				break;
			case OP_BITAND_FI:
				OPC->_float = (int) OPA->_float & OPB->_int;
				break;
			case OP_BITOR_FI:
				OPC->_float = (int) OPA->_float | OPB->_int;
				break;
			case OP_GE_I:
				OPC->_float = OPA->_int >= OPB->_int;
				break;
			case OP_LE_I:
				OPC->_float = OPA->_int <= OPB->_int;
				break;
			case OP_GT_I:
				OPC->_float = OPA->_int > OPB->_int;
				break;
			case OP_LT_I:
				OPC->_float = OPA->_int < OPB->_int;
				break;
			case OP_AND_I:
				OPC->_float = OPA->_int && OPB->_int;
				break;
			case OP_OR_I:
				OPC->_float = OPA->_int || OPB->_int;
				break;
			case OP_GE_IF:
				OPC->_float = (float) OPA->_int >= OPB->_float;
				break;
			case OP_LE_IF:
				OPC->_float = (float) OPA->_int <= OPB->_float;
				break;
			case OP_GT_IF:
				OPC->_float = (float) OPA->_int > OPB->_float;
				break;
			case OP_LT_IF:
				OPC->_float = (float) OPA->_int < OPB->_float;
				break;
			case OP_AND_IF:
				OPC->_float = (float) OPA->_int && OPB->_float;
				break;
			case OP_OR_IF:
				OPC->_float = (float) OPA->_int || OPB->_float;
				break;
			case OP_GE_FI:
				OPC->_float = OPA->_float >= (float) OPB->_int;
				break;
			case OP_LE_FI:
				OPC->_float = OPA->_float <= (float) OPB->_int;
				break;
			case OP_GT_FI:
				OPC->_float = OPA->_float > (float) OPB->_int;
				break;
			case OP_LT_FI:
				OPC->_float = OPA->_float < (float) OPB->_int;
				break;
			case OP_AND_FI:
				OPC->_float = OPA->_float && (float) OPB->_int;
				break;
			case OP_OR_FI:
				OPC->_float = OPA->_float || (float) OPB->_int;
				break;
			case OP_NOT_I:
				OPC->_float = !OPA->_int;
				break;
			case OP_EQ_I:
				OPC->_float = OPA->_int == OPB->_int;
				break;
			case OP_EQ_IF:
				OPC->_float = (float) OPA->_int == OPB->_float;
				break;
			case OP_EQ_FI:
				OPC->_float = OPA->_float == (float) OPB->_int;
				break;
			case OP_NE_I:
				OPC->_float = OPA->_int != OPB->_int;
				break;
			case OP_NE_IF:
				OPC->_float = (float) OPA->_int != OPB->_float;
				break;
			case OP_NE_FI:
				OPC->_float = OPA->_float != (float) OPB->_int;
				break;
			case OP_STORE_I:
				OPB->_int = OPA->_int;
				break;
			case OP_STOREP_I:
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int + 4 > pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an out of bounds edict\n");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPB->_int % pr->pr_edict_size <
						((byte *) & (*pr->edicts)->v - (byte *) *pr->edicts))) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an engine edict field\n");
					return;
				}
				ptr = (eval_t *) ((byte *) *pr->edicts + OPB->_int);
				ptr->_int = OPA->_int;
				break;
			case OP_LOAD_I:
				if (pr_boundscheck->int_val
					&& (OPA->edict < 0 || OPA->edict >= pr->pr_edictareasize)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an out of bounds edict number\n");
					return;
				}
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int >= pr->progs->entityfields)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an invalid field in an edict\n");
					return;
				}
				ed = PROG_TO_EDICT (pr, OPA->edict);
				OPC->_int = ((eval_t *) ((int *) &ed->v + OPB->_int))->_int;
				break;

			case OP_GSTOREP_I:
			case OP_GSTOREP_F:
			case OP_GSTOREP_ENT:
			case OP_GSTOREP_FLD:	// integers
			case OP_GSTOREP_S:
			case OP_GSTOREP_FNC:	// pointers
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int >= pr->pr_globaldefs)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an invalid indexed global\n");
					return;
				}
				pr->pr_globals[OPB->_int] = OPA->_float;
				break;
			case OP_GSTOREP_V:
				if (pr_boundscheck->int_val
					&& (OPB->_int < 0 || OPB->_int + 2 >= pr->pr_globaldefs)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to write to an invalid indexed global\n");
					return;
				}
				pr->pr_globals[OPB->_int] = OPA->vector[0];
				pr->pr_globals[OPB->_int + 1] = OPA->vector[1];
				pr->pr_globals[OPB->_int + 2] = OPA->vector[2];
				break;

			case OP_GADDRESS:
				i = OPA->_int + (int) OPB->_float;
				if (pr_boundscheck->int_val
					&& (i < 0 || i >= pr->pr_globaldefs)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to address an out of bounds global\n");
					return;
				}
				OPC->_float = pr->pr_globals[i];
				break;

			case OP_GLOAD_I:
			case OP_GLOAD_F:
			case OP_GLOAD_FLD:
			case OP_GLOAD_ENT:
			case OP_GLOAD_S:
			case OP_GLOAD_FNC:
				if (pr_boundscheck->int_val
					&& (OPA->_int < 0 || OPA->_int >= pr->pr_globaldefs)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an invalid indexed global\n");
					return;
				}
				OPC->_float = pr->pr_globals[OPA->_int];
				break;

			case OP_GLOAD_V:
				if (pr_boundscheck->int_val
					&& (OPA->_int < 0 || OPA->_int + 2 >= pr->pr_globaldefs)) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs attempted to read an invalid indexed global\n");
					return;
				}
				OPC->vector[0] = pr->pr_globals[OPA->_int];
				OPC->vector[1] = pr->pr_globals[OPA->_int + 1];
				OPC->vector[2] = pr->pr_globals[OPA->_int + 2];
				break;

			case OP_BOUNDCHECK:
				if (OPA->_int < 0 || OPA->_int >= st->b) {
					pr->pr_xstatement = st - pr->pr_statements;
					PR_RunError
						(pr, "Progs boundcheck failed at line number %d, value is < 0 or >= %d\n",
						 st->b, st->c);
					return;
				}
				break;

*/
			default:
				pr->pr_xstatement = st - pr->pr_statements;
				PR_RunError (pr, "Bad opcode %i", st->op);
		}
	}
}

char       *
PR_GetString (progs_t *pr, int num)
{
	if (num < 0) {
		// Con_DPrintf("GET:%d == %s\n", num, pr->pr_strtbl[-num]);
		return pr->pr_strtbl[-num];
	}
	return pr->pr_strings + num;
}

int
PR_SetString (progs_t *pr, char *s)
{
	int         i;

	if (s - pr->pr_strings < 0) {
		for (i = 0; i <= pr->num_prstr; i++)
			if (pr->pr_strtbl[i] == s)
				break;
		if (i < pr->num_prstr)
			return -i;
		if (pr->num_prstr == MAX_PRSTR - 1)
			Sys_Error ("MAX_PRSTR");
		pr->num_prstr++;
		pr->pr_strtbl[pr->num_prstr] = s;
		// Con_DPrintf("SET:%d == %s\n", -pr->num_prstr, s);
		return -pr->num_prstr;
	}
	return (int) (s - pr->pr_strings);
}
