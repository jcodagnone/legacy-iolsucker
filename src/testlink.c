#include <stdio.h>

#include "link.h"

static void
link_fnc( const char *link, const char *comment, void *data)
{

	 
	printf( comment[0] ? "%s (%s)\n" : "%s\n",link,comment);
}

int
main(void)
{	link_parser_t parser;
	int c;

	parser = link_parser_new();
	link_parser_set_link_callback(parser,link_fnc,NULL);
	while( (c=getchar())!=EOF && link_parser_proccess_char(parser,c)==0 )
		;

	link_parser_destroy(parser);

	return 0;
}
