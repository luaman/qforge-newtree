/*
	r_view.c

	player eye positioning

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

#include <stdlib.h>
#include <math.h>

#include "bothdefs.h"
#include "client.h"
#include "cmd.h"
#include "cvar.h"
#include "host.h"
#include "msg.h"
#include "pmove.h"
#include "screen.h"
#include "vid.h"
#include "view.h"

/*

The view is allowed to move slightly from it's true position for bobbing,
but if it exceeds 8 pixels linear distance (spherical, not box), the list of
entities sent from the server may not include everything in the pvs, especially
when crossing a water boudnary.

*/

cvar_t     *cl_rollspeed;
cvar_t     *cl_rollangle;

cvar_t     *cl_bob;
cvar_t     *cl_bobcycle;
cvar_t     *cl_bobup;

cvar_t     *v_kicktime;
cvar_t     *v_kickroll;
cvar_t     *v_kickpitch;

cvar_t     *v_iyaw_cycle;
cvar_t     *v_iroll_cycle;
cvar_t     *v_ipitch_cycle;
cvar_t     *v_iyaw_level;
cvar_t     *v_iroll_level;
cvar_t     *v_ipitch_level;

cvar_t     *v_idlescale;

float       v_dmg_time, v_dmg_roll, v_dmg_pitch;

extern int  in_forward, in_forward2, in_back;

frame_t    *view_frame;
player_state_t *view_message;

void BuildGammaTable (float, float);

/*
	V_CalcRoll
*/
float
V_CalcRoll (vec3_t angles, vec3_t velocity)
{
	vec3_t      forward, right, up;
	float       sign;
	float       side;
	float       value;

	AngleVectors (angles, forward, right, up);
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs (side);

	value = cl_rollangle->value;

	if (side < cl_rollspeed->value)
		side = side * value / cl_rollspeed->value;
	else
		side = value;

	return side * sign;

}


/*
	V_CalcBob
*/
float
V_CalcBob (void)
{
	static double bobtime;
	static float bob;
	float       cycle;

	if (cl.spectator)
		return 0;

	if (onground == -1)
		return bob;						// just use old value

	bobtime += host_frametime;
	cycle = bobtime - (int) (bobtime / cl_bobcycle->value) * cl_bobcycle->value;
	cycle /= cl_bobcycle->value;
	if (cycle < cl_bobup->value)
		cycle = M_PI * cycle / cl_bobup->value;
	else
		cycle =
			M_PI + M_PI * (cycle - cl_bobup->value) / (1.0 - cl_bobup->value);

// bob is proportional to simulated velocity in the xy plane
// (don't count Z, or jumping messes it up)

	bob =
		sqrt (cl.simvel[0] * cl.simvel[0] +
			  cl.simvel[1] * cl.simvel[1]) * cl_bob->value;
	bob = bob * 0.3 + bob * 0.7 * sin (cycle);
	if (bob > 4)
		bob = 4;
	else if (bob < -7)
		bob = -7;
	return bob;

}


//=============================================================================


cvar_t     *v_centermove;
cvar_t     *v_centerspeed;


void
V_StartPitchDrift (void)
{
#if 1
	if (cl.laststop == cl.time) {
		return;							// something else is keeping it from
										// drifting
	}
#endif
	if (cl.nodrift || !cl.pitchvel) {
		cl.pitchvel = v_centerspeed->value;
		cl.nodrift = false;
		cl.driftmove = 0;
	}
}

void
V_StopPitchDrift (void)
{
	cl.laststop = cl.time;
	cl.nodrift = true;
	cl.pitchvel = 0;
}

/*
	V_DriftPitch

	Moves the client pitch angle towards cl.idealpitch sent by the server.

	If the user is adjusting pitch manually, either with lookup/lookdown,
	mlook and mouse, or klook and keyboard, pitch drifting is constantly
	stopped.

	Drifting is enabled when the center view key is hit, mlook is released
	and lookspring is non 0, or when 
*/
void
V_DriftPitch (void)
{
	float       delta, move;

	if (view_message->onground == -1 || cls.demoplayback) {
		cl.driftmove = 0;
		cl.pitchvel = 0;
		return;
	}
// don't count small mouse motion
	if (cl.nodrift) {
		if (fabs
			(cl.frames[(cls.netchan.outgoing_sequence - 1) & UPDATE_MASK].cmd.
			 forwardmove) < 200)
			cl.driftmove = 0;
		else
			cl.driftmove += host_frametime;

		if (cl.driftmove > v_centermove->value) {
			V_StartPitchDrift ();
		}
		return;
	}

	delta = 0 - cl.viewangles[PITCH];

	if (!delta) {
		cl.pitchvel = 0;
		return;
	}

	move = host_frametime * cl.pitchvel;
	cl.pitchvel += host_frametime * v_centerspeed->value;

//Con_Printf ("move: %f (%f)\n", move, host_frametime);

	if (delta > 0) {
		if (move > delta) {
			cl.pitchvel = 0;
			move = delta;
		}
		cl.viewangles[PITCH] += move;
	} else if (delta < 0) {
		if (move > -delta) {
			cl.pitchvel = 0;
			move = -delta;
		}
		cl.viewangles[PITCH] -= move;
	}
}

