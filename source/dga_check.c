/*
	dga_check.c

	Routines to check for XFree86 DGA and VidMode extensions

	Copyright (C) 2000 Marcus Sundberg [mackan@stacken.kth.se]
	Copyright (C) 2000  contributors of the QuakeForge project
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <X11/Xlib.h>

#ifdef HAVE_DGA
# include <X11/extensions/xf86dga.h>
#endif
#ifdef HAVE_VIDMODE
# include <X11/extensions/xf86vmode.h>
# ifdef X_XF86VidModeSetClientVersion
#  define VIDMODE_VERSION 2
# else
#  define VIDMODE_VERSION 0
# endif
#endif

#include "dga_check.h"


/*
  VID_CheckDGA

  Check for the presence of the XFree86-DGA X server extension
*/
qboolean
VID_CheckDGA (Display * dpy, int *maj_ver, int *min_ver, int *hasvideo)
{
#ifdef HAVE_DGA
	int 	event_base, error_base, dgafeat;
	int 	dummy, dummy_major, dummy_minor, dummy_video;

	if (!XQueryExtension (dpy, "XFree86-DGA", &dummy, &dummy, &dummy)) {
		return false;
	}

	if (!XF86DGAQueryExtension (dpy, &event_base, &error_base)) {
		return false;
	}

	if (!maj_ver)
		maj_ver = &dummy_major;
	if (!min_ver)
		min_ver = &dummy_minor;

	if (!XF86DGAQueryVersion (dpy, maj_ver, min_ver)) {
		return false;
	}

	if (!hasvideo)
		hasvideo = &dummy_video;

	if (!XF86DGAQueryDirectVideo (dpy, DefaultScreen (dpy), &dgafeat)) {
		*hasvideo = 0;
	} else {
		*hasvideo = (dgafeat & XF86DGADirectGraphics);
	}

	if (!(dgafeat & (XF86DGADirectPresent | XF86DGADirectMouse))) {
		return false;
	}

	return true;
#else
	return false;
#endif // HAVE_DGA
}


/*
  VID_CheckVMode

  Check for the presence of the XFree86-VidMode X server extension
*/
qboolean
VID_CheckVMode (Display * dpy, int *maj_ver, int *min_ver)
{
#ifdef HAVE_VIDMODE
	int 	event_base, error_base;
	int 	dummy, dummy_major, dummy_minor;

	if (!XQueryExtension (dpy, "XFree86-VidModeExtension", &dummy, &dummy, &dummy)) {
		return false;
	}

	if (!XF86VidModeQueryExtension (dpy, &event_base, &error_base)) {
		return false;
	}

	if (!maj_ver)
		maj_ver = &dummy_major;
	if (!min_ver)
		min_ver = &dummy_minor;

	if (!XF86VidModeQueryVersion (dpy, maj_ver, min_ver))
		return false;

	if (maj_ver && min_ver)
		printf("VidMode version:%d.%d\n", *maj_ver, *min_ver);
	if ((!maj_ver) || (*maj_ver != VIDMODE_VERSION))
		return false;

	return true;
#else
	return false;
#endif // HAVE_VIDMODE
}
