/*
	sv_progs.c

	Quick QuakeC server code

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
#include "string.h"
#endif
#ifdef HAVE_STRINGS_H
#include "strings.h"
#endif

#include "cmd.h"
#include "progs.h"
#include "server.h"

int eval_alpha, eval_scale, eval_glowsize, eval_glowcolor, eval_colormod;
progs_t	sv_progs;

func_t	EndFrame;
func_t	SpectatorConnect;
func_t	SpectatorDisconnect;
func_t	SpectatorThink;

void
FindEdictFieldOffsets (progs_t *pr)
{
	dfunction_t *f;

	if (pr == &sv_progs) {
		// Zoid, find the spectator functions
		SpectatorConnect = SpectatorThink = SpectatorDisconnect = 0;

		if ((f = ED_FindFunction (&sv_progs, "SpectatorConnect")) != NULL)
			SpectatorConnect = (func_t) (f - sv_progs.pr_functions);
		if ((f = ED_FindFunction (&sv_progs, "SpectatorThink")) != NULL)
			SpectatorThink = (func_t) (f - sv_progs.pr_functions);
		if ((f = ED_FindFunction (&sv_progs, "SpectatorDisconnect")) != NULL)
			SpectatorDisconnect = (func_t) (f - sv_progs.pr_functions);

		// 2000-01-02 EndFrame function by Maddes/FrikaC
		EndFrame = 0;
		if ((f = ED_FindFunction (&sv_progs, "EndFrame")) != NULL)
			EndFrame = (func_t) (f - sv_progs.pr_functions);

		eval_alpha = FindFieldOffset (&sv_progs, "alpha");
		eval_scale = FindFieldOffset (&sv_progs, "scale");
		eval_glowsize = FindFieldOffset (&sv_progs, "glow_size");
		eval_glowcolor = FindFieldOffset (&sv_progs, "glow_color");
		eval_colormod = FindFieldOffset (&sv_progs, "colormod");
	}
}

void
SV_Progs_Init (void)
{
	sv_progs.edicts = &sv.edicts;
	sv_progs.num_edicts = &sv.num_edicts;
	sv_progs.time = &sv.time;
}

void
ED_PrintEdicts_f (void)
{
	ED_PrintEdicts (&sv_progs);
}

/*
	ED_PrintEdict_f

	For debugging, prints a single edicy
*/
void
ED_PrintEdict_f (void)
{
	int         i;

	i = atoi (Cmd_Argv (1));
	Con_Printf ("\n EDICT %i:\n", i);
	ED_PrintNum (&sv_progs, i);
}

void
ED_Count_f (void)
{
	ED_Count (&sv_progs);
}

void
PR_Profile_f (void)
{
	PR_Profile (&sv_progs);
}

int
ED_Parse_Extra_Fields (progs_t *pr, char *key, char *value)
{
	// If skyname is set, we want to allow skyboxes and set what
	// the skybox name should be.  "qlsky" is supported since
	// at least one other map uses it already.  --KB
	if (stricmp (key, "sky") == 0 ||		// LordHavoc: added "sky" key 
											// (Quake2 and DarkPlaces use 
											// this)
		stricmp (key, "skyname") == 0 ||
		stricmp (key, "qlsky") == 0) {
		Info_SetValueForKey (svs.info, "skybox",
							 "1", MAX_SERVERINFO_STRING);
		Cvar_Set (r_skyname, value);
		return 1;
	}
	return 0;
}
