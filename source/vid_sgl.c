/*
	vid_sgl.c

	Video driver for OpenGL-using versions of SDL

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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <values.h>

#include <SDL.h>

#include "console.h"
#include "cvar.h"
#include "draw.h"
#include "va.h"
#include "glquake.h"
#include "input.h"
#include "joystick.h"
#include "keys.h"
#include "menu.h"
#include "sys.h"
#include "qargs.h"
#include "qendian.h"
#include "qtypes.h"
#include "quakedef.h"

#define	WARP_WIDTH	320
#define	WARP_HEIGHT	200

static qboolean		vid_initialized = false;

cvar_t	*vid_fullscreen;
cvar_t	*_windowed_mouse;
cvar_t	*m_filter;

#ifdef WIN32
/* fixme: this is evil hack */
#include <windows.h>
HWND 		mainwindow;
#endif

unsigned short	d_8to16table[256];
unsigned int	d_8to24table[256];
unsigned char   d_15to8table[65536];

extern int scr_width, scr_height;
int VID_options_items = 1;

int texture_mode = GL_LINEAR;

int texture_extension_number = 1;

float	gldepthmin, gldepthmax;

const char	*gl_vendor;
const char	*gl_renderer;
const char	*gl_version;
const char	*gl_extensions;

qboolean is8bit = false;
qboolean gl_mtexable = false;
int gl_mtex_enum = TEXTURE0_SGIS;

int modestate;

static qboolean mouse_avail;
static float	mouse_x, mouse_y;
static float	old_mouse_x, old_mouse_y;
static float	old_windowed_mouse;

void
D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void
D_EndDirectRect (int x, int y, int width, int height)
{
}

void
VID_Shutdown (void)
{
	if (!vid_initialized)
		return;

	Con_Printf ("VID_Shutdown\n");

	SDL_Quit ();
}
#ifndef WIN32
static void
signal_handler (int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(sig);
}

static void
InitSig (void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
//	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
}
#endif

void
VID_SetPalette (unsigned char *palette)
{
	byte		*pal;
	unsigned int	r,g,b;
	unsigned int	v;
	int 		r1,g1,b1;
	int 		k;
	unsigned short i;
	unsigned int	*table;
	QFile		*f;
	char		s[256];
	float		dist, bestdist;
	static		qboolean palflag = false;

//
// 8 8 8 encoding
//
//	Con_Printf("Converting 8to24\n");

	pal = palette;
	table = d_8to24table;
	for (i=0; i<256; i++) {
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

//	      v = (255<<24) + (r<<16) + (g<<8) + (b<<0);
//	      v = (255<<0) + (r<<8) + (g<<16) + (b<<24);
		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] = 0;	// 255 is transparent

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
			pal = (unsigned char *) d_8to24table;
			for (v=0, k=0, bestdist = 10000.0; v<256; v++, pal += 4) {
				r1 = (int) r - (int) pal[0];
				g1 = (int) g - (int) pal[1];
				b1 = (int) b - (int) pal[2];
				dist = sqrt (((r1 * r1) + (g1 * g1) + (b1 * b1)));
				if (dist < bestdist) {
					k = v;
					bestdist = dist;
				}
			}
			d_15to8table[i]=k;
		}
		snprintf (s, sizeof (s), "%s/glquake", com_gamedir);
		Sys_mkdir (s);
		snprintf(s, sizeof (s), "%s/glquake/15to8.pal", com_gamedir);
		if ((f = Qopen (s, "wb")) != NULL) {
			Qwrite (f, d_15to8table, 1<<15);
			Qclose (f);
		}
	}
}

void
VID_ShiftPalette (unsigned char *palette)
{
    VID_SetPalette(palette);
}

