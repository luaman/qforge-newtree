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
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

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

int         texture_extension_number = 1;
float       gldepthmin, gldepthmax;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

// ARB Multitexture
qboolean    gl_mtex_capable = false;
GLenum		gl_mtex_enum = GL_TEXTURE0_ARB;

QF_glColorTableEXT	qglColorTableEXT = NULL;
qboolean			is8bit = false;

cvar_t	*vid_use8bit;
cvar_t	*brightness;
cvar_t	*contrast;

extern int gl_filter_min, gl_filter_max;

/*-----------------------------------------------------------------------*/

void
GL_Common_Init_Cvars (void)
{
	vid_use8bit = Cvar_Get ("vid_use8bit", "0", CVAR_ROM, "Use 8-bit shared palettes.");
	brightness = Cvar_Get ("brightness", "1", CVAR_ARCHIVE, "Brightness level");
	contrast = Cvar_Get ("contrast", "1", CVAR_ARCHIVE, "Contrast level");
	gl_multitexture = Cvar_Get ("gl_multitexture", "0", CVAR_ARCHIVE, "Use multitexture when available");
}

/*
	CheckMultiTextureExtensions

	Check for ARB multitexture support
*/
void
CheckMultiTextureExtensions (void)
{
	Con_Printf ("Checking for multitexture: ");
	if (COM_CheckParm ("-nomtex")) {
		Con_Printf ("disabled.\n");
		return;
	}

	if (QFGL_ExtensionPresent ("GL_ARB_multitexture")) {

		int max_texture_units = 0;

		glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &max_texture_units);
		if (max_texture_units >= 2) {
			Con_Printf ("enabled, %d TMUs.\n", max_texture_units);
			qglMultiTexCoord2f = QFGL_ExtensionAddress ("glMultiTexCoord2fARB");
			qglActiveTexture = QFGL_ExtensionAddress ("glActiveTextureARB");
			gl_mtex_enum = GL_TEXTURE0_ARB;
			gl_mtex_capable = true;
		} else {
			Con_Printf ("disabled, not enough TMUs.\n");
		}
	} else {
		Con_Printf ("not found.\n");
	}
}

// LordHavoc: finds closest color match
byte
LHFindColor(unsigned char *palette, int first, int last, int r, int g, int b)
{
	int i, bestdistance, bestcolor, distance, color[3];
	bestdistance = 1000000000;
	bestcolor = first;
	color[0] = r;
	color[1] = g;
	color[2] = b;
	palette += first * 3;
	for (i = first;i < last;i++)
	{
		// LordHavoc: distance in color cube from desired color
		distance = VectorDistance_fast(palette, color);
		if (distance < bestdistance)
		{
			bestdistance = distance;
			bestcolor = i;
		}
		palette += 3;
	}
	return bestcolor;
}

void
VID_SetPalette (unsigned char *palette)
{
	byte       *pal, *out;
	unsigned short i;
	QFile      *f;
	char        s[255];
	static qboolean palflag = false;

//  Con_Printf("Converting 8to24\n");

	// LordHavoc: cleaned up stupid endian-specific table building,
	//            now builds directly as bytes
	pal = palette;
	out = (byte *)&d_8to24table;
	for (i = 0; i < 255; i++) {
		*out++ = *pal++;
		*out++ = *pal++;
		*out++ = *pal++;
		*out++ = 255;
	}
	d_8to24table[255] = 0;				// 255 is transparent

	if (palflag)
		return;
	palflag = true;

	COM_FOpenFile ("glquake/15to8.pal", &f);
	if (f) {
		Qread (f, d_15to8table, 32768);
		Qclose (f);
	} else {
		// LordHavoc: cleaned this up
		Con_Printf("Building 15bit to 8bit color conversion table\n");
		for (i = 0; i < 32768; i++) {
			int r, g, b;
			// split out to 8bit components
			r = ((i & 0x001F) >>  0) << 3;
			g = ((i & 0x03E0) >>  5) << 3;
			b = ((i & 0x7C00) >> 10) << 3;
			d_15to8table[i] = LHFindColor(palette, 0, 256, r, g, b);
		}
		snprintf (s, sizeof (s), "%s/glquake/15to8.pal", com_gamedir);
		COM_CreatePath (s);
		if ((f = Qopen (s, "wb")) != NULL) {
			Qwrite (f, d_15to8table, 32768);
			Qclose (f);
		}
	}
}

/*
	GL_Init_Common
*/
void
GL_Init_Common (void)
{
	GL_Common_Init_Cvars ();

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

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_filter_min);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_filter_max);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	CheckMultiTextureExtensions ();
}

/*
	GL_BeginRendering
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
void
Shared_Init8bitPalette (void)
{
	int 		i;
	GLubyte 	thePalette[256 * 3];
	GLubyte 	*oldPalette, *newPalette;

	if (is8bit) {
		return;
	}

	if (QFGL_ExtensionPresent ("GL_EXT_shared_texture_palette")) {
		if (!(qglColorTableEXT = QFGL_ExtensionAddress ("glColorTableEXT"))) {
			return;
		}

		Con_Printf ("GL_EXT_shared_texture_palette\n");

		glEnable (GL_SHARED_TEXTURE_PALETTE_EXT);
		oldPalette = (GLubyte *) d_8to24table;	// d_8to24table3dfx;
		newPalette = thePalette;
		for (i = 0; i < 256; i++) {
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			*newPalette++ = *oldPalette++;
			oldPalette++;
		}
		qglColorTableEXT (GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB,
							GL_UNSIGNED_BYTE, (GLvoid *) thePalette);
		is8bit = true;
	}
}
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
		Shared_Init8bitPalette ();
#endif
		if (!is8bit) {
			Con_Printf ("not found.\n");
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
