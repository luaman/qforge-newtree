/*
	gl_dyn_part.c

	Particle system!

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

#include <stdlib.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "cmd.h"
#include "console.h"
#include "glquake.h"
#include "host.h"
#include "r_dynamic.h"
#include "qargs.h"
#include "sys.h"

#define MAX_PARTICLES			2048	// default max # of particles at one
										// time
#define ABSOLUTE_MIN_PARTICLES	512		// no fewer than this no matter
										// what's
										// on the command line

typedef enum {
	pt_static, pt_grav, pt_blob, pt_blob2,
	pt_smoke, pt_smokecloud, pt_bloodcloud,
	pt_fadespark, pt_fadespark2, pt_fallfadespark
} ptype_t;

typedef struct particle_s {
	// driver-usable fields
	vec3_t      org;
	int         tex;
	float       color;
	float       alpha;
	float       scale;
	// drivers never touch the following fields
	struct particle_s *next;
	vec3_t      vel;
	float       ramp;
	float       die;
	ptype_t     type;
} particle_t;


static particle_t *particles, **freeparticles;
static short r_numparticles, numparticles;

extern qboolean lighthalf;

extern void GDT_Init (void);
extern int  part_tex_smoke[8];
extern int  part_tex_dot;

int         ramp[8] = { 0x6d, 0x6b, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01 };

inline particle_t *
particle_new (ptype_t type, int texnum, vec3_t org, float scale, vec3_t vel,
			  float die, byte color, byte alpha)
{
	particle_t *part;

	if (numparticles >= r_numparticles) {
		// Con_Printf("FAILED PARTICLE ALLOC!\n");
		return NULL;
	}

	part = &particles[numparticles++];

	part->type = type;
	VectorCopy (org, part->org);
	VectorCopy (vel, part->vel);
	part->die = die;
	part->color = color;
	part->alpha = alpha;
	part->tex = texnum;
	part->scale = scale;

	return part;
}

inline particle_t *
particle_new_random (ptype_t type, int texnum, vec3_t org, int org_fuzz,
					 float scale, int vel_fuzz, float die, byte color,
					 byte alpha)
{
	vec3_t      porg, pvel;
	int         j;

	for (j = 0; j < 3; j++) {
		if (org_fuzz)
			porg[j] = lhrandom (-org_fuzz, org_fuzz) + org[j];
		if (vel_fuzz)
			pvel[j] = lhrandom (-vel_fuzz, vel_fuzz);
	}
	return particle_new (type, texnum, porg, scale, pvel, die, color, alpha);
}

/*
===============
R_InitParticles
===============
*/
void
R_InitParticles (void)
{
	int         i;

	i = COM_CheckParm ("-particles");

	if (i) {
		r_numparticles = max (ABSOLUTE_MIN_PARTICLES, atoi (com_argv[i + 1]));
	} else {
		r_numparticles = MAX_PARTICLES;
	}

	particles = (particle_t *)
		Hunk_AllocName (r_numparticles * sizeof (particle_t), "particles");

	freeparticles = (void *)
		Hunk_AllocName (r_numparticles * sizeof (particle_t), "particles");


	GDT_Init ();
}


/*
===============
R_ClearParticles
===============
*/
void
R_ClearParticles (void)
{
	numparticles = 0;
}

void
R_ReadPointFile_f (void)
{
	QFile      *f;
	vec3_t      org;
	int         r;
	int         c;
	char        name[MAX_OSPATH], *mapname, *t1;

	mapname = strdup (cl.worldmodel->name);
	if (!mapname)
		Sys_Error ("Can't duplicate mapname!");
	t1 = strrchr (mapname, '.');
	if (!t1)
		Sys_Error ("Can't find .!");
	t1[0] = '\0';

	snprintf (name, sizeof (name), "%s.pts", mapname);
	free (mapname);

	COM_FOpenFile (name, &f);
	if (!f) {
		Con_Printf ("couldn't open %s\n", name);
		return;
	}

	Con_Printf ("Reading %s...\n", name);
	c = 0;
	for (;;) {
		char        buf[64];

		Qgets (f, buf, sizeof (buf));
		r = sscanf (buf, "%f %f %f\n", &org[0], &org[1], &org[2]);
		if (r != 3)
			break;
		c++;

		if (!particle_new (pt_static, part_tex_dot, org, 2, vec3_origin, 99999,
						   (-c) & 15, 255)) {
			Con_Printf ("Not enough free particles\n");
			break;
		}
	}

	Qclose (f);
	Con_Printf ("%i points read\n", c);
}

