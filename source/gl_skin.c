/*
	gl_skin.c

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

#include "glquake.h"
#include "host.h"
#include "protocol.h"
#include "skin.h"
#include "sys.h"

/*
=====================
CL_NewTranslation
=====================
*/
void
CL_NewTranslation (int slot)
{
	int         top, bottom;
	byte        translate[256];
	unsigned int translate32[256];
	int         i, j;
	byte       *original;
	unsigned int pixels[512 * 256], *out;
	unsigned int scaled_width, scaled_height;
	int         inwidth, inheight;
	int         tinwidth, tinheight;
	byte       *inrow;
	unsigned int frac, fracstep;
	player_info_t *player;
	extern byte player_8bit_texels[320 * 200];
	char        s[512];
	int         playernum = slot;

	if (slot > MAX_CLIENTS)
		Host_EndGame ("CL_NewTranslation: slot > MAX_CLIENTS");

	player = &cl.players[playernum];
	if (!player->name[0])
		return;

	strcpy (s, Info_ValueForKey (player->userinfo, "skin"));
	COM_StripExtension (s, s);
	if (player->skin && !strequal (s, player->skin->name))
		player->skin = NULL;

	if (player->_topcolor != player->topcolor ||
		player->_bottomcolor != player->bottomcolor || !player->skin) {
		player->_topcolor = player->topcolor;
		player->_bottomcolor = player->bottomcolor;

		top = player->topcolor;
		bottom = player->bottomcolor;
		top = (top < 0) ? 0 : ((top > 13) ? 13 : top);
		bottom = (bottom < 0) ? 0 : ((bottom > 13) ? 13 : bottom);
		top *= 16;
		bottom *= 16;

		for (i = 0; i < 256; i++)
			translate[i] = i;

		for (i = 0; i < 16; i++) {
			if (top < 128)	// the artists made some backwards ranges.  sigh.
				translate[TOP_RANGE + i] = top + i;
			else
				translate[TOP_RANGE + i] = top + 15 - i;

			if (bottom < 128)
				translate[BOTTOM_RANGE + i] = bottom + i;
			else
				translate[BOTTOM_RANGE + i] = bottom + 15 - i;
		}

		// locate the original skin pixels
		tinwidth = 296;		// real model width
		tinheight = 194;	// real model height

		if (!player->skin)
			Skin_Find (player);
		if ((original = Skin_Cache (player->skin)) != NULL) {
			// skin data width
			inwidth = 320;
			inheight = 200;
		} else {
			original = player_8bit_texels;
			inwidth = 296;
			inheight = 194;
		}

		// because this happens during gameplay, do it fast
		// instead of sending it through GL_Upload8()
		glBindTexture (GL_TEXTURE_2D, playertextures + playernum);

		// FIXME deek: This 512x256 limit sucks!
		scaled_width = min (gl_max_size->int_val, 512);
		scaled_height = min (gl_max_size->int_val, 256);

		// allow users to crunch sizes down even more if they want
		scaled_width >>= gl_playermip->int_val;
		scaled_height >>= gl_playermip->int_val;

		if (VID_Is8bit ()) {			// 8bit texture upload
			byte	*out2;

			out2 = (byte *) pixels;
			memset (pixels, 0, sizeof (pixels));
			fracstep = tinwidth * 0x10000 / scaled_width;
			for (i = 0; i < scaled_height; i++, out2 += scaled_width) {
				inrow = original + inwidth * (i * tinheight / scaled_height);
				frac = fracstep >> 1;
				for (j = 0; j < scaled_width; j += 4) {
					out2[j] = translate[inrow[frac >> 16]];
					frac += fracstep;
					out2[j + 1] = translate[inrow[frac >> 16]];
					frac += fracstep;
					out2[j + 2] = translate[inrow[frac >> 16]];
					frac += fracstep;
					out2[j + 3] = translate[inrow[frac >> 16]];
					frac += fracstep;
				}
			}

			GL_Upload8_EXT ((byte *) pixels, scaled_width, scaled_height, false,
							false);
			return;
		}

		for (i = 0; i < 256; i++)
			translate32[i] = d_8to24table[translate[i]];

		out = pixels;
		memset (pixels, 0, sizeof (pixels));
		fracstep = tinwidth * 0x10000 / scaled_width;
		for (i = 0; i < scaled_height; i++, out += scaled_width) {
			inrow = original + inwidth * (i * tinheight / scaled_height);
			frac = fracstep >> 1;
			for (j = 0; j < scaled_width; j += 4) {
				out[j] = translate32[inrow[frac >> 16]];
				frac += fracstep;
				out[j + 1] = translate32[inrow[frac >> 16]];
				frac += fracstep;
				out[j + 2] = translate32[inrow[frac >> 16]];
				frac += fracstep;
				out[j + 3] = translate32[inrow[frac >> 16]];
				frac += fracstep;
			}
		}

		glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format,
					  scaled_width, scaled_height, 0, GL_RGBA,
					  GL_UNSIGNED_BYTE, pixels);

		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}