void
GL_Init (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

	glClearColor (0,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glShadeModel (GL_FLAT);

	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glEnable(GL_BLEND);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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

void
GL_EndRendering (void)
{
	glFlush();
	SDL_GL_SwapBuffers ();
}

qboolean
VID_Is8bit (void)
{
	return is8bit;
}

#ifdef GL_EXT_SHARED
void
VID_Init8bitPalette (void)
{
	// Check for 8bit Extensions and initialize them.
	int 	i;
	char	thePalette[256*3];
	char	*oldPalette, *newPalette;

	if (strstr (gl_extensions, "GL_EXT_shared_texture_palette") == NULL)
		return;

	Con_Printf ("8-bit GL extensions enabled.\n");
	glEnable (GL_SHARED_TEXTURE_PALETTE_EXT);
	oldPalette = (char *) d_8to24table; //d_8to24table3dfx;
	newPalette = thePalette;
	for (i=0; i<256; i++) {
		*newPalette++ = *oldPalette++;
		*newPalette++ = *oldPalette++;
		*newPalette++ = *oldPalette++;
		oldPalette++;
	}
	glColorTableEXT (GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGB, GL_UNSIGNED_BYTE, (void *) thePalette);
	is8bit = true;
}

#else

void
VID_Init8bitPalette (void)
{
}

#endif

void
VID_Init (unsigned char *palette)
{
	Uint32 flags = SDL_OPENGL;
	int i;
	char gldir[MAX_OSPATH];

	VID_GetWindowSize (640, 480);

	vid_fullscreen = Cvar_Get ("vid_fullscreen","0",0,"None");

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

	// Interpret command-line params

	// Set vid parameters
	if ((i = COM_CheckParm ("-conwidth")) != 0)
		vid.conwidth = atoi(com_argv[i+1]);
	else
		vid.conwidth = scr_width;

	vid.conwidth &= 0xfff8; // make it a multiple of eight
	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth * 3 / 4;

	i = COM_CheckParm ("-conheight");
	if ( i != 0 )   // Set console height, but no smaller than 200 px
		vid.conheight = max (atoi (com_argv[i+1]), 200);

	// Initialize the SDL library 
	if (SDL_Init (SDL_INIT_VIDEO) < 0) 
	   Sys_Error ("Couldn't initialize SDL: %s\n", SDL_GetError ());

	// Check if we want fullscreen
	if (vid_fullscreen->int_val) {
		flags |= SDL_FULLSCREEN;
		// Don't annoy Mesa/3dfx folks
#ifndef WIN32
		// FIXME: Maybe this could be put in a different spot, but I don't know where.
		// Anyway, it's to work around a 3Dfx Glide bug.
		SDL_ShowCursor (0);
		SDL_WM_GrabInput (SDL_GRAB_ON);
		setenv ("MESA_GLX_FX", "fullscreen", 1);
	} else {
		setenv ("MESA_GLX_FX", "window", 1);
#endif
	}

	// Setup GL Attributes
	SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 1);
	SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 1);
	SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 1);
	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 1);
	
	if (SDL_SetVideoMode (scr_width, scr_height, 8, flags) == NULL) {
	   Sys_Error ("Couldn't set video mode: %s\n", SDL_GetError ());
	   SDL_Quit ();
	}

	vid.height = vid.conheight = min (vid.conheight, scr_height);
	vid.width = vid.conwidth = min (vid.conwidth, scr_width);

	vid.aspect = ((float) vid.height / (float) vid.width) * (320.0 / 240.0);
	vid.numpages = 2;
#ifndef WIN32
	InitSig (); // trap evil signals
#endif

	GL_Init ();

	snprintf(gldir, sizeof(gldir), "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);

	VID_SetPalette (palette);

	// Check for 3DFX Extensions and initialize them.
	VID_Init8bitPalette();

	Con_Printf ("Video mode %dx%d initialized.\n",
			scr_width, scr_height);

	vid_initialized = true;
#ifdef WIN32
	// fixme: EVIL thing - but needed for win32 until we get
	// SDL_sound ready - without this DirectSound fails.
	// could replace this with SDL_SysWMInfo
	mainwindow=GetActiveWindow();
#endif
	vid.recalc_refdef = 1;	  // force a surface cache flush
}

void
VID_InitCvars ()
{
}

/*
================
IN_SendKeyEvents
================
*/

