/*
	cl_sys_sdl.c

	(description)

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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <io.h>
#include <conio.h>

#ifndef _WIN32
# include <unistd.h>
# include <stdarg.h>
# include <string.h>
# include <ctype.h>
# include <fcntl.h>
# include <signal.h>
# include <limits.h>
# include <sys/types.h>
# include <sys/mman.h>
#endif

#include <SDL.H>
#include <SDL_main.H>

#include "sys.h"
#include "qargs.h"
#include "qargs.h"
#include "client.h"
#include "host.h"

qboolean    is_server = false;
char       *svs_info;

int         starttime;

#ifdef _WIN32
# include "winquake.h"
						// fixme: minimized is not currently supported under
						// SDL
qboolean    Minimized = false;
void        MaskExceptions (void);
#endif

void
Sys_DebugLog (char *file, char *fmt, ...)
{
	int         fd;
	static char data[1024];				// why static ?
	va_list     argptr;

	va_start (argptr, fmt);
	vsnprintf (data, sizeof (data), fmt, argptr);
	va_end (argptr);
	fd = open (file, O_WRONLY | O_CREAT | O_APPEND, 0666);
	write (fd, data, strlen (data));
	close (fd);
};

/*
	FILE IO
*/

int
Sys_FileTime (char *path)
{
	QFile      *f;
	int         t, retval;

	f = Qopen (path, "rb");

	if (f) {
		Qclose (f);
		retval = 1;
	} else {
		retval = -1;
	}

	return retval;
}


/*
	SYSTEM IO
*/

/*
	Sys_MakeCodeWriteable
*/
void
Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{

#ifdef _WIN32
	DWORD       flOldProtect;

	// copy on write or just read-write?
	if (!VirtualProtect
		((LPVOID) startaddr, length, PAGE_READWRITE,
		 &flOldProtect)) Sys_Error ("Protection change failed\n");
#else
	int         r;
	unsigned long addr;
	int         psize = getpagesize ();

	addr = (startaddr & ~(psize - 1)) - psize;

//  fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//          addr, startaddr+length, length);

	r = mprotect ((char *) addr, length + startaddr - addr + psize, 7);

	if (r < 0)
		Sys_Error ("Protection change failed\n");
#endif
}


/*
	Sys_Init
*/
void
Sys_Init (void)
{

#ifdef WIN32
	OSVERSIONINFO vinfo;
#endif

#ifdef USE_INTEL_ASM
#ifdef _WIN32
	MaskExceptions ();
#endif
	Sys_SetFPCW ();
#endif

#ifdef _WIN32
	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod (1);

	vinfo.dwOSVersionInfoSize = sizeof (vinfo);

	if (!GetVersionEx (&vinfo))
		Sys_Error ("Couldn't get OS info");

	if ((vinfo.dwMajorVersion < 4)
		|| (vinfo.dwPlatformId == VER_PLATFORM_WIN32s)) {
		Sys_Error ("This version of " PROGRAM
				   " requires at least Win95 or NT 4.0");
	}
#endif
}


void
Sys_Error (char *error, ...)
{
	va_list     argptr;
	char        text[1024];

#ifndef _WIN32
	fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~O_NONBLOCK);
#endif
	va_start (argptr, error);
	vsnprintf (text, sizeof (text), error, argptr);
	va_end (argptr);

#ifdef WIN32
	MessageBox (NULL, text, "Error", 0 /* MB_OK */ );
#endif
	fprintf (stderr, "Error: %s\n", text);

	Host_Shutdown ();
	exit (1);
}

void
Sys_Quit (void)
{
	Host_Shutdown ();
	exit (0);
}

char       *
Sys_ConsoleInput (void)
{
	return NULL;
}

void
Sys_Sleep (void)
{
}


void
Sys_Init_Cvars (void)
{
	sys_nostdout = Cvar_Get ("sys_nostdout", "0", CVAR_NONE, "Set to disable std out");
	if (COM_CheckParm ("-nostdout"))
		Cvar_Set (sys_nostdout, "1");
}

C_LINKAGE int
SDL_main (int c, char **v)
{

	double      time, oldtime, newtime;
	int         j;

	static char cwd[1024];
	int         t;

#ifndef WIN32
	signal (SIGFPE, SIG_IGN);
#endif

	memset (&host_parms, 0, sizeof (host_parms));

	COM_InitArgv (c, v);
	host_parms.argc = com_argc;
	host_parms.argv = com_argv;

	host_parms.memsize = 8 * 1024 * 1024;

	j = COM_CheckParm ("-mem");
	if (j)
		host_parms.memsize = (int) (atof (com_argv[j + 1]) * 1024 * 1024);
	host_parms.membase = malloc (host_parms.memsize);

	if (!host_parms.membase) {
		printf ("Can't allocate memory for zone.\n");
		return 1;
	}
#ifndef WIN32
	noconinput = COM_CheckParm ("-noconinput");
	if (!noconinput)
		fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) | O_NONBLOCK);
#endif

	Host_Init ();

	oldtime = Sys_DoubleTime ();
	while (1) {
// find time spent rendering last frame
		newtime = Sys_DoubleTime ();
		time = newtime - oldtime;

		Host_Frame (time);
		oldtime = newtime;
	}

}

/* fixme: evil stub for directsound crap */
IN_Accumulate (void)
{
}
