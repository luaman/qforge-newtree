/*
	gl_mesh.c

	gl_mesh.c: triangle model functions

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
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include <stdio.h>

#include "console.h"
#include "cvar.h"
#include "mdfour.h"
#include "model.h"
#include "quakefs.h"
#include "sys.h"

#include "compat.h"

/*
	ALIAS MODEL DISPLAY LIST GENERATION
*/

extern cvar_t *gl_mesh_cache;

model_t    *aliasmodel;
aliashdr_t *paliashdr;

qboolean   *used;
int         used_size;

// the command list holds counts and s/t values that are valid for
// every frame
int        *commands;
int         numcommands;
int         commands_size;

// all frames will have their vertexes rearranged and expanded
// so they are in the order expected by the command list
int        *vertexorder;
int         numorder;
int         vertexorder_size;

int         allverts, alltris;

int        *stripverts;
int        *striptris;
int         stripcount;
int         strip_size;

void
alloc_used (int size)
{
	if (size <= used_size)
		return;
	size = (size + 1023) & ~1023;
	used = realloc (used, size * sizeof (used[0]));
	if (!used)
		Sys_Error ("gl_mesh: out of memory");
	used_size = size;
}

void
add_command (int cmd)
{
	if (numcommands + 1 > commands_size) {
		commands_size += 1024;
		commands = realloc (commands, commands_size * sizeof (commands[0]));
		if (!commands)
			Sys_Error ("gl_mesh: out of memory");
	}
	commands[numcommands++] = cmd;
}

void
add_vertex (int vert)
{
	if (numorder + 1 > vertexorder_size) {
		vertexorder_size += 1024;
		vertexorder = realloc (vertexorder, vertexorder_size * sizeof (vertexorder[0]));
		if (!vertexorder)
			Sys_Error ("gl_mesh: out of memory");
	}
	vertexorder[numorder++] = vert;
}

void
add_strip (int vert, int tri)
{
	if (stripcount + 1 > strip_size) {
		strip_size += 1024;
		stripverts = realloc (stripverts, strip_size * sizeof (stripverts[0]));
		striptris = realloc (striptris, strip_size * sizeof (striptris[0]));
		if (!stripverts || !striptris)
			Sys_Error ("gl_mesh: out of memory");
	}
	stripverts[stripcount] = vert;
	striptris[stripcount] = tri;
	stripcount++;
}

