/*
        pr_offs.c

        Quick QuakeC offset access

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
# include <config.h>
#endif
#include "pr_comp.h"
#include "progs.h"
#include "console.h"
#include "server.h"
#include "world.h"
#include "msg.h"
#include "cmd.h"
#include "commdef.h"
#include "info.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>


int eval_alpha, eval_fullbright, eval_colormod, eval_glowsize, eval_glowcolor;

int FindFieldOffset(char *field)
{
	ddef_t *d;
	d = ED_FindField(field);
        if (!d)
		return 0;

	return d->ofs*4;
}


eval_t *GETEDICTFIELDVALUE(edict_t *ed, int fieldoffset)
{
        if (!fieldoffset)
               return NULL;

	return (eval_t*)((char*)&ed->v + fieldoffset);
}

void FindEdictFieldOffsets()
{
	eval_alpha = FindFieldOffset("alpha");
        eval_fullbright = FindFieldOffset("fullbright");
        eval_colormod = FindFieldOffset("colormod");
        eval_glowsize = FindFieldOffset("glow_size");
        eval_glowcolor = FindFieldOffset("glow_color");
};
