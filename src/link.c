/*
 * link.c -- 
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
  #include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "link.h"

#define PARSER_SIGNATURE	0xE2F3
#define IS_PARSER(m)	( m!=NULL && m->signature==PARSER_SIGNATURE)

const char *link_debug(int state);

enum state {	
/*extract*/	ST_START,
/*extract*/	ST_TAG,
/*extract*/	ST_ISTAG_A,
/*extract*/	ST_OTHERTAG,
/*extract*/	ST_TAG_A,
/*extract*/	ST_TAG_A_END,
/*extract*/	ST_TAG_A_END_IS_SLASH,
/*extract*/	ST_TAG_A_END_IS_SLASH_A,
/*extract*/	ST_TAG_A_OTHER,
/*extract*/	ST_TAG_A_H,
/*extract*/	ST_TAG_A_HR,
/*extract*/	ST_TAG_A_HRE,
/*extract*/	ST_TAG_A_HREF,
/*extract*/	ST_TAG_A_HREF_EQ,
/*extract*/	ST_TAG_A_HREF_EQ_READ
};

struct link_parserCDT
{	unsigned signature;
	unsigned char link[4096];
	unsigned char comment[4096];
	unsigned i,j;
	enum state state;
	link_callback link_fnc;
	void *user_data;
	int debug;
};

link_parser_t 
link_parser_new(void)
{	link_parser_t parser;

	parser = malloc(sizeof(*parser));
	if( parser )
	{	parser->signature = PARSER_SIGNATURE;
		parser->i = parser->j = 0;
		parser->state = ST_START;
		parser->link_fnc = NULL ;
		parser->user_data = NULL;
		parser->debug = 0;
	}

	return  parser;
}

void
link_parser_destroy(link_parser_t parser)
{
	free(parser);
}

void
link_parser_set_debug(link_parser_t parser, int b)
{	
	if( IS_PARSER(parser) )
		parser->debug = b !=0 ;
}

void
link_parser_set_link_callback(link_parser_t parser,link_callback call, void *d )
{
	if( IS_PARSER(parser) )
	{
		parser->link_fnc =  call;
		parser->user_data = d;
		
	}
}

/* large, straight foward, and boring
 */
int
link_parser_proccess_char( link_parser_t parser, int c )
{
	if(!IS_PARSER(parser))
		return -1;
		
	if( parser->debug )
	{	if( isprint(c) )
			printf("%c - %s \n",c,link_debug(parser->state));
		else
			printf("0x%x - %s \n",c,link_debug(parser->state));
	}

	switch(parser->state)
	{
		case ST_START:
			if( c == '<' )
				parser->state = ST_TAG;
			else
				parser->state = ST_START;
			break;
		case ST_TAG:
			if( c == '>' )
				parser->state = ST_START;
			else if( tolower(c) == 'a' )
				parser->state = ST_ISTAG_A;
			else
				parser->state = ST_OTHERTAG;
			break;
		case ST_ISTAG_A:
			parser->link[0] = 0;
			parser->comment[0] = 0;

			if( isspace(c) )
				parser->state = ST_TAG_A;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else
				parser->state = ST_OTHERTAG;
			break;
		case ST_OTHERTAG:
			if( c == '>' )
				parser->state = ST_START;
			else
				parser->state = ST_OTHERTAG;
			break;
		case ST_TAG_A:
			if( tolower(c) == 'h' )
				parser->state = ST_TAG_A_H;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else if( isspace(c) )
				parser->state = ST_TAG_A_H;
			else
				parser->state = ST_TAG_A_OTHER;
			break;
		case ST_TAG_A_OTHER:
			if( isspace(c) )
				parser->state = ST_TAG_A;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else
				parser->state = ST_TAG_A_OTHER;
			break;
		case ST_TAG_A_H:
			if( tolower(c) == 'r' )
				parser->state = ST_TAG_A_HR;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else
				parser->state = ST_TAG_A;
			break;
		case ST_TAG_A_HR:
			if( tolower(c) == 'e' )
				parser->state = ST_TAG_A_HRE;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else
				parser->state = ST_TAG_A;
			break;
		case ST_TAG_A_HRE:
			if( tolower(c) == 'f' )
				parser->state = ST_TAG_A_HREF;
			else if( c == '>' )
				parser->state = ST_TAG_A_END;
			else
				parser->state = ST_TAG_A;
			break;
		case ST_TAG_A_HREF:
			if( c == '>' )
				parser->state = ST_TAG_A_END;
			else if ( c == '=')
				parser->state = ST_TAG_A_HREF_EQ;
			else if( isspace(c) )
				parser->state = ST_TAG_A_HREF;
			else
				parser->state = ST_TAG_A;
			break;
		case ST_TAG_A_HREF_EQ:
			if( c == '>' )
				parser->state = ST_TAG_A_END;
			else if( isspace(c) )
				;
			else
			{	parser->i=0;
				parser->link[parser->i] = c;
				parser->state = ST_TAG_A_HREF_EQ_READ;
			}
			break;
		case ST_TAG_A_HREF_EQ_READ:
			if( c=='"' || c == '>' || c=='\'' )
			{	parser->link[parser->i]=0;
				parser->state = ( c == '>' ) ?  ST_TAG_A_END :
				                                ST_TAG_A;
			}
			else 
				parser->link[parser->i++]=c;
			break;
		case ST_TAG_A_END:
			if( c == '<' )
				parser->state = ST_TAG_A_END_IS_SLASH;
			else
			{	if( parser->j != 0 && isspace(c) && 
				    isspace(parser->comment[parser->j-1]) )
				    	;
				else
				{	if( isspace(c) )
						c = ' ';
					parser->comment[parser->j++]=c;
				}
			}
			break;
		case ST_TAG_A_END_IS_SLASH:
			if( isspace(c) )
				;
			else if( c == '>' )
			{	parser->comment[parser->j++] = '<';
				parser->comment[parser->j++] = '>';
			}
			else if( c == '/' )
				parser->state = ST_TAG_A_END_IS_SLASH_A;
			else
			{	parser->comment[parser->j++] = '<';
				parser->comment[parser->j++] = c;
				parser->state = ST_TAG_A_END;
			}
			break;
		case ST_TAG_A_END_IS_SLASH_A:
			if( tolower(c) == 'a' )
			{	parser->state = ST_OTHERTAG;
				parser->comment[parser->j]=0;
				parser->j = 0;
				if( parser->link_fnc )
				(*parser->link_fnc)(parser->link,
				                    parser->comment,
				                    parser->user_data);
			}
			else
			{	parser->comment[parser->j++] = '<';
				parser->comment[parser->j++] =  c;
				parser->state = ST_TAG_A_END;
			}
			break;
		default:
			printf("%d\n",parser->state);
			assert(0);
	}
	
	return 0;
}

