/*
	gl_sky_clip.c

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
#include "sys.h"

extern qboolean    skyloaded;
extern vec5_t      skyvec[6][4];

/* cube face to sky texture offset conversion */
static const int skytex_offs[] = {3, 0, 4, 1, 2, 5};
/* clockwise loop through the cube faces adjoining the current face */
static const int face_loop[6][5] = {
	{1, 2, 4, 5, 1},
	{0, 5, 3, 2, 0},
	{0, 1, 3, 4, 0},
	{1, 5, 4, 2, 1},
	{0, 2, 3, 5, 0},
	{0, 4, 3, 1, 0},
};
/* convert axis and face distance into face*/
static const int faces_table[3][6] = {
	{-1, 0, 0, -1, 3, 3},
	{-1, 4, 4, -1, 1, 1},
	{-1, 2, 2, -1, 5, 5},
};
/* axis the cube face cuts (also index into vec3_t for) */
static const int face_axis[] = {0, 1, 2, 0, 1, 2};
/* offset on the axis the cube face cuts */
static const vec_t face_offset[] = {1024, 1024, 1024, -1024, -1024, -1024};

/* our cube */
struct box_def {
	int			tex;					// texture to bind to
	int			enter_face;				// cube face this face was entered from
	int			leave_face;				// cube face departed to
	int			enter_vertex;			// vertex number entered on
	int			leave_vertex;			// vertex number left on
	glpoly_t	poly;					// describe the polygon of this face
	float		verts[32][VERTEXSIZE];
};

/*
	determine_face

	return the face of the cube which v hits first
	0	+x
	1	+y
	2	+z
	3	-x
	4	-y
	5	-z
	Also scales v so it touches that face.
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
	if (!m) {
		Sys_Error ("%s speared by sky poly edge\n", name->string);
	}
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
static int
find_intersect (int face1, vec3_t x1, int face2, vec3_t x2, vec3_t y)
{
	vec3_t n;					// normal to the plane formed by the eye and
								// the two points on the cube.
	vec3_t x = {0, 0, 0};		// point on cube edge of adjoining faces.
								// always on an axis plane.
	vec3_t v = {0, 0, 0};		// direction vector of cube edge. always +ve
	vec_t x_n, v_n;				// x.n and v.n
	int axis;
	vec3_t t;

	x[face1 % 3] = 1024 * (1 - 2 * (face1 / 3));
	x[face2 % 3] = 1024 * (1 - 2 * (face2 / 3));

	axis = 3 - ((face1 % 3) + (face2 % 3));
	v[axis] = 1;

	CrossProduct (x1, x2, n);

	x_n = DotProduct (x, n);
	v_n = DotProduct (v, n);
	VectorScale (v, x_n / v_n, t);
	VectorSubtract (x, t, y);

	return axis;
}

/*
	set_vertex

	add the vertex to the polygon describing the face of the cube. Offsets
	the vertex relative to r_refdef.vieworg so the cube is always centered
	on the player and also calculates the texture coordinates of the vertex
	(wish I could find a cleaner way of calculating s and t).
*/
static void
set_vertex (struct box_def *box, int face, int ind, vec3_t v)
{
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
		box[face].poly.verts[ind][3] = (1024 + v[0]) / 2048;
		box[face].poly.verts[ind][4] = (1024 - v[1]) / 2048;
		break;
	}
}

/*
 	add_vertex
*/
static void
add_vertex (struct box_def *box, int face, vec3_t v)
{
	set_vertex (box, face, box[face].poly.numverts++, v);
}

/*
	find_cube_vertex

	get the coords of the vertex common to the three specified faces of the
	cube. NOTE: this WILL break if the three faces do not share a common
	vertex. ie works = ((face1 % 3 != face2 % 3)
	                    && (face2 % 3 != face3 % 3)
						&& (face1 % 3 != face3 % 3))
*/
static void
find_cube_vertex (int face1, int face2, int face3, vec3_t v)
{
	v[face1 % 3] = 1024 * (1 - 2 * (face1 / 3));
	v[face2 % 3] = 1024 * (1 - 2 * (face2 / 3));
	v[face3 % 3] = 1024 * (1 - 2 * (face3 / 3));
}

/*
	enter_face

	if we left this face on an adjoining face with a common vertex, add
	that vertex to the cube face polygon.
*/
static void
enter_face (struct box_def *box, int prev_face, int face)
{
	if (box[face].leave_face >=0 && (box[face].leave_face % 3) != (prev_face % 3)) {
		vec3_t t;
		find_cube_vertex (prev_face, face, box[face].leave_face, t);
		add_vertex(box, face, t);
		box[face].enter_face = -1;
	} else {
		box[face].enter_face = prev_face;
	}
	box[face].leave_face = -1;
}

