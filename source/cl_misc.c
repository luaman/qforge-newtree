#include "sys.h"
#include "quakedef.h"

qboolean ServerPaused (void)
{
	return false;
}

void SV_Error (char *error, ...)
{
	va_list		argptr;
	static	char		string[1024];

	va_start (argptr,error);
	vsprintf (string,error,argptr);
	va_end (argptr);

	Sys_Error ("%s\n",string);
}
