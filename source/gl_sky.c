/*
	gl_sky.c

	sky polygons

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

#include <string.h>

#include "console.h"
#include "glquake.h"
#include "tga.h"

extern double realtime;
extern model_t *loadmodel;

extern int  skytexturenum;
extern qboolean lighthalf;

int         solidskytexture;
int         alphaskytexture;
float       speedscale;					// for top sky and bottom sky

// Set to true if a valid skybox is loaded --KB
qboolean    skyloaded = false;

msurface_t *warpface;


/*
	Quake 2 environment sky
*/

#define	SKY_TEX		2000

/*
	R_LoadSkys
*/
char       *suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };
void
R_LoadSkys (char *skyname)
{
	int         i;
	QFile      *f;
	char        name[64];

	if (stricmp (skyname, "none") == 0) {
		skyloaded = false;
		return;
	}

	skyloaded = true;
	for (i = 0; i < 6; i++) {
		byte       *targa_rgba;

		glBindTexture (GL_TEXTURE_2D, SKY_TEX + i);
		snprintf (name, sizeof (name), "env/%s%s.tga", skyname, suf[i]);
		COM_FOpenFile (name, &f);
		if (!f) {
			Con_DPrintf ("Couldn't load %s\n", name);
			skyloaded = false;
			continue;
		}
		targa_rgba = LoadTGA (f);

		glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, 256, 256, 0, GL_RGBA,
					  GL_UNSIGNED_BYTE, targa_rgba);

		free (targa_rgba);

		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	if (!skyloaded)
		Con_Printf ("Unable to load skybox %s, using normal sky\n", skyname);
}

void
R_SkyBoxPolyVec (vec5_t v)
{
	// avoid interpolation seams
//  s = s * (254.0/256.0) + (1.0/256.0);
//  t = t * (254.0/256.0) + (1.0/256.0);
	glTexCoord2fv (v);
	glVertex3f (r_refdef.vieworg[0] + v[2],
				r_refdef.vieworg[1] + v[3], r_refdef.vieworg[2] + v[4]);
}

#define ftc(x) (x * (254.0/256.0) + (1.0/256.0))

vec5_t      skyvec[6][4] = {
	{
	 // right +y
	 {ftc (1), ftc (0), 1024, 1024, 1024},
	 {ftc (1), ftc (1), 1024, 1024, -1024},
	 {ftc (0), ftc (1), -1024, 1024, -1024},
	 {ftc (0), ftc (0), -1024, 1024, 1024}
	 },
	{
	 // back -x
	 {ftc (1), ftc (0), -1024, 1024, 1024},
	 {ftc (1), ftc (1), -1024, 1024, -1024},
	 {ftc (0), ftc (1), -1024, -1024, -1024},
	 {ftc (0), ftc (0), -1024, -1024, 1024}
	 },
	{
	 // left -y
	 {ftc (1), ftc (0), -1024, -1024, 1024},
	 {ftc (1), ftc (1), -1024, -1024, -1024},
	 {ftc (0), ftc (1), 1024, -1024, -1024},
	 {ftc (0), ftc (0), 1024, -1024, 1024}
	 },
	{
	 // front +x
	 {ftc (1), ftc (0), 1024, -1024, 1024},
	 {ftc (1), ftc (1), 1024, -1024, -1024},
	 {ftc (0), ftc (1), 1024, 1024, -1024},
	 {ftc (0), ftc (0), 1024, 1024, 1024}
	 },
	{
	 // up +z
	 {ftc (1), ftc (0), 1024, -1024, 1024},
	 {ftc (1), ftc (1), 1024, 1024, 1024},
	 {ftc (0), ftc (1), -1024, 1024, 1024},
	 {ftc (0), ftc (0), -1024, -1024, 1024}
	 },
	{
	 // down -z
	 {ftc (1), ftc (0), 1024, 1024, -1024},
	 {ftc (1), ftc (1), 1024, -1024, -1024},
	 {ftc (0), ftc (1), -1024, -1024, -1024},
	 {ftc (0), ftc (0), -1024, 1024, -1024}
	 }
};

#undef ftc

