
/* 
 * Copyright (C) 2000 David Jeffery
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LINUX_JOYSTICK_H

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>

#include "console.h"
#include "client.h"
#include "cvar.h"
#include "keys.h"

//joystick variables and structures
#define MAX_JOYAXIS 6
#define MAX_JOYBUTTON 10
#define JOYLOOK_MAX 200
#define JOYMOVE_MAX 200
int joydev;

typedef struct {
	char *name;
	char *string;
} ocvar_t;

struct joyaxis{
	int current;
	ocvar_t var;
	cvar_t *axis;
};

struct joybutton{
	int old;
	int current;
};

struct joyaxis joyaxiscontrol[MAX_JOYAXIS] = {
	{0,{"joyaxis1","1"}} ,
	{0,{"joyaxis2","2"}} ,
	{0,{"joyaxis3","3"}} ,
	{0,{"joyaxis4","0"}} ,
	{0,{"joyaxis5","0"}} ,
	{0,{"joyaxis6","0"}}
};

struct joybutton joybuttoncontrol[MAX_JOYBUTTON];
int joyfound = 0;

void joystick_command(void)
{
	struct js_event event;

	if(!joyfound)
		return;
	while(read(joydev, &event, sizeof(struct js_event)) > -1){
		if(event.type & JS_EVENT_BUTTON){
			if(event.number >= MAX_JOYBUTTON)
				continue;
			joybuttoncontrol[event.number].current = event.value;
			if(joybuttoncontrol[event.number].current >
					joybuttoncontrol[event.number].old)
				Key_Event(K_AUX1 + event.number, true);
			else if(joybuttoncontrol[event.number].current <
					joybuttoncontrol[event.number].old)
				Key_Event(K_AUX1 + event.number, false);
			joybuttoncontrol[event.number].old =
				joybuttoncontrol[event.number].current;
		}
		else if(event.type & JS_EVENT_AXIS){
			if(event.number >= MAX_JOYAXIS)
				continue;
			joyaxiscontrol[event.number].current = event.value;
		}
	}
}

void joystick_move(usercmd_t *cmd)
{
	int i;
	if(!joyfound)
		return;

	for(i = 0; i < MAX_JOYAXIS; i++) {
		switch(joyaxiscontrol[i].axis->int_val){
		case 1:
			cl.viewangles[YAW] -= m_yaw->value * (float)joyaxiscontrol[i].current/JOYLOOK_MAX;	
			break;
		case 2:
			cmd->forwardmove -= m_forward->value * (float)joyaxiscontrol[i].current/JOYMOVE_MAX;
			break;
		case 3:
			cmd->sidemove += m_side->value * (float)joyaxiscontrol[i].current/JOYMOVE_MAX;
			break;
		case 4:
			if(joyaxiscontrol[i].current){
				V_StopPitchDrift();
				cl.viewangles[PITCH] -= m_pitch->value * (float)joyaxiscontrol[i].current/JOYLOOK_MAX;
				if(cl.viewangles[PITCH] > 80)
					cl.viewangles[PITCH] = 80;
				else if(cl.viewangles[PITCH] < -70)
					cl.viewangles[PITCH] = -70;
			}
			break;
		}
	}
}

void joystick_init(void)
{
	/*initialize joystick if found */
	if ((joydev = open("/dev/js0", O_RDONLY | O_NONBLOCK)) < 0) {
		joyfound = 0;
		Con_DPrintf("no joystick\n");
	} else {
		int i;
		joyfound = 1;
		for(i = 0; i < MAX_JOYAXIS; i++) {
			joyaxiscontrol[i].axis = Cvar_Get(joyaxiscontrol[i].var.name,
											  joyaxiscontrol[i].var.string,
											  CVAR_ARCHIVE, "None");
		}
		for(i = 0; i < MAX_JOYBUTTON; i++) {
				joybuttoncontrol[i].old = 0;
				joybuttoncontrol[i].current = 0;
		}
		Con_DPrintf("joystick enabled\n");
	}
}

#else
void joystick_init(void){
}

void joystick_command(void){
}

void joystick_move(usercmd_t *cmd){
}
#endif

