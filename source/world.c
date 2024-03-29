/*
	world.c

	world query functions

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

#include "commdef.h"
#include "console.h"
#include "crc.h"
#include "server.h"
#include "world.h"

/*

entities never clip against themselves, or their owner

line of sight checks trace->crosscontent, but bullets don't

*/

typedef struct {
	vec3_t      boxmins, boxmaxs;		// enclose the test object along
										// entire move
	float      *mins, *maxs;			// size of the moving object
	vec3_t      mins2, maxs2;			// size when clipping against
										// monsters
	float      *start, *end;
	trace_t     trace;
	int         type;
	edict_t    *passedict;
} moveclip_t;

extern qboolean RecursiveHullCheck (hull_t *hull, int num, float p1f, float p2f, vec3_t p1,
					   vec3_t p2, trace_t *trace);

extern int HullPointContents (hull_t *hull, int num, vec3_t p);

extern void InitBoxHull (void);

extern hull_t     *HullForBox (vec3_t mins, vec3_t maxs);

/*
	SV_HullForEntity

	Returns a hull that can be used for testing or clipping an object of
	mins/maxs size.  Offset is filled in to contain the adjustment that
	must be added to the testing object's origin to get a point to use with
	the returned hull.
*/
hull_t     *
SV_HullForEntity (edict_t *ent, vec3_t mins, vec3_t maxs, vec3_t offset)
{
	model_t    *model;
	vec3_t      size;
	vec3_t      hullmins, hullmaxs;
	hull_t     *hull;

// decide which clipping hull to use, based on the size
	if (ent->v.v.solid == SOLID_BSP) {
		// explicit hulls in the BSP model
		if (ent->v.v.movetype != MOVETYPE_PUSH)
			SV_Error ("SOLID_BSP without MOVETYPE_PUSH");

		model = sv.models[(int) ent->v.v.modelindex];

		if (!model || model->type != mod_brush)
			SV_Error ("SOLID_BSP with a non bsp model");

		VectorSubtract (maxs, mins, size);
		if (size[0] < 3)
			hull = &model->hulls[0];
		else if (size[0] <= 32)
			hull = &model->hulls[1];
		else
			hull = &model->hulls[2];

// calculate an offset value to center the origin
		VectorSubtract (hull->clip_mins, mins, offset);
		VectorAdd (offset, ent->v.v.origin, offset);
	} else {							// create a temp hull from bounding
										// box sizes

		VectorSubtract (ent->v.v.mins, maxs, hullmins);
		VectorSubtract (ent->v.v.maxs, mins, hullmaxs);
		hull = HullForBox (hullmins, hullmaxs);

		VectorCopy (ent->v.v.origin, offset);
	}


	return hull;
}

/*
	ENTITY AREA CHECKING
*/


areanode_t  sv_areanodes[AREA_NODES];
int         sv_numareanodes;

/*
	SV_CreateAreaNode
*/
areanode_t *
SV_CreateAreaNode (int depth, vec3_t mins, vec3_t maxs)
{
	areanode_t *anode;
	vec3_t      size;
	vec3_t      mins1, maxs1, mins2, maxs2;

	anode = &sv_areanodes[sv_numareanodes];
	sv_numareanodes++;

	ClearLink (&anode->trigger_edicts);
	ClearLink (&anode->solid_edicts);

	if (depth == AREA_DEPTH) {
		anode->axis = -1;
		anode->children[0] = anode->children[1] = NULL;
		return anode;
	}

	VectorSubtract (maxs, mins, size);
	if (size[0] > size[1])
		anode->axis = 0;
	else
		anode->axis = 1;

	anode->dist = 0.5 * (maxs[anode->axis] + mins[anode->axis]);
	VectorCopy (mins, mins1);
	VectorCopy (mins, mins2);
	VectorCopy (maxs, maxs1);
	VectorCopy (maxs, maxs2);

	maxs1[anode->axis] = mins2[anode->axis] = anode->dist;

	anode->children[0] = SV_CreateAreaNode (depth + 1, mins2, maxs2);
	anode->children[1] = SV_CreateAreaNode (depth + 1, mins1, maxs1);

	return anode;
}

/*
	SV_ClearWorld
*/
void
SV_ClearWorld (void)
{
	InitBoxHull ();

	memset (sv_areanodes, 0, sizeof (sv_areanodes));
	sv_numareanodes = 0;
	SV_CreateAreaNode (0, sv.worldmodel->mins, sv.worldmodel->maxs);
}


/*
	SV_UnlinkEdict
*/
void
SV_UnlinkEdict (edict_t *ent)
{
	if (!ent->area.prev)
		return;							// not linked in anywhere
	RemoveLink (&ent->area);
	ent->area.prev = ent->area.next = NULL;
}


