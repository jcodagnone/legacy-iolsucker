#include <stdio.h>
#include <string.h>

#include "link.h"

static void
link_fnc( const char *link, const char *comment, void *data)
{	int d = *((int*)data);

	printf( comment[0] && d ? "%s (%s)\n" : "%s\n",link,comment);
}

int
main( int argc, char **argv )
{	link_parser_t parser;
	int c, i;
	int print_comment=1;

	parser = link_parser_new();

	for( i=1; i< argc ; i++ )
	{
		if( !strcmp(argv[i], "-d") )
			link_parser_set_debug(parser,1);
		else if( !strcmp(argv[i], "-n"))
			print_comment = 0;
	}
	link_parser_set_link_callback(parser,link_fnc,&print_comment);
	while( (c=getchar())!=EOF && link_parser_proccess_char(parser,c)==0 )
		;

	link_parser_destroy(parser);

	return 0;
}
