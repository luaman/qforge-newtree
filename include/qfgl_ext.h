/*
	qfgl_ext.h

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

#ifndef __qfgl_ext_h_
#define __qfgl_ext_h_

#ifdef HAVE_CONFIG_H
# include "config.h"
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

#include "qtypes.h"

/* Standard OpenGL external function defs */
typedef void (GLAPIENTRY *QF_glBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (GLAPIENTRY *QF_glBlendEquation) (GLenum mode);
typedef void (GLAPIENTRY *QF_glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
typedef void (GLAPIENTRY *QF_glColorTable) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY *QF_glColorTableParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY *QF_glColorTableParameteriv) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY *QF_glCopyColorTable) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY *QF_glGetColorTable) (GLenum target, GLenum format, GLenum type, GLvoid *table);
typedef void (GLAPIENTRY *QF_glGetColorTableParameterfv) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY *QF_glGetColorTableParameteriv) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *QF_glColorSubTable) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
typedef void (GLAPIENTRY *QF_glCopyColorSubTable) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY *QF_glConvolutionFilter1D) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY *QF_glConvolutionFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
typedef void (GLAPIENTRY *QF_glConvolutionParameterf) (GLenum target, GLenum pname, GLfloat params);
typedef void (GLAPIENTRY *QF_glConvolutionParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
typedef void (GLAPIENTRY *QF_glConvolutionParameteri) (GLenum target, GLenum pname, GLint params);
typedef void (GLAPIENTRY *QF_glConvolutionParameteriv) (GLenum target, GLenum pname, const GLint *params);
typedef void (GLAPIENTRY *QF_glCopyConvolutionFilter1D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
typedef void (GLAPIENTRY *QF_glCopyConvolutionFilter2D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (GLAPIENTRY *QF_glGetConvolutionFilter) (GLenum target, GLenum format, GLenum type, GLvoid *image);
typedef void (GLAPIENTRY *QF_glGetConvolutionParameterfv) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY *QF_glGetConvolutionParameteriv) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *QF_glGetSeparableFilter) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
typedef void (GLAPIENTRY *QF_glSeparableFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
typedef void (GLAPIENTRY *QF_glGetHistogram) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (GLAPIENTRY *QF_glGetHistogramParameterfv) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY *QF_glGetHistogramParameteriv) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *QF_glGetMinmax) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
typedef void (GLAPIENTRY *QF_glGetMinmaxParameterfv) (GLenum target, GLenum pname, GLfloat *params);
typedef void (GLAPIENTRY *QF_glGetMinmaxParameteriv) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *QF_glHistogram) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY *QF_glMinmax) (GLenum target, GLenum internalformat, GLboolean sink);
typedef void (GLAPIENTRY *QF_glResetHistogram) (GLenum target);
typedef void (GLAPIENTRY *QF_glResetMinmax) (GLenum target);
typedef void (GLAPIENTRY *QF_glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *QF_glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (GLAPIENTRY *QF_glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

// GL_EXT_paletted_texture
typedef void (GLAPIENTRY *QF_glColorTableEXT) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
typedef void (GLAPIENTRY *QF_glGetColorTableEXT) (GLenum target, GLenum format, GLenum type, GLvoid *data);
typedef void (GLAPIENTRY *QF_glGetColorTableParameterivEXT) (GLenum target, GLenum pname, GLint *params);
typedef void (GLAPIENTRY *QF_glGetColorTableParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);

// 3Dfx
typedef void (GLAPIENTRY *QF_gl3DfxSetPaletteEXT) (GLuint *pal);

// GLX 1.3
typedef void *(GLAPIENTRY *QF_glXGetProcAddressARB) (const GLubyte *procName);

// WGL (Windows GL)
typedef const GLubyte *(GLAPIENTRY *QF_wglGetExtensionsStringEXT) (void);

/* QuakeForge extension functions */
qboolean QFGL_ExtensionPresent (const char *);
void *QFGL_ExtensionAddress (const char *);

#endif	// __qfgl_ext_h_