/*
	SV_TouchLinks
*/
void
SV_TouchLinks (edict_t *ent, areanode_t *node)
{
	link_t     *l, *next;
	edict_t    *touch;
	int         old_self, old_other;

// touch linked edicts
	for (l = node->trigger_edicts.next; l != &node->trigger_edicts; l = next) {
		next = l->next;
		touch = EDICT_FROM_AREA (l);
		if (touch == ent)
			continue;
		if (!touch->v.v.touch || touch->v.v.solid != SOLID_TRIGGER)
			continue;
		if (ent->v.v.absmin[0] > touch->v.v.absmax[0]
			|| ent->v.v.absmin[1] > touch->v.v.absmax[1]
			|| ent->v.v.absmin[2] > touch->v.v.absmax[2]
			|| ent->v.v.absmax[0] < touch->v.v.absmin[0]
			|| ent->v.v.absmax[1] < touch->v.v.absmin[1]
			|| ent->v.v.absmax[2] < touch->v.v.absmin[2])
			continue;

		old_self = sv_pr_state.pr_global_struct->self;
		old_other = sv_pr_state.pr_global_struct->other;

		sv_pr_state.pr_global_struct->self = EDICT_TO_PROG (&sv_pr_state, touch);
		sv_pr_state.pr_global_struct->other = EDICT_TO_PROG (&sv_pr_state, ent);
		sv_pr_state.pr_global_struct->time = sv.time;
		PR_ExecuteProgram (&sv_pr_state, touch->v.v.touch);

		sv_pr_state.pr_global_struct->self = old_self;
		sv_pr_state.pr_global_struct->other = old_other;
	}

// recurse down both sides
	if (node->axis == -1)
		return;

	if (ent->v.v.absmax[node->axis] > node->dist)
		SV_TouchLinks (ent, node->children[0]);
	if (ent->v.v.absmin[node->axis] < node->dist)
		SV_TouchLinks (ent, node->children[1]);
}


/*
	SV_FindTouchedLeafs
*/
void
SV_FindTouchedLeafs (edict_t *ent, mnode_t *node)
{
	mplane_t   *splitplane;
	mleaf_t    *leaf;
	int         sides;
	int         leafnum;

	if (node->contents == CONTENTS_SOLID)
		return;

// add an efrag if the node is a leaf

	if (node->contents < 0) {
		if (ent->num_leafs == MAX_ENT_LEAFS)
			return;

		leaf = (mleaf_t *) node;
		leafnum = leaf - sv.worldmodel->leafs - 1;

		ent->leafnums[ent->num_leafs] = leafnum;
		ent->num_leafs++;
		return;
	}
// NODE_MIXED

	splitplane = node->plane;
	sides = BOX_ON_PLANE_SIDE (ent->v.v.absmin, ent->v.v.absmax, splitplane);

// recurse down the contacted sides
	if (sides & 1)
		SV_FindTouchedLeafs (ent, node->children[0]);

	if (sides & 2)
		SV_FindTouchedLeafs (ent, node->children[1]);
}

/*
	SV_LinkEdict
*/
void
SV_LinkEdict (edict_t *ent, qboolean touch_triggers)
{
	areanode_t *node;

	if (ent->area.prev)
		SV_UnlinkEdict (ent);			// unlink from old position

	if (ent == sv.edicts)
		return;							// don't add the world

	if (ent->free)
		return;

// set the abs box
	VectorAdd (ent->v.v.origin, ent->v.v.mins, ent->v.v.absmin);
	VectorAdd (ent->v.v.origin, ent->v.v.maxs, ent->v.v.absmax);

//
// to make items easier to pick up and allow them to be grabbed off
// of shelves, the abs sizes are expanded
//
	if ((int) ent->v.v.flags & FL_ITEM) {
		ent->v.v.absmin[0] -= 15;
		ent->v.v.absmin[1] -= 15;
		ent->v.v.absmax[0] += 15;
		ent->v.v.absmax[1] += 15;
	} else {							// because movement is clipped an
										// epsilon away from an actual edge,
		// we must fully check even when bounding boxes don't quite touch
		ent->v.v.absmin[0] -= 1;
		ent->v.v.absmin[1] -= 1;
		ent->v.v.absmin[2] -= 1;
		ent->v.v.absmax[0] += 1;
		ent->v.v.absmax[1] += 1;
		ent->v.v.absmax[2] += 1;
	}

// link to PVS leafs
	ent->num_leafs = 0;
	if (ent->v.v.modelindex)
		SV_FindTouchedLeafs (ent, sv.worldmodel->nodes);

	if (ent->v.v.solid == SOLID_NOT)
		return;

// find the first node that the ent's box crosses
	node = sv_areanodes;
	while (1) {
		if (node->axis == -1)
			break;
		if (ent->v.v.absmin[node->axis] > node->dist)
			node = node->children[0];
		else if (ent->v.v.absmax[node->axis] < node->dist)
			node = node->children[1];
		else
			break;						// crosses the node
	}

// link it in   

	if (ent->v.v.solid == SOLID_TRIGGER)
		InsertLinkBefore (&ent->area, &node->trigger_edicts);
	else
		InsertLinkBefore (&ent->area, &node->solid_edicts);

// if touch_triggers, touch all entities at this node and descend for more
	if (touch_triggers)
		SV_TouchLinks (ent, sv_areanodes);
}

