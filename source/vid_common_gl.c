/*
	vid_common_gl.c

	Common OpenGL video driver functions

	Copyright (C) 1996-1997 Id Software, Inc.
	Copyright (C) 2000      Marcus Sundberg [mackan@stacken.kth.se]

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

#ifdef _WIN32
// must be BEFORE include gl/gl.h
# include "winquake.h"
#endif

#include <GL/gl.h>

#include "console.h"
#include "glquake.h"
#include "input.h"
#include "qargs.h"
#include "qfgl_ext.h"
#include "quakefs.h"
#include "sbar.h"

#define WARP_WIDTH              320
#define WARP_HEIGHT             200

//unsigned short    d_8to16table[256];
unsigned int d_8to24table[256];
unsigned char d_15to8table[65536];

cvar_t     *vid_mode;

/*-----------------------------------------------------------------------*/

int         texture_mode = GL_LINEAR;
int         texture_extension_number = 1;
float       gldepthmin, gldepthmax;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

// ARB Multitexture
int         gl_mtex_enum = TEXTURE0_SGIS;
qboolean    gl_arb_mtex = false;
qboolean    gl_mtexable = false;

static qboolean is8bit = false;
cvar_t     *vid_use8bit;

/*-----------------------------------------------------------------------*/

/*
	CheckMultiTextureExtensions

	Check for ARB, SGIS, or EXT multitexture support
*/

void
CheckMultiTextureExtensions (void)
{
	Con_Printf ("Checking for multitexture: ");
	if (COM_CheckParm ("-nomtex")) {
		Con_Printf ("disabled\n");
		return;
	}

	if (QFGL_ExtensionPresent ("GL_ARB_multitexture")) {
		Con_Printf ("GL_ARB_multitexture\n");
		qglMTexCoord2f = QFGL_ExtensionAddress ("glMultiTexCoord2fARB");
		qglSelectTexture = QFGL_ExtensionAddress ("glActiveTextureARB");
		gl_mtex_enum = GL_TEXTURE0_ARB;
		gl_mtexable = true;
		gl_arb_mtex = true;
	} else if (QFGL_ExtensionPresent ("GL_SGIS_multitexture")) {
		Con_Printf ("GL_SGIS_multitexture\n");
		qglMTexCoord2f = QFGL_ExtensionAddress ("glMTexCoord2fSGIS");
		qglSelectTexture = QFGL_ExtensionAddress ("glSelectTextureSGIS");
		gl_mtex_enum = TEXTURE0_SGIS;
		gl_mtexable = true;
		gl_arb_mtex = false;
	} else if (QFGL_ExtensionPresent ("GL_EXT_multitexture")) {
		Con_Printf ("GL_EXT_multitexture\n");
		qglMTexCoord2f = QFGL_ExtensionAddress ("glMTexCoord2fEXT");
		qglSelectTexture = QFGL_ExtensionAddress ("glSelectTextureEXT");
		gl_mtex_enum = TEXTURE0_SGIS;
		gl_mtexable = true;
		gl_arb_mtex = false;
	} else {
		Con_Printf ("none\n");
	}
}

void
VID_SetPalette (unsigned char *palette)
{
	byte       *pal;
	unsigned int r, g, b;
	unsigned int v;
	int         r1, g1, b1;
	int         k;
	unsigned short i;
	unsigned int *table;
	QFile      *f;
	char        s[255];
	float       dist, bestdist;
	static qboolean palflag = false;

//
// 8 8 8 encoding
//
//  Con_Printf("Converting 8to24\n");

	pal = palette;
	table = d_8to24table;
	for (i = 0; i < 255; i++) {			// used to be i<256, see d_8to24table 
										// below
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

#ifdef WORDS_BIGENDIAN
		v = (255 << 0) + (r << 24) + (g << 16) + (b << 8);
#else
		v = (255 << 24) + (r << 0) + (g << 8) + (b << 16);
#endif
		*table++ = v;
	}
	d_8to24table[255] = 0;				// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.
	if (palflag)
		return;
	palflag = true;

	COM_FOpenFile ("glquake/15to8.pal", &f);
	if (f) {
		Qread (f, d_15to8table, 1 << 15);
		Qclose (f);
	} else {
		for (i = 0; i < (1 << 15); i++) {
			/* Maps 000000000000000 000000000011111 = Red  = 0x1F
			   000001111100000 = Blue = 0x03E0 111110000000000 = Grn  =
			   0x7C00 */
			r = ((i & 0x1F) << 3) + 4;
			g = ((i & 0x03E0) >> 2) + 4;
			b = ((i & 0x7C00) >> 7) + 4;

			pal = (unsigned char *) d_8to24table;

			for (v = 0, k = 0, bestdist = 10000.0; v < 256; v++, pal += 4) {
				r1 = (int) r - (int) pal[0];
				g1 = (int) g - (int) pal[1];
				b1 = (int) b - (int) pal[2];
				dist = sqrt (((r1 * r1) + (g1 * g1) + (b1 * b1)));
				if (dist < bestdist) {
					k = v;
					bestdist = dist;
				}
			}
			d_15to8table[i] = k;
		}
		snprintf (s, sizeof (s), "%s/glquake/15to8.pal", com_gamedir);
		COM_CreatePath (s);
		if ((f = Qopen (s, "wb")) != NULL) {
			Qwrite (f, d_15to8table, 1 << 15);
			Qclose (f);
		}
	}
}

