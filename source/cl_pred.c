/*
	cl_pred.c

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

#include <math.h>

#include "bothdefs.h"
#include "cl_ents.h"
#include "client.h"
#include "commdef.h"
#include "console.h"
#include "cvar.h"
#include "pmove.h"

cvar_t     *cl_nopred;
cvar_t     *cl_pushlatency;

extern frame_t *view_frame;

/*
	CL_PredictUsercmd
*/
void
CL_PredictUsercmd (player_state_t * from, player_state_t * to, usercmd_t *u,
				   qboolean spectator)
{
	// split up very long moves
	if (u->msec > 50) {
		player_state_t temp;
		usercmd_t   split;

		split = *u;
		split.msec /= 2;

		CL_PredictUsercmd (from, &temp, &split, spectator);
		CL_PredictUsercmd (&temp, to, &split, spectator);
		return;
	}

	VectorCopy (from->origin, pmove.origin);
//  VectorCopy (from->viewangles, pmove.angles);
	VectorCopy (u->angles, pmove.angles);
	VectorCopy (from->velocity, pmove.velocity);

	pmove.oldbuttons = from->oldbuttons;
	pmove.waterjumptime = from->waterjumptime;
	pmove.dead = cl.stats[STAT_HEALTH] <= 0;
	pmove.spectator = spectator;
	pmove.flying = cl.stats[STAT_FLYMODE];

	pmove.cmd = *u;

	PlayerMove ();
//for (i=0 ; i<3 ; i++)
//pmove.origin[i] = ((int)(pmove.origin[i]*8))*0.125;
	to->waterjumptime = pmove.waterjumptime;
	to->oldbuttons = pmove.oldbuttons;	// Tonik
//  to->oldbuttons = pmove.cmd.buttons;
	VectorCopy (pmove.origin, to->origin);
	VectorCopy (pmove.angles, to->viewangles);
	VectorCopy (pmove.velocity, to->velocity);
	to->onground = onground;

	to->weaponframe = from->weaponframe;
}



/*
	CL_PredictMove
*/
void
CL_PredictMove (void)
{
	int         i;
	float       f;
	frame_t    *from, *to = NULL;
	int         oldphysent;

	if (cl_pushlatency->value > 0)
		Cvar_Set (cl_pushlatency, "0");

	if (cl.paused)
		return;

	cl.onground = 0;	// assume on ground unless prediction says different

	cl.time = realtime - cls.latency - cl_pushlatency->value * 0.001;
	if (cl.time > realtime)
		cl.time = realtime;

	if (cl.intermission)
		return;

	if (!cl.validsequence)
		return;

	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_sequence >=
		UPDATE_BACKUP - 1)
		return;

	VectorCopy (cl.viewangles, cl.simangles);

	// this is the last frame received from the server
	from = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];

	// we can now render a frame
	if (cls.state == ca_onserver) {		// first update is the final signon
										// stage
		VID_SetCaption (cls.servername);
		cls.state = ca_active;
	}

	if (cl_nopred->int_val) {
		VectorCopy (from->playerstate[cl.playernum].velocity, cl.simvel);
		VectorCopy (from->playerstate[cl.playernum].origin, cl.simorg);
		return;
	}
	// predict forward until cl.time <= to->senttime
	oldphysent = pmove.numphysent;
	CL_SetSolidPlayers (cl.playernum);

//  to = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];

	for (i = 1; i < UPDATE_BACKUP - 1 && cls.netchan.incoming_sequence + i <
		 cls.netchan.outgoing_sequence; i++) {
		to = &cl.frames[(cls.netchan.incoming_sequence + i) & UPDATE_MASK];
		CL_PredictUsercmd (&from->playerstate[cl.playernum]
						   , &to->playerstate[cl.playernum], &to->cmd,
						   cl.spectator);
		cl.onground = onground;
		if (to->senttime >= cl.time)
			break;
		from = to;
	}

	pmove.numphysent = oldphysent;

	if (i == UPDATE_BACKUP - 1 || !to)
		return;							// net hasn't deliver packets in a
										// long time...

	// now interpolate some fraction of the final frame
	if (to->senttime == from->senttime)
		f = 0;
	else {
		f = (cl.time - from->senttime) / (to->senttime - from->senttime);

		if (f < 0)
			f = 0;
		if (f > 1)
			f = 1;
	}

	for (i = 0; i < 3; i++)
		if (fabs
			(from->playerstate[cl.playernum].origin[i] -
			 to->playerstate[cl.playernum].origin[i]) > 128) {	// teleported, 
																// so don't
																// lerp
			VectorCopy (to->playerstate[cl.playernum].velocity, cl.simvel);
			VectorCopy (to->playerstate[cl.playernum].origin, cl.simorg);
			return;
		}

	for (i = 0; i < 3; i++) {
		cl.simorg[i] = from->playerstate[cl.playernum].origin[i]
			+ f * (to->playerstate[cl.playernum].origin[i] -
				   from->playerstate[cl.playernum].origin[i]);
		cl.simvel[i] = from->playerstate[cl.playernum].velocity[i]
			+ f * (to->playerstate[cl.playernum].velocity[i] -
				   from->playerstate[cl.playernum].velocity[i]);
	}
}


/*
	CL_Prediction_Init_Cvars
*/
void
CL_Prediction_Init_Cvars (void)
{
/* I'm not totally sure what cl_pushlatency is for. Or if it is SUPPOSED TO BE SETTABLE. */
	cl_pushlatency = Cvar_Get ("pushlatency", "-999", CVAR_NONE, "How much prediction should the client make");
	cl_nopred = Cvar_Get ("cl_nopred", "0", CVAR_NONE, "Set to turn off client prediction");
}
