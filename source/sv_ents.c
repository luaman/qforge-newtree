/*
	sv_ents.c

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
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#include "msg.h"
#include "server.h"
#include "sys.h"

// LordHavoc: added and removed certain eval_ items
// Ender Extends (QSG - Begin)
extern int  eval_alpha, eval_scale, eval_glowsize, eval_glowcolor,

	eval_colormod;
// Ender Extends (QSG - End)
extern eval_t *GETEDICTFIELDVALUE (edict_t *ed, int fieldoffset);

/*
	The PVS must include a small area around the client to allow head
	bobbing or other small motion on the client side.  Otherwise, a bob
	might cause an entity that should be visible to not show up, especially
	when the bob crosses a waterline.
*/

int         fatbytes;
byte        fatpvs[MAX_MAP_LEAFS / 8];

void
SV_AddToFatPVS (vec3_t org, mnode_t *node)
{
	int         i;
	byte       *pvs;
	mplane_t   *plane;
	float       d;

	while (1) {
		// if this is a leaf, accumulate the pvs bits
		if (node->contents < 0) {
			if (node->contents != CONTENTS_SOLID) {
				pvs = Mod_LeafPVS ((mleaf_t *) node, sv.worldmodel);
				for (i = 0; i < fatbytes; i++)
					fatpvs[i] |= pvs[i];
			}
			return;
		}

		plane = node->plane;
		d = DotProduct (org, plane->normal) - plane->dist;
		if (d > 8)
			node = node->children[0];
		else if (d < -8)
			node = node->children[1];
		else {							// go down both
			SV_AddToFatPVS (org, node->children[0]);
			node = node->children[1];
		}
	}
}

/*
	SV_FatPVS

	Calculates a PVS that is the inclusive or of all leafs within 8 pixels
	of the given point.
*/
byte       *
SV_FatPVS (vec3_t org)
{
	fatbytes = (sv.worldmodel->numleafs + 31) >> 3;
	memset (fatpvs, 0, fatbytes);
	SV_AddToFatPVS (org, sv.worldmodel->nodes);
	return fatpvs;
}

//=============================================================================

// because there can be a lot of nails, there is a special
// network protocol for them
#define	MAX_NAILS	32
edict_t    *nails[MAX_NAILS];
int         numnails;

extern int  sv_nailmodel, sv_supernailmodel, sv_playermodel;

qboolean
SV_AddNailUpdate (edict_t *ent)
{
	if (ent->v.v.modelindex != sv_nailmodel
		&& ent->v.v.modelindex != sv_supernailmodel) return false;
	if (numnails == MAX_NAILS)
		return true;
	nails[numnails] = ent;
	numnails++;
	return true;
}

void
SV_EmitNailUpdate (sizebuf_t *msg)
{
	byte        bits[6];				// [48 bits] xyzpy 12 12 12 4 8 
	int         n, i;
	edict_t    *ent;
	int         x, y, z, p, yaw;

	if (!numnails)
		return;

	MSG_WriteByte (msg, svc_nails);
	MSG_WriteByte (msg, numnails);

	for (n = 0; n < numnails; n++) {
		ent = nails[n];
		x = (int) (ent->v.v.origin[0] + 4096) >> 1;
		y = (int) (ent->v.v.origin[1] + 4096) >> 1;
		z = (int) (ent->v.v.origin[2] + 4096) >> 1;
		p = (int) (16 * ent->v.v.angles[0] / 360) & 15;
		yaw = (int) (256 * ent->v.v.angles[1] / 360) & 255;

		bits[0] = x;
		bits[1] = (x >> 8) | (y << 4);
		bits[2] = (y >> 4);
		bits[3] = z;
		bits[4] = (z >> 8) | (p << 4);
		bits[5] = yaw;

		for (i = 0; i < 6; i++)
			MSG_WriteByte (msg, bits[i]);
	}
}

//=============================================================================


