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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdlib.h>
#include <ctype.h>
#include <stm.h>
#include "link.h"

enum state {
/*extract*/     ST_START,
/*extract*/     ST_TAG,
/*extract*/     ST_ISTAG_A,
/*extract*/     ST_OTHERTAG,
/*extract*/     ST_TAG_A,
/*extract*/     ST_TAG_A_END,
/*extract*/     ST_TAG_A_END_IS_SLASH,
/*extract*/     ST_TAG_A_END_IS_SLASH_A,
/*extract*/     ST_TAG_A_OTHER,
/*extract*/     ST_TAG_A_H,
/*extract*/     ST_TAG_A_HR,
/*extract*/     ST_TAG_A_HRE,
/*extract*/     ST_TAG_A_HREF,
/*extract*/     ST_TAG_A_HREF_EQ,
/*extract*/     ST_TAG_A_HREF_EQ_READ,
/*extract*/     ST_TAG_A_END_IS_SLASH_A_OTHER,
/*extract*/	ST_TAG_A_END_IS_A_AGAIN
};

struct link_parserCDT
{	unsigned signature;
	unsigned char link[4096];
	unsigned char comment[4096];
	unsigned i,j;
	link_callback link_fnc;
	void *user_data;
	stm_t st;
};

/* transition functions */
/* 
  isspace  @fextract@
 */
static int
init_parser(int c, void *data)	/* @fextract@ */
{	link_parser_t parser = data;

	parser->link[0] = 0;
	parser->comment[0] = 0;

	return 0;
}

static int
link_first_char(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	parser->i=0;
	parser->link[parser->i] = c;

	return 0;
}

static int
endlink(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	parser->link[parser->i]=0;

	return 0;
}

static int
add_char(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	if( parser->i + 1 < sizeof(parser->link) )
	{ 	parser->link[parser->i++]=c;
		parser->link[parser->i]=0;
	}
	
	return 0;
}

static int
addcomment(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

       if( parser->j != 0 && isspace(c) &&
	    isspace(parser->comment[parser->j-1]) )
		;
	else
	{       if( isspace(c) )
			c = ' ';
		if( parser->j + 1 < sizeof(parser->comment) )
		{ 	parser->comment[parser->j++]=c;
			parser->comment[parser->j]=0;
		}
	}

	return 0;
}

static int
e_is_slash(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] = '<';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] = c;

	parser->comment[parser->j] = 0;
	
	return 0;
}

static int
e_slash_a(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] = '<';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] =  '/';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] =  c;

	parser->comment[parser->j] = 0;

	return 0;
}

static int
e_slash_a_other(int c, void *data) /* @fextract@ */
{       link_parser_t parser = data;

	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] = '<';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] =  '/';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] =  'a';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++] =  c;

	parser->comment[parser->j] = 0;

	return 0;
}

static int
done_link(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	parser->comment[parser->j]=0;
	parser->j = 0; 
	if( parser->link_fnc )
		 (*parser->link_fnc)(parser->link, parser->comment,
		                     parser->user_data);

	return 0;
}

static int
embeeded_goto_end(int c, void *data) /* @fextract@ */
{	link_parser_t parser = data;

	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++]='<';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++]='/';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++]='a';
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++]=c;
	if( parser->j + 1 < sizeof(parser->comment) )
		parser->comment[parser->j++]=0;
	
	parser->comment[parser->j] = 0;

	return 0;
}

/* state table */
static ST_PARSE st_start[]=
{	{ ST_CHAR,	'<',	ST_TAG  ,	NULL },
	{ ST_FUNC,	ELSE,	ST_START,	NULL },
};

static ST_PARSE st_tag[]=
{	{ ST_CHAR,	'>',	ST_START  ,	NULL	},
	{ ST_LCHAR,	'a',	ST_ISTAG_A,	init_parser},
	{ ST_FUNC,	ELSE,	ST_OTHERTAG,	NULL	}
};

