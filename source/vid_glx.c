/*
	vid_glx.c

	OpenGL GLX video driver

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2000       Marcus Sundberg [mackan@stacken.kth.se]

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif
#ifndef RTLD_LAZY
# ifdef DL_LAZY
#  define RTLD_LAZY	DL_LAZY
# else
#  define RTLD_LAZY	0
# endif
#endif

#include <GL/glx.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef HAVE_DGA
# include <X11/extensions/xf86dga.h>
#endif

#ifdef XMESA
# include <GL/xmesa.h>
#endif

#include "qtypes.h"
#include "va.h"
#include "quakedef.h"
#include "qendian.h"
#include "glquake.h"
#include "cvar.h"
#include "qargs.h"
#include "console.h"
#include "keys.h"
#include "menu.h"
#include "sys.h"
#include "quakefs.h"
#include "draw.h"
#include "input.h"
#include "sbar.h"
#include "context_x11.h"
#include "dga_check.h"

#define WARP_WIDTH              320
#define WARP_HEIGHT             200

static qboolean		vid_initialized = false;

static GLXContext	ctx = NULL;

unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];
unsigned char	d_15to8table[65536];

#ifdef HAVE_DGA
static int	hasdgavideo = 0;
static int	hasdga = 0;
#endif


#ifdef HAVE_DLOPEN
static void	*dlhand = NULL;
#endif
static GLboolean (*QF_XMesaSetFXmode)(GLint mode) = NULL;


int scr_width, scr_height;

#if defined(XMESA) || defined(HAVE_DGA)
int VID_options_items = 2;
#else
int VID_options_items = 1;
#endif

/*-----------------------------------------------------------------------*/

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

float		gldepthmin, gldepthmax;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

qboolean is8bit = false;

// ARB Multitexture
int gl_mtex_enum = TEXTURE0_SGIS;
qboolean gl_arb_mtex = false;
qboolean gl_mtexable = false;


/*-----------------------------------------------------------------------*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

void
VID_Shutdown(void)
{
	if (!vid_initialized)
		return;

	Con_Printf("VID_Shutdown\n");
	glXDestroyContext(x_disp, ctx);
	XDestroyWindow(x_disp, x_win);

	x11_restore_vidmode();
#ifdef HAVE_DLOPEN
	if (dlhand) {
		dlclose(dlhand);
		dlhand = NULL;
	}
#endif
	x11_close_display();
}
#if 0
static void
signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(sig);
}

static void
InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
/*	signal(SIGFPE, signal_handler); */
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}
#endif
void VID_ShiftPalette(unsigned char *p)
{
	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned int r,g,b;
	unsigned int v;
	int     r1,g1,b1;
	int		k;
	unsigned short i;
	unsigned int	*table;
	QFile *f;
	char s[255];
	float dist, bestdist;
	static qboolean palflag = false;

//
// 8 8 8 encoding
//
//	Con_Printf("Converting 8to24\n");

	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

//		v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//		v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0;		// 255 is transparent

	// JACK: 3D distance calcs - k is last closest, l is the distance.
	// FIXME: Precalculate this and cache to disk.
	if (palflag)
		return;
	palflag = true;

	COM_FOpenFile("glquake/15to8.pal", &f);
	if (f) {
		Qread(f, d_15to8table, 1<<15);
		Qclose(f);
	} else {
		for (i=0; i < (1<<15); i++) {
			/* Maps
 			000000000000000
 			000000000011111 = Red  = 0x1F
 			000001111100000 = Blue = 0x03E0
 			111110000000000 = Grn  = 0x7C00
 			*/
 			r = ((i & 0x1F) << 3)+4;
 			g = ((i & 0x03E0) >> 2)+4;
 			b = ((i & 0x7C00) >> 7)+4;
			pal = (unsigned char *)d_8to24table;
			for (v=0,k=0,bestdist=10000.0; v<256; v++,pal+=4) {
 				r1 = (int)r - (int)pal[0];
 				g1 = (int)g - (int)pal[1];
 				b1 = (int)b - (int)pal[2];
				dist = sqrt(((r1*r1)+(g1*g1)+(b1*b1)));
				if (dist < bestdist) {
					k=v;
					bestdist = dist;
				}
			}
			d_15to8table[i]=k;
		}
		snprintf(s, sizeof(s), "%s/glquake", com_gamedir);
 		Sys_mkdir (s);
		snprintf(s, sizeof(s), "%s/glquake/15to8.pal", com_gamedir);
		if ((f = Qopen(s, "wb")) != NULL) {
			Qwrite(f, d_15to8table, 1<<15);
			Qclose(f);
		}
	}
}


