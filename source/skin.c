/*
	skin.c

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

#include "cl_parse.h"
#include "console.h"
#include "cmd.h"
#include "msg.h"
#include "pcx.h"
#include "qendian.h"
#include "screen.h"
#include "skin.h"
#include "sys.h"
#include "va.h"

cvar_t     *baseskin;
cvar_t     *noskins;
cvar_t     *skin;
cvar_t     *topcolor;
cvar_t     *bottomcolor;

char        allskins[128];

#define	MAX_CACHED_SKINS		128
skin_t      skins[MAX_CACHED_SKINS];
int         numskins;

/*
================
Skin_Find

  Determines the best skin for the given scoreboard
  slot, and sets scoreboard->skin

================
*/
void
Skin_Find (player_info_t *sc)
{
	skin_t     *skin;
	int         i;
	char        name[128], *s;

	if (allskins[0])
		strcpy (name, allskins);
	else {
		s = Info_ValueForKey (sc->userinfo, "skin");
		if (s && s[0])
			strcpy (name, s);
		else
			strcpy (name, baseskin->string);
	}

	if (strstr (name, "..") || *name == '.')
		strcpy (name, "base");

	COM_StripExtension (name, name);

	for (i = 0; i < numskins; i++) {
		if (!strcmp (name, skins[i].name)) {
			sc->skin = &skins[i];
			Skin_Cache (sc->skin);
			return;
		}
	}

	if (numskins == MAX_CACHED_SKINS) {	// ran out of spots, so flush
										// everything
		Skin_Skins_f ();
		return;
	}

	skin = &skins[numskins];
	sc->skin = skin;
	numskins++;

	memset (skin, 0, sizeof (*skin));
	strncpy (skin->name, name, sizeof (skin->name) - 1);
}


