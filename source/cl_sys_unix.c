/*
	cl_sys_unix.c

	(description)

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
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "sys.h"
#include "quakedef.h"

int noconinput = 0;

#define BASEDIR	"."

/* cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display
 CVAR_FIXME */
cvar_t  *sys_linerefresh;// set for entity display


// =======================================================================
// General routines
// =======================================================================

/*
void Sys_Printf (char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];
	
	va_start (argptr,fmt);
#ifdef HAVE_VSNPRINTF
        vsnprintf (text,sizeof(text),fmt,argptr);
#else
        vsprintf (text,fmt,argptr);
#endif
	va_end (argptr);
	fprintf(stderr, "%s", text);
	
	Con_Print (text);
}

void Sys_Printf (char *fmt, ...)
{

    va_list     argptr;
    char        text[1024], *t_p;
    int         l, r;

    if (nostdout)
        return;

    va_start (argptr,fmt);
#ifdef HAVE_VSNPRINTF
    vsnprintf (text,sizeof(text),fmt,argptr);
#else
    vsprintf (text,fmt,argptr);
#endif
    va_end (argptr);

    l = strlen(text);
    t_p = text;

// make sure everything goes through, even though we are non-blocking
    while (l)
    {
        r = write (1, text, l);
        if (r != l)
            sleep (0);
        if (r > 0)
        {
            t_p += r;
            l -= r;
        }
    }

}
*/

void Sys_Quit (void)
{
	Host_Shutdown();
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
	exit(0);
}

void Sys_Init(void)
{
	sys_nostdout = Cvar_Get("sys_nostdout", "0", CVAR_NONE, "None");
#ifdef USE_INTEL_ASM
	Sys_SetFPCW();
#endif
}

void Sys_Error (char *error, ...)
{ 
    va_list     argptr;
    char        string[1024];

// change stdin to non blocking
    fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    
    va_start (argptr, error);
    vsnprintf (string, sizeof(string), error, argptr);
    va_end (argptr);
	fprintf(stderr, "Error: %s\n", string);

	Host_Shutdown ();
	exit (1);

} 

void Sys_Warn (char *warning, ...)
{ 
    va_list     argptr;
    char        string[1024];
    
    va_start (argptr, warning);
    vsnprintf (string, sizeof(string), warning,argptr);
    va_end (argptr);
	fprintf(stderr, "Warning: %s", string);
} 

void Sys_DebugLog(char *file, char *fmt, ...)
{
    va_list argptr; 
    static char data[1024];   // why static ?
    int fd;

    va_start (argptr, fmt);
    vsnprintf (data, sizeof(data), fmt, argptr);
    va_end(argptr);
//    fd = open(file, O_WRONLY | O_BINARY | O_CREAT | O_APPEND, 0666);
    fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0666);
    write(fd, data, strlen(data));
    close(fd);
}

void floating_point_exception_handler(int whatever)
{
//	Sys_Warn("floating point exception\n");
	signal(SIGFPE, floating_point_exception_handler);
}

char *Sys_ConsoleInput(void)
{
#if 0
    static char text[256];
    int     len;

	if (cls.state == ca_dedicated) {
		len = read (0, text, sizeof(text));
		if (len < 1)
			return NULL;
		text[len-1] = 0;    // rip off the /n and terminate

		return text;
	}
#endif
	return NULL;
}

#ifndef USE_INTEL_ASM
void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}
#endif

int		skipframes;

int main (int c, char **v)
{

	double		time, oldtime, newtime;
	quakeparms_t parms;
	int j;

//	static char cwd[1024];

//	signal(SIGFPE, floating_point_exception_handler);
	signal(SIGFPE, SIG_IGN);

	memset(&parms, 0, sizeof(parms));

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = 16*1024*1024;

	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
	parms.membase = malloc (parms.memsize);
	if (!parms.membase) {
		printf("Can't allocate memroy for zone.\n");
		return 1;
	}

	parms.basedir = BASEDIR;

// caching is disabled by default, use -cachedir to enable
//	parms.cachedir = cachedir;

	noconinput = COM_CheckParm("-noconinput");
	if (!noconinput)
		fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
	if (COM_CheckParm("-nostdout")) Cvar_Set(sys_nostdout, "1");

    Host_Init(&parms);

    oldtime = Sys_DoubleTime ();
    while (1)
    {
// find time spent rendering last frame
        newtime = Sys_DoubleTime ();
        time = newtime - oldtime;

		Host_Frame(time);
		oldtime = newtime;
    }

}


/*
================
Sys_MakeCodeWriteable
================
*/
void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{

	int r;
	unsigned long addr;
	int psize = getpagesize();

	addr = (startaddr & ~(psize-1)) - psize;

//	fprintf(stderr, "writable code %lx(%lx)-%lx, length=%lx\n", startaddr,
//			addr, startaddr+length, length);

	r = mprotect((char*)addr, length + startaddr - addr + psize, 7);

	if (r < 0)
    		Sys_Error("Protection change failed\n");

}