void
R_DrawSkyBox (void)
{
	int         i, j;

	glDisable (GL_DEPTH_TEST);
	glDepthRange (gldepthmax, gldepthmax);
	for (i = 0; i < 6; i++) {
		glBindTexture (GL_TEXTURE_2D, SKY_TEX + i);
		glBegin (GL_QUADS);
		for (j = 0; j < 4; j++)
			R_SkyBoxPolyVec (skyvec[i][j]);
		glEnd ();
	}

	glEnable (GL_DEPTH_TEST);
	glDepthRange (gldepthmin, gldepthmax);
}


vec3_t      domescale;
void
R_DrawSkyLayer (float s)
{
	int         a, b;
	float       x, y, a1x, a1y, a2x, a2y;
	vec3_t      v;

	for (a = 0; a < 16; a++) {
		a1x = bubble_costable[a * 2];
		a1y = -bubble_sintable[a * 2];
		a2x = bubble_costable[(a + 1) * 2];
		a2y = -bubble_sintable[(a + 1) * 2];

		glBegin (GL_TRIANGLE_STRIP);
		glTexCoord2f (0.5 + s * (1.0 / 128.0), 0.5 + s * (1.0 / 128.0));
		glVertex3f (r_refdef.vieworg[0],
					r_refdef.vieworg[1], r_refdef.vieworg[2] + domescale[2]);
		for (b = 1; b < 8; b++) {
			x = bubble_costable[b * 2 + 16];
			y = -bubble_sintable[b * 2 + 16];

			v[0] = a1x * x * domescale[0];
			v[1] = a1y * x * domescale[1];
			v[2] = y * domescale[2];
			glTexCoord2f ((v[0] + s) * (1.0 / 128.0),
						  (v[1] + s) * (1.0 / 128.0));
			glVertex3f (v[0] + r_refdef.vieworg[0],
						v[1] + r_refdef.vieworg[1], v[2] + r_refdef.vieworg[2]);

			v[0] = a2x * x * domescale[0];
			v[1] = a2y * x * domescale[1];
			v[2] = y * domescale[2];
			glTexCoord2f ((v[0] + s) * (1.0 / 128.0),
						  (v[1] + s) * (1.0 / 128.0));
			glVertex3f (v[0] + r_refdef.vieworg[0],
						v[1] + r_refdef.vieworg[1], v[2] + r_refdef.vieworg[2]);
		}
		glTexCoord2f (0.5 + s * (1.0 / 128.0), 0.5 + s * (1.0 / 128.0));
		glVertex3f (r_refdef.vieworg[0],
					r_refdef.vieworg[1], r_refdef.vieworg[2] - domescale[2]);
		glEnd ();
	}
}


void
R_DrawSkyDome (void)
{
	glDisable (GL_DEPTH_TEST);
	glDepthRange (gldepthmax, gldepthmax);

	glDisable (GL_BLEND);

	// base sky
	glBindTexture (GL_TEXTURE_2D, solidskytexture);
	domescale[0] = 512;
	domescale[1] = 512;
	domescale[2] = 128;
	speedscale = realtime * 8;
	speedscale -= (int) speedscale & ~127;
	R_DrawSkyLayer (speedscale);
	glEnable (GL_BLEND);

	// clouds
	if (gl_skymultipass->int_val) {
		glBindTexture (GL_TEXTURE_2D, alphaskytexture);
		domescale[0] = 512;
		domescale[1] = 512;
		domescale[2] = 128;
		speedscale = realtime * 16;
		speedscale -= (int) speedscale & ~127;
		R_DrawSkyLayer (speedscale);
	}

	glEnable (GL_DEPTH_TEST);
	glDepthRange (gldepthmin, gldepthmax);
}

void
R_DrawSky (void)
{
	if (skyloaded)
		R_DrawSkyBox ();
	else
		R_DrawSkyDome ();
}

