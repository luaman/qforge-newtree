/*
	gl_model_fullbright.c

	model loading and caching

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

// models are the only shared resource between a client and server running
// on the same machine.

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "checksum.h"
#include "console.h"
#include "glquake.h"
#include "qendian.h"
#include "r_local.h"
#include "sys.h"

int
Mod_CalcFullbright (byte *in, byte *out, int pixels)
{
	int fb = 0;
	
	while (pixels--) {
		if (*in >= 256 - 32) {
			fb = 1;
			*out++ = *in++;
		} else {
			*out++ = 255;
			in++;
		}
	}
	return fb;
}

int
Mod_Fullbright (byte * skin, int width, int height, char *name)
{
	int         pixels;
	int         texnum = 0;
	byte       *ptexels;

	// Check for fullbright pixels..
	pixels = width * height;

	// ptexels = Hunk_Alloc(s);
	ptexels = malloc (pixels);
	if (Mod_CalcFullbright (skin, ptexels, pixels)) {
		Con_DPrintf ("FB Model ID: '%s'\n", name);
		texnum = GL_LoadTexture (name, width, height, ptexels, true, true, 1);
	}
	free (ptexels);
	return texnum;
}
