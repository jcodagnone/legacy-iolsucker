#include <stdlib.h>
#include <trace.h>

#include "iol.h"

const char *rs_program_name;

int
main( int argc, char **argv )
{
	rs_program_name = argv[0];
	rs_trace_to(rs_trace_stderr);

	IOL iol;
	if( iol.login("29503381","--argentina2k") != E_OK )
		return 0;

	iol.resync_all();

	iol.logout();

	return 0;
}