/*
	SV_PointContents
*/
int
SV_PointContents (vec3_t p)
{
	return HullPointContents (&sv.worldmodel->hulls[0], 0, p);
}

//===========================================================================

/*
	SV_TestEntityPosition

	A small wrapper around SV_BoxInSolidEntity that never clips against the
	supplied entity.
*/
edict_t    *
SV_TestEntityPosition (edict_t *ent)
{
	trace_t     trace;

	trace =
		SV_Move (ent->v.v.origin, ent->v.v.mins, ent->v.v.maxs, ent->v.v.origin, 0,
				 ent);

	if (trace.startsolid)
		return sv.edicts;

	return NULL;
}


/*
	SV_ClipMoveToEntity

	Handles selection or creation of a clipping hull, and offseting (and
	eventually rotation) of the end points
*/
trace_t
SV_ClipMoveToEntity (edict_t *ent, vec3_t start, vec3_t mins, vec3_t maxs,
					 vec3_t end)
{
	trace_t     trace;
	vec3_t      offset;
	vec3_t      start_l, end_l;
	hull_t     *hull;

// fill in a default trace
	memset (&trace, 0, sizeof (trace_t));

	trace.fraction = 1;
	trace.allsolid = true;
	VectorCopy (end, trace.endpos);

// get the clipping hull
	hull = SV_HullForEntity (ent, mins, maxs, offset);

	VectorSubtract (start, offset, start_l);
	VectorSubtract (end, offset, end_l);

// trace a line through the apropriate clipping hull
	RecursiveHullCheck (hull, hull->firstclipnode, 0, 1, start_l, end_l,
						   &trace);

// fix trace up by the offset
	if (trace.fraction != 1)
		VectorAdd (trace.endpos, offset, trace.endpos);

// did we clip the move?
	if (trace.fraction < 1 || trace.startsolid)
		trace.ent = ent;

	return trace;
}

//===========================================================================

/*
	SV_ClipToLinks

	Mins and maxs enclose the entire area swept by the move
*/
void
SV_ClipToLinks (areanode_t *node, moveclip_t * clip)
{
	link_t     *l, *next;
	edict_t    *touch;
	trace_t     trace;

// touch linked edicts
	for (l = node->solid_edicts.next; l != &node->solid_edicts; l = next) {
		next = l->next;
		touch = EDICT_FROM_AREA (l);
		if (touch->v.v.solid == SOLID_NOT)
			continue;
		if (touch == clip->passedict)
			continue;
		if (touch->v.v.solid == SOLID_TRIGGER)
			SV_Error ("Trigger in clipping list");

		if (clip->type == MOVE_NOMONSTERS && touch->v.v.solid != SOLID_BSP)
			continue;

		if (clip->boxmins[0] > touch->v.v.absmax[0]
			|| clip->boxmins[1] > touch->v.v.absmax[1]
			|| clip->boxmins[2] > touch->v.v.absmax[2]
			|| clip->boxmaxs[0] < touch->v.v.absmin[0]
			|| clip->boxmaxs[1] < touch->v.v.absmin[1]
			|| clip->boxmaxs[2] < touch->v.v.absmin[2])
			continue;

		// LordHavoc: err...  FIXME??  I decided not to touch this weird code...
		if (clip->passedict != 0 && clip->passedict->v.v.size[0]
			&& !touch->v.v.size[0]) continue;	// points never interact

		// might intersect, so do an exact clip
		if (clip->trace.allsolid)
			return;
		if (clip->passedict) {
			if (PROG_TO_EDICT (&sv_pr_state, touch->v.v.owner) == clip->passedict)
				continue;				// don't clip against own missiles
			if (PROG_TO_EDICT (&sv_pr_state, clip->passedict->v.v.owner) == touch)
				continue;				// don't clip against owner
		}

		if ((int) touch->v.v.flags & FL_MONSTER)
			trace =
				SV_ClipMoveToEntity (touch, clip->start, clip->mins2,
									 clip->maxs2, clip->end);
		else
			trace =
				SV_ClipMoveToEntity (touch, clip->start, clip->mins, clip->maxs,
									 clip->end);
		if (trace.allsolid || trace.startsolid
			|| trace.fraction < clip->trace.fraction) {
			trace.ent = touch;
			if (clip->trace.startsolid) {
				clip->trace = trace;
				clip->trace.startsolid = true;
			} else
				clip->trace = trace;
		} else if (trace.startsolid)
			clip->trace.startsolid = true;
	}

// recurse down both sides
	if (node->axis == -1)
		return;

	if (clip->boxmaxs[node->axis] > node->dist)
		SV_ClipToLinks (node->children[0], clip);
	if (clip->boxmins[node->axis] < node->dist)
		SV_ClipToLinks (node->children[1], clip);
}


