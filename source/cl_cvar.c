/*
	cl_cvar.c

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

#include "quakedef.h"

void
Cvar_Info(cvar_t *var)
{
        if (var->flags & CVAR_USERINFO)
        {
                Info_SetValueForKey (cls.userinfo, var->name, var->string, MAX_INFO_STRING);
                if (cls.state >= ca_connected)
                {
                        MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
                        SZ_Print (&cls.netchan.message, va("setinfo \"%s\" \"%s\"\n", var->name, var->string));
                }
        }
}
