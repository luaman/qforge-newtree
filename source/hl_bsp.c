// Halflife BSP Loading
// Hack #1:
//  Use this LoadLighting instead of the existing one in gl_model.c
// Hack #2:
//  Use this LoadTexture instead of the existing one in gl_model.c
// Hack #?:
//  Add  int             lhcsum; to gltexture_t in gl_draw.c
// Hack #4:
//  save bspversion in 'int bspversion;'. Allow 30.
// Hack #5:
//  New hl_wad.c
// Hack #6:
//  In gl_model.h, texture_s, add "int transparent;"
// Hack #7:
//    LoadEntities:
//        if (bspversion > 29) // hlbsp
//                CL_ParseEntityLump(loadmodel->entities);
//
// Hack #8:
//  Top of GL_RenderBrushPoly (gl_rsurf.c - and add the function :)
//        if (fa->texinfo->texture->transparent) {
//          R_RenderBrushPolyTransparent(fa);
//          return;
//        }
//
// Hack #9: (mod_loadfaces)
//                if (i == -1)
//                        out->samples = NULL;
//                else if (bspversion > 29)
//                        out->samples = loadmodel->lightdata + i;
//                else
//                        out->samples = loadmodel->lightdata + (i * 3);
// 

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "bothdefs.h"   // needed by: common.h, net.h, client.h
#include "qendian.h"
#include "msg.h"
#include "bspfile.h"    // needed by: glquake.h
#include "vid.h"
#include "sys.h"
#include "zone.h"       // needed by: client.h, gl_model.h
#include "mathlib.h"    // needed by: protocol.h, render.h, client.h,
                        //  modelgen.h, glmodel.h
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "crc.h"
#include "net.h"        // needed by: client.h
#include "protocol.h"   // needed by: client.h
#include "cmd.h"
#include "sbar.h"
#include "render.h"     // needed by: client.h, gl_model.h, glquake.h
#include "client.h"     // need cls in this file
#include "model.h"		// needed by: glquake.h
#include "console.h"
#include "glquake.h"
#include "quakefs.h"
#include "checksum.h"
#include "hl.h"


int		image_width;
int		image_height;

// From gl_draw.c
typedef struct
{
	int		texnum;
	char	identifier[64];
	int		width, height;
	qboolean	mipmap;
        int             lhcsum;
} gltexture_t;

extern  int     bspversion;
extern  model_t *loadmodel;
extern  char    loadname[32];
extern  byte    *mod_base;
extern  gltexture_t     gltextures[];
extern  int             numgltextures;

int HL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha);

typedef struct miptex_hls
{
	char		name[16];
	unsigned	width, height;
        char            junk[16];
} miptex_hl;

void HL_Mod_LoadLighting (lump_t *l)
{
	if (!l->filelen)
	{
		loadmodel->lightdata = NULL;
		return;
	}

        if (bspversion > 29) { // Load coloured lighting data
         loadmodel->lightdata = Hunk_AllocName ( l->filelen, loadname);
         memcpy (loadmodel->lightdata, mod_base + l->fileofs, l->filelen);
        } else {     // Expand white lighting data
         byte *in, *out, d;
         int i;
         Con_DPrintf("Converting lightmap data\n");
         loadmodel->lightdata = Hunk_AllocName (l->filelen*3, loadname);
         in = loadmodel->lightdata + l->filelen*2; // place the file at the end, so it will not be overwritten until the very last write
         out = loadmodel->lightdata;
         memcpy (in, mod_base + l->fileofs, l->filelen);
         for (i = 0;i < l->filelen;i++) {
          d = *in++;
          *out++ = d;
          *out++ = d;
          *out++ = d;
         }
        }
}

