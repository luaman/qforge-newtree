/*
	client.h

	Client definitions

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

#ifndef _SKIN_H
#define _SKIN_H

#include "client.h"

typedef struct skin_s
{
	char		name[16];
	qboolean	failedload;		// the name isn't a valid skin
	cache_user_t	cache;
} skin_t;

extern byte player_8bit_texels[320 * 200];
struct tex_s;
struct player_info_s;

void	Skin_Find (struct player_info_s *sc);
byte	*Skin_Cache (skin_t *skin);
void	Skin_Skins_f (void);
void	Skin_AllSkins_f (void);
void	Skin_NextDownload (void);
void	Skin_Init (void);
void	Skin_Init_Cvars (void);
void	Skin_Init_Translation (void);
void	Skin_Set_Translate (struct player_info_s *player);
void	Skin_Do_Translation (player_info_t *player);
void	Skin_Process (skin_t *skin, struct tex_s *);

#define MAX_CACHED_SKINS 128

#define RSSHOT_WIDTH 320
#define RSSHOT_HEIGHT 200

#endif