/*
						PALETTE FLASHES 
*/

extern cvar_t	*cl_cshift_bonus;
extern cvar_t	*cl_cshift_contents;
extern cvar_t	*cl_cshift_damage;
// extern cvar_t	*cl_cshift_powerup;

cshift_t    cshift_empty = { {130, 80, 50}, 0 };
cshift_t    cshift_water = { {130, 80, 50}, 128 };
cshift_t    cshift_slime = { {0, 25, 5}, 150 };
cshift_t    cshift_lava = { {255, 80, 0}, 150 };

extern byte 	gammatable[256];			// palette is sent through this
extern cvar_t	*vid_gamma;

/*
	V_CheckGamma
*/
qboolean
V_CheckGamma (void)
{
	static float oldgamma;

	if (vid_gamma) {		// might get called before vid_gamma gets set
		if (oldgamma == vid_gamma->value)
			return false;

		oldgamma = vid_gamma->value;
	}

	vid.recalc_refdef = 1;				// force a surface cache flush

	return true;
}

/*
	V_ParseDamage
*/
void
V_ParseDamage (void)
{

	int         armor, blood;
	vec3_t      from;
	int         i;
	vec3_t      forward, right, up;
	float       side;
	float       count;

	armor = MSG_ReadByte ();
	blood = MSG_ReadByte ();
	for (i = 0; i < 3; i++)
		from[i] = MSG_ReadCoord ();

	count = blood * 0.5 + armor * 0.5;
	if (count < 10)
		count = 10;

	cl.faceanimtime = cl.time + 0.2;	// but sbar face into pain frame

	if (cl_cshift_damage->int_val
		|| (atoi (Info_ValueForKey (cl.serverinfo, "cshifts")) & INFO_CSHIFT_DAMAGE)) {

		cl.cshifts[CSHIFT_DAMAGE].percent += 3 * count;
		cl.cshifts[CSHIFT_DAMAGE].percent = bound (0, cl.cshifts[CSHIFT_DAMAGE].percent, 150);

		if (armor > blood) {
			cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 200;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 100;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 100;
		} else if (armor) {
			cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 220;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 50;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 50;
		} else {
			cl.cshifts[CSHIFT_DAMAGE].destcolor[0] = 255;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[1] = 0;
			cl.cshifts[CSHIFT_DAMAGE].destcolor[2] = 0;
		}
	}
	
//
// calculate view angle kicks
//
	VectorSubtract (from, cl.simorg, from);
	VectorNormalize (from);

	AngleVectors (cl.simangles, forward, right, up);

	side = DotProduct (from, right);
	v_dmg_roll = count * side * v_kickroll->value;

	side = DotProduct (from, forward);
	v_dmg_pitch = count * side * v_kickpitch->value;

	v_dmg_time = v_kicktime->value;
}


/*
	V_cshift_f
*/
void
V_cshift_f (void)
{
	cshift_empty.destcolor[0] = atoi (Cmd_Argv (1));
	cshift_empty.destcolor[1] = atoi (Cmd_Argv (2));
	cshift_empty.destcolor[2] = atoi (Cmd_Argv (3));
	cshift_empty.percent = atoi (Cmd_Argv (4));
}


/*
	V_BonusFlash_f

	When you run over an item, the server sends this command
*/
void
V_BonusFlash_f (void)
{
	if (!cl_cshift_bonus->int_val
		&& !(atoi (Info_ValueForKey (cl.serverinfo, "cshifts")) & INFO_CSHIFT_BONUS))
		return;
		
	cl.cshifts[CSHIFT_BONUS].destcolor[0] = 215;
	cl.cshifts[CSHIFT_BONUS].destcolor[1] = 186;
	cl.cshifts[CSHIFT_BONUS].destcolor[2] = 69;
	cl.cshifts[CSHIFT_BONUS].percent = 50;
}

/*
	V_SetContentsColor

	Underwater, lava, etc each has a color shift
*/