/*
===============
GL_Init_Common
===============
*/
void
GL_Init_Common (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

	glClearColor (0, 0, 0, 0);
	glCullFace (GL_FRONT);
	glEnable (GL_TEXTURE_2D);

	glEnable (GL_ALPHA_TEST);
	glAlphaFunc (GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glShadeModel (GL_FLAT);

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	CheckMultiTextureExtensions ();
}

/*
=================
GL_BeginRendering
=================
*/
void
GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;
}

qboolean
VID_Is8bit (void)
{
	return is8bit;
}


#ifdef GL_SHARED_TEXTURE_PALETTE_EXT
void
Tdfx_Init8bitPalette (void)
{
	// Check for 8bit Extensions and initialize them.
	int         i;

	if (is8bit) {
		return;
	}

	if (QFGL_ExtensionPresent ("3DFX_set_global_palette")) {

		char       *oldpal;
		GLubyte     table[256][4];
		QF_gl3DfxSetPaletteEXT qgl3DfxSetPaletteEXT = NULL;

		if (!(qgl3DfxSetPaletteEXT = QFGL_ExtensionAddress ("gl3DfxSetPaletteEXT"))) {
			return;
		}

		Con_Printf ("3DFX_set_global_palette.\n");

		oldpal = (char *) d_8to24table;	// d_8to24table3dfx;
		for (i = 0; i < 256; i++) {
			table[i][2] = *oldpal++;
			table[i][1] = *oldpal++;
			table[i][0] = *oldpal++;
			table[i][3] = 255;
			oldpal++;
		}
		glEnable (GL_SHARED_TEXTURE_PALETTE_EXT);
		qgl3DfxSetPaletteEXT ((GLuint *) table);
		is8bit = true;
	}
}

/*
 * The GL_EXT_shared_texture_palette seems like an idea which is 
 * /almost/ a good idea, but seems to be severely broken with many
 * drivers, as such it is disabled.
 *
 * It should be noted, that a palette object extension as suggested by
 * the GL_EXT_shared_texture_palette spec might be a very good idea in
 * general.
 */
#if 0
void
Shared_Init8bitPalette (void)
{
	int         i;
	GLubyte     thePalette[256][4];
	GLubyte    *oldPalette;

	QF_glColorTableEXT qglColorTableEXT = NULL;

	if (is8bit) {
		return;
	}

	if (QFGL_ExtensionPresent ("GL_EXT_shared_texture_palette")) {
		if (!(qglColorTableEXT = QFGL_ExtensionAddress ("glColorTableEXT"))) {
			return;
		}

		Con_Printf ("GL_EXT_shared_texture_palette\n");

		oldPalette = (GLubyte *) d_8to24table;	// d_8to24table3dfx;
		for (i = 0; i < 256; i++) {
			thePalette[i][0] = *oldPalette++;
			thePalette[i][1] = *oldPalette++;
			thePalette[i][2] = *oldPalette++;
			thePalette[i][3] = *oldPalette++;
		}
		glEnable (GL_SHARED_TEXTURE_PALETTE_EXT);
		qglColorTableEXT (GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGBA, 256, GL_RGBA,
						  GL_UNSIGNED_BYTE, (GLvoid *) thePalette);
		is8bit = true;
	}
}
#endif
#endif

void
VID_Init8bitPalette (void)
{
	vid_use8bit =
		Cvar_Get ("vid_use8bit", "0", CVAR_ROM, "Use 8-bit shared palettes.");

	Con_Printf ("Checking for 8-bit extension: ");
	if (vid_use8bit->int_val) {
#ifdef GL_SHARED_TEXTURE_PALETTE_EXT
		Tdfx_Init8bitPalette ();
		//Shared_Init8bitPalette ();
#endif
		if (!is8bit) {
			Con_Printf ("none\n");
		}
	} else {
		Con_Printf ("disabled.\n");
	}
}

void
VID_LockBuffer (void)
{
}

void
VID_UnlockBuffer (void)
{
}

void
D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void
D_EndDirectRect (int x, int y, int width, int height)
{
}
