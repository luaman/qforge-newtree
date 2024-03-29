/*
	glquake.h

	OpenGL-specific definitions and prototypes

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

#ifndef _GLQUAKE_H
#define _GLQUAKE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif

#include <GL/gl.h>

#include "client.h"
#include "cvar.h"
#include "model.h"
#include "render.h"
#include "qfgl_ext.h"
#include "wad.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

extern int		texture_extension_number;
extern int		texture_mode;

extern float	gldepthmin, gldepthmax;

void GL_Upload8 (byte *data, int width, int height,  qboolean mipmap, qboolean alpha);
void GL_Upload8_EXT (byte *data, int width, int height,  qboolean mipmap, qboolean alpha);
int GL_LoadTexture (char *identifier, int width, int height, byte *data, qboolean mipmap, qboolean alpha, int bytesperpixel);

typedef struct {
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;

extern glvert_t glv;

extern	int glx, gly, glwidth, glheight;

// r_local.h -- private refresh defs

#define ALIAS_BASE_SIZE_RATIO		(1.0 / 11.0)
					// normalizing factor so player model works out to about
					//  1 pixel per triangle

#define	MAX_LBM_HEIGHT	480

#define MAX_GLTEXTURES	2048

#define TILE_SIZE		128		// size of textures generated by R_GenTiledSurf

#define SKYSHIFT		7
#define	SKYSIZE			(1 << SKYSHIFT)
#define SKYMASK			(SKYSIZE - 1)
#define SKY_TEX			2000	// Quake 2 environment sky

#define BACKFACE_EPSILON	0.01


void R_TimeRefresh_f (void);
void R_ReadPointFile_f (void);
texture_t *R_TextureAnimation (texture_t *base);

typedef struct surfcache_s {
	struct surfcache_s	*next;
	struct surfcache_s 	**owner;		// NULL is an empty chunk of memory
	int					lightadj[MAXLIGHTMAPS]; // checked for strobe flush
	int					dlight;
	int					size;		// including header
	unsigned int		width;
	unsigned int		height;		// DEBUG only needed for debug
	float				mipscale;
	struct texture_s	*texture;	// checked for animating textures
	byte				data[4];	// width*height elements
} surfcache_t;


//====================================================


extern	entity_t	r_worldentity;
extern	qboolean	r_cache_thrash;		// compatability
extern	vec3_t		modelorg, r_entorigin;
extern	entity_t	*currententity;
extern	int			r_visframecount;	// ??? what difs?
extern	int			r_framecount;
extern	mplane_t	frustum[4];
extern	int		c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_refdef;
extern	mleaf_t		*r_viewleaf, *r_oldviewleaf;
extern	texture_t	*r_notexture_mip;
extern	int		d_lightstylevalue[256];	// 8.8 fraction of base light value

extern	qboolean	envmap;
extern	int	netgraphtexture;	// netgraph texture
extern	int	playertextures;
extern	int	player_fb_textures;

extern	int	skytexturenum;		// index in cl.loadmodel, not gl texture object

extern cvar_t	*r_norefresh;
extern cvar_t	*r_drawentities;
extern cvar_t	*r_drawworld;
extern cvar_t	*r_drawviewmodel;
extern cvar_t	*r_particles;
extern cvar_t	*r_speeds;
extern cvar_t	*r_waterwarp;
extern cvar_t	*r_shadows;
extern cvar_t	*r_wateralpha;
extern cvar_t	*r_waterripple;
extern cvar_t	*r_dynamic;
extern cvar_t	*r_novis;
extern cvar_t	*r_netgraph;

extern cvar_t	*gl_affinemodels;
extern cvar_t	*gl_clear;
extern cvar_t	*gl_cull;
extern cvar_t	*gl_fb_bmodels;
extern cvar_t	*gl_fb_models;
extern cvar_t   *gl_dlight_lightmap;
extern cvar_t	*gl_dlight_polyblend;
extern cvar_t	*gl_dlight_smooth;
extern cvar_t	*gl_keeptjunctions;
extern cvar_t	*gl_multitexture;
extern cvar_t	*gl_nocolors;
extern cvar_t	*gl_poly;
extern cvar_t	*gl_polyblend;

extern cvar_t	*gl_max_size;
extern cvar_t	*gl_playermip;

extern cvar_t	*r_skyname;
extern cvar_t	*gl_skymultipass;
extern cvar_t	*gl_sky_clip;
extern cvar_t	*gl_sky_divide;

extern int		gl_lightmap_format;
extern int		gl_solid_format;
extern int		gl_alpha_format;

extern float	r_world_matrix[16];

extern const char *gl_vendor;
extern const char *gl_renderer;
extern const char *gl_version;
extern const char *gl_extensions;

void R_TranslatePlayerSkin (int playernum);

// Multitexturing
extern QF_glActiveTextureARB	qglActiveTexture;
extern QF_glMultiTexCoord2fARB	qglMultiTexCoord2f;
extern qboolean 				gl_mtex_capable;
extern GLenum					gl_mtex_enum;
// convenience check
#define gl_mtex_active	(gl_mtex_capable && gl_multitexture->int_val)

void GL_DisableMultitexture (void);
void GL_EnableMultitexture (void);
void GL_SelectTexture (GLenum target);

//
// gl_rpart.c
//
typedef struct {
	int		key;                    // allows reusability
	vec3_t	origin, owner;
	float	size;
	float	die, decay;             // duration settings
	float	minlight;               // lighting threshold
	float	color[3];              // !RGBA
} fire_t;

void R_AddFire (vec3_t, vec3_t, entity_t *ent);
fire_t *R_AllocFire (int);
void R_DrawFire (fire_t *);
void R_UpdateFires (void);

//
// gl_warp.c
//
void GL_SubdivideSurface (msurface_t *fa);
void EmitBothSkyLayers (msurface_t *fa);
void EmitWaterPolys (msurface_t *fa);
void EmitSkyPolys (msurface_t *fa);
void R_DrawSkyChain (msurface_t *s);
void R_LoadSkys (char *);
void R_DrawSky (void);
void R_DrawSkyChain (msurface_t *sky_chain);

//
// gl_draw.c
//
void GL_Set2D (void);

//
// gl_rmain.c
//
//qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_RotateForEntity (entity_t *e);

static inline qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
	int i;

	for (i=0 ; i<4 ; i++)
		if (BoxOnPlaneSide (mins, maxs, &frustum[i]) == 2)
			return true;
	return false;
}


//
// gl_rlight.c
//

extern float bubble_sintable[], bubble_costable[];
extern float v_blend[4];

void R_MarkLights (vec3_t lightorigin, dlight_t *light, int bit, mnode_t *node);
void R_AnimateLight (void);
void R_RenderDlights (void);
int R_LightPoint (vec3_t p);
void AddLightBlend (float, float, float, float);

//
// gl_refrag.c
//
void R_StoreEfrags (efrag_t **ppefrag);

//
// gl_screen.c
//

extern qboolean lighthalf;
extern unsigned char lighthalf_v[3];

//
// gl_rsurf.c
//
void R_DrawBrushModel (entity_t *e);
void R_DrawWorld (void);
void GL_BuildLightmaps (void);

//
// gl_ngraph.c
//
void R_NetGraph (void);

#endif // _GLQUAKE_H
