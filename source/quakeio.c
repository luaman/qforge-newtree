/*
	quakeio.c

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef HAVE_MALLOC_H
# include <malloc.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <quakeio.h>
#include <string.h>
#ifdef WIN32
# include <io.h>
# include <fcntl.h>
#else
# include <pwd.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef _MSC_VER
# define _POSIX_
#endif
#include <limits.h>

void
Qexpand_squiggle(const char *path, char *dest)
{
	char *home;
#ifndef _WIN32
	struct passwd *pwd_ent;
#endif

	if (strncmp (path, "~/",2) != 0) {
		strcpy (dest,path);
		return;
	}

#ifndef _WIN32
	if ((pwd_ent = getpwuid (getuid()))) {
		home = pwd_ent->pw_dir;
	} else
#endif
		home = getenv("HOME");

	if (home) {
		strcpy (dest, home);
		strcat (dest, path+1); // skip leading ~
	} else
		strcpy (dest,path);
}

int
Qrename(const char *old, const char *new)
{
	char e_old[PATH_MAX];
	char e_new[PATH_MAX];

	Qexpand_squiggle (old, e_old);
	Qexpand_squiggle (new, e_new);
	return rename (e_old, e_new);
}

FILE *
Qopen(const char *path, const char *mode)
{
	FILE *file;
	char e_path[PATH_MAX];

	Qexpand_squiggle (path, e_path);
	path = e_path;

	file=fopen(path,mode);
	return file;
}

FILE *
Qdopen(int fd, const char *mode)
{
	FILE *file;

	file=fdopen(fd,mode);
#ifdef WIN32
# ifdef __BORLANDC__
	setmode(_fileno(file),O_BINARY);
# else
	_setmode(_fileno(file),_O_BINARY);
# endif
#endif
	return file;
}

void
Qclose(FILE *file)
{
	fclose(file);
}

int
Qread(FILE *file, void *buf, int count)
{
	return fread(buf, 1, count, file);
}

int
Qwrite(FILE *file, void *buf, int count)
{
	return fwrite(buf, 1, count, file);
}

int
Qprintf(FILE *file, const char *fmt, ...)
{
	va_list args;
	int ret=-1;

	va_start(args,fmt);
	ret=vfprintf(file, fmt, args);
	va_end(args);
	return ret;
}

char *
Qgets(FILE *file, char *buf, int count)
{
	return fgets(buf, count, file);
}

int
Qgetc(FILE *file)
{
	return fgetc(file);
}

int
Qputc(FILE *file, int c)
{
	return fputc(c, file);
}

int
Qseek(FILE *file, long offset, int whence)
{
	return fseek(file, offset, whence);
}

long
Qtell(FILE *file)
{
	return ftell(file);
}

int
Qflush(FILE *file)
{
	return fflush(file);
}

int
Qeof(FILE *file)
{
	return feof(file);
}
