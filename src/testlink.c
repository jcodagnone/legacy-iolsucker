#include <stdio.h>

#include "link.h"

static void
link_fnc( const char *link, const char *comment, void *data)
{

	 
	printf( comment[0] ? "%s (%s)\n" : "%s\n",link,comment);
}

int
main( int argc, char **argv )
{	link_parser_t parser;
	int c;

	parser = link_parser_new();
	if( argc != 1 )
		link_parser_set_debug(parser,1);
	link_parser_set_link_callback(parser,link_fnc,NULL);
	while( (c=getchar())!=EOF && link_parser_proccess_char(parser,c)==0 )
		;

	link_parser_destroy(parser);

	return 0;
}
