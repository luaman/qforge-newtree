/*
	quakedef.h

	primary header for client

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

#ifndef _QUAKEDEF_H
#define _QUAKEDEF_H

#include "gcc_attr.h"

#define	QUAKE_GAME			// as opposed to utilities

//define	PARANOID			// speed sapping error checking

#if defined(_WIN32) && !defined(__GNUC__)
#pragma warning( disable : 4244 4127 4201 4214 4514 4305 4115 4018)
#endif

// FIXME: clean those includes -- yan

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>

#include "qtypes.h"
#include "cvar.h"
#include "commdef.h"

#define MAX_NUM_ARGVS	50

extern qboolean noclip_anglehack;

extern cvar_t	*sys_ticrate;
extern cvar_t	*password;

extern double	host_frametime;		// Tonik

extern byte		*host_basepal;
extern byte		*host_colormap;
extern int		host_framecount;	// incremented every frame, never reset

extern qboolean	msg_suppress_1;		// Suppresses resolution and cache size console 
									// output and fullscreen DIB focus gain/loss

void Host_ServerFrame (void);
void Host_InitCommands (void);
void Host_Init (void);
void Host_Shutdown(void);
void Host_Error (char *error, ...) __attribute__((format(printf,1,2)));
void Host_EndGame (char *message, ...) __attribute__((format(printf,1,2)));
qboolean Host_SimulationTime(float time);
void Host_Frame (float time);
void Host_Quit_f (void);
void Host_ClientCommands (char *fmt, ...) __attribute__((format(printf,1,2)));
void Host_ShutdownServer (qboolean crash);

#endif // _QUAKEDEH_H