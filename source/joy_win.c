/*
        joy_win.c

        Joystick device driver for Win32

        Copyright (C) 2000 Jeff Teunissen <deek@dusknet.dhs.org>
        Copyright (C) 2000 Jukka Sorjonen <jukka.sorjone@asikkala.fi>
        
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

// fixme: THIS IS NOT FINISHED YET

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef __MINGW32__
# define INITGUID
#endif

#define byte __byte
#include <dinput.h>
#undef byte

#include "cl_input.h"
#include "client.h"
#include "cmd.h"
#include "console.h"
#include "cvar.h"
#include "host.h"
#include "input.h"
#include "keys.h"
#include "protocol.h"
#include "qargs.h"
#include "view.h"

// Joystick variables and structures
cvar_t     *joy_device;					// Joystick device name
cvar_t     *joy_enable;					// Joystick enabling flag
cvar_t     *joy_sensitivity;			// Joystick sensitivity

qboolean    joy_found = false;
qboolean    joy_active = false;


// joystick defines and variables
// where should defines be moved?

#define JOY_ABSOLUTE_AXIS	0x00000000	// control like a joystick
#define JOY_RELATIVE_AXIS	0x00000010	// control like a mouse, spinner,
										// trackball
#define JOY_MAX_AXES		6			// X, Y, Z, R, U, V
#define JOY_AXIS_X			0
#define JOY_AXIS_Y			1
#define JOY_AXIS_Z			2
#define JOY_AXIS_R			3
#define JOY_AXIS_U			4
#define JOY_AXIS_V			5

enum _ControlList {
	AxisNada = 0, AxisForward, AxisLook, AxisSide, AxisTurn
};

DWORD dwAxisFlags[JOY_MAX_AXES] = {
	JOY_RETURNX, JOY_RETURNY, JOY_RETURNZ, JOY_RETURNR, JOY_RETURNU, JOY_RETURNV
};

DWORD dwAxisMap[JOY_MAX_AXES];
DWORD dwControlMap[JOY_MAX_AXES];
PDWORD pdwRawValue[JOY_MAX_AXES];

JOYINFOEX ji;

// none of these cvars are saved over a session
// this means that advanced controller configuration needs to be executed
// each time.  this avoids any problems with getting back to a default usage
// or when changing from one controller to another.  this way at least something
// works.

cvar_t *in_joystick;
cvar_t *joy_name;
cvar_t *joy_advanced;
cvar_t *joy_advaxisx;
cvar_t *joy_advaxisy;
cvar_t *joy_advaxisz;
cvar_t *joy_advaxisr;
cvar_t *joy_advaxisu;
cvar_t *joy_advaxisv;
cvar_t *joy_forwardthreshold;
cvar_t *joy_sidethreshold;
cvar_t *joy_pitchthreshold;
cvar_t *joy_yawthreshold;
cvar_t *joy_forwardsensitivity;
cvar_t *joy_sidesensitivity;
cvar_t *joy_pitchsensitivity;
cvar_t *joy_yawsensitivity;
cvar_t *joy_wwhack1;
cvar_t *joy_wwhack2;

cvar_t *joy_debug;

qboolean joy_advancedinit, joy_haspov;
DWORD joy_oldbuttonstate, joy_oldpovstate;
int  joy_id;
DWORD joy_flags;
DWORD joy_numbuttons;

//
//
//
void JOY_AdvancedUpdate_f (void);
void JOY_StartupJoystick (void);
void JOY_Move (usercmd_t *cmd);
void JOY_Init_Cvars(void);

PDWORD RawValuePointer (int axis);

qboolean
JOY_Read (void)
{
	memset (&ji, 0, sizeof (ji));
	ji.dwSize = sizeof (ji);
	ji.dwFlags = joy_flags;

	if (joyGetPosEx (joy_id, &ji) == JOYERR_NOERROR) {
		// HACK HACK HACK -- there's a bug in the Logitech Wingman Warrior's
		// DInput driver that causes it to make 32668 the center point
		// instead
		// of 32768
		if (joy_wwhack1->int_val) {
			ji.dwUpos += 100;
		}
		if (joy_debug->int_val) {
                        if (ji.dwXpos) Con_Printf("X: %ld\n",ji.dwXpos);
                        if (ji.dwYpos) Con_Printf("Y: %ld\n",ji.dwYpos);
                        if (ji.dwZpos) Con_Printf("Z: %ld\n",ji.dwZpos);
                        if (ji.dwRpos) Con_Printf("R: %ld\n",ji.dwRpos);
                        if (ji.dwUpos) Con_Printf("U: %ld\n",ji.dwUpos);
                        if (ji.dwVpos) Con_Printf("V: %ld\n",ji.dwVpos);
                        if (ji.dwButtons) Con_Printf("B: %ld\n",ji.dwButtons);
		}
		return true;
	} else {							// read error
		return false;
	}
}

void
JOY_Command (void)
{
	int         i, key_index;
	DWORD       buttonstate, povstate;

        if (!joy_found) {
		return;
	}
	// loop through the joystick buttons
	// key a joystick event or auxillary event for higher number buttons for
	// each state change
	buttonstate = ji.dwButtons;
	for (i = 0; i < joy_numbuttons; i++) {
		if ((buttonstate & (1 << i)) && !(joy_oldbuttonstate & (1 << i))) {
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, -1, true);
		}

		if (!(buttonstate & (1 << i)) && (joy_oldbuttonstate & (1 << i))) {
			key_index = (i < 4) ? K_JOY1 : K_AUX1;
			Key_Event (key_index + i, -1, false);
		}
	}
	joy_oldbuttonstate = buttonstate;

	if (joy_haspov) {
		// convert POV information into 4 bits of state information
		// this avoids any potential problems related to moving from one
		// direction to another without going through the center position
		povstate = 0;
		if (ji.dwPOV != JOY_POVCENTERED) {
			if (ji.dwPOV == JOY_POVFORWARD)
				povstate |= 0x01;
			if (ji.dwPOV == JOY_POVRIGHT)
				povstate |= 0x02;
			if (ji.dwPOV == JOY_POVBACKWARD)
				povstate |= 0x04;
			if (ji.dwPOV == JOY_POVLEFT)
				povstate |= 0x08;
		}
		// determine which bits have changed and key an auxillary event for
		// each change
		for (i = 0; i < 4; i++) {
			if ((povstate & (1 << i)) && !(joy_oldpovstate & (1 << i))) {
				Key_Event (K_AUX29 + i, -1, true);
			}

			if (!(povstate & (1 << i)) && (joy_oldpovstate & (1 << i))) {
				Key_Event (K_AUX29 + i, -1, false);
			}
		}
		joy_oldpovstate = povstate;
	}
}

void
JOY_Move (usercmd_t *cmd)
{
	float       speed, aspeed;
	float       fAxisValue, fTemp;
	int         i;
        static int lastjoy=0;

	// complete initialization if first time in
	// this is needed as cvars are not available at initialization time
        if (!joy_advancedinit || lastjoy!=joy_advanced->int_val) {
		JOY_AdvancedUpdate_f ();
		joy_advancedinit = true;
                lastjoy=joy_advanced->int_val;
	}
	// verify joystick is available and that the user wants to use it
	if (!joy_active || !joy_enable->int_val) {
		return;
	}
	// collect the joystick data, if possible
	if (!JOY_Read ()) {
		return;
	}

	if (in_speed.state & 1)
		speed = cl_movespeedkey->value;
	else
		speed = 1;
	aspeed = speed * host_frametime;

	// loop through the axes
	for (i = 0; i < JOY_MAX_AXES; i++) {
		// get the floating point zero-centered, potentially-inverted data
		// for the current axis
		fAxisValue = (float) *pdwRawValue[i];
		// move centerpoint to zero
		fAxisValue -= 32768.0;

		if (joy_wwhack2->int_val) {
			if (dwAxisMap[i] == AxisTurn) {
				// this is a special formula for the Logitech WingMan Warrior
				// y=ax^b; where a = 300 and b = 1.3
				// also x values are in increments of 800 (so this is
				// factored out)
				// then bounds check result to level out excessively high
				// spin rates
				fTemp = 300.0 * pow (abs (fAxisValue) / 800.0, 1.3);
				if (fTemp > 14000.0)
					fTemp = 14000.0;
				// restore direction information
				fAxisValue = (fAxisValue > 0.0) ? fTemp : -fTemp;
			}
		}
		// convert range from -32768..32767 to -1..1 
		fAxisValue /= 32768.0;

		switch (dwAxisMap[i]) {
			case AxisForward:
				if (!joy_advanced->int_val && freelook) {
					// user wants forward control to become look control
					if (fabs (fAxisValue) > joy_pitchthreshold->value) {
						// if mouse invert is on, invert the joystick pitch
						// value
						// only absolute control support here (joy_advanced
						// is false)
						if (m_pitch->value < 0.0) {
							cl.viewangles[PITCH] -=
								(fAxisValue * joy_pitchsensitivity->value) *
								aspeed * cl_pitchspeed->value;
						} else {
							cl.viewangles[PITCH] +=
								(fAxisValue * joy_pitchsensitivity->value) *
								aspeed * cl_pitchspeed->value;
						}
						V_StopPitchDrift ();
					} else {
						// no pitch movement
						// disable pitch return-to-center unless requested by 
						// user
						// *** this code can be removed when the lookspring
						// bug is fixed
						// *** the bug always has the lookspring feature on
						if (!lookspring->int_val)
							V_StopPitchDrift ();
					}
				} else {
					// user wants forward control to be forward control
					if (fabs (fAxisValue) > joy_forwardthreshold->value) {
						cmd->forwardmove +=
							(fAxisValue * joy_forwardsensitivity->value) *
							speed * cl_forwardspeed->value;
					}
				}
				break;

			case AxisSide:
				if (fabs (fAxisValue) > joy_sidethreshold->value) {
					cmd->sidemove +=
						(fAxisValue * joy_sidesensitivity->value) * speed *
						cl_sidespeed->value;
				}
				break;

			case AxisTurn:
				if ((in_strafe.state & 1) || (lookstrafe->int_val && freelook)) {
					// user wants turn control to become side control
					if (fabs (fAxisValue) > joy_sidethreshold->value) {
						cmd->sidemove -=
							(fAxisValue * joy_sidesensitivity->value) * speed *
							cl_sidespeed->value;
					}
				} else {
					// user wants turn control to be turn control
					if (fabs (fAxisValue) > joy_yawthreshold->value) {
						if (dwControlMap[i] == JOY_ABSOLUTE_AXIS) {
							cl.viewangles[YAW] +=
								(fAxisValue * joy_yawsensitivity->value) *
								aspeed * cl_yawspeed->value;
						} else {
							cl.viewangles[YAW] +=
								(fAxisValue * joy_yawsensitivity->value) *
								speed * 180.0;
						}
					}
				}
				break;

			case AxisLook:
				if (freelook) {
					if (fabs (fAxisValue) > joy_pitchthreshold->value) {
						// pitch movement detected and pitch movement desired 
						// by user
						if (dwControlMap[i] == JOY_ABSOLUTE_AXIS) {
							cl.viewangles[PITCH] +=
								(fAxisValue * joy_pitchsensitivity->value) *
								aspeed * cl_pitchspeed->value;
						} else {
							cl.viewangles[PITCH] +=
								(fAxisValue * joy_pitchsensitivity->value) *
								speed * 180.0;
						}
						V_StopPitchDrift ();
					} else {
						// no pitch movement
						// disable pitch return-to-center unless requested by 
						// user
						// *** this code can be removed when the lookspring
						// bug is fixed
						// *** the bug always has the lookspring feature on
						if (!lookspring->int_val)
							V_StopPitchDrift ();
					}
				}
				break;

			default:
				break;
		}
	}

	// bounds check pitch
	cl.viewangles[PITCH] = bound (-70, cl.viewangles[PITCH], 80);
}

void
JOY_Init (void)
{
        JOY_StartupJoystick();
	Cmd_AddCommand ("joyadvancedupdate", JOY_AdvancedUpdate_f, "FIXME: This appears to update the joystick poll? No Description");

//        Con_DPrintf ("This system does not have joystick support.\n");
}

void
JOY_Shutdown (void)
{
	joy_active = false;
	joy_found = false;
}

/*
===========
Joy_AdvancedUpdate_f
===========
*/
void
JOY_AdvancedUpdate_f (void)
{
	// called once by JOY_ReadJoystick and by user whenever an update is
	// needed
	// cvars are now available
	int         i;
	DWORD       dwTemp;

	// initialize all the maps
	for (i = 0; i < JOY_MAX_AXES; i++) {
		dwAxisMap[i] = AxisNada;
		dwControlMap[i] = JOY_ABSOLUTE_AXIS;
		pdwRawValue[i] = RawValuePointer (i);
	}

	if (joy_advanced->int_val) {
		// default joystick initialization
		// 2 axes only with joystick control
		dwAxisMap[JOY_AXIS_X] = AxisTurn;
		// dwControlMap[JOY_AXIS_X] = JOY_ABSOLUTE_AXIS;
		dwAxisMap[JOY_AXIS_Y] = AxisForward;
		// dwControlMap[JOY_AXIS_Y] = JOY_ABSOLUTE_AXIS;
	} else {
		if (strcmp (joy_name->string, "joystick") != 0) {
			// notify user of advanced controller
			Con_Printf ("\n%s configured\n\n", joy_name->string);
		}
		// advanced initialization here
		// data supplied by user via joy_axisn cvars
		dwTemp = joy_advaxisx->int_val;
		dwAxisMap[JOY_AXIS_X] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_X] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = joy_advaxisy->int_val;
		dwAxisMap[JOY_AXIS_Y] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Y] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = joy_advaxisz->int_val;
		dwAxisMap[JOY_AXIS_Z] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_Z] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = joy_advaxisr->int_val;
		dwAxisMap[JOY_AXIS_R] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_R] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = joy_advaxisu->int_val;
		dwAxisMap[JOY_AXIS_U] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_U] = dwTemp & JOY_RELATIVE_AXIS;
		dwTemp = joy_advaxisv->int_val;
		dwAxisMap[JOY_AXIS_V] = dwTemp & 0x0000000f;
		dwControlMap[JOY_AXIS_V] = dwTemp & JOY_RELATIVE_AXIS;
	}

	// compute the axes to collect from DirectInput
	joy_flags = JOY_RETURNCENTERED | JOY_RETURNBUTTONS | JOY_RETURNPOV;
	for (i = 0; i < JOY_MAX_AXES; i++) {
		if (dwAxisMap[i] != AxisNada) {
			joy_flags |= dwAxisFlags[i];
		}
	}
}