int
StripLength (int starttri, int startv)
{
	int         m1, m2;
	int         j;
	mtriangle_t *last, *check;
	int         k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripcount = 0;
	add_strip (last->vertindex[(startv) % 3], starttri);
	add_strip (last->vertindex[(startv + 1) % 3], starttri);
	add_strip (last->vertindex[(startv + 2) % 3], starttri);

	m1 = last->vertindex[(startv + 2) % 3];
	m2 = last->vertindex[(startv + 1) % 3];

	// look for a matching triangle
nexttri:
	for (j = starttri + 1, check = &triangles[starttri + 1];
		 j < pheader->mdl.numtris; j++, check++) {
		if (check->facesfront != last->facesfront)
			continue;
		for (k = 0; k < 3; k++) {
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[(k + 1) % 3] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			if (stripcount & 1)
				m2 = check->vertindex[(k + 2) % 3];
			else
				m1 = check->vertindex[(k + 2) % 3];

			add_strip (check->vertindex[(k + 2) % 3], j);

			used[j] = 2;
			goto nexttri;
		}
	}
done:

	// clear the temp used flags
	for (j = starttri + 1; j < pheader->mdl.numtris; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount - 2;
}

int
FanLength (int starttri, int startv)
{
	int         m1, m2;
	int         j;
	mtriangle_t *last, *check;
	int         k;

	used[starttri] = 2;

	last = &triangles[starttri];

	stripcount = 0;
	add_strip (last->vertindex[(startv) % 3], starttri);
	add_strip (last->vertindex[(startv + 1) % 3], starttri);
	add_strip (last->vertindex[(startv + 2) % 3], starttri);

	m1 = last->vertindex[(startv + 0) % 3];
	m2 = last->vertindex[(startv + 2) % 3];


	// look for a matching triangle
  nexttri:
	for (j = starttri + 1, check = &triangles[starttri + 1];
		 j < pheader->mdl.numtris; j++, check++) {
		if (check->facesfront != last->facesfront)
			continue;
		for (k = 0; k < 3; k++) {
			if (check->vertindex[k] != m1)
				continue;
			if (check->vertindex[(k + 1) % 3] != m2)
				continue;

			// this is the next part of the fan

			// if we can't use this triangle, this tristrip is done
			if (used[j])
				goto done;

			// the new edge
			m2 = check->vertindex[(k + 2) % 3];

			add_strip (m2, j);

			used[j] = 2;
			goto nexttri;
		}
	}
  done:

	// clear the temp used flags
	for (j = starttri + 1; j < pheader->mdl.numtris; j++)
		if (used[j] == 2)
			used[j] = 0;

	return stripcount - 2;
}


/*
	BuildTris

	Generate a list of trifans or strips
	for the model, which holds for all frames
*/
void
BuildTris (void)
{
	int         i, j, k;
	int         startv;
	float       s, t;
	int         len, bestlen, besttype = 0;
	int        *bestverts = 0;
	int        *besttris = 0;
	int         type;

	// build tristrips
	numorder = 0;
	numcommands = 0;
	stripcount = 0;
	alloc_used (pheader->mdl.numtris);
	memset (used, 0, used_size * sizeof (used[0]));

	for (i = 0; i < pheader->mdl.numtris; i++) {
		// pick an unused triangle and start the trifan
		if (used[i])
			continue;

		bestlen = 0;
		for (type = 0; type < 2; type++)
//  type = 1;
		{
			for (startv = 0; startv < 3; startv++) {
				if (type == 1)
					len = StripLength (i, startv);
				else
					len = FanLength (i, startv);
				if (len > bestlen) {
					besttype = type;
					bestlen = len;
					if (bestverts)
						free (bestverts);
					if (besttris)
						free (besttris);
					bestverts = stripverts;
					besttris = striptris;
					stripverts = striptris = 0;
					strip_size = 0;
				}
			}
		}

		// mark the tris on the best strip as used
		for (j = 0; j < bestlen; j++)
			used[besttris[j + 2]] = 1;

		if (besttype == 1)
			add_command (bestlen + 2);
		else
			add_command (-(bestlen + 2));

		for (j = 0; j < bestlen + 2; j++) {
			// emit a vertex into the reorder buffer
			k = bestverts[j];
			add_vertex (k);

			// emit s/t coords into the commands stream
			s = stverts[k].s;
			t = stverts[k].t;
			if (!triangles[besttris[0]].facesfront && stverts[k].onseam)
				s += pheader->mdl.skinwidth / 2;	// on back side
			s = (s + 0.5) / pheader->mdl.skinwidth;
			t = (t + 0.5) / pheader->mdl.skinheight;

			add_command (*(int*)&s);
			add_command (*(int*)&t);
		}
	}

	add_command (0);					// end of list marker

	Con_DPrintf ("%3i tri %3i vert %3i cmd\n", pheader->mdl.numtris, numorder,
				 numcommands);

	allverts += numorder;
	alltris += pheader->mdl.numtris;

	if (bestverts)
		free (bestverts);
	if (besttris)
		free (besttris);
}

void
GL_MakeAliasModelDisplayLists (model_t *m, aliashdr_t *hdr, void *_m, int _s)
{
	int         i, j;
	int        *cmds;
	trivertx_t *verts;
	char        cache[MAX_QPATH], fullpath[MAX_OSPATH];
	VFile      *f;
	unsigned char model_digest[MDFOUR_DIGEST_BYTES];
	unsigned char mesh_digest[MDFOUR_DIGEST_BYTES];
	qboolean    remesh = true;
	qboolean    do_cache = false;

	aliasmodel = m;
	paliashdr = hdr;					// (aliashdr_t *)Mod_Extradata (m);

	if (gl_mesh_cache->int_val
		&& gl_mesh_cache->int_val <= paliashdr->mdl.numtris) {
		do_cache = true;

		mdfour (model_digest, (unsigned char *) _m, _s);

		// look for a cached version
		strcpy (cache, "glquake/");
		COM_StripExtension (m->name + strlen ("progs/"),
							cache + strlen ("glquake/"));
		strncat (cache, ".qfms", sizeof (cache) - strlen (cache));

		COM_FOpenFile (cache, &f);
		if (f) {
			unsigned char d1[MDFOUR_DIGEST_BYTES];
			unsigned char d2[MDFOUR_DIGEST_BYTES];
			struct mdfour md;
			int        *c = 0;
			int         nc = 0;
			int        *vo = 0;
			int         no = 0;
			int			len;
			int			vers;

			memset (d1, 0, sizeof (d1));
			memset (d2, 0, sizeof (d2));

			Qread (f, &vers, sizeof (int));
			Qread (f, &len, sizeof (int));
			Qread (f, &nc, sizeof (int));
			Qread (f, &no, sizeof (int));

			if (vers == 1 && (nc + no) == len) {
				c = malloc (((nc + 1023) & ~1023) * sizeof (c[0]));
				vo = malloc (((no + 1023) & ~1023) * sizeof (vo[0]));
				if (!c || !vo)
					Sys_Error ("gl_mesh.c: out of memory");
				Qread (f, c, nc * sizeof (c[0]));
				Qread (f, vo, no * sizeof (vo[0]));
				Qread (f, d1, MDFOUR_DIGEST_BYTES);
				Qread (f, d2, MDFOUR_DIGEST_BYTES);
				Qclose (f);

				mdfour_begin (&md);
				mdfour_update (&md, (unsigned char *) &vers, sizeof(int));
				mdfour_update (&md, (unsigned char *) &len, sizeof(int));
				mdfour_update (&md, (unsigned char *) &nc, sizeof(int));
				mdfour_update (&md, (unsigned char *) &no, sizeof(int));
				mdfour_update (&md, (unsigned char *) c, nc * sizeof (c[0]));
				mdfour_update (&md, (unsigned char *) vo, no * sizeof (vo[0]));
				mdfour_update (&md, d1, MDFOUR_DIGEST_BYTES);
				mdfour_result (&md, mesh_digest);

				if (memcmp (d2, mesh_digest, MDFOUR_DIGEST_BYTES) == 0
					&& memcmp (d1, model_digest, MDFOUR_DIGEST_BYTES) == 0) {
					remesh = false;
					numcommands = nc;
					numorder = no;
					if (numcommands > commands_size) {
						if (commands)
							free (commands);
						commands_size = (numcommands + 1023) & ~1023;
						commands = c;
					} else {
						memcpy (commands, c, numcommands * sizeof (c[0]));
						free(c);
					}
					if (numorder > vertexorder_size) {
						if (vertexorder)
							free (vertexorder);
						vertexorder_size = (numorder + 1023) & ~1023;
						vertexorder = vo;
					} else {
						memcpy (vertexorder, vo, numorder * sizeof (vo[0]));
						free (vo);
					}
				}
			}
		}
	}
	if (remesh) {
		// build it from scratch
		Con_DPrintf ("meshing %s...\n", m->name);

		BuildTris ();					// trifans or lists

		if (do_cache) {
			// save out the cached version
			snprintf (fullpath, sizeof (fullpath), "%s/%s", com_gamedir, cache);
			f = Qopen (fullpath, "wbz9");
			if (!f) {
				COM_CreatePath (fullpath);
				f = Qopen (fullpath, "wb");
			}

			if (f) {
				struct mdfour md;
				int         vers = 1;
				int         len = numcommands + numorder;

				mdfour_begin (&md);
				mdfour_update (&md, (unsigned char *) &vers, sizeof (int));
				mdfour_update (&md, (unsigned char *) &len, sizeof (int));
				mdfour_update (&md, (unsigned char *) &numcommands, sizeof (int));
				mdfour_update (&md, (unsigned char *) &numorder, sizeof (int));
				mdfour_update (&md, (unsigned char *) commands,
							   numcommands * sizeof (commands[0]));
				mdfour_update (&md, (unsigned char *) vertexorder,
							   numorder * sizeof (vertexorder[0]));
				mdfour_update (&md, model_digest, MDFOUR_DIGEST_BYTES);
				mdfour_result (&md, mesh_digest);

				Qwrite (f, &vers, sizeof (int));
				Qwrite (f, &len, sizeof (int));
				Qwrite (f, &numcommands, sizeof (int));
				Qwrite (f, &numorder, sizeof (int));
				Qwrite (f, commands, numcommands * sizeof (commands[0]));
				Qwrite (f, vertexorder, numorder * sizeof (vertexorder[0]));
				Qwrite (f, model_digest, MDFOUR_DIGEST_BYTES);
				Qwrite (f, mesh_digest, MDFOUR_DIGEST_BYTES);
				Qclose (f);
			}
		}
	}

	// save the data out
	paliashdr->poseverts = numorder;

	cmds = Hunk_Alloc (numcommands * sizeof (int));
	paliashdr->commands = (byte *) cmds - (byte *) paliashdr;
	memcpy (cmds, commands, numcommands * sizeof (int));

	verts = Hunk_Alloc (paliashdr->numposes * paliashdr->poseverts

						* sizeof (trivertx_t));
	paliashdr->posedata = (byte *) verts - (byte *) paliashdr;
	for (i = 0; i < paliashdr->numposes; i++)
		for (j = 0; j < numorder; j++)
			*verts++ = poseverts[i][vertexorder[j]];
}