/*
	determine_face

	return the face of the cube which v hits first
	0	+x
	1	+y
	2	+z
	3	-x
	4	-y
	5	-z
*/
static int
determine_face (vec3_t v)
{
	float a[3];
	float m;
	int i=0;

	m = a[0] = fabs (v[0]);
	a[1] = fabs (v[1]);
	a[2] = fabs (v[2]);
	if (a[1] > m) {
		m = a[1];
		i = 1;
	}
	if (a[2] > m) {
		m = a[2];
		i = 2;
	}
	if (!m)
		abort();
	if (v[i] < 0)
		i += 3;
	VectorScale (v, 1024/m, v);
	return i;
}

/*
	find_intersect (for want of a better name)

	finds the point of intersection of the plane formed by the eye and the two
	points on the cube and the edge of the cube defined by the two faces.
	Currently, this will break if the two points are not on adjoining cube
	faces (ie either on opposing faces or the same face).

	The equation for the point of intersection of a line and a plane is:

		        (x - p).n
		y = x - _________ v
		           v.n

	where n is the normal to the plane, p is a point on the plane, x is a
	point on the line, and v is the direction vector of the line. n is found
	by (x1 - e) cross (x2 - e) and p is taken to be e (e = eye coords) for
	simplicity. However, because e is at 0,0,0, this simplifies to n = x1
	cross x2 and p = 0,0,0, so the equation above simplifies to:

		        x.n
		y = x - ___ v
		        v.n
*/
static void
find_intersect (int face1, vec3_t x1, int face2, vec3_t x2, vec3_t y)
{
	vec3_t n;					// normal to the plane formed by the eye and
								// the two points on the cube.
	vec3_t x = {0, 0, 0};		// point on cube edge of adjoining faces.
								// always on an axis plane.
	vec3_t v = {0, 0, 0};		// direction vector of cube edge. always +ve
	vec_t x_n, v_n;				// x.n and v.n
	vec3_t t;

	x[face1 % 3] = 1024 * (1 - 2 * (face1 / 3));
	x[face2 % 3] = 1024 * (1 - 2 * (face2 / 3));

	v[3 - ((face1 % 3) + (face2 % 3))] = 1;

	CrossProduct (x1, x2, n);

	x_n = DotProduct (x, n);
	v_n = DotProduct (v, n);
	VectorScale (v, x_n / v_n, t);
	VectorSubtract (x, t, y);
}

struct box_def {
	int			tex, enter, leave;
	glpoly_t	poly;
	float		verts[32][VERTEXSIZE];
};

static void
set_vertex (struct box_def *box, vec3_t v, int face)
{
	int ind = box[face].poly.numverts++;
	VectorCopy (v, box[face].poly.verts[ind]);
	VectorAdd (v, r_refdef.vieworg, box[face].poly.verts[ind]);
	switch (face) {
	case 0:
		box[face].poly.verts[ind][3] = (1024 - v[1]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[2]) / 2048;
		break;
	case 1:
		box[face].poly.verts[ind][3] = (1024 + v[0]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[2]) / 2048;
		break;
	case 2:
		box[face].poly.verts[ind][3] = (1024 + v[0]) / 2048;
		box[face].poly.verts[ind][4] = (1024 + v[1]) / 2048;
		break;
	case 3:
		box[face].poly.verts[ind][3] = (1024 + v[1]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[2]) / 2048;
		break;
	case 4:
		box[face].poly.verts[ind][3] = (1024 - v[0]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[2]) / 2048;
		break;
	case 5:
		box[face].poly.verts[ind][3] = (1024 - v[0]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[1]) / 2048;
		break;
	}
}

/*
	find_cube_vertex

	get the coords of the vertex common to the three specified faces of the
	cube. NOTE: this WILL break if the three faces do not share a common
	vertex.
*/
static void
find_cube_vertex (int face1, int face2, int face3, vec3_t v)
{
	v[face1 % 3] = 1024 * (1 - 2 * (face1 / 3));
	v[face2 % 3] = 1024 * (1 - 2 * (face2 / 3));
	v[face3 % 3] = 1024 * (1 - 2 * (face3 / 3));
}

static void
enter_face (struct box_def *box, int prev_face, int face)
{
	if (box[face].leave >=0 && (box[face].leave % 3) != (prev_face % 3)) {
		vec3_t t;
		find_cube_vertex (prev_face, face, box[face].leave, t);
		set_vertex(box, t, face);
		box[face].enter = -1;
	} else {
		box[face].enter = prev_face;
	}
	box[face].leave = -1;
}