/* 
=============== 
IN_StartupJoystick 
=============== 
*/
void
JOY_StartupJoystick (void)
{
	int /* i, */ numdevs;
	JOYCAPS     jc;
	MMRESULT    mmr = !JOYERR_NOERROR;

	// assume no joystick
        joy_found = false;

	// abort startup if user requests no joystick
	if (COM_CheckParm ("-nojoy"))
		return;

	// verify joystick driver is present
	if ((numdevs = joyGetNumDevs ()) == 0) {
		Con_Printf ("\njoystick not found -- driver not present\n\n");
		return;
	}
	// cycle through the joystick ids for the first valid one
	for (joy_id = 0; joy_id < numdevs; joy_id++) {
		memset (&ji, 0, sizeof (ji));
		ji.dwSize = sizeof (ji);
		ji.dwFlags = JOY_RETURNCENTERED;

		if ((mmr = joyGetPosEx (joy_id, &ji)) == JOYERR_NOERROR)
			break;
	}

	// abort startup if we didn't find a valid joystick
	if (mmr != JOYERR_NOERROR) {
		Con_Printf ("\njoystick not found -- no valid joysticks (%x)\n\n", mmr);
		return;
	}
	// get the capabilities of the selected joystick
	// abort startup if command fails
	memset (&jc, 0, sizeof (jc));
	if ((mmr = joyGetDevCaps (joy_id, &jc, sizeof (jc))) != JOYERR_NOERROR) {
		Con_Printf
			("\njoystick not found -- invalid joystick capabilities (%x)\n\n",
			 mmr);
		return;
	}
	// save the joystick's number of buttons and POV status
	joy_numbuttons = jc.wNumButtons;
	joy_haspov = jc.wCaps & JOYCAPS_HASPOV;

	// old button and POV states default to no buttons pressed
	joy_oldbuttonstate = joy_oldpovstate = 0;

	// mark the joystick as available and advanced initialization not
	// completed
	// this is needed as cvars are not available during initialization

	joy_advancedinit = false;
        joy_found = true;
        // fixme: do this right
        joy_active = true;
	Con_Printf ("\njoystick detected\n\n");
}

