/*
	gl_dyn_textures.c

	Dynamic texture generation.

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

#include "fractalnoise.h"
#include "glquake.h"

static void GDT_InitDotParticleTexture (void);
static void GDT_InitSmokeParticleTexture (void);

int         part_tex_smoke[8];
int         part_tex_dot;

void
GDT_Init (void)
{
	GDT_InitDotParticleTexture ();
	GDT_InitSmokeParticleTexture ();
}

static void
GDT_InitDotParticleTexture (void)
{
	int         x, y, dx, dy, d;
	byte        data[16][16][2];

	// 
	// particle texture
	// 
	part_tex_dot = texture_extension_number++;
	glBindTexture (GL_TEXTURE_2D, part_tex_dot);

	for (x = 0; x < 16; x++) {
		for (y = 0; y < 16; y++) {
			data[y][x][0] = 255;
			dx = x - 8;
			dy = y - 8;
			d = 255 - 4 * (dx * dx + dy * dy);
			if (d<0) d = 0;
			if (d>255) d = 255;
			data[y][x][1] = (byte) d;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, 2, 16, 16, 0, GL_LUMINANCE_ALPHA,
				  GL_UNSIGNED_BYTE, data);

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void
GDT_InitSmokeParticleTexture (void)
{
	int         i, x, y, d;
	float       dx, dy;
	byte        data[32][32][2], noise1[32][32], noise2[32][32];

	for (i = 0; i < 8; i++) {
		fractalnoise (&noise1[0][0], 32);
		fractalnoise (&noise2[0][0], 32);
		for (y = 0; y < 32; y++)
			for (x = 0; x < 32; x++) {
				data[y][x][0] = (noise1[y][x] >> 1) + 128;
				dx = x - 16;
				dy = y - 16;
				d = noise2[y][x] * 4 - 512;
				if (d > 0) {
					if (d > 255)
						d = 255;
					d = (d * (255 - (int) (dx * dx + dy * dy))) >> 8;
					if (d < 0)
						d = 0;
					if (d > 255)
						d = 255;
					data[y][x][1] = (byte) d;
				} else
					data[y][x][1] = 0;
			}
		part_tex_smoke[i] = texture_extension_number++;
		glBindTexture (GL_TEXTURE_2D, part_tex_smoke[i]);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D (GL_TEXTURE_2D, 0, 2, 32, 32, 0, GL_LUMINANCE_ALPHA,
					  GL_UNSIGNED_BYTE, data);

	}
}
