/*
	context_x11.c

	general x11 context layer

	Copyright (C) 1996-1997  Id Software, Inc.
	Copyright (C) 2000       Zephaniah E. Hull <warp@whitestar.soark.net>
	Copyright (C) 1999,2000  contributors of the QuakeForge project
	Please see the file "AUTHORS" for a list of contributors

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

#include <config.h>

#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/XShm.h>
#include <errno.h>
#include <values.h>

#ifdef HAVE_VIDMODE
# include <X11/extensions/xf86vmode.h>
#endif

#include "context_x11.h"
#include "dga_check.h"
#include "qtypes.h"
#include "vid.h"
#include "sys.h"
#include "console.h"
#include "cvar.h"
#include "input.h"

static void (*event_handlers[LASTEvent])	(XEvent *);
qboolean	oktodraw = false;
int 		x_shmeventtype;

static int	x_disp_ref_count = 0;

Display		*x_disp = NULL;
int			x_screen;
Window		x_root = None;
XVisualInfo	*x_visinfo;
Visual		*x_vis;
Window		x_win;
Cursor		nullcursor = None;
static Atom aWMDelete = 0;

#define X_MASK (VisibilityChangeMask | StructureNotifyMask | ExposureMask)

#ifdef HAVE_VIDMODE
static XF86VidModeModeInfo **vidmodes;
static int  nummodes, hasvidmode = 0;
#endif

cvar_t	*vid_fullscreen;

static int xss_timeout;
static int xss_interval;
static int xss_blanking;
static int xss_exposures;

qboolean
x11_add_event(int event, void (*event_handler)(XEvent *))
{
	if (event >= LASTEvent) {
		printf("event: %d, LASTEvent: %d\n", event, LASTEvent);
		return false;
	}
	if (event_handlers[event] != NULL)
		return false;

	event_handlers[event] = event_handler;
	return true;
}

qboolean
x11_del_event(int event, void (*event_handler)(XEvent *))
{
	if (event >= LASTEvent)
		return false;
	if (event_handlers[event] != event_handler)
		return false;

	event_handlers[event] = NULL;
	return true;
}

void
x11_process_event( void )
{
	XEvent	x_event;

	XNextEvent(x_disp, &x_event);
	if ( x_event.type >= LASTEvent ) {
		// FIXME: KLUGE!!!!!!
		if (x_event.type == x_shmeventtype)
			oktodraw = 1;
		return;
	}
	if (event_handlers[x_event.type])
		event_handlers[x_event.type](&x_event);
}

void
x11_process_events(void)
{
	/* Get events from X server. */
	while ( XPending( x_disp )) {
		x11_process_event();
	}
}

// ========================================================================
// Tragic death handler
// ========================================================================

static void
TragicDeath(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(sig);
	//XCloseDisplay(x_disp);
	//VID_Shutdown();
	//Sys_Error("This death brought to you by the number %d\n", signal_num);
}

void
x11_open_display( void )
{
	if ( !x_disp ) {
		x_disp = XOpenDisplay( NULL );
		if ( !x_disp ) {
			Sys_Error("x11_open_display: Could not open display [%s]\n", XDisplayName( NULL ));
		}

		x_screen = DefaultScreen (x_disp);
		x_root = RootWindow (x_disp, x_screen);

		// catch signals
		signal(SIGHUP, TragicDeath);
		signal(SIGINT, TragicDeath);
		signal(SIGQUIT, TragicDeath);
		signal(SIGILL, TragicDeath);
		signal(SIGTRAP, TragicDeath);
		signal(SIGIOT, TragicDeath);
		signal(SIGBUS, TragicDeath);
		/*	signal(SIGFPE, TragicDeath); */
		signal(SIGSEGV, TragicDeath);
		signal(SIGTERM, TragicDeath);

		// for debugging only
		XSynchronize( x_disp, True );

		x_disp_ref_count=1;
	} else {
		x_disp_ref_count++;
	}
}

void
x11_close_display( void )
{
	if (nullcursor != None) {
		XFreeCursor(x_disp, nullcursor);
		nullcursor = None;
	}
	if (!--x_disp_ref_count) {
		XCloseDisplay( x_disp );
		x_disp = NULL;
	}
}

