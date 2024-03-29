/*
	in_win.h

	Win32 input prototypes

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

*/

#ifndef _IN_WIN_H
#define _IN_WIN_H

#include "protocol.h"

extern qboolean      mouseactive;
extern float         mouse_x, mouse_y;
extern unsigned int  uiWheelMessage;

extern void IN_UpdateClipCursor (void);
extern void IN_ShowMouse (void);
extern void IN_HideMouse (void);
extern void IN_ActivateMouse (void);
extern void IN_SetQuakeMouseState (void);
extern void IN_DeactivateMouse (void);
extern void IN_RestoreOriginalMouseState (void);
extern void IN_Init (void);
extern void IN_Shutdown (void);
extern void IN_MouseEvent (int mstate);
extern void IN_MouseMove (usercmd_t *cmd);
extern void IN_Move (usercmd_t *cmd);
extern void IN_Accumulate (void);
extern void IN_ClearStates (void);
extern void IN_Commands (void);

#endif // _IN_WIN_H


