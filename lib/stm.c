/*
 * \file state machine engine based on EAM@itba.edu.ar idea (2001)
 *
 * Copyright (C) 2003 by Juan F. Codagnone <juam@users.sourceforge.net>
 *
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "stm.h"

struct stCDT
{ 	ST_PARSE **root;	/**< NULL implies slot free	*/
	unsigned nstates;	/**< number of states 	*/
	int state;      	/**< current state 	*/
	int init_state;  	/**< init state...	*/
	void (*debug) (int, int, int); /** debug function */
	void *data;     	/**< user data */
};

#define IS_ST(l)	((l)!=NULL && (l)->root!=NULL )

stm_t
stm_new( ST_PARSE **sp, unsigned nstates, int init_state, void *data )
{ 	stm_t st = 0;

	st = malloc(sizeof(*st));
	if( st )
	{ 	st->root = sp;
		st->nstates = nstates;
		st->init_state = st->state = init_state;
		st->debug = 0;
		st->data = data;
	}
	
	return st;
}

int
stm_set_debug( stm_t st, void (*debug)(int, int, int) )
{	int ret = EST_OK;

	if( IS_ST(st) )
		st->debug = debug;
	else
		ret = EST_BAD_PD;
		
	return ret;
}

int
stm_parse( stm_t st, int c )
{ 	ST_PARSE *p;
	int next;
	int found;

	if( !IS_ST(st) )
		return EST_BAD_PD;

	assert( st->state < st->nstates  );	/* this is checked elsewhere*/
	p = st->root[ st->state ];

	if( p == NULL )
		return EST_NULL_PTR;

	for( found =0 ;
	     !( p->function == ST_FUNC && p->cmpfnc == ELSE ) ;
	     p++ )
	{	found = 
		(p->function == ST_CHAR  && c == (int)p->cmpfnc ) ||
		(p->function == ST_LCHAR && tolower(c) == (int)p->cmpfnc) ||
		(p->function == ST_UCHAR && toupper(c) == (int)p->cmpfnc) ||
		(p->function == ST_FUNC  && p->cmpfnc(c) );
		if( found )
			break;
	}

	next =  p->next_state;
	if( p->action )
		p->action(c,st->data);
	if( next >= st->nstates )
		return EST_BAD_STATE;

	if( st->debug )
		st->debug(st->state, next, c);

	st->state=next;
	return EST_OK;
}

int
stm_get_state( stm_t st )
{ 	int ret;

	if( !IS_ST(st) )
		ret = EST_BAD_PD;
	else	
		ret = st->state;

	return ret;
}

int
stm_reinit( stm_t st )
{ 	int ret = EST_OK;

	if( !IS_ST(st) )
		ret = EST_BAD_PD;
	else
		st->state = st->init_state;
		
	return ret;
}

int
stm_destroy( stm_t st )
{ 	int ret = EST_OK;

	if( !IS_ST(st) )
		ret = EST_BAD_PD;
	else
		free(st);
	
	return EST_OK;
}