void
IN_SendKeyEvents (void)
{
	SDL_Event	event;
	int 		sym, state, but;
	int 		modstate;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		    case SDL_KEYDOWN:
		    case SDL_KEYUP:
				sym = event.key.keysym.sym;
				state = event.key.state;
				modstate = SDL_GetModState ();
				switch (sym) {
					case SDLK_DELETE: sym = K_DEL; break;
					case SDLK_BACKSPACE: sym = K_BACKSPACE; break;
					case SDLK_F1: sym = K_F1; break;
					case SDLK_F2: sym = K_F2; break;
					case SDLK_F3: sym = K_F3; break;
					case SDLK_F4: sym = K_F4; break;
					case SDLK_F5: sym = K_F5; break;
					case SDLK_F6: sym = K_F6; break;
					case SDLK_F7: sym = K_F7; break;
					case SDLK_F8: sym = K_F8; break;
					case SDLK_F9: sym = K_F9; break;
					case SDLK_F10: sym = K_F10; break;
					case SDLK_F11: sym = K_F11; break;
					case SDLK_F12: sym = K_F12; break;
					case SDLK_BREAK:
					case SDLK_PAUSE: sym = K_PAUSE; break;
					case SDLK_UP: sym = K_UPARROW; break;
					case SDLK_DOWN: sym = K_DOWNARROW; break;
					case SDLK_RIGHT: sym = K_RIGHTARROW; break;
					case SDLK_LEFT: sym = K_LEFTARROW; break;
					case SDLK_INSERT: sym = K_INS; break;
					case SDLK_HOME: sym = K_HOME; break;
					case SDLK_END: sym = K_END; break;
					case SDLK_PAGEUP: sym = K_PGUP; break;
					case SDLK_PAGEDOWN: sym = K_PGDN; break;
					case SDLK_RSHIFT:
					case SDLK_LSHIFT: sym = K_SHIFT; break;
					case SDLK_RCTRL:
					case SDLK_LCTRL: sym = K_CTRL; break;
					case SDLK_RALT:
					case SDLK_LALT: sym = K_ALT; break;
					case SDLK_CAPSLOCK:  sym = K_CAPSLOCK; break;
					case SDLK_KP0:
						if (modstate & KMOD_NUM)
							sym = K_INS;
						else
							sym = SDLK_0;
						break;
					case SDLK_KP1:
						if (modstate & KMOD_NUM)
							sym = K_END;
						else
							sym = SDLK_1;
						break;
					case SDLK_KP2:
						if (modstate & KMOD_NUM)
							sym = K_DOWNARROW;
						else
							sym = SDLK_2;
						break;
					case SDLK_KP3:
						if (modstate & KMOD_NUM)
							sym = K_PGDN;
						else
							sym = SDLK_3;
						break;
					case SDLK_KP4:
						if (modstate & KMOD_NUM)
							sym = K_LEFTARROW;
						else
							sym = SDLK_4;
						break;
					case SDLK_KP5: sym = SDLK_5; break;
					case SDLK_KP6:
						if (modstate & KMOD_NUM)
							sym = K_RIGHTARROW;
						else
							sym = SDLK_6;
						break;
					case SDLK_KP7:
						if (modstate & KMOD_NUM)
							sym = K_HOME;
						else
							sym = SDLK_7;
						break;
					case SDLK_KP8:
						if (modstate & KMOD_NUM)
							sym = K_UPARROW;
						else
							sym = SDLK_8;
						break;
					case SDLK_KP9:
						if (modstate & KMOD_NUM)
							sym = K_PGUP;
						else
							sym = SDLK_9;
						break;
					case SDLK_KP_PERIOD:
						if (modstate & KMOD_NUM)
							sym = K_DEL;
						else
							sym = SDLK_PERIOD;
						break;
					case SDLK_KP_DIVIDE: sym = SDLK_SLASH; break;
					case SDLK_KP_MULTIPLY: sym = SDLK_ASTERISK; break;
					case SDLK_KP_MINUS: sym = SDLK_MINUS; break;
					case SDLK_KP_PLUS: sym = SDLK_PLUS; break;
					case SDLK_KP_ENTER: sym = SDLK_RETURN; break;
					case SDLK_KP_EQUALS: sym = SDLK_EQUALS; break;
				}
				// If we're not directly handled and still above 255
				// just force it to 0
				if (sym > 255)
					sym = 0;
				Key_Event(sym, state);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				but = event.button.button;
				if (but == 2) {
					but = 3;
				} else if (but == 3) {
					but = 2;
				}
				switch (but) {
					case 1:
					case 2:
					case 3:
						Key_Event(K_MOUSE1 + but - 1, event.type == SDL_MOUSEBUTTONDOWN);
						break;
					case 4:
						Key_Event(K_MWHEELUP, 1);
						Key_Event(K_MWHEELUP, 0);
						break;
					case 5:
						Key_Event(K_MWHEELDOWN, 1);
						Key_Event(K_MWHEELDOWN, 0);
						break;
					default:
						break;
				}
				break;

			case SDL_MOUSEMOTION:
				if (_windowed_mouse->int_val) {
					if ((event.motion.x != (vid.width/2)) ||
							(event.motion.y != (vid.height/2)) ) {
						mouse_x = event.motion.xrel*2;
						mouse_y = event.motion.yrel*2;
						if ( (event.motion.x < ((vid.width/2)-(vid.width/4))) ||
								(event.motion.x > ((vid.width/2)+(vid.width/4))) ||
								(event.motion.y < ((vid.height/2)-(vid.height/4))) ||
								(event.motion.y > ((vid.height/2)+(vid.height/4))) ) {
							SDL_WarpMouse(vid.width/2, vid.height/2);
						}
					}
				} else {
					mouse_x = event.motion.xrel*2;
					mouse_y = event.motion.yrel*2;
				}
				break;

			case SDL_QUIT:
				CL_Disconnect ();
				Sys_Quit ();
				break;
			default:
				break;
		}
	}
}

