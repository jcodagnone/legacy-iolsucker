/**
 * \file main.c --  A map editor for the daedalus game
 *
 */
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <newopt.h>
#include <basename.h>
#include <i18n.h>

#include "ui.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

struct opt
{
	int foo;
};

const char *progname;
const char *rs_program_name;

static void
help( void )
{
	g_print(
	"Usage: %s [OPTION]\n"
	"Create a config file for iolsucker\n"
	"\n"
	"OPTIONS\n"
	/* X   X                      X */
	" -h   --help                 prints this messages and dies\n"
	" -V   --version              prints the version info and dies\n"
	"\n"
	"Send bugs to ...\n"
	"\n",progname);

	exit( EXIT_SUCCESS );
}


static void 
usage ( void )
{
	g_print( "%s [-hV] [--version] [--help]\n", progname);

	exit( EXIT_SUCCESS );
}

static void
version( void )
{
	g_print( "%s %s\n"
		"\n"
		"This is free software:\n"
		" There is NO warranty; not even for MERCHANTABILITY or\n"
		" FITNESS FOR A PARTICULAR PURPOSE\n",progname,VERSION);

	exit( EXIT_SUCCESS );
}

static int
parseOptions( int argc, char * const * argv, struct opt *opt)
{	int i = 0;
	static optionT lopt[]=
	{/*0*/	{"help",	OPT_NORMAL, 0,	OPT_T_FUNCT, (void *) help },
	 /*1*/	{"h",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) help },
	 /*2*/	{"version",	OPT_NORMAL, 0,  OPT_T_FUNCT, (void *) version},
	 /*3*/	{"V",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) version},
		{NULL,		OPT_NORMAL, 1,  OPT_T_FUNCT, NULL}
	};

	g_assert( argv && opt );
	memset(opt,0,sizeof(*opt) );
	i = GetOptions( argv, lopt, 0, NULL);
	if( i < 0 )
	{	g_message("parsing options");
		usage();
		return -1;
	}

	return i;
}


int
main(int argc,char *argv[])
{	struct opt opt;

	progname = basename(argv[0]);

	gtk_init(&argc,&argv);

	if( parseOptions( argc, argv, &opt) < 0 )
		return EXIT_FAILURE;

	create_ui((void *)&opt);

	gtk_main();

	g_mem_profile();
	
	return EXIT_SUCCESS;
}
