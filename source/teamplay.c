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

#include <string.h>

#include "bothdefs.h"
#include "cmd.h"
#include "client.h"
#include "teamplay.h"
#include "locs.h"
#include "sys.h"

extern cvar_t	*skin;
cvar_t	*cl_deadbodyfilter;
cvar_t	*cl_gibfilter;
cvar_t	*cl_parsesay;
cvar_t	*cl_nofake;
static qboolean	died = false, recorded_location = false;
static vec3_t	death_location, last_recorded_location;


void Team_BestWeaponImpulse (void)
{
	int			best, i, imp, items;
	extern int	in_impulse;

	items = cl.stats[STAT_ITEMS];
	best = 0;

	for (i = Cmd_Argc() - 1; i > 0; i--)
	{
		imp = atoi(Cmd_Argv(i));
		if (imp < 1 || imp > 8)
			continue;

		switch (imp)
		{
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


char *Team_ParseSay (char *s)
{
	static char	buf[1024];
	int		i, bracket;
	char	c, chr, *t1, t2[128], t3[128];
	static location_t	*location = NULL;

	if (!cl_parsesay->value)
		return s;

	i = 0;

	while (*s && (i <= sizeof(buf))) {
		if (*s == '$') {
			c = 0;
			switch (s[1]) {
				case '\\': c = 13; break;	// fake message
				case '[': c = 0x90; break;	// colored brackets
				case ']': c = 0x91; break;
				case 'G': c = 0x86; break;	// ocrana leds
				case 'R': c = 0x87; break;
				case 'Y': c = 0x88; break;
				case 'B': c = 0x89; break;
			}
			
			if (c) {
				buf[i++] = c;
				s += 2;
				continue;
			}
		} else if (*s == '%') {
			t1 = NULL;
			memset(t2, '\0', sizeof(t2));
			memset(t3, '\0', sizeof(t3));

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
				case 's':
					bracket = 0;
					t1 = skin->string;
					break;
				case 'd':
					bracket = 0;
					if (died) {
						location = locs_find(death_location);
						if (location) {
							recorded_location = true;
							memcpy(last_recorded_location, death_location, 
									sizeof(last_recorded_location));
							t1 = location->name;
							break;
						}
					}
					goto location;
				case 'r':
					bracket = 0;
					if (recorded_location) {
						location = locs_find(last_recorded_location);
						if (location) {
							t1 = location->name;
							break;
						}
					}
					goto location;
				case 'l':
location:
					bracket = 0;
					location = locs_find(r_origin);
					if (location) {
						recorded_location = true;
						memcpy(last_recorded_location, r_origin, 
								sizeof(last_recorded_location));
						t1 = location->name;
					} else
						snprintf(t2, sizeof(t2), "Unknown!\n");
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

						snprintf(t2, sizeof(t2), "%sa:%i", t3, cl.stats[STAT_ARMOR]);
					} else
						snprintf(t2, sizeof(t2), "%i", cl.stats[STAT_ARMOR]);
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
						snprintf(t2, sizeof(t2), "h:%i", cl.stats[STAT_HEALTH]);
					} else
						snprintf(t2, sizeof(t2), "%i", cl.stats[STAT_HEALTH]);
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
				buf[i++] = 0x90; // '['

			if (t1) {
				int len;
				len = strlen(t1);
				if (i + len >= sizeof(buf))
					continue;	// No more space in buffer, icky.
				strncpy(buf + i, t1, len);
				i += len;
			}

			if (bracket)
				buf[i++] = 0x91; // ']'

			continue;
		}
		buf[i++] = *s++;
	}
	buf[i] = 0;

	return	buf;
}

void Team_Dead ()
{
	died = true;
	memcpy(death_location, r_origin, sizeof(death_location));
}

void Team_NewMap ()
{
	char *mapname, *t1, *t2;

	died = false;
	recorded_location = false;

	mapname = strdup(cl.worldmodel->name);
	if (!mapname)
		Sys_Error("Can't duplicate mapname!");
	t1 = strrchr(mapname, '/');
	t2 = strrchr(mapname, '.');
	if (!t1 || !t2)
		Sys_Error("Can't find / or .!");
	t2[0] = '\0';

	locs_reset();
	locs_load(t1);
	free(mapname);
}

void Team_InitTeamplay (void)
{
	cl_deadbodyfilter = Cvar_Get("cl_deadbodyfilter", "0", CVAR_NONE, "Hide dead player models");
	cl_gibfilter = Cvar_Get("cl_gibfilter", "0", CVAR_NONE, "Hide gibs");
	cl_parsesay = Cvar_Get("cl_parsesay", "0", CVAR_NONE, "None");
	cl_nofake = Cvar_Get("cl_nofake", "0", CVAR_NONE, "Unhide fake messages");
}
