/*
	r_part.c

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <string.h>

#include "cmd.h"
#include "console.h"
#include "glquake.h"
#include "cl_main.h"

#include <stdlib.h>

#define MAX_FIRES				128		// rocket flames

static fire_t r_fires[MAX_FIRES];
extern cvar_t *gl_fires;

/*
	R_AddFire

	Nifty ball of fire GL effect.  Kinda a meshing of the dlight and
	particle engine code.
*/
float       r_firecolor[3] = { 0.9, 0.4, 0 };

void
R_AddFire (vec3_t start, vec3_t end, entity_t *ent)
{
	float       len;
	fire_t     *f;
	dlight_t   *dl;
	vec3_t      vec;
	int         key;

	if (!gl_fires->int_val)
		return;

	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	key = ent->keynum;

	if (len) {
		f = R_AllocFire (key);
		VectorCopy (end, f->origin);
		VectorCopy (start, f->owner);
		f->size = 20;
		f->die = cl.time + 0.5;
		f->decay = -1;
		f->color = r_firecolor;

		dl = CL_AllocDlight (-key);
		VectorCopy (end, dl->origin);
		dl->radius = 200;
		dl->die = cl.time + 0.5;
		dl->color = r_firecolor;
	}
}

/*
	R_AllocFire

	Clears out and returns a new fireball
*/
fire_t     *
R_AllocFire (int key)
{
	int         i;
	fire_t     *f;

	if (key)							// first try to find/reuse a keyed
										// spot
	{
		f = r_fires;
		for (i = 0; i < MAX_FIRES; i++, f++)
			if (f->key == key) {
				memset (f, 0, sizeof (*f));
				f->key = key;
				f->color = f->_color;
				return f;
			}
	}

	f = r_fires;						// no match, look for a free spot
	for (i = 0; i < MAX_FIRES; i++, f++) {
		if (f->die < cl.time) {
			memset (f, 0, sizeof (*f));
			f->key = key;
			f->color = f->_color;
			return f;
		}
	}

	f = &r_fires[0];
	memset (f, 0, sizeof (*f));
	f->key = key;
	f->color = f->_color;
	return f;
}

/*
	R_DrawFire

	draws one fireball - probably never need to call this directly
*/
void
R_DrawFire (fire_t *f)
{
	int         i, j;
	vec3_t      vec, vec2;
	float       radius;
	float      *b_sin, *b_cos;

	b_sin = bubble_sintable;
	b_cos = bubble_costable;

	radius = f->size + 0.35;

	// figure out if we're inside the area of effect
	VectorSubtract (f->origin, r_origin, vec);
	if (Length (vec) < radius) {
		AddLightBlend (1, 0.5, 0, f->size * 0.0003);	// we are
		return;
	}
	// we're not - draw it
	glBegin (GL_TRIANGLE_FAN);
	if (lighthalf)
		glColor3f (f->color[0] * 0.5, f->color[1] * 0.5, f->color[2] * 0.5);
	else
		glColor3fv (f->color);
	for (i = 0; i < 3; i++)
		vec[i] = f->origin[i] - vpn[i] * radius;
	glVertex3fv (vec);
	glColor3f (0.0, 0.0, 0.0);

	// don't panic, this just draws a bubble...
	for (i = 16; i >= 0; i--) {
		for (j = 0; j < 3; j++) {
			vec[j] = f->origin[j] + (*b_cos * vright[j]
									 + vup[j] * (*b_sin)) * radius;
			vec2[j] = f->owner[j] + (*b_cos * vright[j]
									 + vup[j] * (*b_sin)) * radius;
		}
		glVertex3fv (vec);
		glVertex3fv (vec2);

		b_sin += 2;
		b_cos += 2;
	}
	glEnd ();
	glColor3ubv (lighthalf_v);
}

/*
	R_UpdateFires

	Draws each fireball in sequence
*/
void
R_UpdateFires (void)
{
	int         i;
	fire_t     *f;

	if (!gl_fires->int_val)
		return;

	glDepthMask (GL_FALSE);
	glDisable (GL_TEXTURE_2D);
	glBlendFunc (GL_ONE, GL_ONE);

	f = r_fires;
	for (i = 0; i < MAX_FIRES; i++, f++) {
		if (f->die < cl.time || !f->size)
			continue;
		f->size += f->decay;
		R_DrawFire (f);
	}

	glEnable (GL_TEXTURE_2D);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask (GL_TRUE);
}

void
R_FireColor_f (void)
{
	int         i;

	if (Cmd_Argc () == 1) {
		Con_Printf ("r_firecolor %g %g %g\n",
					r_firecolor[0], r_firecolor[1], r_firecolor[2]);
		return;
	}
	if (Cmd_Argc () == 5 || Cmd_Argc () == 6) {
		Con_Printf
			("Warning: obsolete 4th and 5th parameters to r_firecolor ignored\n");
	} else if (Cmd_Argc () != 4) {
		Con_Printf ("Usage r_firecolor R G B\n");
		return;
	}
	for (i = 0; i < 4; i++) {
		r_firecolor[i] = atof (Cmd_Argv (i + 1));
	}
}