/*
===============
R_ParticleExplosion

===============
*/
void
R_ParticleExplosion (vec3_t org)
{
	if (!gl_particles->int_val)
		return;

	particle_new_random (pt_smokecloud, part_tex_smoke[rand () & 7], org, 8, 30,
						 8, cl.time + 5, (rand () & 7) + 8,
						 128 + (rand () & 63));
}

/*
===============
R_BlobExplosion

===============
*/
void
R_BlobExplosion (vec3_t org)
{
	int         i;

	if (!gl_particles->int_val)
		return;

	for (i = 0; i < 512; i++) {
		particle_new_random (pt_blob, part_tex_dot, org, 16, 2, 256,
							 (cl.time + 1 + (rand () & 8) * 0.05),
							 (66 + rand () % 6), 255);
	}
	for (i = 0; i < 512; i++) {
		particle_new_random (pt_blob2, part_tex_dot, org, 16, 2, 256,
							 (cl.time + 1 + (rand () & 8) * 0.05),
							 (150 + rand () % 6), 255);
	}
}

static void
R_RunSparkEffect (vec3_t org, int count, int ofuzz)
{
	if (!gl_particles->int_val)
		return;

	particle_new (pt_smokecloud, part_tex_smoke[rand () & 7], org, ofuzz / 8,
				  vec3_origin, cl.time + 99, 12 + (rand () & 3), 96);
	while (count--)
		particle_new_random (pt_fallfadespark, part_tex_dot, org, ofuzz, 1, 96,
							 cl.time + 5, ramp[rand () % 6], lhrandom (0, 255));
}

static void
R_RunGunshotEffect (vec3_t org, int count)
{
	int         scale;

	if (!gl_particles->int_val)
		return;

	if (count > 6)
		scale = 3;
	else
		scale = 2;

	R_RunSparkEffect (org, count * 10, 8 * scale);
	return;
}

static void
R_BloodPuff (vec3_t org, int count)
{
	if (!gl_particles->int_val)
		return;

	particle_new (pt_bloodcloud, part_tex_smoke[rand () & 7], org, 12,
				  vec3_origin, cl.time + 99, 68 + (rand () & 3), 128);
}

/*
===============
R_RunPuffEffect

===============
*/
void
R_RunPuffEffect (vec3_t org, byte type, byte count)
{
	if (!gl_particles->int_val)
		return;

	switch (type) {
		case TE_GUNSHOT:
			R_RunGunshotEffect (org, count);
			break;
		case TE_BLOOD:
			R_BloodPuff (org, count);
			break;
		case TE_LIGHTNINGBLOOD:
			R_BloodPuff (org, 5 + (rand () & 1));
			break;
	}
}

/*
===============
R_RunParticleEffect

===============
*/
void
R_RunParticleEffect (vec3_t org, int color, int count)
{
	int         i, j, scale;
	vec3_t      porg;

	if (!gl_particles->int_val)
		return;

	if (count > 130)
		scale = 3;
	else if (count > 20)
		scale = 2;
	else
		scale = 1;

	for (i = 0; i < count; i++) {
		for (j = 0; j < 3; j++) {
			porg[j] = org[j] + scale * ((rand () & 15) - 8);
		}
		particle_new (pt_grav, part_tex_dot, porg, 2, vec3_origin,
					  (cl.time + 0.1 * (rand () % 5)),
					  (color & ~7) + (rand () & 7), 255);
	}
}

void
R_RunSpikeEffect (vec3_t org, byte type)
{
	switch (type) {
		case TE_SPIKE:
			R_RunSparkEffect (org, 5, 8);
			break;
		case TE_SUPERSPIKE:
			R_RunSparkEffect (org, 10, 8);
			break;
		case TE_KNIGHTSPIKE:
			R_RunSparkEffect (org, 10, 8);
			break;
		case TE_WIZSPIKE:
			R_RunSparkEffect (org, 15, 16);
			break;
	}
}

/*
===============
R_LavaSplash

===============
*/
void
R_LavaSplash (vec3_t org)
{
	int         i, j;
	float       vel;
	vec3_t      dir, porg, pvel;

	if (!gl_particles->int_val)
		return;

	for (i = -16; i < 16; i++) {
		for (j = -16; j < 16; j++) {
			dir[0] = j * 8 + (rand () & 7);
			dir[1] = i * 8 + (rand () & 7);
			dir[2] = 256;

			porg[0] = org[0] + dir[0];
			porg[1] = org[1] + dir[1];
			porg[2] = org[2] + (rand () & 63);

			VectorNormalize (dir);
			vel = 50 + (rand () & 63);
			VectorScale (dir, vel, pvel);
			particle_new (pt_grav, part_tex_dot, porg, 2, pvel,
						  (cl.time + 2 + (rand () & 31) * 0.02),
						  (224 + (rand () & 7)), 255);
		}
	}
}