/*
	CheckMultiTextureExtensions

	Check for ARB, SGIS, or EXT multitexture support
*/
void
CheckMultiTextureExtensions ( void )
{
	Con_Printf ("Checking for multitexture... ");
	if (COM_CheckParm ("-nomtex"))
	{
		Con_Printf ("disabled\n");
		return;
	}
#ifdef HAVE_DLOPEN
	dlhand = dlopen (NULL, RTLD_LAZY);
	if (dlhand == NULL)
	{
		Con_Printf ("unable to check\n");
		return;
	}
	if (strstr(gl_extensions, "GL_ARB_multitexture "))
	{
		Con_Printf ("GL_ARB_multitexture\n");
		qglMTexCoord2f = (void *)dlsym(dlhand, "glMultiTexCoord2fARB");
		qglSelectTexture = (void *)dlsym(dlhand, "glActiveTextureARB");
		gl_mtex_enum = GL_TEXTURE0_ARB;
		gl_mtexable = true;
		gl_arb_mtex = true;
	} else if (strstr(gl_extensions, "GL_SGIS_multitexture "))
	{
		Con_Printf ("GL_SGIS_multitexture\n");
		qglMTexCoord2f = (void *)dlsym(dlhand, "glMTexCoord2fSGIS");
		qglSelectTexture = (void *)dlsym(dlhand, "glSelectTextureSGIS");
		gl_mtex_enum = TEXTURE0_SGIS;
		gl_mtexable = true;
		gl_arb_mtex = false;
	} else if (strstr(gl_extensions, "GL_EXT_multitexture "))
	{
		Con_Printf ("GL_EXT_multitexture\n");
		qglMTexCoord2f = (void *)dlsym(dlhand, "glMTexCoord2fEXT");
		qglSelectTexture = (void *)dlsym(dlhand, "glSelectTextureEXT");
		gl_mtex_enum = TEXTURE0_SGIS;
		gl_mtexable = true;
		gl_arb_mtex = false;
	} else {
		Con_Printf ("none found\n");
	}
	dlclose(dlhand);
	dlhand = NULL;		
#else
	gl_mtexable = false;
#endif
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

	CheckMultiTextureExtensions ();

	glClearColor (0,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_BLEND);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;
}


void GL_EndRendering (void)
{
	glFlush();
	glXSwapBuffers(x_disp, x_win);
	Sbar_Changed ();
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

#ifdef GL_EXT_SHARED
void VID_Init8bitPalette()
{
	// Check for 8bit Extensions and initialize them.
	int i;
	char thePalette[256*3];
	char *oldPalette, *newPalette;

	if (strstr(gl_extensions, "GL_EXT_shared_texture_palette") == NULL)
		return;

	Con_Printf("8-bit GL extensions enabled.\n");
	glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
	oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
	newPalette = thePalette;
	for (i=0;i<256;i++) {
		*newPalette++ = *oldPalette++;
		*newPalette++ = *oldPalette++;
		*newPalette++ = *oldPalette++;
		oldPalette++;
	}
	glColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
	is8bit = true;
}

#else

void VID_Init8bitPalette(void)
{
}

#endif

void VID_Init(unsigned char *palette)
{
	int i;
	static int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};
	char	gldir[MAX_OSPATH];

	VID_GetWindowSize (640, 480);
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	/* Interpret command-line params
	 */

	/* Set vid parameters */

	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = atoi(com_argv[i+1]);
	else
		vid.conwidth = scr_width;

	vid.conwidth &= 0xfff8; // make it a multiple of eight
	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth * 3 / 4;

	i = COM_CheckParm ("-conheight");
	if ( i != 0 )	// Set console height, but no smaller than 200 px
		vid.conheight = atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	x11_open_display();

	x_visinfo = glXChooseVisual(x_disp, x_screen, attrib);
	if (!x_visinfo) {
		fprintf(stderr, "Error couldn't get an RGB, Double-buffered, Depth visual\n");
		exit(1);
	}
	x_vis = x_visinfo->visual;

#ifdef HAVE_DGA
	{
		int maj_ver;

		hasdga = VID_CheckDGA(x_disp, &maj_ver, NULL, &hasdgavideo);
		if (!hasdga || maj_ver < 1) {
			hasdga = hasdgavideo = 0;
		}
	}
	Con_Printf ("hasdga = %i\nhasdgavideo = %i\n", hasdga, hasdgavideo);
#endif
#ifdef HAVE_DLOPEN
	dlhand = dlopen(NULL, RTLD_LAZY);
	if (dlhand) {
		QF_XMesaSetFXmode = dlsym(dlhand, "XMesaSetFXmode");
		if (!QF_XMesaSetFXmode) {
			QF_XMesaSetFXmode = dlsym(dlhand, "_XMesaSetFXmode");
		}
	} else {
		QF_XMesaSetFXmode = NULL;
	}
#else
#ifdef XMESA
	QF_XMesaSetFXmode = XMesaSetFXmode;
#endif
#endif
	if (QF_XMesaSetFXmode) {
#ifdef XMESA
		const char *str = getenv("MESA_GLX_FX");
		if (str != NULL && *str != 'd') {
			if (tolower(*str) == 'w') {
				Cvar_Set (vid_fullscreen, "0");
			} else {
				Cvar_Set (vid_fullscreen, "1");
			}
		}
#endif
		/* Glide uses DGA internally, so we don't want to
		   mess with it. */
//		hasdga = 0;
	}

	x11_set_vidmode(scr_width, scr_height);
	x11_create_window(scr_width, scr_height);
	/* Invisible cursor */
	x11_create_null_cursor();

	x11_grab_keyboard();

	XSync(x_disp, 0);

	ctx = glXCreateContext(x_disp, x_visinfo, NULL, True);

	glXMakeCurrent(x_disp, x_win, ctx);

	if (vid.conheight > scr_height)
		vid.conheight = scr_height;
	if (vid.conwidth > scr_width)
		vid.conwidth = scr_width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 2;

	//InitSig(); // trap evil signals

	GL_Init();

	snprintf(gldir, sizeof(gldir), "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);

	GL_CheckGamma (palette);
	VID_SetPalette (palette);

	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();

	Con_Printf ("Video mode %dx%d initialized.\n",
			scr_width, scr_height);

	vid_initialized = true;

	vid.recalc_refdef = 1;		// force a surface cache flush
}

void
VID_InitCvars (void)
{
	// It may not look like it, but this is important
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
VID_SetCaption (char *text)
{
	if (text && *text) {
		char *temp = strdup (text);
		x11_set_caption (va ("%s %s: %s", PROGRAM, VERSION, temp));
		free (temp);
	} else {
		x11_set_caption (va ("%s %s", PROGRAM, VERSION));
	}
}
