/*
	teamplay.c

	Teamplay enhancements ("proxy features")

	Copyright (C) 2000       Anton Gavrilov (tonik@quake.ru)

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

#include "bothdefs.h"
#include "client.h"
#include "cmd.h"
#include "cvar.h"
#include "teamplay.h"

cvar_t	*cl_deadbodyfilter;
cvar_t	*cl_gibfilter;


void CL_BestWeaponImpulse (void)
{
	int			best, i, imp, items;
	extern int	in_impulse;

	items = cl.stats[STAT_ITEMS];
	best = 0;

	for (i = Cmd_Argc() - 1; i > 0; i--)
	{
		imp = atoi(Cmd_Argv(i));
		if (imp < 1 || imp > 8)
			continue;

		switch (imp)
		{
			case 1:
				if (items & IT_AXE)
					best = 1;
				break;
			case 2:
				if (items & IT_SHOTGUN && cl.stats[STAT_SHELLS] >= 1)
					best = 2;
				break;
			case 3:
				if (items & IT_SUPER_SHOTGUN && cl.stats[STAT_SHELLS] >= 2)
					best = 3;
				break;
			case 4:
				if (items & IT_NAILGUN && cl.stats[STAT_NAILS] >= 1)
					best = 4;
				break;
			case 5:
				if (items & IT_SUPER_NAILGUN && cl.stats[STAT_NAILS] >= 2)
					best = 5;
				break;
			case 6:
				if (items & IT_GRENADE_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
					best = 6;
				break;
			case 7:
				if (items & IT_ROCKET_LAUNCHER && cl.stats[STAT_ROCKETS] >= 1)
					best = 7;
				break;
			case 8:
				if (items & IT_LIGHTNING && cl.stats[STAT_CELLS] >= 1)
					best = 8;
			
		}
	}
	
	if (best)
		in_impulse = best;
}


void CL_InitTeamplay (void)
{
	cl_deadbodyfilter = Cvar_Get("cl_deadbodyfilter", "0", CVAR_NONE, "None");
	cl_gibfilter = Cvar_Get("cl_gibfilter", "0", CVAR_NONE, "None");
}
