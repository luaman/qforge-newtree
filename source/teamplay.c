/*
	teamplay.c

	Teamplay enhancements ("proxy features")

	Copyright (C) 2000       Anton Gavrilov (tonik@quake.ru)

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
#include "config.h"
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <errno.h>

#include "bothdefs.h"
#include "console.h"
#include "cmd.h"
#include "client.h"
#include "locs.h"
#include "quakefs.h"
#include "sys.h"
#include "teamplay.h"

extern cvar_t *skin;
cvar_t     *cl_deadbodyfilter;
cvar_t     *cl_gibfilter;
cvar_t     *cl_parsesay;
cvar_t     *cl_nofake;
static qboolean died = false, recorded_location = false;
static vec3_t death_location, last_recorded_location;

void
Team_BestWeaponImpulse (void)
{
	int         best, i, imp, items;
	extern int  in_impulse;

	items = cl.stats[STAT_ITEMS];
	best = 0;

	for (i = Cmd_Argc () - 1; i > 0; i--) {
		imp = atoi (Cmd_Argv (i));
		if (imp < 1 || imp > 8)
			continue;

		switch (imp) {
			case 1:
				if (items & IT_AXE)
					best = 1;
				break;
			case 2:
				if (items & IT_SHOTGUN && cl.stats[STAT_SHELLS] >= 1)
					best = 2;
				break;
			case 3:
				if (items & IT_SUPER_SHOTGUN && cl.stats[STAT_SHELLS] >= 2)
					best = 3;
				break;
			case 4:
				if (items & IT_NAILGUN && cl.stats[STAT_NAILS] >= 1)
					best = 4;
				break;
			case 5:
				if (items & IT_SUPER_NAILGUN && cl.stats[STAT_NAILS] >= 2)
					best = 5;
				break;
			case 6:
				if (items & IT_GRENADE_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
					best = 6;
				break;
			case 7:
				if (items & IT_ROCKET_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
					best = 7;
				break;
			case 8:
				if (items & IT_LIGHTNING && cl.stats[STAT_CELLS] >= 1)
					best = 8;

		}
	}

	if (best)
		in_impulse = best;
}


char       *
Team_ParseSay (char *s)
{
	static char buf[1024];
	int         i, bracket;
	char        c, chr, *t1, t2[128], t3[128];
	static location_t *location = NULL;

	if (!cl_parsesay->int_val)
		return s;

	i = 0;

	while (*s && (i <= sizeof (buf))) {
		if ((*s == '$') && (s[1] != '\0')) {
			c = 0;
			switch (s[1]) {
				case '\\':
					c = 13;
					break;				// fake message
				case '[':
					c = 0x90;
					break;				// colored brackets
				case ']':
					c = 0x91;
					break;
				case 'G':
					c = 0x86;
					break;				// ocrana leds
				case 'R':
					c = 0x87;
					break;
				case 'Y':
					c = 0x88;
					break;
				case 'B':
					c = 0x89;
					break;
			}

			if (c) {
				buf[i++] = c;
				s += 2;
				continue;
			}
		} else if ((*s == '%') && (s[1] != '\0')) {
			t1 = NULL;
			memset (t2, '\0', sizeof (t2));
			memset (t3, '\0', sizeof (t3));

			if ((s[1] == '[') && (s[3] == ']')) {
				bracket = 1;
				chr = s[2];
				s += 4;
			} else {
				bracket = 0;
				chr = s[1];
				s += 2;
			}
			switch (chr) {
				case '%':
					t2[0] = '%';
					t2[1] = 0;
					t1 = t2;
					break;
				case 's':
					bracket = 0;
					t1 = skin->string;
					break;
				case 'd':
					bracket = 0;
					if (died) {
						location = locs_find (death_location);
						if (location) {
							recorded_location = true;
							VectorCopy (death_location, last_recorded_location);
							t1 = location->name;
							break;
						}
					}
					goto location;
				case 'r':
					bracket = 0;
					if (recorded_location) {
						location = locs_find (last_recorded_location);
						if (location) {
							t1 = location->name;
							break;
						}
					}
					goto location;
				case 'l':
				  location:
					bracket = 0;
					location = locs_find (cl.simorg);
					if (location) {
						recorded_location = true;
						VectorCopy (cl.simorg, last_recorded_location);
						t1 = location->name;
					} else
						snprintf (t2, sizeof (t2), "Unknown!\n");
					break;
				case 'a':
					if (bracket) {
						if (cl.stats[STAT_ARMOR] > 50)
							bracket = 0;

						if (cl.stats[STAT_ITEMS] & IT_ARMOR3)
							t3[0] = 'R' | 0x80;
						else if (cl.stats[STAT_ITEMS] & IT_ARMOR2)
							t3[0] = 'Y' | 0x80;
						else if (cl.stats[STAT_ITEMS] & IT_ARMOR1)
							t3[0] = 'G' | 0x80;
						else {
							t2[0] = 'N' | 0x80;
							t2[1] = 'O' | 0x80;
							t2[2] = 'N' | 0x80;
							t2[3] = 'E' | 0x80;
							t2[4] = '!' | 0x80;
						}

						snprintf (t2, sizeof (t2), "%sa:%i", t3,
								  cl.stats[STAT_ARMOR]);
					} else
						snprintf (t2, sizeof (t2), "%i", cl.stats[STAT_ARMOR]);
					break;
				case 'A':
					bracket = 0;
					if (cl.stats[STAT_ITEMS] & IT_ARMOR3)
						t2[0] = 'R' | 0x80;
					else if (cl.stats[STAT_ITEMS] & IT_ARMOR2)
						t2[0] = 'Y' | 0x80;
					else if (cl.stats[STAT_ITEMS] & IT_ARMOR1)
						t2[0] = 'G' | 0x80;
					else {
						t2[0] = 'N' | 0x80;
						t2[1] = 'O' | 0x80;
						t2[2] = 'N' | 0x80;
						t2[3] = 'E' | 0x80;
						t2[4] = '!' | 0x80;
					}
					break;
				case 'h':
					if (bracket) {
						if (cl.stats[STAT_HEALTH] > 50)
							bracket = 0;
						snprintf (t2, sizeof (t2), "h:%i",
								  cl.stats[STAT_HEALTH]);
					} else
						snprintf (t2, sizeof (t2), "%i", cl.stats[STAT_HEALTH]);
					break;
				default:
					bracket = 0;
			}

			if (!t1) {
				if (!t2[0]) {
					t2[0] = '%';
					t2[1] = chr;
				}

				t1 = t2;
			}

			if (bracket)
				buf[i++] = 0x90;		// '['

			if (t1) {
				int         len;

				len = strlen (t1);
				if (i + len >= sizeof (buf))
					continue;			// No more space in buffer, icky.
				strncpy (buf + i, t1, len);
				i += len;
			}

			if (bracket)
				buf[i++] = 0x91;		// ']'

			continue;
		}
		buf[i++] = *s++;
	}
	buf[i] = 0;

	return buf;
}

void
Team_Dead ()
{
	died = true;
	VectorCopy (cl.simorg, death_location);
}

void
Team_NewMap ()
{
	char       *mapname, *t1, *t2;

	died = false;
	recorded_location = false;

	mapname = strdup (cl.worldmodel->name);
	if (!mapname)
		Sys_Error ("Can't duplicate mapname!");
	t1 = strrchr (mapname, '/');
	t2 = strrchr (mapname, '.');
	if (!t1 || !t2)
		Sys_Error ("Can't find / or .!");
	t1++;								// skip over /
	t2[0] = '\0';

	locs_reset ();
	locs_load (t1);
	free (mapname);
}

void
Team_Init_Cvars (void)
{
	cl_deadbodyfilter =
		Cvar_Get ("cl_deadbodyfilter", "0", CVAR_NONE,
				  "Hide dead player models");
	cl_gibfilter = Cvar_Get ("cl_gibfilter", "0", CVAR_NONE, "Hide gibs");
	cl_parsesay = Cvar_Get ("cl_parsesay", "0", CVAR_NONE, "None");
	cl_nofake = Cvar_Get ("cl_nofake", "0", CVAR_NONE, "Unhide fake messages");
}

/*
 * locs_markloc
 *
 * Record the current co-ords plus description into a loc file for current map
 */

