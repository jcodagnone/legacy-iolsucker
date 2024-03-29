/*
 * main.c -- entry point for iolwizard
 *
 * Copyright (C) 2003, 2004 by Juan F. Codagnone <juam@users.sourceforge.net>
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
#include <string.h>

#include <gtk/gtk.h>

#include <libmisc/newopt.h>
#include <libmisc/trace.h>
#include <libmisc/basename.h>
#include <libmisc/i18n.h>

#include "ui.h"
#include "../src/main.h"
#include "../src/fconfig.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

const char *progname;
const char *rs_program_name = ""; 

void
version( void )
{	char buff[4096];
	printf( "%s %s\n"
		"\n"
		"This is free software:\n"
		"   There is NO warranty; not even for MERCHANTABILITY or\n"
		"   FITNESS FOR A PARTICULAR PURPOSE\n"
		"\n"
		,rs_program_name, VERSION);

	exit( EXIT_SUCCESS );
}


int
main(int argc, char **argv)
{	struct opt opt;

	rs_program_name = progname = basename(argv[0]);
	rs_trace_to(rs_trace_stderr);

	gtk_init(&argc,&argv);

	if( parseOptions( argc, argv, &opt) < 0 )
		return EXIT_FAILURE;

	load_config_file(&opt);	/* ignore if the file can't loaded */
	create_ui((void *)&opt);

	gtk_main();

	g_mem_profile();
	
	return EXIT_SUCCESS;
}

#ifdef WIN32
#include <windows.h>

int WINAPI 
WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpszCmdLine, int nCmdShow)
{	extern int __argc;
	extern char ** __argv;
	
	return main(__argc, __argv);
}
#endif
