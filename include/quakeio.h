/*
	quakeio.h

	(description)

	Copyright (C) 1996-1997  Id Software, Inc.
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
#ifndef _QUAKEIO_H
#define _QUAKEIO_H

#include "gcc_attr.h"
#include <stdio.h>

#include <config.h>

void Qexpand_squiggle(const char *path, char *dest);
int Qrename(const char *old, const char *new);
FILE *Qopen(const char *path, const char *mode);
FILE *Qdopen(int fd, const char *mode);
void Qclose(FILE *file);
int Qread(FILE *file, void *buf, int count);
int Qwrite(FILE *file, void *buf, int count);
int Qprintf(FILE *file, const char *fmt, ...) __attribute__((format(printf,2,3)));
char *Qgets(FILE *file, char *buf, int count);
int Qgetc(FILE *file);
int Qputc(FILE *file, int c);
int Qseek(FILE *file, long offset, int whence);
long Qtell(FILE *file);
int Qflush(FILE *file);
int Qeof(FILE *file);


#endif /*_QUAKEIO_H*/