void
locs_markloc ()
{
	vec3_t      loc;
	char       *mapname, *t1;
	QFile      *locfd;
	char        locfile[MAX_OSPATH];

	if (Cmd_Argc () != 2) {
		Con_Printf
			("markloc <description> :marks the current location with the description and records the information into a loc file.\n");
		return;
	}
	VectorCopy (cl.simorg, loc);
	locs_add (loc, Cmd_Argv (1));
#ifdef HAVE_ZLIB
	if(locisgz) {
		Cmd_ExecuteString ("zdumploc");
		return;
	}
#endif
	loc[0] *= 8;
	loc[1] *= 8;
	loc[2] *= 8;
	mapname = strdup (cl.worldmodel->name);
	if (!mapname)
		Sys_Error ("Can't duplicate mapname!");
	t1 = strrchr (mapname, '.');
	if (!t1)
		Sys_Error ("Can't find .!");
	t1++;						// skip over .
	t1[0] = 'l';
	t1[1] = 'o';
	t1[2] = 'c';
	snprintf (locfile, sizeof (locfile), "%s/%s", com_gamedir, mapname);
	locfd = Qopen (locfile, "a+");
	if (locfd == 0) {
		locfd = Qopen (locfile, "w+");
		if (locfd == 0) {
			Con_Printf ("ERROR: Unable to open %s : %s\n", mapname,
						strerror (errno));
			free (mapname);
			return;
		}
	}
	Qprintf (locfd, "%.0f %.0f %.0f %s\n", loc[0], loc[1], loc[2],
			 Cmd_Argv (1));
	Qclose (locfd);
	Con_Printf("Marked Current Location: %s\n",Cmd_Argv(1));
	free (mapname);
}