void
V_SetContentsColor (int contents)
{

	if (!cl_cshift_contents->int_val
		&& !(atoi (Info_ValueForKey (cl.serverinfo, "cshifts")) & INFO_CSHIFT_CONTENTS)) {
		cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
		return;
	}

	switch (contents) {
		case CONTENTS_EMPTY:
			cl.cshifts[CSHIFT_CONTENTS] = cshift_empty;
			break;
		case CONTENTS_LAVA:
			cl.cshifts[CSHIFT_CONTENTS] = cshift_lava;
			break;
		case CONTENTS_SOLID:
		case CONTENTS_SLIME:
			cl.cshifts[CSHIFT_CONTENTS] = cshift_slime;
			break;
		default:
			cl.cshifts[CSHIFT_CONTENTS] = cshift_water;
	}
}


/* 
						VIEW RENDERING 
*/

float
angledelta (float a)
{
	a = anglemod (a);
	if (a > 180)
		a -= 360;
	return a;
}

/*
	CalcGunAngle
*/
void
CalcGunAngle (void)
{
	float       yaw, pitch, move;
	static float oldyaw = 0;
	static float oldpitch = 0;

	yaw = r_refdef.viewangles[YAW];
	pitch = -r_refdef.viewangles[PITCH];

	yaw = angledelta (yaw - r_refdef.viewangles[YAW]) * 0.4;
	yaw = bound (-10, yaw, 10);
	pitch = angledelta (-pitch - r_refdef.viewangles[PITCH]) * 0.4;
	pitch = bound (-10, pitch, 10);

	move = host_frametime * 20;
	if (yaw > oldyaw) {
		if (oldyaw + move < yaw)
			yaw = oldyaw + move;
	} else {
		if (oldyaw - move > yaw)
			yaw = oldyaw - move;
	}

	if (pitch > oldpitch) {
		if (oldpitch + move < pitch)
			pitch = oldpitch + move;
	} else {
		if (oldpitch - move > pitch)
			pitch = oldpitch - move;
	}

	oldyaw = yaw;
	oldpitch = pitch;

	cl.viewent.angles[YAW] = r_refdef.viewangles[YAW] + yaw;
	cl.viewent.angles[PITCH] = -(r_refdef.viewangles[PITCH] + pitch);
}

/*
	V_BoundOffsets
*/
void
V_BoundOffsets (void)
{
// absolutely bound refresh reletive to entity clipping hull
// so the view can never be inside a solid wall

	if (r_refdef.vieworg[0] < cl.simorg[0] - 14)
		r_refdef.vieworg[0] = cl.simorg[0] - 14;
	else if (r_refdef.vieworg[0] > cl.simorg[0] + 14)
		r_refdef.vieworg[0] = cl.simorg[0] + 14;
	if (r_refdef.vieworg[1] < cl.simorg[1] - 14)
		r_refdef.vieworg[1] = cl.simorg[1] - 14;
	else if (r_refdef.vieworg[1] > cl.simorg[1] + 14)
		r_refdef.vieworg[1] = cl.simorg[1] + 14;
	if (r_refdef.vieworg[2] < cl.simorg[2] - 22)
		r_refdef.vieworg[2] = cl.simorg[2] - 22;
	else if (r_refdef.vieworg[2] > cl.simorg[2] + 30)
		r_refdef.vieworg[2] = cl.simorg[2] + 30;
}

/*
	V_AddIdle

	Idle swaying
*/
void
V_AddIdle (void)
{
	r_refdef.viewangles[ROLL] +=
		v_idlescale->value * sin (cl.time * v_iroll_cycle->value) *
		v_iroll_level->value;
	r_refdef.viewangles[PITCH] +=
		v_idlescale->value * sin (cl.time * v_ipitch_cycle->value) *
		v_ipitch_level->value;
	r_refdef.viewangles[YAW] +=
		v_idlescale->value * sin (cl.time * v_iyaw_cycle->value) *
		v_iyaw_level->value;

	cl.viewent.angles[ROLL] -=
		v_idlescale->value * sin (cl.time * v_iroll_cycle->value) *
		v_iroll_level->value;
	cl.viewent.angles[PITCH] -=
		v_idlescale->value * sin (cl.time * v_ipitch_cycle->value) *
		v_ipitch_level->value;
	cl.viewent.angles[YAW] -=
		v_idlescale->value * sin (cl.time * v_iyaw_cycle->value) *
		v_iyaw_level->value;
}


