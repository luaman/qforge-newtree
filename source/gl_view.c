/*
	gl_view.c

	player eye positioning

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
#include <stdio.h>

#include "bothdefs.h"					// needed by: common.h, net.h,
										// client.h

#include "bspfile.h"					// needed by: glquake.h
#include "vid.h"
#include "sys.h"
#include "mathlib.h"					// needed by: protocol.h, render.h,
										// client.h,
						// modelgen.h, glmodel.h
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "net.h"						// needed by: client.h
#include "protocol.h"					// needed by: client.h
#include "cmd.h"
#include "sbar.h"
#include "render.h"						// needed by: client.h, model.h,
										// glquake.h
#include "client.h"						// need cls in this file
#include "model.h"						// needed by: glquake.h
#include "console.h"
#include "glquake.h"

extern byte *host_basepal;
extern double host_frametime;
extern int  onground;
extern byte gammatable[256];

extern cvar_t	*contrast;

byte        ramps[3][256];
float       v_blend[4];

void        V_CalcPowerupCshift (void);
qboolean    V_CheckGamma (void);

/*
	V_CalcBlend

	LordHavoc made this a real, (messy,) true alpha blend.  Cleaned it up
	 a bit, but otherwise this is his code.  --KB
*/
void
V_CalcBlend (void)
{
	float       r, g, b, a, a2, a3;
	int         j;

	r = 0;
	g = 0;
	b = 0;
	a = 0;

	for (j = 0; j < NUM_CSHIFTS; j++) {
		a2 = cl.cshifts[j].percent / 255.0;

		if (!a2)
			continue;

		a2 = min (a2, 1.0);
		r += (cl.cshifts[j].destcolor[0] - r) * a2;
		g += (cl.cshifts[j].destcolor[1] - g) * a2;
		b += (cl.cshifts[j].destcolor[2] - b) * a2;

		a3 = (1.0 - a) * (1.0 - a2);
		a = 1.0 - a3;
	}

	if ((a2 = 1 - bound (0.0, contrast->value, 1.0)) < 0.999) { // add contrast
		r += (128 - r) * a2;
		g += (128 - g) * a2;
		b += (128 - b) * a2;

		a3 = (1.0 - a) * (1.0 - a2);
		a = 1.0 - a3;
	}

	// LordHavoc: saturate color
	if (a) {
		a2 = 1.0 / a;
		r *= a2;
		g *= a2;
		b *= a2;
	}

	v_blend[0] = min (r, 255.0) / 255.0;
	v_blend[1] = min (g, 255.0) / 255.0;
	v_blend[2] = min (b, 255.0) / 255.0;
	v_blend[3] = bound (0.0, a, 1.0);
}

/*
=============
V_UpdatePalette
=============
*/

void
V_UpdatePalette (void)
{
	int         i, j;
	qboolean    new;
	qboolean    force;

	V_CalcPowerupCshift ();

	new = false;

	for (i = 0; i < NUM_CSHIFTS; i++) {
		if (cl.cshifts[i].percent != cl.prev_cshifts[i].percent) {
			new = true;
			cl.prev_cshifts[i].percent = cl.cshifts[i].percent;
		}
		for (j = 0; j < 3; j++) {
			if (cl.cshifts[i].destcolor[j] != cl.prev_cshifts[i].destcolor[j]) {
				new = true;
				cl.prev_cshifts[i].destcolor[j] = cl.cshifts[i].destcolor[j];
			}
		}
	}

	// drop the damage value
	cl.cshifts[CSHIFT_DAMAGE].percent -= host_frametime * 150;
	if (cl.cshifts[CSHIFT_DAMAGE].percent <= 0)
		cl.cshifts[CSHIFT_DAMAGE].percent = 0;

	// drop the bonus value
	cl.cshifts[CSHIFT_BONUS].percent -= host_frametime * 100;
	if (cl.cshifts[CSHIFT_BONUS].percent <= 0)
		cl.cshifts[CSHIFT_BONUS].percent = 0;

	force = V_CheckGamma ();
	if (!new && !force)
		return;

	V_CalcBlend ();
}

/*
	BuildGammaTable

	In software mode, this function gets the palette ready for changing...in
	in GL, it does very little as you can see.
*/

void
BuildGammaTable (float b, float c)
{
	int         i;

	for (i = 0; i < 256; i++)
		gammatable[i] = i;
	return;
}
