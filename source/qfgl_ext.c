/*
	qfgl_ext.c

	QuakeForge OpenGL extension interface definitions

	Copyright (C) 2000 Jeff Teunissen <deek@dusknet.dhs.org>

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

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif
#ifndef RTLD_LAZY
# ifdef DL_LAZY
#  define RTLD_LAZY     DL_LAZY
# else
#  define RTLD_LAZY     0
# endif
#endif

#include <GL/gl.h>

#ifdef HAVE_GL_GLX_H
# include <GL/glx.h>
#endif
#ifdef HAVE_GL_GLEXT_H
# include <GL/glext.h>
#endif
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif

#include <string.h>

#include "qfgl_ext.h"
#include "qtypes.h"

/*
	ParseExtensionList

	It takes a bit of care to be fool-proof about parsing an OpenGL extensions
	string. Don't be fooled by sub-strings, etc.
*/
qboolean
QFGL_ParseExtensionList (const GLubyte * list, const char *name)
{
	const char *start;
	char       *where, *terminator;

	// Extension names must not have spaces.
	where = (GLubyte *) strchr (name, ' ');
	if (where || *name == '\0')
		return 0;

	start = list;
	for (;;) {
		where = strstr (start, name);
		if (!where)
			break;
		terminator = where + strlen (name);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return 1;
		start = terminator;
	}
	return 0;
}

qboolean
QFGL_ExtensionPresent (const char *name)
{
	static const GLubyte *gl_extensions = NULL;

#ifdef _WIN32
	static const GLubyte *wgl_extensions = NULL;

	static qboolean extensionFunc_present = true;
	static QF_wglGetExtensionsStringEXT extensionFunc = NULL;
#endif

	if (!gl_extensions) {				// get and save GL extension list
		gl_extensions = glGetString (GL_EXTENSIONS);
	}
#ifdef _WIN32
	if (!wgl_extensions) {				// get and save WGL extension list
		if (extensionFunc_present && !extensionFunc) {
			if (!
				(QFGL_ParseExtensionList
				 (gl_extensions, "WGL_EXT_extensions_string"))) {
				extensionFunc_present = false;
			} else {
				extensionFunc =
					QFGL_ExtensionAddress ("WGL_EXT_extension_string");
			}
		}

		if (extensionFunc_present)
			wgl_extensions = extensionFunc ();
		else
			wgl_extensions = "";
	}

	if (QFGL_ParseExtensionList (gl_extensions, name))
		return true;
	if (QFGL_ParseExtensionList (wgl_extensions, name))
		return true;

	return false;
#else
	return QFGL_ParseExtensionList (gl_extensions, name);
#endif
}


void       *
QFGL_ExtensionAddress (const char *name)
{
#if defined(HAVE_GLX) && defined(HAVE_DLOPEN)
	void       *dlhand = NULL;

	static qboolean glProcAddress_present = true;
	static QF_glXGetProcAddressARB qfglXGetProcAddress = NULL;

	if (glProcAddress_present && !qfglXGetProcAddress) {
		if (QFGL_ExtensionPresent ("GLX_ARB_get_proc_address")) {
			if ((dlhand = dlopen (NULL, RTLD_LAZY))) {
				qfglXGetProcAddress = dlsym (dlhand, "glXGetProcAddressARB");
				dlclose (dlhand);
			} else {
				glProcAddress_present = false;
			}
		} else {
			glProcAddress_present = false;
		}
	}
#endif

	if (name) {
#if defined(HAVE_GLX) && defined(HAVE_DLOPEN)
		if (glProcAddress_present) {
			return qfglXGetProcAddress ((const GLubyte *) name);
		} else {
			if ((dlhand = dlopen (NULL, RTLD_LAZY))) {
				void       *handle;

				handle = dlsym (dlhand, name);
				dlclose (dlhand);
				return handle;
			}
			return NULL;
		}
#else
# ifdef _WIN32
		return wglGetProcAddress (name);
# else
		return NULL;
# endif
#endif
	}
	return NULL;
}
