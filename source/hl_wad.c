/*
        hl_wad.c

        WAD loading support for WAD3 texture wads. By LordHavoc

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
# include <config.h>
#endif
#include "sys.h"
#include "wad.h"
#include "quakefs.h"
#include "qendian.h"


#define TEXWAD_MAXIMAGES 8192
typedef struct
{
	char name[16];
	FILE *file;
	int position;
} texwadlump_t;
extern int image_width, image_height;
texwadlump_t	texwadlump[TEXWAD_MAXIMAGES];

/*
====================
W_LoadTextureWadFile
====================
*/
void W_LoadTextureWadFile (char *filename, int complain)
{
	lumpinfo_t		*lumps, *lump_p;
	wadinfo_t		header;
	unsigned		i, j;
	int				infotableofs;
	FILE			*file;
	int				numlumps;
	int				temp;

	
	COM_FOpenFile (filename, &file);
	if (!file)
	{
		if (complain)
			Con_Printf ("W_LoadTextureWadFile: couldn't find %s", filename);
		return;
	}

	if (fread(&header, sizeof(wadinfo_t), 1, file) != 1)
	{Con_Printf ("W_LoadTextureWadFile: unable to read wad header");return;}
	
	if(header.identification[0] != 'W'
	|| header.identification[1] != 'A'
	|| header.identification[2] != 'D'
	|| header.identification[3] != '3')
	{Con_Printf ("W_LoadTextureWadFile: Wad file %s doesn't have WAD3 id\n",filename);return;}

	numlumps = LittleLong(header.numlumps);
	if (numlumps < 1 || numlumps > TEXWAD_MAXIMAGES)
	{Con_Printf ("W_LoadTextureWadFile: invalid number of lumps (%i)\n", numlumps);return;}
	infotableofs = LittleLong(header.infotableofs);
	if (fseek(file, infotableofs, SEEK_SET))
	{Con_Printf ("W_LoadTextureWadFile: unable to seek to lump table");return;}
	if (!(lumps = malloc(sizeof(lumpinfo_t)*numlumps)))
	{Con_Printf ("W_LoadTextureWadFile: unable to allocate temporary memory for lump table");return;}

	if (fread(lumps, sizeof(lumpinfo_t), numlumps, file) != numlumps)
	{Con_Printf ("W_LoadTextureWadFile: unable to read lump table");return;}
	
	for (i=0, lump_p = lumps ; i<numlumps ; i++,lump_p++)
	{
		W_CleanupName (lump_p->name, lump_p->name);
		for (j = 0;j < TEXWAD_MAXIMAGES;j++)
		{
			if (texwadlump[j].name[0]) // occupied slot, check the name
			{
				if (!strcmp(lump_p->name, texwadlump[j].name)) // name match, replace old one
					break;
			}
			else // empty slot
				break;
		}
		if (j >= TEXWAD_MAXIMAGES)
			break; // abort loading
		W_CleanupName (lump_p->name, texwadlump[j].name);
		texwadlump[j].file = file;
		texwadlump[j].position = LittleLong(lump_p->filepos);
	}
	free(lumps);
	// leaves the file open
}

byte *W_GetTexture(char *name, int matchwidth, int matchheight)
{
	int i, c, datasize;
	short colorcount;
	FILE *file;
	struct
	{
		char name[16];
		int width;
		int height;
		int ofs[4];
	} t;
	byte pal[256][3], *indata, *outdata, *data;
	for (i = 0;i < TEXWAD_MAXIMAGES;i++)
	{
		if (texwadlump[i].name[0])
		{
			if (!strcmp(name, texwadlump[i].name)) // found it
			{
				file = texwadlump[i].file;
				if (fseek(file, texwadlump[i].position, SEEK_SET))
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}
				if (fread(&t, sizeof(t), 1, file) != 1)
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}
				image_width = LittleLong(t.width);
				image_height = LittleLong(t.height);
				if (matchwidth && image_width != matchwidth)
					continue;
				if (matchheight && image_height != matchheight)
					continue;
				if (image_width & 15 || image_height & 15)
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}

				// allocate space for expanded image,
				// and load incoming image into upper area (overwritten as it expands)
				if (!(data = outdata = malloc(image_width*image_height*4)))
                                {Con_Printf("W_GetTexture: out of memory");return false;}
				indata = outdata + image_width*image_height*3;
				datasize = image_width*image_height*85/64;

				// read the image data
				if (fseek(file, texwadlump[i].position + sizeof(t), SEEK_SET))
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}
				if (fread(indata, 1, image_width*image_height, file) != image_width*image_height)
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}

				// read the number of colors used (always 256)
				if (fseek(file, texwadlump[i].position + sizeof(t) + datasize, SEEK_SET))
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}
				if (fread(&colorcount, 2, 1, file) != 1)
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}
				colorcount = LittleShort(colorcount);

				// sanity checking
				if (colorcount < 0) colorcount = 0;
				if (colorcount > 256) colorcount = 256;

				// read the palette
				if (fread(&pal, 3, colorcount, file) != colorcount)
                                {Con_Printf("W_GetTexture: corrupt WAD3 file");return false;}

				// expand the image to 32bit RGBA
				for (i = 0;i < image_width*image_height;i++)
				{
					c = *indata++;
					if (c == 255) // transparent color
					{
						*outdata++ = 0;
						*outdata++ = 0;
						*outdata++ = 0;
						*outdata++ = 0;
					}
					else
					{
						*outdata++ = pal[c][0];
						*outdata++ = pal[c][1];
						*outdata++ = pal[c][2];
						*outdata++ = 255;
					}
				}
				return data;
			}
		}
		else
			break;
	}
	image_width = image_height = 0;
	return NULL;
}