/*
===============
R_TeleportSplash

===============
*/
void
R_TeleportSplash (vec3_t org)
{
	int         i, j, k;
	float       vel;
	vec3_t      dir, porg, pvel;

	if (!gl_particles->int_val)
		return;

	for (i = -16; i < 16; i += 4)
		for (j = -16; j < 16; j += 4)
			for (k = -24; k < 32; k += 4) {
				dir[0] = j * 8;
				dir[1] = i * 8;
				dir[2] = k * 8;

				porg[0] = org[0] + i + (rand () & 3);
				porg[1] = org[1] + j + (rand () & 3);
				porg[2] = org[2] + k + (rand () & 3);

				VectorNormalize (dir);
				vel = 50 + (rand () & 63);
				VectorScale (dir, vel, pvel);
				particle_new (pt_grav, part_tex_dot, porg, 2, pvel,
							  (cl.time + 0.2 + (rand () & 7) * 0.02),
							  (7 + (rand () & 7)), 255);
			}
}

void
R_RocketTrail (int type, entity_t *ent)
{
	vec3_t      vec;
	float       len;
	int         j, ptex;
	ptype_t     ptype;
	vec3_t      porg, pvel;
	float       pdie, pscale;
	byte        palpha, pcolor;

	if (type == 0)
		R_AddFire (ent->old_origin, ent->origin, ent);

	if (!gl_particles->int_val)
		return;

	VectorSubtract (ent->origin, ent->old_origin, vec);
	len = VectorNormalize (vec);
	while (len > 0) {
		VectorCopy (vec3_origin, pvel);
		pdie = cl.time + 2;
		ptype = pt_static;
		ptex = part_tex_dot;
		pcolor = 0;
		pscale = 1;
		palpha = 255;

		switch (type) {
			case 0:					// rocket trail
				pcolor = (rand () & 3) + 12;
				goto common_rocket_gren_trail;
			case 1:					// grenade trail
				pcolor = (rand () & 3) + 3;
				goto common_rocket_gren_trail;

			  common_rocket_gren_trail:
				len -= 4;
				ptex = part_tex_smoke[rand () & 7];
				pscale = lhrandom (8, 12);
				palpha = 48 + (rand () & 31);
				ptype = pt_smoke;
				pdie = cl.time + 1;
				VectorCopy (ent->old_origin, porg);
				break;
			case 2:					// blood
			case 4:					// slight blood
				len -= 5;
				ptex = part_tex_dot;
				pscale = 1;
				pcolor = 68 + (rand () & 3);
				pdie = cl.time + 2;
				for (j = 0; j < 3; j++) {
					pvel[j] = (rand () & 15) - 8;
					porg[j] = ent->old_origin[j] + ((rand () % 3) - 2);
				}
				ptype = pt_grav;
				palpha = 255;
				break;
			case 6:					// voor trail
				len -= 3;
				pcolor = 9 * 16 + 8 + (rand () & 3);
				ptype = pt_static;
				pscale = lhrandom (1, 2);
				pdie = cl.time + 0.3;
				for (j = 0; j < 3; j++)
					porg[j] = ent->old_origin[j] + ((rand () & 15) - 8);
				break;
			case 3:
			case 5:					// tracer
				{
					static int  tracercount;

					len -= 3;
					pdie = cl.time + 0.5;
					ptype = pt_static;
					pscale = lhrandom (2, 4);
					if (type == 3)
						pcolor = 52 + ((tracercount & 4) << 1);
					else
						pcolor = 230 + ((tracercount & 4) << 1);

					tracercount++;

					VectorCopy (ent->old_origin, porg);
					if (tracercount & 1) {
						pvel[0] = 30 * vec[1];
						pvel[1] = 30 * -vec[0];
					} else {
						pvel[0] = 30 * -vec[1];
						pvel[1] = 30 * vec[0];
					}
					break;
				}
		}

		VectorAdd (ent->old_origin, vec, ent->old_origin);

		particle_new (ptype, ptex, porg, pscale, pvel, pdie, pcolor, palpha);
	}
}