void
IN_Init (void)
{
	JOY_Init ();

	_windowed_mouse = Cvar_Get ("_windowed_mouse", "0", CVAR_ARCHIVE, "Grab mouse and keyboard input");
	m_filter = Cvar_Get ("m_filter", "0", CVAR_ARCHIVE, "None");

	if (COM_CheckParm ("-nomouse") && !_windowed_mouse->int_val)
		return;

	mouse_x = mouse_y = 0.0;
	mouse_avail = 1;
	SDL_ShowCursor (0);
	SDL_WM_GrabInput (SDL_GRAB_ON);
	// FIXME: disable DGA if in_dgamouse says to
}

void
IN_Shutdown (void)
{
	JOY_Shutdown ();

	Con_Printf ("IN_Shutdown\n");
	mouse_avail = 0;
}

void
IN_Commands (void)
{
	JOY_Command ();

	if (old_windowed_mouse != _windowed_mouse->int_val) {
		old_windowed_mouse = _windowed_mouse->int_val;

		if (_windowed_mouse->int_val) {	// grab the pointer
			SDL_ShowCursor (0);
			SDL_WM_GrabInput (SDL_GRAB_ON);
		} else {	// ungrab the pointer
			SDL_WM_GrabInput (SDL_GRAB_OFF);
			SDL_ShowCursor (1);
		}
	}
}

void
IN_Move (usercmd_t *cmd)
{
	JOY_Move (cmd);
	
	if (!mouse_avail)
		return;

	if (m_filter->int_val) {
		mouse_x = (mouse_x + old_mouse_x) * 0.5;
		mouse_y = (mouse_y + old_mouse_y) * 0.5;
	}

	old_mouse_x = mouse_x;
	old_mouse_y = mouse_y;

	mouse_x *= sensitivity->value;
	mouse_y *= sensitivity->value;

	if ((in_strafe.state & 1) || (lookstrafe->int_val && freelook))
		cmd->sidemove += m_side->value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw->value * mouse_x;

	if (freelook)
		V_StopPitchDrift ();

	if (freelook && !(in_strafe.state & 1)) {
		cl.viewangles[PITCH] += m_pitch->value * mouse_y;
		cl.viewangles[PITCH] = bound (-70, cl.viewangles[PITCH], 80);
	} else {
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward->value * mouse_y;
		else
			cmd->forwardmove -= m_forward->value * mouse_y;
	}
	mouse_x = mouse_y = 0.0;
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
		SDL_WM_SetCaption (va ("%s %s: %s", PROGRAM, VERSION, temp), NULL);
		free (temp);
	} else {
		SDL_WM_SetCaption (va ("%s %s", PROGRAM, VERSION), NULL);
	}
}

