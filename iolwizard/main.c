/**
 * \file main.c --  A map editor for the daedalus game
 *
 */
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <newopt.h>
#include <trace.h>
#include <basename.h>
#include <i18n.h>

#include "ui.h"
#include "../src/main.h"

#ifdef HAVE_CONFIG_H
  #ifdef WIN32
    #include "../winconfig.h"
  #else
    #include <config.h>
  #endif
#endif

const char *progname;
const char *rs_program_name;

int
main(int argc, char **argv)
{	struct opt opt;

	progname = basename(argv[0]);
	rs_trace_to(rs_trace_stderr);

	gtk_init(&argc,&argv);

	if( parseOptions( argc, argv, &opt) < 0 )
		return EXIT_FAILURE;

	if( load_config_file(&opt) == -1 )
		return EXIT_FAILURE; 

	create_ui((void *)&opt);

	gtk_main();

	g_mem_profile();
	
	return EXIT_SUCCESS;
}

#ifdef WIN32
int
WinMain (HANDLE hInst, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{	extern int argc;
	extern char ** __argv;
	
	return main(__argc, __argv);
}
#endif