/*
==========
Skin_Cache

Returns a pointer to the skin bitmap, or NULL to use the default
==========
*/
byte *
Skin_Cache (skin_t *skin)
{
	char        name[1024];
	byte       *raw;
	byte       *out, *pix;
	pcx_t      *pcx;
	int         x, y;
	int         dataByte;
	int         runLength;

	if (cls.downloadtype == dl_skin)
		return NULL;					// use base until downloaded

	if (noskins->int_val)				// JACK: So NOSKINS > 1 will show
										// skins, but
		return NULL;					// not download new ones.

	if (skin->failedload)
		return NULL;

	out = Cache_Check (&skin->cache);
	if (out)
		return out;

	// load the pic from disk
	snprintf (name, sizeof (name), "skins/%s.pcx", skin->name);
	raw = COM_LoadTempFile (name);
	if (!raw) {
		Con_Printf ("Couldn't load skin %s\n", name);
		snprintf (name, sizeof (name), "skins/%s.pcx", baseskin->string);
		raw = COM_LoadTempFile (name);
		if (!raw) {
			skin->failedload = true;
			return NULL;
		}
	}

	// parse the PCX file
	pcx = (pcx_t *) raw;
	raw = &pcx->data;

	pcx->xmax = LittleShort (pcx->xmax);
	pcx->xmin = LittleShort (pcx->xmin);
	pcx->ymax = LittleShort (pcx->ymax);
	pcx->ymin = LittleShort (pcx->ymin);
	pcx->hres = LittleShort (pcx->hres);
	pcx->vres = LittleShort (pcx->vres);
	pcx->bytes_per_line = LittleShort (pcx->bytes_per_line);
	pcx->palette_type = LittleShort (pcx->palette_type);

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8 || pcx->xmax >= 320 || pcx->ymax >= 200) {
		skin->failedload = true;
		Con_Printf ("Bad skin %s\n", name);
		return NULL;
	}

	out = Cache_Alloc (&skin->cache, 320 * 200, skin->name);
	if (!out)
		Sys_Error ("Skin_Cache: couldn't allocate");

	pix = out;
	memset (out, 0, 320 * 200);

	for (y = 0; y < pcx->ymax; y++, pix += 320) {
		for (x = 0; x <= pcx->xmax;) {
			if (raw - (byte *) pcx > com_filesize) {
				Cache_Free (&skin->cache);
				skin->failedload = true;
				Con_Printf ("Skin %s was malformed.  You should delete it.\n",
							name);
				return NULL;
			}
			dataByte = *raw++;

			if ((dataByte & 0xC0) == 0xC0) {
				runLength = dataByte & 0x3F;
				if (raw - (byte *) pcx > com_filesize) {
					Cache_Free (&skin->cache);
					skin->failedload = true;
					Con_Printf
						("Skin %s was malformed.  You should delete it.\n",
						 name);
					return NULL;
				}
				dataByte = *raw++;
			} else
				runLength = 1;

			// skin sanity check
			if (runLength + x > pcx->xmax + 2) {
				Cache_Free (&skin->cache);
				skin->failedload = true;
				Con_Printf ("Skin %s was malformed.  You should delete it.\n",
							name);
				return NULL;
			}
			while (runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if (raw - (byte *) pcx > com_filesize) {
		Cache_Free (&skin->cache);
		skin->failedload = true;
		Con_Printf ("Skin %s was malformed.  You should delete it.\n", name);
		return NULL;
	}

	skin->failedload = false;

	return out;
}


/*
=================
Skin_NextDownload
=================
*/
void
Skin_NextDownload (void)
{
	player_info_t *sc;
	int         i;

	if (cls.downloadnumber == 0) {
		Con_Printf ("Checking skins...\n");
		SCR_UpdateScreen ();
	}
	cls.downloadtype = dl_skin;

	for (; cls.downloadnumber != MAX_CLIENTS; cls.downloadnumber++) {
		sc = &cl.players[cls.downloadnumber];
		if (!sc->name[0])
			continue;
		Skin_Find (sc);
		if (noskins->int_val)
			continue;
		if (!CL_CheckOrDownloadFile (va ("skins/%s.pcx", sc->skin->name)))
			return;						// started a download
	}

	cls.downloadtype = dl_none;

	// now load them in for real
	for (i = 0; i < MAX_CLIENTS; i++) {
		sc = &cl.players[i];
		if (!sc->name[0])
			continue;
		Skin_Cache (sc->skin);
		sc->skin = NULL;
	}

	if (cls.state != ca_active) {		// get next signon phase
		MSG_WriteByte (&cls.netchan.message, clc_stringcmd);
		MSG_WriteString (&cls.netchan.message, va ("begin %i", cl.servercount));
		Cache_Report ();				// print remaining memory
	}
}


/*
==========
Skin_Skins_f

Refind all skins, downloading if needed.
==========
*/
void
Skin_Skins_f (void)
{
	int         i;

	for (i = 0; i < numskins; i++) {
		if (skins[i].cache.data)
			Cache_Free (&skins[i].cache);
	}
	numskins = 0;

	cls.downloadnumber = 0;
	cls.downloadtype = dl_skin;
	Skin_NextDownload ();
}


/*
==========
Skin_AllSkins_f

Sets all skins to one specific one
==========
*/
void
Skin_AllSkins_f (void)
{
	strcpy (allskins, Cmd_Argv (1));
	Skin_Skins_f ();
}

void
CL_Color_f (void)
{
	// just for quake compatability...
	int         top, bottom;
	char        num[16];

	if (Cmd_Argc () == 1) {
		Con_Printf ("\"color\" is \"%s %s\"\n",
					Info_ValueForKey (cls.userinfo, "topcolor"),
					Info_ValueForKey (cls.userinfo, "bottomcolor"));
		Con_Printf ("color <0-13> [0-13]\n");
		return;
	}

	if (Cmd_Argc () == 2)
		top = bottom = atoi (Cmd_Argv (1));
	else {
		top = atoi (Cmd_Argv (1));
		bottom = atoi (Cmd_Argv (2));
	}

	top &= 15;
	if (top > 13)
		top = 13;
	bottom &= 15;
	if (bottom > 13)
		bottom = 13;

	snprintf (num, sizeof (num), "%i", top);
	Cvar_Set (topcolor, num);
	snprintf (num, sizeof (num), "%i", bottom);
	Cvar_Set (bottomcolor, num);
}

void
Skin_Init (void)
{
	Cmd_AddCommand ("skins", Skin_Skins_f, "No Description");
	Cmd_AddCommand ("allskins", Skin_AllSkins_f, "No Description");
	Cmd_AddCommand ("color", CL_Color_f, "No Description");
}

void
Skin_Init_Cvars (void)
{
	baseskin = Cvar_Get ("baseskin", "base", CVAR_NONE,
						 "default base skin name");
	noskins = Cvar_Get ("noskins", "0", CVAR_NONE,
						"set to 1 to not download new skins");
	skin = Cvar_Get ("skin", "", CVAR_ARCHIVE | CVAR_USERINFO, "Players skin");
	topcolor = Cvar_Get ("topcolor", "0", CVAR_ARCHIVE | CVAR_USERINFO,
						 "Players color on top");
	bottomcolor = Cvar_Get ("bottomcolor", "0", CVAR_ARCHIVE | CVAR_USERINFO,
							"Players color on bottom");
}
