/*
	cl_slist.c

	serverlist addressbook

	Copyright (C) 2000       Brian Koropoff <brian.hk@home.com>

	Author: Brian Koropoff
	Date: 03 May 2000

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
#include "cl_slist.h"
#include "bothdefs.h"
#include "console.h"
#include "commdef.h"
#include "quakefs.h"
#include <string.h>

server_entry_t	*slist;
  
server_entry_t *SL_Add (server_entry_t *start, char *ip, char *desc) {
	server_entry_t *p;
	p = start;
	if (!start) { //Nothing at beginning of list, create it
		start = malloc(sizeof(server_entry_t));
		start->prev = 0;
		start->next = 0;
		start->server = malloc(strlen(ip) + 1);
		start->desc = malloc(strlen(desc) + 1);
		strcpy(start->server,ip);
		strcpy(start->desc,desc);
		return (start);
	}

	for(p=start;p->next;p=p->next); //Get to end of list

	p->next = malloc(sizeof(server_entry_t));
	p->next->prev = p;
	p->next->server = malloc(strlen(ip) + 1);
	p->next->desc = malloc(strlen(desc) + 1);

	strcpy(p->next->server,ip);
	strcpy(p->next->desc,desc);
	p->next->next = 0;

	return (start);
	}

server_entry_t *SL_Del(server_entry_t *start, server_entry_t *del) {
	server_entry_t *n;
	if (del == start) {
		free(start->server);
		free(start->desc);
		n = start->next;
		if (n)
			n->prev = 0;
		free(start);
		return (n);
	}
	
	free(del->server); 
	free(del->desc);
	if (del->prev)
		del->prev->next = del->next;
	if (del->next)
		del->next->prev = del->prev;
	free(del);
	return (start);
}

server_entry_t *SL_InsB (server_entry_t *start, server_entry_t *place, char *ip, char *desc) {
		server_entry_t *new;
		server_entry_t *other;
	
		new = malloc(sizeof(server_entry_t));
		new->server = malloc(strlen(ip) + 1);
		new->desc = malloc(strlen(desc) + 1);
		strcpy(new->server,ip);
		strcpy(new->desc,desc);
		other = place->prev;
		if (other)
			other->next = new;
		place->prev = new;
		new->next = place;
		new->prev = other;
		if (!other)
			return new;
		return start;
}

void SL_Swap (server_entry_t *swap1, server_entry_t *swap2) {
		char *p;
		p = swap1->server;
		swap1->server = swap2->server;
		swap2->server = p;
		p = swap1->desc;
		swap1->desc = swap2->desc;
		swap2->desc = p;
}
  
server_entry_t *SL_Get_By_Num (server_entry_t *start, int n) {
	int i;
	for (i=0;i < n;i++)
		start = start->next;
	if (!start)
		return (0);
	return (start);
}
  
int SL_Len (server_entry_t *start) {
	int i;
	for (i=0;start;i++)
		start=start->next;
	return i;
  }
  
server_entry_t *SL_LoadF (FILE *f,server_entry_t *start) { // This could get messy
  	char line[256]; /* Long lines get truncated. */
  	int c = ' ';    /* int so it can be compared to EOF properly*/
  	int len;
  	int i;
 	char *st;
  	char *addr;
  
 	while (1) {
		//First, get a line
		i = 0;
		c = ' ';
		while (c != '\n' && c != EOF) {
			c = getc(f);
			if (i < 255) {
				line[i] = c;
				i++;
			}
		}
		line[i - 1] = '\0'; // Now we can parse it
 		if ((st = gettokstart(line,1,' ')) != NULL) {
  			len = gettoklen(line,1,' ');
  			addr = malloc(len + 1);
  			strncpy(addr,&line[0],len);
  			addr[len] = '\0';
 			if ((st = gettokstart(line,2,' '))) {
 				start = SL_Add(start,addr,st);
  			}
  			else {
 				start = SL_Add(start,addr,"Unknown");
  			}
  		} 
  		if (c == EOF)  // We're done
 			return start;
  	}
}

 void SL_SaveF (FILE *f,server_entry_t *start) {
 	do {
 		fprintf(f,"%s   %s\n",start->server,start->desc);
 		start = start->next;
 
 	} while (start);
 }
 
 void SL_Shutdown (server_entry_t *start) {
 	FILE	*f;
	char	e_path[MAX_OSPATH];
	
 	if (start) {
		Qexpand_squiggle(fs_userpath->string, e_path);
 		if ((f = fopen(va("%s/servers.txt", e_path),"w"))) {
 			SL_SaveF(f,start);
 			fclose(f);
 		}
 		SL_Del_All (start);
  	}
  }
 
 void SL_Del_All (server_entry_t *start) {
 	server_entry_t *n;
 	while (start) {
 		n = start->next;
 		free(start->server);
 		free(start->desc);
 		free(start);
 		start = n;
 	}
 }
 


char *gettokstart (char *str, int req, char delim) {
	char *start = str;
	
	int tok = 1;

	while (*start == delim) {
		start++;
	}
	if (*start == '\0')
		return '\0';
	while (tok < req) { //Stop when we get to the requested token
		if (*++start == delim) { //Increment pointer and test
			while (*start == delim) { //Get to next token
				start++;
			}
			tok++;
		}
		if (*start == '\0') {
			return '\0';
		}
	}
	return start;
}

int gettoklen (char *str, int req, char delim) {
	char *start = 0;
	
	int len = 0;
	
	start = gettokstart(str,req,delim);
	if (start == '\0') {
		return 0;
	}
	while (*start != delim && *start != '\0') {
		start++;
		len++;
	}
	return len;
}