static void
leave_face (struct box_def *box, int prev_face, int face)
{
	if (box[prev_face].enter >=0 && (box[prev_face].enter) % 3 != (face % 3)) {
		vec3_t t;
		find_cube_vertex (prev_face, face, box[prev_face].enter, t);
		set_vertex(box, t, prev_face);
		box[prev_face].leave = -1;
	} else {
		box[prev_face].leave = face;
	}
	box[prev_face].enter = -1;
}

static void
render_box (struct box_def *box)
{
	int i,j;

	for (i = 0; i < 6; i++) {
		if (box[i].poly.numverts < 2)
			continue;
		glBindTexture (GL_TEXTURE_2D, box[i].tex);
		glBegin (GL_POLYGON);
		for (j=0; j < box[i].poly.numverts; j++) {
			glTexCoord2fv (box[i].poly.verts[j]+3);
			glVertex3fv (box[i].poly.verts[j]);
		}
		glEnd ();
	}
}

void
R_DrawSkyBoxPoly (glpoly_t *poly)
{
	static int skytex_offs[] = {3, 0, 4, 1, 2, 5};
	vec3_t v, last_v;
	vec3_t center = {0, 0, 0};
	struct box_def box[6];
	int i;
	int face, prev_face, c_face;

	memset (box, 0, sizeof (box));
	for (i = 0; i < 6; i++) {
		box[i].tex = SKY_TEX + skytex_offs[i];
		box[i].enter = box[i].leave = -1;
	}

	if (poly->numverts>=32) {
		Con_Printf ("too many verts!");
		abort();
	}

	for (i = 0; i < poly->numverts; i++) {
		VectorAdd (poly->verts[i], center, center);
		VectorSubtract (center, r_refdef.vieworg, center);
	}
	VectorScale (center, 1.0/poly->numverts, center);
	c_face = determine_face (center);

	VectorSubtract (poly->verts[poly->numverts - 1], r_refdef.vieworg, v);
	prev_face = determine_face (v);
	VectorCopy (v, last_v);

	for (i=0; i< poly->numverts; i++) {
		VectorSubtract (poly->verts[i], r_refdef.vieworg, v);
		face = determine_face (v);
		if (face != prev_face) {
			if ((face % 3) == (prev_face % 3)) {
				vec3_t l, x;
				int x_face;

				VectorAdd (v, last_v, x);
				VectorScale (x, 0.5, x);

				x_face = determine_face (x);

				find_intersect (prev_face, last_v, x_face, v, l);

				set_vertex(box, l, prev_face);
				leave_face (box, prev_face, x_face);
				enter_face (box, prev_face, x_face);

				find_intersect (x_face, last_v, face, v, l);

				leave_face (box, x_face, face);
				enter_face (box, x_face, face);
				set_vertex(box, l, face);
			} else {
				vec3_t l;
				find_intersect (prev_face, last_v, face, v, l);

				set_vertex(box, l, prev_face);
				leave_face (box, prev_face, face);
				enter_face (box, prev_face, face);
				set_vertex(box, l, face);
			}
		}
		set_vertex(box, v, face);

		VectorCopy (v, last_v);
		prev_face = face;
	}

	if (box[c_face].poly.numverts == 0) {
		vec3_t v;
		find_cube_vertex (c_face, c_face + 1, c_face + 2, v);
		set_vertex(box, v, c_face);
		set_vertex(box, v, (c_face + 1) % 6);
		set_vertex(box, v, (c_face + 2) % 6);
		find_cube_vertex (c_face, c_face + 2, c_face + 4, v);
		set_vertex(box, v, c_face);
		set_vertex(box, v, (c_face + 2) % 6);
		set_vertex(box, v, (c_face + 4) % 6);
		find_cube_vertex (c_face, c_face + 4, c_face + 5, v);
		set_vertex(box, v, c_face);
		set_vertex(box, v, (c_face + 4) % 6);
		set_vertex(box, v, (c_face + 5) % 6);
		find_cube_vertex (c_face, c_face + 5, c_face + 1, v);
		set_vertex(box, v, c_face);
		set_vertex(box, v, (c_face + 5) % 6);
		set_vertex(box, v, (c_face + 1) % 6);
	}

	render_box (box);
}

