/*
	gl_rmisc.c

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
# include <config.h>
#endif
#include <string.h>
#include <stdio.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "bothdefs.h"   // needed by: common.h, net.h, client.h

#include "bspfile.h"    // needed by: glquake.h
#include "vid.h"
#include "sys.h"
#include "mathlib.h"    // needed by: protocol.h, render.h, client.h,
                        //  modelgen.h, glmodel.h
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "net.h"        // needed by: client.h
#include "protocol.h"   // needed by: client.h
#include "cmd.h"
#include "sbar.h"
#include "render.h"     // needed by: client.h, gl_model.h, glquake.h
#include "client.h"     // need cls in this file
#include "model.h"   // needed by: glquake.h
#include "console.h"
#include "glquake.h"

qboolean VID_Is8bit(void);
void R_InitBubble();
void R_FireColor_f(void);

cvar_t *gl_fires;
qboolean	allowskybox;		// allow skyboxes?  --KB

/*
==================
R_InitTextures
==================
*/
void	R_InitTextures (void)
{
	int		x,y, m;
	byte	*dest;

// create a simple checkerboard texture for the default
	r_notexture_mip = Hunk_AllocName (sizeof(texture_t) + 16*16+8*8+4*4+2*2, "notexture");
	
	r_notexture_mip->width = r_notexture_mip->height = 16;
	r_notexture_mip->offsets[0] = sizeof(texture_t);
	r_notexture_mip->offsets[1] = r_notexture_mip->offsets[0] + 16*16;
	r_notexture_mip->offsets[2] = r_notexture_mip->offsets[1] + 8*8;
	r_notexture_mip->offsets[3] = r_notexture_mip->offsets[2] + 4*4;
	
	for (m=0 ; m<4 ; m++)
	{
		dest = (byte *)r_notexture_mip + r_notexture_mip->offsets[m];
		for (y=0 ; y< (16>>m) ; y++)
			for (x=0 ; x< (16>>m) ; x++)
			{
				if (  (y< (8>>m) ) ^ (x< (8>>m) ) )
					*dest++ = 0;
				else
					*dest++ = 0xff;
			}
	}	
}