/*
	SV_WriteDelta

	Writes part of a packetentities message.
	Can delta from either a baseline or a previous packet_entity
*/
void
SV_WriteDelta (entity_state_t *from, entity_state_t *to, sizebuf_t *msg,
			   qboolean force, int stdver)
{
	int         bits;
	int         i;
	float       miss;

// send an update
	bits = 0;

	for (i = 0; i < 3; i++) {
		miss = to->origin[i] - from->origin[i];
		if (miss < -0.1 || miss > 0.1)
			bits |= U_ORIGIN1 << i;
	}

	if (to->angles[0] != from->angles[0])
		bits |= U_ANGLE1;

	if (to->angles[1] != from->angles[1])
		bits |= U_ANGLE2;

	if (to->angles[2] != from->angles[2])
		bits |= U_ANGLE3;

	if (to->colormap != from->colormap)
		bits |= U_COLORMAP;

	if (to->skinnum != from->skinnum)
		bits |= U_SKIN;

	if (to->frame != from->frame)
		bits |= U_FRAME;

	if (to->effects != from->effects)
		bits |= U_EFFECTS;

	if (to->modelindex != from->modelindex)
		bits |= U_MODEL;

	// LordHavoc: cleaned up Endy's coding style, and added missing effects
// Ender (QSG - Begin)
	if (stdver > 1) {
		if (to->alpha != from->alpha)
			bits |= U_ALPHA;

		if (to->scale != from->scale)
			bits |= U_SCALE;

		if (to->glowsize != from->glowsize)
			bits |= U_GLOWSIZE;

		if (to->glowcolor != from->glowcolor)
			bits |= U_GLOWCOLOR;

		if (to->colormod != from->colormod)
			bits |= U_COLORMOD;
	}

	if (bits >= 16777216)
		bits |= U_EXTEND2;

	if (bits >= 65536)
		bits |= U_EXTEND1;
// Ender (QSG - End)
	if (bits & 511)
		bits |= U_MOREBITS;

	if (to->flags & U_SOLID)
		bits |= U_SOLID;

	// 
	// write the message
	// 
	if (!to->number)
		SV_Error ("Unset entity number");
	if (to->number >= 512)
		SV_Error ("Entity number >= 512");

	if (!bits && !force)
		return;							// nothing to send!
	i = to->number | (bits & ~511);
	if (i & U_REMOVE)
		Sys_Error ("U_REMOVE");
	MSG_WriteShort (msg, i);

	if (bits & U_MOREBITS)
		MSG_WriteByte (msg, bits & 255);

	// LordHavoc: cleaned up Endy's tabs
// Ender (QSG - Begin)
	if (bits & U_EXTEND1)
		MSG_WriteByte (msg, bits >> 16);
	if (bits & U_EXTEND2)
		MSG_WriteByte (msg, bits >> 24);
// Ender (QSG - End)

	if (bits & U_MODEL)
		MSG_WriteByte (msg, to->modelindex);
	if (bits & U_FRAME)
		MSG_WriteByte (msg, to->frame);
	if (bits & U_COLORMAP)
		MSG_WriteByte (msg, to->colormap);
	if (bits & U_SKIN)
		MSG_WriteByte (msg, to->skinnum);
	if (bits & U_EFFECTS)
		MSG_WriteByte (msg, to->effects);
	if (bits & U_ORIGIN1)
		MSG_WriteCoord (msg, to->origin[0]);
	if (bits & U_ANGLE1)
		MSG_WriteAngle (msg, to->angles[0]);
	if (bits & U_ORIGIN2)
		MSG_WriteCoord (msg, to->origin[1]);
	if (bits & U_ANGLE2)
		MSG_WriteAngle (msg, to->angles[1]);
	if (bits & U_ORIGIN3)
		MSG_WriteCoord (msg, to->origin[2]);
	if (bits & U_ANGLE3)
		MSG_WriteAngle (msg, to->angles[2]);

	// LordHavoc: cleaned up Endy's tabs, rearranged bytes, and implemented
	// missing effects
// Ender (QSG - Begin)
	if (bits & U_ALPHA)
		MSG_WriteByte (msg, to->alpha);
	if (bits & U_SCALE)
		MSG_WriteByte (msg, to->scale);
	if (bits & U_EFFECTS2)
		MSG_WriteByte (msg, (to->effects >> 8));
	if (bits & U_GLOWSIZE)
		MSG_WriteByte (msg, to->glowsize);
	if (bits & U_GLOWCOLOR)
		MSG_WriteByte (msg, to->glowcolor);
	if (bits & U_COLORMOD)
		MSG_WriteByte (msg, to->colormod);
	if (bits & U_FRAME2)
		MSG_WriteByte (msg, (to->frame >> 8));
// Ender (QSG - End)
}