/*
	leave_face

	if we entered this face on an adjoining face with a common vertex, add
	that vertex to the cube face polygon.
*/
static void
leave_face (struct box_def *box, int prev_face, int face)
{
	if (box[prev_face].enter_face >=0 && (box[prev_face].enter_face) % 3 != (face % 3)) {
		vec3_t t;
		find_cube_vertex (prev_face, face, box[prev_face].enter_face, t);
		add_vertex(box, prev_face, t);
		box[prev_face].leave_face = -1;
	} else {
		box[prev_face].leave_face = face;
	}
	box[prev_face].enter_face = -1;
}

/*
	render_box

	draws all faces of the cube with 3 or more vertexen.
*/
static void
render_box (struct box_def *box)
{
	int i,j;

	for (i = 0; i < 6; i++) {
		if (box[i].poly.numverts <= 2)
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

/*
	insert_cube_vertexen

	insert the given cube vertexen into the vertex list of the poly in the
	correct location.
*/
static void
insert_cube_vertexen (struct box_def *box, int face, vec3_t v1, vec3_t v2)
{
	if (box[face].leave_vertex == box[face].poly.numverts - 1) {
		// the vertex the sky poly left this cube fase through is very
		// conveniently the last vertex of the face poly. this means we
		// can just append the two vetexen
		add_vertex (box, face, v1);
		add_vertex (box, face, v2);
	} else {
		// we have to insert the two cube vertexen into the face poly
		// vertex list
		glpoly_t *p = &box[face].poly;
		int insert = box[face].leave_vertex + 1;
		int count = p->numverts - insert;
		const int vert_size = sizeof (p->verts[0]);
		memmove (p->verts[insert + 2], p->verts[insert], count * vert_size);
		p->numverts += 2;
		set_vertex (box, face, insert, v1);
		set_vertex (box, face, insert + 1, v2);
	}
}

/*
	fixup_center_face

	add the vertexen of the cube face that should be draw but was not
	clipped by the sky polygon because it was fully enclosed by the
	polygon. Also adds the missing vertexen to the surrounding cube faces.
*/
static void
fixup_center_face (struct box_def *box, int c_face)
{
	vec3_t v[4];
	int i;

	for (i = 0; i < 4; i++) {
		find_cube_vertex (c_face, face_loop[c_face][i],
						  face_loop[c_face][i + 1], v[i]);
		add_vertex(box, c_face, v[i]);
	}
	for (i = 0; i < 4; i++) {
		int ind = face_loop[c_face][i];
		insert_cube_vertexen (box, ind, v[i], v[(i - 1) & 3]);
	}
}

/*
	cross_cube_edge

	add the vertex formed by the poly edge crossing the cube edge to the
	polygon for the two faces on that edge. Actually, the two faces define
	the edge :). The poly edge is going from face 1 to face 2 (for
	enter/leave purposes).
*/
static int
cross_cube_edge (struct box_def *box, int face1, vec3_t v1, int face2,
				 vec3_t v2)
{
	vec3_t l;
	int axis;

	axis = find_intersect (face1, v1, face2, v2, l);
	if (l[axis] > 1024)
		return axis;
	else if (l[axis] < -1024)
		return axis + 3;

	box[face1].leave_vertex = box[face1].poly.numverts;
	add_vertex(box, face1, l);
	leave_face (box, face1, face2);
	enter_face (box, face1, face2);
	box[face2].enter_vertex = box[face2].poly.numverts;
	add_vertex(box, face2, l);

	return -1;
}

static void
fix_missed_vertexen (struct box_def *box, int *faces, int face_count)
{
	if (face_count == 4) {
		if (abs (faces[2] - faces[0]) == 3
			&& abs (faces[3] - faces[1]) == 3) {
			int framed_face;
			int sum, diff;
			sum = faces[0] + faces[1] + faces[2] + faces[3];
			diff = faces[1] - faces[0];
			sum %= 3;
			diff = (diff + 6) % 6;
			framed_face = faces_table[sum][diff];
			if (box[framed_face].poly.numverts == 0)
				fixup_center_face (box, framed_face);
			else
				printf ("email bill@taniwha.org re framed face > 0 verts\n");
		} else {
			int l_f, t_f, r_f, b_f;
			vec3_t v_l, v_r;

			if (abs (faces[2] - faces[0]) == 3) {
				l_f = faces[0];
				t_f = faces[1];
				r_f = faces[2];
				b_f = faces[3];
			} else if (abs (faces[3] - faces[1]) == 3) {
				l_f = faces[1];
				t_f = faces[2];
				r_f = faces[3];
				b_f = faces[0];
			} else {
				return;
			}
			find_cube_vertex (l_f, t_f, b_f, v_l);
			find_cube_vertex (r_f, t_f, b_f, v_r);

			insert_cube_vertexen (box, t_f, v_r, v_l);
			insert_cube_vertexen (box, b_f, v_l, v_r);
		}
	}
}

static void
visit_cube_face(int *visited_faces, int *faces_flags, int *face_count, int face)
{
	if (!faces_flags[face]) {
		faces_flags[face] = 1;
		visited_faces[(*face_count)++] = face;
	}
}

void
R_DrawSkyBoxPoly (glpoly_t *poly)
{
	int i;
	struct box_def box[6];
	/* projected vertex and face of the previous sky poly vertex */
	vec3_t last_v;
	int prev_face;
	/* projected vertex and face of the current sky poly vertex */
	vec3_t v;
	int face;
	/* keep track of which cube faces we visit and in what order */
	int visited_faces [6];
	int faces_flags [6];
	int face_count = 0;

	memset (box, 0, sizeof (box));
	memset (faces_flags, 0, sizeof faces_flags);
	for (i = 0; i < 6; i++) {
		box[i].tex = SKY_TEX + skytex_offs[i];
		box[i].enter_face = box[i].leave_face = -1;
	}

	if (poly->numverts>=32) {
		Sys_Error ("too many verts!");
	}

	VectorSubtract (poly->verts[poly->numverts - 1], r_refdef.vieworg, last_v);
	prev_face = determine_face (last_v);

	for (i=0; i< poly->numverts; i++) {
		VectorSubtract (poly->verts[i], r_refdef.vieworg, v);
		face = determine_face (v);
		if (face != prev_face) {
			int x_face = -1;
			if ((face % 3) == (prev_face % 3)
				|| (x_face = cross_cube_edge (box, prev_face, last_v,
											  face, v)) >= 0) {
				vec3_t x, y;
				int y_face;

				VectorAdd (v, last_v, x);
				VectorScale (x, 0.5, x);
				if (x_face == -1) {
					x_face = determine_face (x);
				}

				if ((y_face = cross_cube_edge (box, prev_face, last_v,
											   x_face, x)) >= 0) {
					VectorAdd (last_v, x, y);
					VectorScale (y, 0.5, y);
					cross_cube_edge (box, prev_face, last_v, y_face, y);
					cross_cube_edge (box, y_face, y, x_face, x);

					visit_cube_face (visited_faces, faces_flags, &face_count, y_face);
				}

				visit_cube_face (visited_faces, faces_flags, &face_count, x_face);

				if ((y_face = cross_cube_edge (box, x_face, x, face, v)) >= 0) {
					VectorAdd (x, v, y);
					VectorScale (y, 0.5, y);
					cross_cube_edge (box, x_face, x, y_face, y);
					cross_cube_edge (box, y_face, y, face, v);

					visit_cube_face (visited_faces, faces_flags, &face_count, y_face);
				}
			}
		}
		visit_cube_face (visited_faces, faces_flags, &face_count, face);
		add_vertex(box, face, v);

		VectorCopy (v, last_v);
		prev_face = face;
	}

	fix_missed_vertexen (box, visited_faces, face_count);

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
	sc = sky_chain;
	glColor3f (1, 1, 1);
	while (sc) {
		glpoly_t *p = sc->polys;
		while (p) {
			int i;
			glBegin (GL_LINE_LOOP);
			for (i=0; i<p->numverts; i++) {
				glVertex3fv (p->verts[i]);
			}
			glEnd();
			p = p->next;
		}
		sc = sc->texturechain;
	}
	sc = sky_chain;
	glColor3f (0, 1, 0);
	glBegin (GL_POINTS);
	while (sc) {
		glpoly_t *p = sc->polys;
		while (p) {
			int i;
			vec3_t x, c = {0, 0, 0};
			for (i=0; i<p->numverts; i++) {
				VectorSubtract (p->verts[i], r_refdef.vieworg, x);
				VectorAdd (x, c, c);
			}
			VectorScale (c, 1.0/p->numverts, c);
			VectorAdd (c, r_refdef.vieworg, c);
			glVertex3fv (c);
			p = p->next;
		}
		sc = sc->texturechain;
	}
	glEnd ();
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