/*
 * locs_dumploc
 *
 * copies the entire loc data from memory to disk
 * supports zgip files via zdumploc
 */
void 
locs_dumploc ()
{
	char       *mapname, *t1;
	QFile      *locfd = 0;
	char        locfile[MAX_OSPATH];
	int         i;
	if (Cmd_Argc () != 1) {
		Con_Printf
			("markloc <description> :marks the current location with the description and records the information into a loc file.\n");
		return;
	}
	mapname = strdup (cl.worldmodel->name);
	if (!mapname)
		Sys_Error ("Can't duplicate mapname!");
	t1 = strrchr (mapname, '.');
	if (!t1)
		Sys_Error ("Can't find .!");
	t1++;                                                  // skip over .
	t1[0] = 'l';
	t1[1] = 'o';
	t1[2] = 'c';
	if (strncasecmp(Cmd_Argv(0),"dumploc",7) == 0) {
		snprintf (locfile, sizeof (locfile), "%s/%s", com_gamedir, mapname);
		locfd = Qopen (locfile, "w+");
#ifdef HAVE_ZLIB
	} else {
		snprintf (locfile, sizeof (locfile), "%s/%s.gz", com_gamedir, mapname);
		locfd = Qopen (locfile, "z9w+");
#endif
	}

	if (locfd == 0) {
		Con_Printf ("ERROR: Unable to open %s : %s\n", mapname,
			strerror (errno));
		free (mapname);
		return;
	}
	for(i=0; i < locations_count ;i++)
		Qprintf (locfd, "%.0f %.0f %.0f %s\n", 
				locations[i]->loc[0] * 8,
				locations[i]->loc[1] * 8,
				locations[i]->loc[2] * 8,
				locations[i]->name);
	Qclose (locfd);
	free(mapname);
}

/*
 * locs_delloc
 *
 * removes a loc mark from memory and file
 */
void
locs_delloc ()
{
	vec3_t      loc;
	location_t *best = NULL, *cur;
	float       best_distance = 9999999, distance;
	int         i, j=0;
	
	if (locations_count) {
		if ((strncasecmp(Cmd_Argv(0),"editloc",7) == 0) && (Cmd_Argc () != 2)) {
			Con_Printf("editloc <description> :changed the description of the nearest location marker\n");
			return;
		}
		if ((strncasecmp(Cmd_Argv(0),"delloc",6) == 0) && (Cmd_Argc () != 1)) {
			Con_Printf("delloc :removes the nearest location marker\n");
			return;
		}				
		VectorCopy (cl.simorg, loc);
		for (i = 0; i < locations_count; i++) {
			cur = locations[i];
			distance = VectorDistance_fast (loc, cur->loc);
			if ((distance < best_distance) || !best) {
				best = cur;
				best_distance = distance;
				j = i;
			}
		}
		if (strncasecmp(Cmd_Argv(0),"delloc",6) == 0) {
			Con_Printf("Removing Location Marker for %s\n",
					locations[j]->name);
			free ((void *) locations[j]->name);
        		free ((void *) locations[j]);
			locations_count--;
			for (i = j; i < locations_count; i++)
				locations[i] = locations[i+1];			
			locations[locations_count] = NULL;
		} else {
			Con_Printf("Changing Location Marker from %s to %s\n",
					locations[j]->name,
					Cmd_Argv(1));
			free ((void *) locations[j]->name);
			locations[j]->name = strdup (Cmd_Argv(1));
		}
#ifdef HAVE_ZLIB
		if (locisgz)
			Cmd_ExecuteString ("zdumploc");
		else
#endif
			Cmd_ExecuteString ("dumploc");
	}
}

void
Locs_Init ()
{
	Cmd_AddCommand ("markloc", locs_markloc);
	Cmd_AddCommand ("dumploc", locs_dumploc);
#ifdef HAVE_ZLIB
	Cmd_AddCommand ("zdumploc", locs_dumploc);
#endif
	Cmd_AddCommand ("delloc",locs_delloc);
	Cmd_AddCommand ("editloc",locs_delloc);
}