/*
	SV_EmitPacketEntities

	Writes a delta update of a packet_entities_t to the message.
*/
void
SV_EmitPacketEntities (client_t *client, packet_entities_t *to, sizebuf_t *msg)
{
	edict_t    *ent;
	client_frame_t *fromframe;
	packet_entities_t *from;
	int         oldindex, newindex;
	int         oldnum, newnum;
	int         oldmax;

	// this is the frame that we are going to delta update from
	if (client->delta_sequence != -1) {
		fromframe = &client->frames[client->delta_sequence & UPDATE_MASK];
		from = &fromframe->entities;
		oldmax = from->num_entities;

		MSG_WriteByte (msg, svc_deltapacketentities);
		MSG_WriteByte (msg, client->delta_sequence);
	} else {
		oldmax = 0;						// no delta update
		from = NULL;

		MSG_WriteByte (msg, svc_packetentities);
	}

	newindex = 0;
	oldindex = 0;
//Con_Printf ("---%i to %i ----\n", client->delta_sequence & UPDATE_MASK
//          , client->netchan.outgoing_sequence & UPDATE_MASK);
	while (newindex < to->num_entities || oldindex < oldmax) {
		newnum =
			newindex >= to->num_entities ? 9999 : to->entities[newindex].number;
		oldnum = oldindex >= oldmax ? 9999 : from->entities[oldindex].number;

		if (newnum == oldnum) {			// delta update from old position
//Con_Printf ("delta %i\n", newnum);
			SV_WriteDelta (&from->entities[oldindex], &to->entities[newindex],
						   msg, false, client->stdver);
			oldindex++;
			newindex++;
			continue;
		}

		if (newnum < oldnum) {			// this is a new entity, send it from 
										// the baseline
			ent = EDICT_NUM (&sv_pr_state, newnum);
//Con_Printf ("baseline %i\n", newnum);
			SV_WriteDelta (&ent->baseline, &to->entities[newindex], msg, true,
						   client->stdver);
			newindex++;
			continue;
		}

		if (newnum > oldnum) {			// the old entity isn't present in
										// the new message
//Con_Printf ("remove %i\n", oldnum);
			MSG_WriteShort (msg, oldnum | U_REMOVE);
			oldindex++;
			continue;
		}
	}

	MSG_WriteShort (msg, 0);			// end of packetentities
}

/*
	SV_WritePlayersToClient
*/
void
SV_WritePlayersToClient (client_t *client, edict_t *clent, byte * pvs,
						 sizebuf_t *msg)
{
	int         i, j;
	client_t   *cl;
	edict_t    *ent;
	int         msec;
	usercmd_t   cmd;
	int         pflags;

	for (j = 0, cl = svs.clients; j < MAX_CLIENTS; j++, cl++) {
		if (cl->state != cs_spawned)
			continue;

		ent = cl->edict;

		// ZOID visibility tracking
		if (ent != clent &&
			!(client->spec_track && client->spec_track - 1 == j)) {
			if (cl->spectator)
				continue;

			// ignore if not touching a PV leaf
			for (i = 0; i < ent->num_leafs; i++)
				if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)))
					break;
			if (i == ent->num_leafs)
				continue;				// not visible
		}

		pflags = PF_MSEC | PF_COMMAND;

		if (ent->v.v.modelindex != sv_playermodel)
			pflags |= PF_MODEL;
		for (i = 0; i < 3; i++)
			if (ent->v.v.velocity[i])
				pflags |= PF_VELOCITY1 << i;
		if (ent->v.v.effects)
			pflags |= PF_EFFECTS;
		if (ent->v.v.skin)
			pflags |= PF_SKINNUM;
		if (ent->v.v.health <= 0)
			pflags |= PF_DEAD;
		if (ent->v.v.mins[2] != -24)
			pflags |= PF_GIB;

		if (cl->spectator) {			// only sent origin and velocity to
										// spectators
			pflags &= PF_VELOCITY1 | PF_VELOCITY2 | PF_VELOCITY3;
		} else if (ent == clent) {		// don't send a lot of data on
										// personal entity
			pflags &= ~(PF_MSEC | PF_COMMAND);
			if (ent->v.v.weaponframe)
				pflags |= PF_WEAPONFRAME;
		}

		if (client->spec_track && client->spec_track - 1 == j &&
			ent->v.v.weaponframe) pflags |= PF_WEAPONFRAME;

		MSG_WriteByte (msg, svc_playerinfo);
		MSG_WriteByte (msg, j);
		MSG_WriteShort (msg, pflags);

		for (i = 0; i < 3; i++)
			MSG_WriteCoord (msg, ent->v.v.origin[i]);

		MSG_WriteByte (msg, ent->v.v.frame);

		if (pflags & PF_MSEC) {
			msec = 1000 * (sv.time - cl->localtime);
			if (msec > 255)
				msec = 255;
			MSG_WriteByte (msg, msec);
		}

		if (pflags & PF_COMMAND) {
			cmd = cl->lastcmd;

			if (ent->v.v.health <= 0) {	// don't show the corpse looking
										// around...
				cmd.angles[0] = 0;
				cmd.angles[1] = ent->v.v.angles[1];
				cmd.angles[0] = 0;
			}

			cmd.buttons = 0;			// never send buttons
			cmd.impulse = 0;			// never send impulses

			MSG_WriteDeltaUsercmd (msg, &nullcmd, &cmd);
		}

		for (i = 0; i < 3; i++)
			if (pflags & (PF_VELOCITY1 << i))
				MSG_WriteShort (msg, ent->v.v.velocity[i]);

		if (pflags & PF_MODEL)
			MSG_WriteByte (msg, ent->v.v.modelindex);

		if (pflags & PF_SKINNUM)
			MSG_WriteByte (msg, ent->v.v.skin);

		if (pflags & PF_EFFECTS)
			MSG_WriteByte (msg, ent->v.v.effects);

		if (pflags & PF_WEAPONFRAME)
			MSG_WriteByte (msg, ent->v.v.weaponframe);
	}
}


