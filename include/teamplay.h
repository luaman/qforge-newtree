/*
	teamplay.h

	Teamplay enhancements ("proxy features")

	Copyright (C) 2000       Anton Gavrilov (tonik@quake.ru)

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

#ifndef __teamplay_h
#define __teamplay_h

#include "cvar.h"

extern cvar_t	*cl_parsesay;
extern cvar_t	*cl_nofake;

void Team_Init_Cvars (void);
void Team_BestWeaponImpulse (void);
void Team_Dead (void);
void Team_NewMap (void);
char *Team_ParseSay (char *);
void Locs_Init (void);

#endif // __teamplay_h