/*
===============
R_DrawParticles
===============
*/
void
R_DrawParticles (void)
{
	byte        i;
	float       grav, fast_grav, dvel;
	float       minparticledist;
	unsigned char *at;
	byte        alpha;
	vec3_t      up, right;
	float       scale, scale2;
	particle_t *part;
	int         activeparticles, maxparticle, j, k;

	// LordHavoc: particles should not affect zbuffer
	glDepthMask (GL_FALSE);

	VectorScale (vup, 1.5, up);
	VectorScale (vright, 1.5, right);

	grav = (fast_grav = host_frametime * 800) * 0.05;
	dvel = 4 * host_frametime;

	minparticledist = DotProduct (r_refdef.vieworg, vpn) + 32.0f;


	activeparticles = 0;
	maxparticle = -1;
	j = 0;

	for (k = 0, part = particles; k < numparticles; k++, part++) {
		if (part->die <= cl.time) {
			freeparticles[j++] = part;
			continue;
		}
		maxparticle = k;
		activeparticles++;

		// Don't render particles too close to us.
		// Note, we must still do physics and such on them.
		if (!(DotProduct (part->org, vpn) < minparticledist)) {
			at = (byte *) & d_8to24table[(byte) part->color];
			alpha = part->alpha;

			if (lighthalf)
				glColor4ub ((byte) ((int) at[0] >> 1),
							(byte) ((int) at[1] >> 1),
							(byte) ((int) at[2] >> 1), alpha);
			else
				glColor4ub (at[0], at[1], at[2], alpha);

			scale = part->scale * 0.75;
			scale2 = part->scale * -0.75;

			glBindTexture (GL_TEXTURE_2D, part->tex);
			glBegin (GL_QUADS);
			glTexCoord2f (0, 1);
			glVertex3f ((part->org[0] + up[0] * scale + right[0] * scale),
						(part->org[1] + up[1] * scale + right[1] * scale),
						(part->org[2] + up[2] * scale + right[2] * scale));

			glTexCoord2f (0, 0);
			glVertex3f ((part->org[0] + up[0] * scale2 + right[0] * scale),
						(part->org[1] + up[1] * scale2 + right[1] * scale),
						(part->org[2] + up[2] * scale2 + right[2] * scale));

			glTexCoord2f (1, 0);
			glVertex3f ((part->org[0] + up[0] * scale2 + right[0] * scale2),
						(part->org[1] + up[1] * scale2 + right[1] * scale2),
						(part->org[2] + up[2] * scale2 + right[2] * scale2));

			glTexCoord2f (1, 1);
			glVertex3f ((part->org[0] + up[0] * scale + right[0] * scale2),
						(part->org[1] + up[1] * scale + right[1] * scale2),
						(part->org[2] + up[2] * scale + right[2] * scale2));

			glEnd ();
		}

		for (i = 0; i < 3; i++)
			part->org[i] += part->vel[i] * host_frametime;


#define alpha_die(p)			if (p->alpha < 1) part->die = -1;

		switch (part->type) {
			case pt_static:
				break;

			case pt_blob:
				for (i = 0; i < 3; i++)
					part->vel[i] += part->vel[i] * dvel;
				part->vel[2] -= grav;
				break;

			case pt_blob2:
				for (i = 0; i < 2; i++)
					part->vel[i] -= part->vel[i] * dvel;
				part->vel[2] -= grav;
				break;

			case pt_grav:
				part->vel[2] -= grav;
				break;

			case pt_smoke:
				part->scale += host_frametime * 6;
				part->alpha -= host_frametime * 128;
				alpha_die (part);
				break;
			case pt_smokecloud:
				part->scale += host_frametime * 60;
				part->alpha -= host_frametime * 128;
				alpha_die (part);
				break;
			case pt_bloodcloud:
				/* 
				   if (Mod_PointInLeaf(part->org, cl.worldmodel)->contents != 
				   CONTENTS_EMPTY) { part->die = -1; break; } */
				part->scale += host_frametime * 4;
				part->alpha -= host_frametime * 64;
				part->vel[2] -= grav;
				alpha_die (part);
				break;
			case pt_fadespark:
				part->alpha -= host_frametime * 256;
				part->vel[2] -= grav;
				if (part->alpha < 1)
					part->die = -1;
				break;
			case pt_fadespark2:
				part->alpha -= host_frametime * 512;
				part->vel[2] -= grav;
				alpha_die (part);
				break;
			case pt_fallfadespark:
				part->alpha -= host_frametime * 256;
				part->vel[2] -= fast_grav;
				if (part->alpha < 1)
					part->die = -1;
				break;
		}
	}
	k = 0;
	while (maxparticle >= activeparticles) {
		*freeparticles[k++] = particles[maxparticle--];
		while (maxparticle >= activeparticles
			   && particles[maxparticle].die < cl.time) maxparticle--;
	}
	numparticles = activeparticles;

	glColor3ubv (lighthalf_v);
	glDepthMask (GL_TRUE);
}
