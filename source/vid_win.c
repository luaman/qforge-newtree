/*
	vid_win.c

	windows video driver functions

	Copyright (C) 1996-1997 Id Software, Inc.

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

#include <windows.h>

#include "vid.h"

extern HWND		mainwindow;

void
VID_RaiseWindow (void)
{
	ShowWindow (mainwindow, SW_RESTORE);
	SetForegroundWindow (mainwindow);  
}

void
VID_MinimiseWindow (void)
{
	SendMessage (mainwindow, WM_SYSKEYUP, VK_TAB, 1 | (0x0F << 16) | (1 << 29));
}
