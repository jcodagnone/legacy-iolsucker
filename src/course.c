/*
 * course.c -- handle iol's courses
 *
 * Copyright (C) 2003-2004 by Juan F. Codagnone <juam@users.sourceforge.net>
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
 #include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <glib.h>

#include "link.h"
#include "course.h"

#define IS_COURSE_T(c)  ( (c) != NULL )

#define COURSE_PARAMETER	"nivel=4"
#define DEPART_PARAMETER	"nivel=3"

struct course_t
{	GSList *list;
};

course_t 
course_new(void)
{	course_t c;

	c = malloc(sizeof(*c));
	if( c )
	{
		memset(c, 0, sizeof(*c));
	}
	
	return c;
}

int 
course_is_valid(course_t c)
{

	return IS_COURSE_T(c);
}


static void
free_courses_list(struct course *data, gpointer user_data )
{
	if(data)
	{	free(data->code);
		free(data->name);
		free(data);
	}
}

void
course_destroy(course_t c)
{
	if( IS_COURSE_T(c) )
	{
		g_slist_foreach(c->list, (GFunc)free_courses_list ,NULL);
		g_slist_free(c->list);
		free(c);
	}
}

/* equivalet to: one_space ( touper ( trim( name ) ) ) */
static char *
normalize_course_name( char *name )
{       char *p, *q;

        for( q = p = name ; *p && isspace(*p) ; p++ )
                ;
        while( *p )
        {
                for( ; *p && !isspace(*p) ; p++ )
                        *(q++) = toupper(*p);

                if( isspace(*p) )
                {       *(q++) = *p++;
                        for( ; *p && isspace(*p) ;  p++ )
                                ;
                }
        }

        *q = 0;
        if( *(q-1) == ' ' )
        	*(q-1) = 0;

        return name;
}

/**
 * callback called by the html parser: it is used to extract the user's courses
 * codes from the html
 */
static void 
course_load_from_page_fnc( unsigned const char *link, 
                  unsigned const char *comment, void *d )
{	GSList **listptr =  d;
	struct course *course; 
	char *s;
	enum course_type type;
	assert(d);

	s = strstr(link, COURSE_PARAMETER);
	if( s == NULL )
	{	s = strstr(link, DEPART_PARAMETER);
		if( s == NULL )
			return;
		else
			type = CT_DEPART;
	}
	else
		type = CT_COURSE;
	
	
	/* get course code */
	s =  strstr(link,"snivel=" );
	if( s )
	{	char *ss;

		s += sizeof("snivel=") -1 ;
		for( ss = s; *ss && *ss != ';' ; ss++ )
			;
		if( *ss == ';' )
			*ss = 0;

		for( ss = (char *)comment; *ss && isspace(*ss) ; ss++)
			;

		course = malloc(sizeof(*course));
		if( course ) 
		{ 	course->code = strdup(s);
			course->name = normalize_course_name(strdup(ss));
			course->type = type;
			course->flags = 0;
			if( course->code && course->name )
				*listptr = g_slist_prepend(*listptr, course);
			else
			{	free(course->code);
				free(course->name);
				free(course);
			}
		}
	}
}


int 
course_load_from_page(course_t c, struct buff *page)
{	link_parser_t parser;
	unsigned i;
	GSList **listptr = &(c->list);

	parser = link_parser_new();
	if( parser == NULL )
		return 0;

	link_parser_set_link_callback(parser,course_load_from_page_fnc,listptr);
	for( i = 0 ; i< page->size &&
	     link_parser_process_char(parser,page->data[i])==0 ; i++ ) 
	     ;
	link_parser_end(parser);
	link_parser_destroy(parser);

	return (*listptr)== NULL ? -1 : 0;
}

int
course_name_is_valid( const char *code )
{	int b = 1;
	unsigned i;

	if( code == NULL )
		b = 0;
	else {
		for( i=0; b && code[i] ; i++ )
		{       /* dangerous characters for the filesystem */
			if( code[i]=='/' || code[i] == '\\' ||
			    code[i]==':' || code[i] == '>' /* win32 */ )
				b = 0;
		}

		if( i == 0 )
			b = 0;
	}

	return b;
}

/*****************************************************************************/
static void 
course_get_capabilities_from_page_fnc( const unsigned char *link,
                  const unsigned char *comment, enum resync_flags *flags)
{	static const struct 
	{	const char *link;
		enum resync_flags flag;
	} table[]=
	{	{ "newmaterialdid.asp", IOL_RF_FILE  },
		{ "reglamentacion.asp", IOL_RF_RULES },
		{ "novlist.asp",        IOL_RF_NEWS  },
		{ "foroDis.asp",        IOL_RF_FORUM },
		{ "AluList.asp",        IOL_RF_ALULIST},
	};
	unsigned i;
	int found=0;

	assert(flags);
	
	for(i=0; !found && i<sizeof(table)/sizeof(*table); i++)
	{
		if( !strcmp(link,table[i].link) )
			found=1;
	}

	if( found )
		*flags |=table[i-1].flag;
}

enum resync_flags
course_get_capabilities_from_page( struct buff *page )
{ 	link_parser_t  parser;
	enum resync_flags flags = 0;
	unsigned i;
	
	parser = link_parser_new();
	if( parser )
	{
		link_parser_set_link_callback(parser,
				(void *)course_get_capabilities_from_page_fnc,
				&flags);
		for( i = 0 ; i< page->size &&
		           link_parser_process_char(parser,page->data[i])==0;
		     i++ ) 
		     	;
		link_parser_end(parser);
		link_parser_destroy(parser);
	}
	
	
	return flags;
}
/*****************************************************************************/

/** temporary structure: used to send parameters from #course_get_by_name to 
 *  #foreach_get_by_name
 */
struct tmp_foreach_get_by_name
{	const char *code;
	struct course *ret;
};

static void
foreach_get_by_name( struct course *course, struct tmp_foreach_get_by_name *t)
{
	if( t && !t->ret )
	{	
		if( !strcmp(t->code, course->code) )
			t->ret = course;
	}
}

struct course *
course_get_by_name( course_t c, const char *code )
{	struct tmp_foreach_get_by_name tmp;

	tmp.code = code;
	tmp.ret = NULL;
	
	g_slist_foreach(c->list, (void *)foreach_get_by_name, (void *)&tmp);
	
	return tmp.ret;
}


/*****************************************************************************/

void
course_foreach_run( course_t c, course_callback_t callback,void *data)
{

	g_slist_foreach(c->list, callback, data);
}