/*
	SV_MoveBounds
*/
void
SV_MoveBounds (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end,
			   vec3_t boxmins, vec3_t boxmaxs)
{
#if 0
// debug to test against everything
	boxmins[0] = boxmins[1] = boxmins[2] = -9999;
	boxmaxs[0] = boxmaxs[1] = boxmaxs[2] = 9999;
#else
	int         i;

	for (i = 0; i < 3; i++) {
		if (end[i] > start[i]) {
			boxmins[i] = start[i] + mins[i] - 1;
			boxmaxs[i] = end[i] + maxs[i] + 1;
		} else {
			boxmins[i] = end[i] + mins[i] - 1;
			boxmaxs[i] = start[i] + maxs[i] + 1;
		}
	}
#endif
}

/*
	SV_Move
*/
trace_t
SV_Move (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int type,
		 edict_t *passedict)
{
	moveclip_t  clip;
	int         i;

	memset (&clip, 0, sizeof (moveclip_t));

// clip to world
	clip.trace = SV_ClipMoveToEntity (sv.edicts, start, mins, maxs, end);

	clip.start = start;
	clip.end = end;
	clip.mins = mins;
	clip.maxs = maxs;
	clip.type = type;
	clip.passedict = passedict;

	if (type == MOVE_MISSILE) {
		for (i = 0; i < 3; i++) {
			clip.mins2[i] = -15;
			clip.maxs2[i] = 15;
		}
	} else {
		VectorCopy (mins, clip.mins2);
		VectorCopy (maxs, clip.maxs2);
	}

// create the bounding box of the entire move
	SV_MoveBounds (start, clip.mins2, clip.maxs2, end, clip.boxmins,
				   clip.boxmaxs);

// clip to entities
	SV_ClipToLinks (sv_areanodes, &clip);

	return clip.trace;
}

//=============================================================================

/*
	SV_TestPlayerPosition
*/
edict_t    *
SV_TestPlayerPosition (edict_t *ent, vec3_t origin)
{
	hull_t     *hull;
	edict_t    *check;
	vec3_t      boxmins, boxmaxs;
	vec3_t      offset;
	int         e;

// check world first
	hull = &sv.worldmodel->hulls[1];
	if (HullPointContents (hull, hull->firstclipnode, origin) !=
		CONTENTS_EMPTY) return sv.edicts;

// check all entities
	VectorAdd (origin, ent->v.v.mins, boxmins);
	VectorAdd (origin, ent->v.v.maxs, boxmaxs);

	check = NEXT_EDICT (&sv_pr_state, sv.edicts);
	for (e = 1; e < sv.num_edicts; e++, check = NEXT_EDICT (&sv_pr_state, check)) {
		if (check->free)
			continue;
		if (check->v.v.solid != SOLID_BSP &&
			check->v.v.solid != SOLID_BBOX && check->v.v.solid != SOLID_SLIDEBOX)
			continue;

		if (boxmins[0] > check->v.v.absmax[0]
			|| boxmins[1] > check->v.v.absmax[1]
			|| boxmins[2] > check->v.v.absmax[2]
			|| boxmaxs[0] < check->v.v.absmin[0]
			|| boxmaxs[1] < check->v.v.absmin[1]
			|| boxmaxs[2] < check->v.v.absmin[2])
			continue;

		if (check == ent)
			continue;

		// get the clipping hull
		hull = SV_HullForEntity (check, ent->v.v.mins, ent->v.v.maxs, offset);

		VectorSubtract (origin, offset, offset);

		// test the point
		if (HullPointContents (hull, hull->firstclipnode, offset) !=
			CONTENTS_EMPTY) return check;
	}

	return NULL;
}