/*
	SV_WriteEntitiesToClient

	Encodes the current state of the world as
	a svc_packetentities messages and possibly
	a svc_nails message and
	svc_playerinfo messages
*/
void
SV_WriteEntitiesToClient (client_t *client, sizebuf_t *msg)
{
	int         e, i;
	byte       *pvs;
	vec3_t      org;
	edict_t    *ent;
	packet_entities_t *pack;
	edict_t    *clent;
	client_frame_t *frame;
	entity_state_t *state;

	// this is the frame we are creating
	frame = &client->frames[client->netchan.incoming_sequence & UPDATE_MASK];

	// find the client's PVS
	clent = client->edict;
	VectorAdd (clent->v.v.origin, clent->v.v.view_ofs, org);
	pvs = SV_FatPVS (org);

	// send over the players in the PVS
	SV_WritePlayersToClient (client, clent, pvs, msg);

	// put other visible entities into either a packet_entities or a nails
	// message
	pack = &frame->entities;
	pack->num_entities = 0;

	numnails = 0;

	for (e = MAX_CLIENTS + 1, ent = EDICT_NUM (&sv_pr_state, e); e < sv.num_edicts;
		 e++, ent = NEXT_EDICT (&sv_pr_state, ent)) {
		// ignore ents without visible models
		if (!ent->v.v.modelindex || !*PR_GetString (&sv_pr_state, ent->v.v.model))
			continue;

		// ignore if not touching a PV leaf
		for (i = 0; i < ent->num_leafs; i++)
			if (pvs[ent->leafnums[i] >> 3] & (1 << (ent->leafnums[i] & 7)))
				break;

		if (i == ent->num_leafs)
			continue;					// not visible

		if (SV_AddNailUpdate (ent))
			continue;					// added to the special update list

		// add to the packetentities
		if (pack->num_entities == MAX_PACKET_ENTITIES)
			continue;					// all full

		state = &pack->entities[pack->num_entities];
		pack->num_entities++;

		state->number = e;
		state->flags = 0;
		VectorCopy (ent->v.v.origin, state->origin);
		VectorCopy (ent->v.v.angles, state->angles);
		state->modelindex = ent->v.v.modelindex;
		state->frame = ent->v.v.frame;
		state->colormap = ent->v.v.colormap;
		state->skinnum = ent->v.v.skin;
		state->effects = ent->v.v.effects;

		// LordHavoc: cleaned up Endy's coding style, shortened the code,
		// and implemented missing effects
// Ender: EXTEND (QSG - Begin)
		{
			eval_t     *val;

			state->alpha = 255;
			state->scale = 16;
			state->glowsize = 0;
			state->glowcolor = 254;
			state->colormod = 255;

			if ((val = GETEDICTFIELDVALUE (ent, eval_alpha))
				&& val->_float != 0)
				state->alpha = bound (0, val->_float, 1) * 255.0;

			if ((val = GETEDICTFIELDVALUE (ent, eval_scale))
				&& val->_float != 0)
				state->scale = bound (0, val->_float, 15.9375) * 16.0;

			if ((val = GETEDICTFIELDVALUE (ent, eval_glowsize))
				&& val->_float != 0)
				state->glowsize = bound (-1024, (int) val->_float, 1016) >> 3;

			if ((val = GETEDICTFIELDVALUE (ent, eval_glowcolor))
				&& val->_float != 0)
				state->glowcolor = (int) val->_float;

			if ((val = GETEDICTFIELDVALUE (ent, eval_colormod))
				&& (val->vector[0] != 0 || val->vector[1] != 0
					|| val->vector[2] != 0))
				state->colormod =
					((int) (bound (0, val->vector[0], 1) * 7.0) << 5) |
					((int) (bound (0, val->vector[1], 1) * 7.0) << 2) |
					(int) (bound (0, val->vector[2], 1) * 3.0);
		}
// Ender: EXTEND (QSG - End)
	}

	// encode the packet entities as a delta from the
	// last packetentities acknowledged by the client

	SV_EmitPacketEntities (client, pack, msg);

	// now add the specialized nail update
	SV_EmitNailUpdate (msg);
}
