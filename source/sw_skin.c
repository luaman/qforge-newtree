/*
	cl_trans.c

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

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "client.h"
#include "host.h"
#include "sys.h"
#include "vid.h"

void
Skin_Set_Translate (player_info_t *player)
{
	int         i, j;
	int         top, bottom;
	byte       *dest, *source;

	top = bound (0, player->topcolor, 13) * 16;
	bottom = bound (0, player->bottomcolor, 13) * 16;

	dest = player->translations;
	source = vid.colormap;
	memcpy (dest, vid.colormap, sizeof (player->translations));

	for (i = 0; i < VID_GRADES; i++, dest += 256, source += 256) {
		if (top < 128)				// the artists made some backwards
									// ranges.  sigh.
			memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j = 0; j < 16; j++)
				dest[TOP_RANGE + j] = source[top + 15 - j];

		if (bottom < 128)
			memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j = 0; j < 16; j++)
				dest[BOTTOM_RANGE + j] = source[bottom + 15 - j];
	}
}

void
Skin_Do_Translation (player_info_t *player)
{
}

void
Skin_Init_Translation (void)
{
}

void
Skin_Process (skin_t *skin, struct tex_s *)
{
}
