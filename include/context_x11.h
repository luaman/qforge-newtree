/*
	context_x11.h

	(description)

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 1999,2000  contributors of the QuakeForge project
	Please see the file "AUTHORS" for a list of contributors

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

#ifndef __context_x11_h_
#define __context_x11_h_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "qtypes.h"

extern Display	*x_disp;
extern Visual	*x_vis;
extern Window	x_root;
extern Window	x_win;
extern XVisualInfo *x_visinfo;
extern double	x_gamma;
extern int		x_screen;
extern int		x_shmeventtype;
extern qboolean doShm;
extern qboolean oktodraw;
extern struct cvar_s *vid_fullscreen;

void GetEvent (void);

double X11_GetGamma (void);
qboolean X11_AddEvent (int event, void (*event_handler)(XEvent *));
qboolean X11_RemoveEvent (int event, void (*event_handler)(XEvent *));
qboolean X11_SetGamma (double);
void X11_CloseDisplay (void);
void X11_CreateNullCursor (void);
void X11_CreateWindow (int, int);
void X11_ForceViewPort (void);
void X11_GrabKeyboard (void);
void X11_Init_Cvars (void);
void X11_OpenDisplay (void);
void X11_ProcessEvent (void);
void X11_ProcessEvents (void);
void X11_RestoreGamma (void);
void X11_RestoreVidMode (void);
void X11_SetCaption (char *);
void X11_SetVidMode (int, int);

#endif	// __context_x11_h_
