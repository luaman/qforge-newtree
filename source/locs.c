/*
	locs.c

	Parsing and handling of location files.

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
# include "config.h"
#endif

#include <limits.h>
#include <string.h>

#include "client.h"
#include "qtypes.h"
#include "sys.h"
#include "locs.h"
#include "console.h"

#define LOCATION_BLOCK	128				// 128 locations per block.

location_t **locations = NULL;
int         locations_alloced = 0;
int         locations_count = 0;
int         location_blocks = 0;
int         locisgz = 0;

void locs_add (vec3_t location, char *name);
void locs_load (char *mapname);
void locs_free (void);
void locs_more (void);

location_t *
locs_find (vec3_t target)
{
	location_t *best = NULL, *cur;
	float       best_distance = 9999999, distance;
	int         i;

	for (i = 0; i < locations_count; i++) {
		cur = locations[i];
		distance = VectorDistance_fast (target, cur->loc);
		// distance = VectorDistance(target, cur->loc);
		if ((distance < best_distance) || !best) {
			best = cur;
			best_distance = distance;
		}
	}

	return best;
}

void
locs_add (vec3_t location, char *name)
{
	int         num;

	locations_count++;
	if (locations_count >= locations_alloced)
		locs_more ();

	num = locations_count - 1;

	locations[num] = malloc (sizeof (location_t));

	locations[num]->loc[0] = location[0];
	locations[num]->loc[1] = location[1];
	locations[num]->loc[2] = location[2];
	locations[num]->name = strdup (name);
	if (!locations[num]->name)
		Sys_Error ("locs_add: Can't strdup name!");
}

void
locs_load (char *mapname)
{
	QFile      *file;
	char       *line, *t1, *t2;
	vec3_t      loc;
	char        tmp[PATH_MAX];
	char        foundname[MAX_OSPATH];
	int         templength = 0;
	
	snprintf (tmp, sizeof (tmp), "maps/%s.loc", mapname);
	templength = _COM_FOpenFile (tmp, &file, foundname, 1);
	if (!file) {
		Con_Printf ("Couldn't load %s\n", tmp);
		return;
	}
#ifdef HAVE_ZLIB
	if (strncmp(foundname + strlen(foundname) - 3,".gz",3) == 0) 
		locisgz = 1;
	else 
		locisgz = 0;
#endif
	while ((line = Qgetline (file))) {
		if (line[0] == '#')
			continue;

		loc[0] = strtol (line, &t1, 0) * (1.0 / 8);
		loc[1] = strtol (t1, &t2, 0) * (1.0 / 8);
		loc[2] = strtol (t2, &t1, 0) * (1.0 / 8);
		t1++;
		t2 = strrchr (t1, '\n');
		if (t2) {
			t2[0] = '\0';
			// handle dos format lines (COM_FOpenFile is binary only)
			// and unix is effectively binary only anyway
			if (t2 > t1 && t2[-1] == '\r')
				t2[-1] = '\0';
		}
		locs_add (loc, t1);
	}
	Qclose (file);
}

void
locs_reset (void)
{
	int         i;

	for (i = 0; i < locations_count; i++) {
		free ((void *) locations[i]->name);
		free ((void *) locations[i]);
		locations[i] = NULL;
	}

	free (locations);
	locations_alloced = 0;
	locations_count = 0;
	locations = NULL;
}

void
locs_more (void)
{
	size_t      size;

	location_blocks++;
	locations_alloced += LOCATION_BLOCK;
	size = (sizeof (location_t *) * LOCATION_BLOCK * location_blocks);

	if (locations)
		locations = realloc (locations, size);
	else
		locations = malloc (size);

	if (!locations)
		Sys_Error ("ERROR! Can not alloc memory for location block!");
}