static ST_PARSE st_istag_a[]=
{	{ ST_FUNC,	isspace,ST_TAG_A,	NULL	},
	{ ST_CHAR,	'>',   	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	ELSE,	ST_OTHERTAG,	NULL	}
};

static ST_PARSE st_othertag[]= 
{	{ ST_CHAR,	'>',	ST_START,	NULL	},
	{ ST_FUNC,	ELSE,	ST_OTHERTAG,	NULL	}
};

static ST_PARSE st_tag_a[]=
{	{ ST_LCHAR,	'h',	ST_TAG_A_H,	NULL	},
	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	isspace,ST_TAG_A_H,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A_OTHER,	NULL	}
};

static ST_PARSE st_tag_a_other[]=
{	{ ST_FUNC,	isspace,ST_TAG_A,	NULL	},
	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A_OTHER,	NULL	}
};

static ST_PARSE st_tag_a_h[]=
{	{ ST_LCHAR,	'r',	ST_TAG_A_HR,	NULL	},
	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A,	NULL	}  
};

static ST_PARSE st_tag_a_hr[]=
{	{ ST_LCHAR,	'e',	ST_TAG_A_HRE,	NULL	},
	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A,	NULL	}
};

static ST_PARSE st_tag_a_hre[]=
{	{ ST_LCHAR,	'f',	ST_TAG_A_HREF,	NULL	},
	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A,	NULL	}
};

static ST_PARSE st_tag_a_href[]=
{	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_CHAR,	'=',	ST_TAG_A_HREF_EQ,NULL	},
	{ ST_FUNC,	isspace,ST_TAG_A_HREF,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A,	NULL	}
};

static ST_PARSE st_tag_a_href_eq[]=
{	{ ST_CHAR,	'>',	ST_TAG_A_END,	NULL	},
	{ ST_FUNC,	isspace,ST_TAG_A_HREF_EQ, NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A_HREF_EQ_READ,	link_first_char},
};

static ST_PARSE st_tag_a_href_eq_read[]=
{	{ ST_CHAR,	'"',	ST_TAG_A,	endlink },
	{ ST_CHAR,	'\'',	ST_TAG_A,	endlink },
	{ ST_CHAR,	'>',	ST_TAG_A_END,	endlink },
	{ ST_FUNC,	ELSE,	ST_TAG_A_HREF_EQ_READ,	add_char   }
};

static ST_PARSE st_tag_a_end[]=
{	{ ST_CHAR, 	'<',	ST_TAG_A_END_IS_SLASH,	NULL        	},
	{ ST_FUNC, 	ELSE,   ST_TAG_A_END,         	addcomment	}
};

static ST_PARSE st_tag_a_end_is_slash[]=
{	{ ST_FUNC,	isspace,	ST_TAG_A_END_IS_SLASH,  	NULL },
	{ ST_CHAR,	'/',   		ST_TAG_A_END_IS_SLASH_A,	NULL },
	{ ST_LCHAR,	'a',   		ST_TAG_A_END_IS_A_AGAIN,	NULL },
	{ ST_FUNC,	ELSE,    	ST_TAG_A_END,              e_is_slash},
};

static ST_PARSE st_tag_a_end_is_slash_a[]=
{	{ ST_LCHAR,	'a',	ST_TAG_A_END_IS_SLASH_A_OTHER,	NULL	},
	{ ST_FUNC,	ELSE,	ST_TAG_A_END,                 	e_slash_a}
};

static ST_PARSE st_tag_a_end_is_slash_a_other[]=
{	{ ST_CHAR,'>',    	ST_START,	done_link },
	{ ST_FUNC,isspace,    	ST_START,	done_link },
	{ ST_FUNC,ELSE,    	ST_TAG_A_END,	e_slash_a_other}
};

/* embeeded link in what is supposed to be the link text */
static ST_PARSE st_tag_a_end_is_a_again[]=
{ 	{ ST_FUNC,      isspace,ST_TAG_A,       done_link },
	{ ST_FUNC,      ELSE,   ST_TAG_A_END,   embeeded_goto_end }
};