/*
	V_CalcViewRoll

	Roll is induced by movement and damage
*/
void
V_CalcViewRoll (void)
{
	float       side;

	side = V_CalcRoll (cl.simangles, cl.simvel);
	r_refdef.viewangles[ROLL] += side;

	if (v_dmg_time > 0) {
		r_refdef.viewangles[ROLL] +=
			v_dmg_time / v_kicktime->value * v_dmg_roll;
		r_refdef.viewangles[PITCH] +=
			v_dmg_time / v_kicktime->value * v_dmg_pitch;
		v_dmg_time -= host_frametime;
	}

}


/*
	V_CalcIntermissionRefdef
*/
void
V_CalcIntermissionRefdef (void)
{
	entity_t   *view;
	float       old;

// view is the weapon model
	view = &cl.viewent;

	VectorCopy (cl.simorg, r_refdef.vieworg);
	VectorCopy (cl.simangles, r_refdef.viewangles);
	view->model = NULL;

// allways idle in intermission
	old = v_idlescale->value;
	Cvar_SetValue (v_idlescale, 1);
	V_AddIdle ();
	Cvar_SetValue (v_idlescale, old);
}

/*
	V_CalcRefdef
*/
void
V_CalcRefdef (void)
{
	entity_t   *view;
	int         i;
	vec3_t      forward, right, up;
	float       bob;
	static float oldz = 0;
	int         zofs = 22;

	if (cl.stdver)
		zofs = cl.stats[STAT_VIEWHEIGHT];

	V_DriftPitch ();

// view is the weapon model (only visible from inside body)
	view = &cl.viewent;

	bob = V_CalcBob ();

// refresh position from simulated origin
	VectorCopy (cl.simorg, r_refdef.vieworg);

	r_refdef.vieworg[2] += bob;

// never let it sit exactly on a node line, because a water plane can
// dissapear when viewed with the eye exactly on it.
// the server protocol only specifies to 1/8 pixel, so add 1/16 in each axis
	r_refdef.vieworg[0] += 1.0 / 16;
	r_refdef.vieworg[1] += 1.0 / 16;
	r_refdef.vieworg[2] += 1.0 / 16;

	VectorCopy (cl.simangles, r_refdef.viewangles);
	V_CalcViewRoll ();
	V_AddIdle ();

	if (view_message->flags & PF_GIB)
		r_refdef.vieworg[2] += 8;		// gib view height
	else if (view_message->flags & PF_DEAD)
		r_refdef.vieworg[2] -= 16;		// corpse view height
	else
		r_refdef.vieworg[2] += zofs;	// view height

	if (view_message->flags & PF_DEAD)	// PF_GIB will also set PF_DEAD
		r_refdef.viewangles[ROLL] = 80;	// dead view angle


// offsets
	AngleVectors (cl.simangles, forward, right, up);

// set up gun position
	VectorCopy (cl.simangles, view->angles);

	CalcGunAngle ();

	VectorCopy (cl.simorg, view->origin);
	view->origin[2] += zofs;

	for (i = 0; i < 3; i++) {
		view->origin[i] += forward[i] * bob * 0.4;
//      view->origin[i] += right[i]*bob*0.4;
//      view->origin[i] += up[i]*bob*0.8;
	}
	view->origin[2] += bob;

// fudge position around to keep amount of weapon visible
// roughly equal with different FOV
	if (scr_viewsize->int_val == 110)
		view->origin[2] += 1;
	else if (scr_viewsize->int_val == 100)
		view->origin[2] += 2;
	else if (scr_viewsize->int_val == 90)
		view->origin[2] += 1;
	else if (scr_viewsize->int_val == 80)
		view->origin[2] += 0.5;

	if (view_message->flags & (PF_GIB | PF_DEAD))
		view->model = NULL;
	else
		view->model = cl.model_precache[cl.stats[STAT_WEAPON]];
	view->frame = view_message->weaponframe;
	view->colormap = vid.colormap;
	// LordHavoc: make gun visible
	view->alpha = 1;
	view->colormod[0] = view->colormod[1] = view->colormod[2] = 1;

// set up the refresh position
	r_refdef.viewangles[PITCH] += cl.punchangle;

// smooth out stair step ups
	if ((cl.onground != -1) && (cl.simorg[2] - oldz > 0)) {
		float       steptime;

		steptime = host_frametime;

		oldz += steptime * 80;
		if (oldz > cl.simorg[2])
			oldz = cl.simorg[2];
		if (cl.simorg[2] - oldz > 12)
			oldz = cl.simorg[2] - 12;
		r_refdef.vieworg[2] += oldz - cl.simorg[2];
		view->origin[2] += oldz - cl.simorg[2];
	} else
		oldz = cl.simorg[2];
}