byte	dottexture[8][8] =
{
	{0,1,1,0,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{1,1,1,1,0,0,0,0},
	{0,1,1,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};
void R_InitParticleTexture (void)
{
	int		x,y;
	byte	data[8][8][4];

	//
	// particle texture
	//
	particletexture = texture_extension_number++;
    glBindTexture (GL_TEXTURE_2D, particletexture);

	for (x=0 ; x<8 ; x++)
	{
		for (y=0 ; y<8 ; y++)
		{
			data[y][x][0] = 255;
			data[y][x][1] = 255;
			data[y][x][2] = 255;
			data[y][x][3] = dottexture[x][y]*255;
		}
	}
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
===============
R_Envmap_f

Grab six views for environment mapping tests
===============
*/
void R_Envmap_f (void)
{
	byte	buffer[256*256*4];

	glDrawBuffer  (GL_FRONT);
	glReadBuffer  (GL_FRONT);
	envmap = true;

	r_refdef.vrect.x = 0;
	r_refdef.vrect.y = 0;
	r_refdef.vrect.width = 256;
	r_refdef.vrect.height = 256;

	r_refdef.viewangles[0] = 0;
	r_refdef.viewangles[1] = 0;
	r_refdef.viewangles[2] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env0.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 90;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env1.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 180;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env2.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[1] = 270;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env3.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[0] = -90;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env4.rgb", buffer, sizeof(buffer));		

	r_refdef.viewangles[0] = 90;
	r_refdef.viewangles[1] = 0;
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
	R_RenderView ();
	glReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	COM_WriteFile ("env5.rgb", buffer, sizeof(buffer));		

	envmap = false;
	glDrawBuffer  (GL_BACK);
	glReadBuffer  (GL_BACK);
	GL_EndRendering ();
}

/*
   R_LoadSky_f
*/
void
R_LoadSky_f (void)
{
	if (Cmd_Argc () != 2)
	{
		Con_Printf ("loadsky <name> : load a skybox\n");
		return;
	}

	R_LoadSkys (Cmd_Argv(1));
}


/*
===============
R_Init
===============
*/
void R_Init (void)
{
	allowskybox = false;	// server will decide if this is allowed  --KB

	Cmd_AddCommand ("timerefresh", R_TimeRefresh_f);	
	Cmd_AddCommand ("envmap", R_Envmap_f);	
	Cmd_AddCommand ("pointfile", R_ReadPointFile_f);
	Cmd_AddCommand ("loadsky", R_LoadSky_f);

	Cmd_AddCommand ("r_firecolor", R_FireColor_f);

	r_norefresh = Cvar_Get("r_norefresh", "0", CVAR_NONE, "None");
	r_lightmap = Cvar_Get("r_lightmap", "0", CVAR_NONE, "None");
	r_fullbright = Cvar_Get("r_fullbright", "0", CVAR_NONE, "None");
	r_drawentities = Cvar_Get("r_drawentities", "1", CVAR_NONE, "None");
	r_drawviewmodel = Cvar_Get("r_drawviewmodel", "1", CVAR_NONE, "None");
	r_shadows = Cvar_Get("r_shadows", "0", CVAR_NONE, "None");
	r_wateralpha = Cvar_Get("r_wateralpha", "1", CVAR_NONE, "None");
	r_waterripple = Cvar_Get ("r_waterripple", "0", CVAR_NONE, "None");
	r_dynamic = Cvar_Get("r_dynamic", "1", CVAR_NONE, "None");
	r_novis = Cvar_Get("r_novis", "0", CVAR_NONE, "None");
	r_speeds = Cvar_Get("r_speeds", "0", CVAR_NONE, "None");
	r_netgraph = Cvar_Get("r_netgraph", "0", CVAR_NONE, "None");

	gl_clear = Cvar_Get("gl_clear", "0", CVAR_NONE, "None");
	gl_texsort = Cvar_Get("gl_texsort", "1", CVAR_NONE, "None");
 
 	if (gl_mtexable)
		Cvar_SetValue(gl_texsort, 0.0);

	gl_cull = Cvar_Get("gl_cull", "1", CVAR_NONE, "None");
	gl_smooth = Cvar_Get("gl_smooth", "1", CVAR_NONE, "None");
	gl_smoothdlights = Cvar_Get("gl_smoothdlights", "1", CVAR_NONE, "None");
	gl_affinemodels = Cvar_Get("gl_affinemodels", "0", CVAR_NONE, "None");
	gl_polyblend = Cvar_Get("gl_polyblend", "1", CVAR_NONE, "None");
	gl_flashblend = Cvar_Get("gl_flashblend",  "0", CVAR_NONE, "None");
	gl_playermip = Cvar_Get("gl_playermip", "0", CVAR_NONE, "None");
	gl_nocolors = Cvar_Get("gl_nocolors", "0", CVAR_NONE, "None");

	gl_fires = Cvar_Get ("gl_fires", "0", CVAR_ARCHIVE,
			"Toggles lavaball and rocket fireballs");

	gl_particles = Cvar_Get ("gl_particles", "1", CVAR_ARCHIVE,
			"whether or not to draw particles");

	gl_fb_models = Cvar_Get ("gl_fb_models", "1", CVAR_ARCHIVE,
			"Toggles fullbright color support for models..  "
			"This is very handy, but costs me 2 FPS.. (=:]");
	gl_fb_bmodels = Cvar_Get ("gl_fb_bmodels", "1", CVAR_ARCHIVE,
			"Toggles fullbright color support for bmodels");

	gl_keeptjunctions = Cvar_Get("gl_keeptjunctions", "1", CVAR_NONE, "None");
	gl_reporttjunctions = Cvar_Get("gl_reporttjunctions", "0", CVAR_NONE, "None");
	
	r_skyname = Cvar_Get("r_skyname", "none", CVAR_NONE, 
			"name of the current skybox");
	gl_skymultipass = Cvar_Get("gl_skymultipass", "1", CVAR_NONE,
			"controls wether the skydome is single or double pass");

	R_InitBubble();
	
	R_InitParticles ();
	R_InitParticleTexture ();

	netgraphtexture = texture_extension_number;
	texture_extension_number++;

	playertextures = texture_extension_number;
	texture_extension_number += MAX_CLIENTS;
}

/*
===============
R_TranslatePlayerSkin

Translates a skin texture by the per-player color lookup
===============
*/
void R_TranslatePlayerSkin (int playernum)
{
	int		top, bottom;
	byte	translate[256];
	unsigned int	translate32[256];
	int		i, j;
	byte	*original;
	unsigned int	pixels[512*256], *out;
	unsigned int	scaled_width, scaled_height;
	int			inwidth, inheight;
	int			tinwidth, tinheight;
	byte		*inrow;
	unsigned int	frac, fracstep;
	player_info_t *player;
	extern	byte		player_8bit_texels[320*200];
	char s[512];

	player = &cl.players[playernum];
	if (!player->name[0])
		return;

	strcpy(s, Info_ValueForKey(player->userinfo, "skin"));
	COM_StripExtension(s, s);
	if (player->skin && !stricmp(s, player->skin->name))
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

		for (i=0 ; i<256 ; i++)
			translate[i] = i;

		for (i=0 ; i<16 ; i++)
		{
			if (top < 128)	// the artists made some backwards ranges.  sigh.
				translate[TOP_RANGE+i] = top+i;
			else
				translate[TOP_RANGE+i] = top+15-i;
					
			if (bottom < 128)
				translate[BOTTOM_RANGE+i] = bottom+i;
			else
				translate[BOTTOM_RANGE+i] = bottom+15-i;
		}

		//
		// locate the original skin pixels
		//
		// real model width
		tinwidth = 296;
		tinheight = 194;

		if (!player->skin)
			Skin_Find(player);
		if ((original = Skin_Cache(player->skin)) != NULL) {
			//skin data width
			inwidth = 320;
			inheight = 200;
		} else {
			original = player_8bit_texels;
			inwidth = 296;
			inheight = 194;
		}


		// because this happens during gameplay, do it fast
		// instead of sending it through gl_upload 8
		glBindTexture (GL_TEXTURE_2D, playertextures + playernum);

#if 0
		s = 320*200;
		byte	translated[320*200];

		for (i=0 ; i<s ; i+=4)
		{
			translated[i] = translate[original[i]];
			translated[i+1] = translate[original[i+1]];
			translated[i+2] = translate[original[i+2]];
			translated[i+3] = translate[original[i+3]];
		}


		// don't mipmap these, because it takes too long
		GL_Upload8 (translated, paliashdr->skinwidth, paliashdr->skinheight, 
			false, false, true);
#endif

		// FIXME deek: This 512x256 limit sucks!
 		scaled_width = min(gl_max_size->value, 512);
 		scaled_height = min(gl_max_size->value, 256);

		// allow users to crunch sizes down even more if they want
		scaled_width >>= (int)gl_playermip->value;
		scaled_height >>= (int)gl_playermip->value;

		if (VID_Is8bit()) { // 8bit texture upload
			byte *out2;

			out2 = (byte *)pixels;
			memset(pixels, 0, sizeof(pixels));
			fracstep = tinwidth*0x10000/scaled_width;
			for (i=0 ; i<scaled_height ; i++, out2 += scaled_width)
			{
				inrow = original + inwidth*(i*tinheight/scaled_height);
				frac = fracstep >> 1;
				for (j=0 ; j<scaled_width ; j+=4)
				{
					out2[j] = translate[inrow[frac>>16]];
					frac += fracstep;
					out2[j+1] = translate[inrow[frac>>16]];
					frac += fracstep;
					out2[j+2] = translate[inrow[frac>>16]];
					frac += fracstep;
					out2[j+3] = translate[inrow[frac>>16]];
					frac += fracstep;
				}
			}

			GL_Upload8_EXT ((byte *)pixels, scaled_width, scaled_height, false, false);
			return;
		}

		for (i=0 ; i<256 ; i++)
			translate32[i] = d_8to24table[translate[i]];

		out = pixels;
		memset(pixels, 0, sizeof(pixels));
		fracstep = tinwidth*0x10000/scaled_width;
		for (i=0 ; i<scaled_height ; i++, out += scaled_width)
		{
			inrow = original + inwidth*(i*tinheight/scaled_height);
			frac = fracstep >> 1;
			for (j=0 ; j<scaled_width ; j+=4)
			{
				out[j] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+1] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+2] = translate32[inrow[frac>>16]];
				frac += fracstep;
				out[j+3] = translate32[inrow[frac>>16]];
				frac += fracstep;
			}
		}

		glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, 
			scaled_width, scaled_height, 0, GL_RGBA, 
			GL_UNSIGNED_BYTE, pixels);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

/*
===============
R_NewMap
===============
*/
void R_NewMap (void)
{
	int		i;
	cvar_t		*r_skyname;
	
	for (i=0 ; i<256 ; i++)
		d_lightstylevalue[i] = 264;		// normal light value

	memset (&r_worldentity, 0, sizeof(r_worldentity));
	r_worldentity.model = cl.worldmodel;

// clear out efrags in case the level hasn't been reloaded
// FIXME: is this one short?
	for (i=0 ; i<cl.worldmodel->numleafs ; i++)
		cl.worldmodel->leafs[i].efrags = NULL;
		 	
	r_viewleaf = NULL;
	R_ClearParticles ();

	GL_BuildLightmaps ();

	// identify sky texture
	skytexturenum = -1;
	for (i=0 ; i<cl.worldmodel->numtextures ; i++)
	{
		if (!cl.worldmodel->textures[i])
			continue;
		if (!strncmp(cl.worldmodel->textures[i]->name,"sky",3) )
			skytexturenum = i;
 		cl.worldmodel->textures[i]->texturechain = NULL;
	}
	r_skyname = Cvar_FindVar ("r_skyname");
	if (r_skyname != NULL)
		R_LoadSkys (r_skyname->string);
	else
		R_LoadSkys ("none");
}


/*
====================
R_TimeRefresh_f

For program optimization
====================
*/
// LordHavoc: improved appearance and accuracy of timerefresh
void R_TimeRefresh_f (void)
{
	int			i;
	double		start, stop, time;

//	glDrawBuffer  (GL_FRONT);
	glFinish ();
	GL_EndRendering ();

	start = Sys_DoubleTime ();
	for (i=0 ; i<128 ; i++)
	{
		GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
		r_refdef.viewangles[1] = i/128.0*360.0;
		R_RenderView ();
		glFinish ();
		GL_EndRendering ();
	}

//	glFinish ();
	stop = Sys_DoubleTime ();
	time = stop-start;
	Con_Printf ("%f seconds (%f fps)\n", time, 128/time);

//	glDrawBuffer  (GL_BACK);
//	GL_EndRendering ();
	GL_BeginRendering (&glx, &gly, &glwidth, &glheight);
}

void D_FlushCaches (void)
{
}