/*
	x11_create_null_cursor

	Create an empty cursor
*/
void
x11_create_null_cursor(void)
{
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	XColor dummycolour;

	if (nullcursor != None) return;

	cursormask = XCreatePixmap(x_disp, x_root, 1, 1, 1/*depth*/);
	xgc.function = GXclear;
	gc =  XCreateGC(x_disp, cursormask, GCFunction, &xgc);
	XFillRectangle(x_disp, cursormask, gc, 0, 0, 1, 1);
	dummycolour.pixel = 0;
	dummycolour.red = 0;
	dummycolour.flags = 04;
	nullcursor = XCreatePixmapCursor(x_disp, cursormask, cursormask,
									&dummycolour,&dummycolour, 0,0);
	XFreePixmap(x_disp,cursormask);
	XFreeGC(x_disp,gc);
	XDefineCursor(x_disp, x_win, nullcursor);
}

void
x11_set_vidmode(int width, int height)
{
	int i;

	vid_fullscreen = Cvar_Get ("vid_fullscreen","0",0,"None");

	XGetScreenSaver (x_disp, &xss_timeout, &xss_interval, &xss_blanking,
					&xss_exposures);
	XSetScreenSaver (x_disp, 0, xss_interval, xss_blanking,
					xss_exposures);

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

#ifdef HAVE_VIDMODE
	hasvidmode = VID_CheckVMode(x_disp, NULL, NULL);
	if (hasvidmode) {
		if (! XF86VidModeGetAllModeLines(x_disp, DefaultScreen(x_disp),
										&nummodes, &vidmodes)
			|| nummodes <= 0) { 
			hasvidmode = 0;
		}
	}
	Con_SafePrintf ("hasvidmode = %i\nnummodes = %i\n", hasvidmode, nummodes);

	if (hasvidmode && vid_fullscreen->value) {
		int smallest_mode=0, x=MAXINT, y=MAXINT;

		// FIXME: does this depend on mode line order in XF86Config?
		for (i=0; i<nummodes; i++) {
			if (x>vidmodes[i]->hdisplay || y>vidmodes[i]->vdisplay) {
				smallest_mode=i;
				x=vidmodes[i]->hdisplay;
				y=vidmodes[i]->vdisplay;
			}
			printf("%dx%d\n",vidmodes[i]->hdisplay,vidmodes[i]->vdisplay);
		}
		// chose the smallest mode that our window fits into;
		for (i=smallest_mode;
			 i!=(smallest_mode+1)%nummodes;
			 i=(i?i-1:nummodes-1)) {
			if (vidmodes[i]->hdisplay>=width
				&& vidmodes[i]->vdisplay>=height) {
				XF86VidModeSwitchToMode (x_disp, DefaultScreen (x_disp),
										 vidmodes[i]);
				break;
			}
		}
		XF86VidModeSetViewPort(x_disp, DefaultScreen (x_disp), 0, 0);
		_windowed_mouse = Cvar_Get ("_windowed_mouse","1",CVAR_ARCHIVE|CVAR_ROM,"None");
	} else
#endif
		_windowed_mouse = Cvar_Get ("_windowed_mouse","0",CVAR_ARCHIVE,"None");
}

void
x11_create_window(int width, int height)
{
	XSetWindowAttributes attr;
	unsigned long mask;

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(x_disp, x_root, x_vis, AllocNone);
	attr.event_mask = X_MASK;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	if (hasvidmode && vid_fullscreen->value) {
		attr.override_redirect=1;
		mask|=CWOverrideRedirect;
	}

	x_win = XCreateWindow(x_disp, x_root, 0, 0, width, height,
						0, x_visinfo->depth, InputOutput,
						x_vis, mask, &attr);
	/* Give it a title */
	XStoreName(x_disp, x_win, "XQuake");

	/* Make window respond to Delete events */
	aWMDelete = XInternAtom(x_disp, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(x_disp, x_win, &aWMDelete, 1);

	XMapWindow(x_disp, x_win);
	XRaiseWindow(x_disp, x_win);
}

void
x11_restore_vidmode(void)
{
	XSetScreenSaver (x_disp, xss_timeout, xss_interval, xss_blanking,
					xss_exposures);

#ifdef HAVE_VIDMODE
	if (hasvidmode) {
		XF86VidModeSwitchToMode (x_disp, DefaultScreen (x_disp),
								 vidmodes[0]);
		XFree(vidmodes);
	}
#endif
}

void
x11_grab_keyboard(void)
{
#ifdef HAVE_VIDMODE
	if (hasvidmode && vid_fullscreen->value) {
		XGrabKeyboard(x_disp, x_win, 1, GrabModeAsync, GrabModeAsync,
					  CurrentTime);
	}
#endif
}