/*
===========
RawValuePointer
===========
*/
PDWORD
RawValuePointer (int axis)
{
	switch (axis) {
		case JOY_AXIS_X:
			return &ji.dwXpos;
		case JOY_AXIS_Y:
			return &ji.dwYpos;
		case JOY_AXIS_Z:
			return &ji.dwZpos;
		case JOY_AXIS_R:
			return &ji.dwRpos;
		case JOY_AXIS_U:
			return &ji.dwUpos;
		case JOY_AXIS_V:
			return &ji.dwVpos;
	}
	return NULL;
}


void
JOY_Init_Cvars(void)
{
	joy_device =
		Cvar_Get ("joy_device", "none", CVAR_NONE | CVAR_ROM,
				  "Joystick device");
	joy_enable =
		Cvar_Get ("joy_enable", "1", CVAR_NONE | CVAR_ARCHIVE,
				  "Joystick enable flag");
	joy_sensitivity =
		Cvar_Get ("joy_sensitivity", "1", CVAR_NONE | CVAR_ARCHIVE,
				  "Joystick sensitivity");

	// joystick variables

	in_joystick = 
		Cvar_Get ("joystick", "0", CVAR_ARCHIVE, "FIXME: No Description");
	joy_name = 
		Cvar_Get ("joyname", "joystick", CVAR_NONE, "FIXME: No Description");
	joy_advanced = 
		Cvar_Get ("joyadvanced", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisx = 
		Cvar_Get ("joyadvaxisx", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisy = 
		Cvar_Get ("joyadvaxisy", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisz = 
		Cvar_Get ("joyadvaxisz", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisr = 
		Cvar_Get ("joyadvaxisr", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisu = 
		Cvar_Get ("joyadvaxisu", "0", CVAR_NONE, "FIXME: No Description");
	joy_advaxisv = 
		Cvar_Get ("joyadvaxisv", "0", CVAR_NONE, "FIXME: No Description");
	joy_forwardthreshold =
		Cvar_Get ("joyforwardthreshold", "0.15", CVAR_NONE, "FIXME: No Description");
	joy_sidethreshold =
		Cvar_Get ("joysidethreshold", "0.15", CVAR_NONE, "FIXME: No Description");
	joy_pitchthreshold =
		Cvar_Get ("joypitchthreshold", "0.15", CVAR_NONE, "FIXME: No Description");
	joy_yawthreshold = Cvar_Get ("joyyawthreshold", "0.15", CVAR_NONE, "FIXME: No Description");
	joy_forwardsensitivity =
		Cvar_Get ("joyforwardsensitivity", "-1.0", CVAR_NONE, "FIXME: No Description");
	joy_sidesensitivity =
		Cvar_Get ("joysidesensitivity", "-1.0", CVAR_NONE, "FIXME: No Description");
	joy_pitchsensitivity =
		Cvar_Get ("joypitchsensitivity", "1.0", CVAR_NONE, "FIXME: No Description");
	joy_yawsensitivity =
		Cvar_Get ("joyyawsensitivity", "-1.0", CVAR_NONE, "FIXME: No Description");
	joy_wwhack1 = Cvar_Get ("joywwhack1", "0.0", CVAR_NONE, "FIXME: No Description");
	joy_wwhack2 = Cvar_Get ("joywwhack2", "0.0", CVAR_NONE, "FIXME: No Description");

        joy_debug = Cvar_Get ("joy_debug", "0.0", CVAR_NONE, "FIXME: No Description");

	return;
}