/*
	DropPunchAngle
*/
void
DropPunchAngle (void)
{
	cl.punchangle -= 10 * host_frametime;
	if (cl.punchangle < 0)
		cl.punchangle = 0;
}

/*
	V_RenderView

	The player's clipping box goes from (-16 -16 -24) to (16 16 32) from
	the entity origin, so any view position inside that will be valid
*/
extern vrect_t scr_vrect;

void
V_RenderView (void)
{
//  if (cl.simangles[ROLL])
//      Sys_Error ("cl.simangles[ROLL]");   // DEBUG
	cl.simangles[ROLL] = 0;				// FIXME @@@ 

	if (cls.state != ca_active)
		return;

	view_frame = &cl.frames[cls.netchan.incoming_sequence & UPDATE_MASK];
	view_message = &view_frame->playerstate[cl.playernum];

	DropPunchAngle ();
	if (cl.intermission) {				// intermission / finale rendering
		V_CalcIntermissionRefdef ();
	} else {
		V_CalcRefdef ();
	}

	R_PushDlights (vec3_origin);

	R_RenderView ();
}

//============================================================================

/*
	V_Init
*/
void
V_Init (void)
{
	Cmd_AddCommand ("v_cshift", V_cshift_f, "This adjusts all of the colors currently being displayed.\n"
		"Used when you are underwater, hit, have the Ring of Shadows, or Quad Damage. (v_cshift r g b intensity)");
		
	Cmd_AddCommand ("bf", V_BonusFlash_f, "Background flash, used when you pick up an item");
	Cmd_AddCommand ("centerview", V_StartPitchDrift, "Centers the player's view ahead after +lookup or +lookdown \n"
		"Will not work while mlook is active or freelook is 1.");
}

void
V_Init_Cvars (void)
{
	v_centermove = Cvar_Get ("v_centermove", "0.15", CVAR_NONE, NULL,
			"How far the player must move forward before the view re-centers");
	v_centerspeed = Cvar_Get ("v_centerspeed", "500", CVAR_NONE, NULL,
			"How quickly you return to a center view after a lookup or lookdown");

	v_iyaw_cycle = Cvar_Get ("v_iyaw_cycle", "2", CVAR_NONE, NULL,
			"How far you tilt right and left when v_idlescale is enabled");
	v_iroll_cycle = Cvar_Get ("v_iroll_cycle", "0.5", CVAR_NONE, NULL,
			"How quickly you tilt right and left when v_idlescale is enabled");
	v_ipitch_cycle = Cvar_Get ("v_ipitch_cycle", "1", CVAR_NONE, NULL,
			"How quickly you lean forwards and backwards when v_idlescale is enabled");
	v_iyaw_level = Cvar_Get ("v_iyaw_level", "0.3", CVAR_NONE, NULL,
			"How far you tilt right and left when v_idlescale is enabled");
	v_iroll_level = Cvar_Get ("v_iroll_level", "0.1", CVAR_NONE, NULL,
			"How far you tilt right and left when v_idlescale is enabled");
	v_ipitch_level = Cvar_Get ("v_ipitch_level", "0.3", CVAR_NONE, NULL,
			"How far you lean forwards and backwards when v_idlescale is enabled");

	v_idlescale = Cvar_Get ("v_idlescale", "0", CVAR_NONE, NULL,
			"Toggles whether the view remains idle");

	cl_rollspeed = Cvar_Get ("cl_rollspeed", "200", CVAR_NONE, NULL,
			"How quickly you straighten out after strafing");
	cl_rollangle = Cvar_Get ("cl_rollangle", "2.0", CVAR_NONE, NULL,
			"How much your screen tilts when strafing");

	cl_bob = Cvar_Get ("cl_bob", "0.02", CVAR_NONE, NULL,
			"How much your weapon moves up and down when walking");
	cl_bobcycle = Cvar_Get ("cl_bobcycle", "0.6", CVAR_NONE, NULL,
			"How quickly your weapon moves up and down when walking");
	cl_bobup = Cvar_Get ("cl_bobup", "0.5", CVAR_NONE, NULL,
			"How long your weapon stays up before cycling when walking");

	v_kicktime = Cvar_Get ("v_kicktime", "0.5", CVAR_NONE, NULL,
			"How long the kick from an attack lasts");
	v_kickroll = Cvar_Get ("v_kickroll", "0.6", CVAR_NONE, NULL,
			"How much you lean when hit");
	v_kickpitch = Cvar_Get ("v_kickpitch", "0.6", CVAR_NONE, NULL,
			"How much you look up when hit");
}