byte* loadimagepixels (char* filename, qboolean complain, int matchwidth, int matchheight)
{
	FILE	*f;
	char	basename[128], name[128];
	byte	*image_rgba;
	COM_StripExtension(filename, basename); // strip the extension to allow TGA skins on Q2 models despite the .pcx in the skin name
	sprintf (name, "%s.tga", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadTGA (f);
		//return LoadTGA (f, matchwidth, matchheight);
	sprintf (name, "%s.pcx", basename);
	COM_FOpenFile (name, &f);
	if (f)
		return LoadPCX (f);
		//return LoadPCX (f, matchwidth, matchheight);
	if ((image_rgba = W_GetTexture(basename, matchwidth, matchheight)))
		return image_rgba;
	if (complain)
		Con_Printf ("Couldn't load %s.tga or .pcx\n", filename);
	return NULL;
}

void HL_Mod_LoadTextures (lump_t *l)
{
	int             i, j, num, max, altmax, freeimage, transparent, bytesperpixel;
	miptex_t	*mt;
	texture_t	*tx, *tx2;
	texture_t	*anims[10];
	texture_t	*altanims[10];
	dmiptexlump_t *m;
	byte *data;
	char	imagename[32];

	if (!l->filelen)
	{
		loadmodel->textures = NULL;
		return;
	}

	m = (dmiptexlump_t *)(mod_base + l->fileofs);
	m->nummiptex = LittleLong (m->nummiptex);
	loadmodel->numtextures = m->nummiptex;
	loadmodel->textures = Hunk_AllocName (m->nummiptex * sizeof(*loadmodel->textures) , loadname);

	for (i=0 ; i<m->nummiptex ; i++)
	{
		m->dataofs[i] = LittleLong(m->dataofs[i]);
		if (m->dataofs[i] == -1)
			continue;
		mt = (miptex_t *)((byte *)m + m->dataofs[i]);
		mt->width = LittleLong (mt->width);
		mt->height = LittleLong (mt->height);
		for (j=0 ; j<MIPLEVELS ; j++)
			mt->offsets[j] = LittleLong (mt->offsets[j]);
		
		if ( (mt->width & 15) || (mt->height & 15) )
			Sys_Error ("Texture %s is not 16 aligned", mt->name);

		// LordHavoc: rewriting the map texture loader for GLQuake
		tx = Hunk_AllocName (sizeof(texture_t), loadname );
		loadmodel->textures[i] = tx;

		memcpy (tx->name, mt->name, sizeof(tx->name));
		tx->width = mt->width;
		tx->height = mt->height;
		for (j=0 ; j<MIPLEVELS ; j++)
			tx->offsets[j] = 0;

		strcpy(imagename, "textures/");
		strcat(imagename, mt->name);

		freeimage = true;
                transparent = false;
                bytesperpixel = 4;
		data = loadimagepixels(imagename, false, tx->width, tx->height);
		if (!data) // no external texture found
		{
			strcpy(imagename, mt->name);
			data = loadimagepixels(imagename, false, tx->width, tx->height);
			if (!data) // no external texture found
			{
				freeimage = false;
                                bytesperpixel = 1;
				if (mt->offsets[0]) // texture included
					data = (byte *)((int) mt + mt->offsets[0]);
				else // no texture, and no external replacement texture was found
				{
					tx->width = tx->height = 16;
					data = (byte *)((int) r_notexture_mip + r_notexture_mip->offsets[0]);
				}
			}
			else
			{
				for (j = 0;j < image_width*image_height;j++)
					if (data[j*4+3] < 255)
					{
						transparent = true;
						break;
					}
			}
		}
		else
		{
			for (j = 0;j < image_width*image_height;j++)
				if (data[j*4+3] < 255)
				{
					transparent = true;
					break;
				}
		}
                if (!strncmp(mt->name,"sky",3)) 
		{
			tx->transparent = false;
                        R_InitSky_32 (data, bytesperpixel);
		}
		else
		{
			tx->transparent = transparent;
			texture_mode = GL_LINEAR_MIPMAP_NEAREST; //_LINEAR;
                        if (bytesperpixel == 1)
                         tx->gl_texturenum = GL_LoadTexture (tx->name, tx->width, tx->height, data, true, transparent);
                        else
                         tx->gl_texturenum = HL_LoadTexture (tx->name, tx->width, tx->height, data, true, transparent);
			texture_mode = GL_LINEAR;
		}
		if (freeimage)
			free(data);
	}
//
// sequence the animations
//
	for (i=0 ; i<m->nummiptex ; i++)
	{
		tx = loadmodel->textures[i];
		if (!tx || tx->name[0] != '+')
			continue;
		if (tx->anim_next)
			continue;	// allready sequenced

	// find the number of frames in the animation
		memset (anims, 0, sizeof(anims));
		memset (altanims, 0, sizeof(altanims));

		max = tx->name[1];
		altmax = 0;
		if (max >= 'a' && max <= 'z')
			max -= 'a' - 'A';
		if (max >= '0' && max <= '9')
		{
			max -= '0';
			altmax = 0;
			anims[max] = tx;
			max++;
		}
		else if (max >= 'A' && max <= 'J')
		{
			altmax = max - 'A';
			max = 0;
			altanims[altmax] = tx;
			altmax++;
		}
		else
			Sys_Error ("Bad animating texture %s", tx->name);

		for (j=i+1 ; j<m->nummiptex ; j++)
		{
			tx2 = loadmodel->textures[j];
			if (!tx2 || tx2->name[0] != '+')
				continue;
			if (strcmp (tx2->name+2, tx->name+2))
				continue;

			num = tx2->name[1];
			if (num >= 'a' && num <= 'z')
				num -= 'a' - 'A';
			if (num >= '0' && num <= '9')
			{
				num -= '0';
				anims[num] = tx2;
				if (num+1 > max)
					max = num + 1;
			}
			else if (num >= 'A' && num <= 'J')
			{
				num = num - 'A';
				altanims[num] = tx2;
				if (num+1 > altmax)
					altmax = num+1;
			}
			else
				Sys_Error ("Bad animating texture %s", tx->name);
		}
		
#define	ANIM_CYCLE	2
	// link them all together
		for (j=0 ; j<max ; j++)
		{
			tx2 = anims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = max * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = anims[ (j+1)%max ];
			if (altmax)
				tx2->alternate_anims = altanims[0];
		}
		for (j=0 ; j<altmax ; j++)
		{
			tx2 = altanims[j];
			if (!tx2)
				Sys_Error ("Missing frame %i of %s",j, tx->name);
			tx2->anim_total = altmax * ANIM_CYCLE;
			tx2->anim_min = j * ANIM_CYCLE;
			tx2->anim_max = (j+1) * ANIM_CYCLE;
			tx2->anim_next = altanims[ (j+1)%altmax ];
			if (max)
				tx2->alternate_anims = anims[0];
		}
	}
}


void CL_ParseEntityLump(char *entdata)
{
	char *data;
	char key[128], value[1024];
	char wadname[128];
	int i, j, k;

	data = entdata;
	if (!data)
		return;

	data = COM_Parse(data);

	if (!data)
		return; // valid exit

	if (com_token[0] != '{')
              return; // error


	while (1)
	{
		data = COM_Parse(data);
		if (!data)
			return; // error
		if (com_token[0] == '}')
			return; // since we're just parsing the first ent (worldspawn), exit
		strcpy(key, com_token);
		while (key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_Parse(data);
		if (!data)
			return; // error
		strcpy(value, com_token);
//              if (!strcmp("sky", key))
//                        strcpy(skyname, value);
//              else if (!strcmp("skyboxsize", key))
//              {
//                        r_skyboxsize.value = atof(value);
//                        if (r_skyboxsize.value < 64)
//                                r_skyboxsize.value = 64;
//                }

                if (!strcmp("wad", key)) // for HalfLife maps
		{
			j = 0;
			for (i = 0;i < 128;i++)
				if (value[i] != ';' && value[i] != '\\' && value[i] != '/' && value[i] != ':')
					break;
			if (value[i])
			{
				for (;i < 128;i++)
				{
					// ignore path - the \\ check is for HalfLife... stupid windoze 'programmers'...
					if (value[i] == '\\' || value[i] == '/' || value[i] == ':')
						j = i+1;
					else if (value[i] == ';' || value[i] == 0)
					{
						k = value[i];
						value[i] = 0;
						strcpy(wadname, "textures/");
                                                Con_DPrintf("Wad: %s\n", &value[j]);
						strcat(wadname, &value[j]);
						W_LoadTextureWadFile (wadname, false);
						j = i+1;                                                
						if (!k)
							break;
					}
				}
			}
		}
	}
}

/*
================
GL_LoadTexture
================
*/
int lhcsumtable2[256];
int HL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha)
{
	int			i, s, lhcsum;
	gltexture_t	*glt;

	// LordHavoc: do a checksum to confirm the data really is the same as previous
	// occurances. well this isn't exactly a checksum, it's better than that but
	// not following any standards.
	lhcsum = 0;
        s = width*height*4;
        for (i = 0;i < 256;i++) lhcsumtable2[i] = i + 1;
        for (i = 0;i < s;i++) lhcsum += (lhcsumtable2[data[i] & 255]++);

	// see if the texture is allready present
	if (identifier[0])
	{
		for (i=0, glt=gltextures ; i<numgltextures ; i++, glt++)
		{
			if (!strcmp (identifier, glt->identifier))
			{
				// LordHavoc: everyone hates cache mismatchs, so I fixed it
				if (lhcsum != glt->lhcsum || width != glt->width || height != glt->height)
				{
					Con_DPrintf("GL_LoadTexture: cache mismatch, replacing old texture\n");
                                        goto HL_LoadTexture_setup; // drop out with glt pointing to the texture to replace
				}
				return glt->texnum;
			}
		}
	}

	glt = &gltextures[numgltextures];
	numgltextures++;

	strcpy (glt->identifier, identifier);
	glt->texnum = texture_extension_number;
	texture_extension_number++;
// LordHavoc: label to drop out of the loop into the setup code
HL_LoadTexture_setup:
	glt->lhcsum = lhcsum; // LordHavoc: used to verify textures are identical
	glt->width = width;
	glt->height = height;
	glt->mipmap = mipmap;

        GL_Bind(glt->texnum);
        GL_Upload32 ((unsigned *) data, width, height, mipmap, true);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	return glt->texnum;
}