static ST_PARSE *link_table[]=
{
	st_start,
	st_tag,
	st_istag_a,
	st_othertag,
	st_tag_a,
	st_tag_a_end,
	st_tag_a_end_is_slash,
	st_tag_a_end_is_slash_a,
	st_tag_a_other,
	st_tag_a_h,
	st_tag_a_hr,
	st_tag_a_hre,
	st_tag_a_href,
	st_tag_a_href_eq,
	st_tag_a_href_eq_read,
	st_tag_a_end_is_slash_a_other,
	st_tag_a_end_is_a_again
};


/*****************************************************************************/
#include <stdio.h>
#include <string.h>

#define GEN_LIN
#include "link_debug.c"

#define NELEMS(a)	(sizeof(a)/sizeof(*(a)))
#define PARSER_SIGNATURE	0xE2F3
#define IS_PARSER(m)	( m!=NULL && m->signature==PARSER_SIGNATURE)

const char *link_debug(int state);

static void
link_internal_debug(int old, int new, int c)
{
	printf("%c - %s\n",c,link_debug(old));
}

link_parser_t 
link_parser_new(void)
{	link_parser_t parser;

	

	parser = malloc(sizeof(*parser));
	if( parser )
	{	memset(parser, 0, sizeof(*parser) );
		parser->signature = PARSER_SIGNATURE;
		parser->i = parser->j = 0;
		parser->link_fnc = NULL ;
		parser->user_data = NULL;
		parser->st = stm_new(link_table,  NELEMS(link_table), ST_START,
		                    parser);
		if( parser->st == NULL )
		{	free(parser);
			parser = NULL;
		}
		
	}

	return  parser;
}

void
link_parser_destroy(link_parser_t parser)
{
	if( IS_PARSER(parser) ) 
	{	stm_destroy(parser->st);
		free(parser);
	}

}

void
link_parser_set_debug(link_parser_t parser, int b)
{	
	if( IS_PARSER(parser) )
		stm_set_debug(parser->st, b ? link_internal_debug : NULL );
}

void
link_parser_set_link_callback(link_parser_t parser,link_callback call, void *d )
{
	if( IS_PARSER(parser) )
	{ 	parser->link_fnc =  call;
		parser->user_data = d;
	}
}

int
link_parser_process_char( link_parser_t parser, int c )
{
	return stm_parse(parser->st, c);
}

void
link_parser_end(link_parser_t parser)
{	int state;

	/* we have a link in the buffer, but the author forgot about
	 * the </a>. the comment should be ignored
	 */

	state = stm_get_state(parser->st);
	if( state == ST_TAG_A_END || 
	    state == ST_TAG_A_END_IS_SLASH ||
	    state == ST_TAG_A_END_IS_SLASH_A_OTHER)
	    	(*parser->link_fnc)(parser->link, "", parser->user_data);

}


/*****************************************************************************/
#ifdef LINK_TEST
static void
link_fnc( unsigned const char *link, const unsigned char *comment, void *data)
{	int d = *((int*)data);

	printf( comment[0] && d ? "%s (%s)\n" : "%s\n",link,comment);
}

int
main(int argc, char **argv)
{	link_parser_t parser;
	int c, i;
	int print_comment=1;

	parser = link_parser_new();

	
	for( i=1; i< argc ; i++ )
	{
		if( !strcmp(argv[i], "-d") )
			link_parser_set_debug(parser,1);
		else if( !strcmp(argv[i], "-g") )
			stm_print_digraph(stdout, link_table,
			                  NELEMS(link_table), link_debug,
			                  links_fnc_name);
		else if( !strcmp(argv[i], "-n"))
			print_comment = 0;
	}
	link_parser_set_link_callback(parser,link_fnc,&print_comment);
	while( (c=getchar())!=EOF && link_parser_process_char(parser,c)==0 )
		;
	link_parser_end(parser);
	link_parser_destroy(parser);

	return 0;
}

#endif