void
R_DrawSkyDomePoly (glpoly_t *poly)
{
	int i;

	glDisable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glColor3f (0, 0, 0);
	glBegin (GL_POLYGON);
	for (i=0; i<poly->numverts; i++) {
		glVertex3fv (poly->verts[i]);
	}
	glEnd ();
	glEnable (GL_TEXTURE_2D);
	glEnable (GL_BLEND);
}

void
R_DrawSkyChain (msurface_t *sky_chain)
{
	msurface_t *sc = sky_chain;
	if (skyloaded) {
		while (sc) {
			glpoly_t *p = sc->polys;
			while (p) {
				R_DrawSkyBoxPoly (p);
				p = p->next;
			}
			sc = sc->texturechain;
		}
	} else {
		while (sc) {
			glpoly_t *p = sc->polys;
			while (p) {
				R_DrawSkyDomePoly (p);
				p = p->next;
			}
			sc = sc->texturechain;
		}
	}
#if 1
	glDisable (GL_TEXTURE_2D);
	glColor3f (1, 1, 1);
	while (sky_chain) {
		glpoly_t *p = sky_chain->polys;
		while (p) {
			int i;
			glBegin (GL_LINE_LOOP);
			for (i=0; i<p->numverts; i++) {
				glVertex3fv (p->verts[i]);
			}
			glEnd();
			p = p->next;
		}
		sky_chain = sky_chain->texturechain;
	}
	if (skyloaded) {
		int i,j;
		glColor3f (1, 0, 0);
		for (i=0; i<6; i++) {
			vec3_t v;
			glBegin (GL_LINE_LOOP);
			for (j=0; j<4; j++) {
				memcpy (v, &skyvec[i][j][2], sizeof(v));
				VectorAdd (v, r_refdef.vieworg, v);
				glVertex3fv (v);
			}
			glEnd ();
		}
	}
	glColor3ubv (lighthalf_v);
	glEnable (GL_TEXTURE_2D);
#endif
}


//===============================================================

/*
	R_InitSky

	A sky texture is 256*128, with the right side being a masked overlay
*/
void
R_InitSky (texture_t *mt)
{
	int         i, j, p;
	byte       *src;
	unsigned int trans[128 * 128];
	unsigned int transpix;
	int         r, g, b;
	unsigned int *rgba;

	src = (byte *) mt + mt->offsets[0];

	// make an average value for the back to avoid
	// a fringe on the top level

	r = g = b = 0;
	for (i = 0; i < 128; i++)
		for (j = 0; j < 128; j++) {
			p = src[i * 256 + j + 128];
			rgba = &d_8to24table[p];
			trans[(i * 128) + j] = *rgba;
			r += ((byte *) rgba)[0];
			g += ((byte *) rgba)[1];
			b += ((byte *) rgba)[2];
		}

	((byte *) & transpix)[0] = r / (128 * 128);
	((byte *) & transpix)[1] = g / (128 * 128);
	((byte *) & transpix)[2] = b / (128 * 128);
	((byte *) & transpix)[3] = 0;


	if (!solidskytexture)
		solidskytexture = texture_extension_number++;
	glBindTexture (GL_TEXTURE_2D, solidskytexture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_solid_format, 128, 128, 0, GL_RGBA,
				  GL_UNSIGNED_BYTE, trans);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	for (i = 0; i < 128; i++)
		for (j = 0; j < 128; j++) {
			p = src[i * 256 + j];
			if (p == 0)
				trans[(i * 128) + j] = transpix;
			else
				trans[(i * 128) + j] = d_8to24table[p];
		}

	if (!alphaskytexture)
		alphaskytexture = texture_extension_number++;
	glBindTexture (GL_TEXTURE_2D, alphaskytexture);
	glTexImage2D (GL_TEXTURE_2D, 0, gl_alpha_format, 128, 128, 0, GL_RGBA,
				  GL_UNSIGNED_BYTE, trans);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
