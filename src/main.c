/*
 * main.c -- IOLsucker web robot implementation
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

#include "iol.h"

#ifdef HAVE_CONFIG_H
  #include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <trace.h>
#include <newopt.h>
#include <basename.h>

#include "i18n.h"
#include "main.h"
#include "config.h"

const char *rs_program_name;

static void
help ( void )
{	printf (
_("Usage: %s [OPTION]\n"
"Downloads things from IOL\n"
"\n"
"OPTIONS\n"
/* X   X                      X */
" -V   --version              prints the version info and dies\n"
" -h   --help                 prints this message\n"
" -u   --user username        specify the login username\n"
" -n   --dry-run              dry-run: don't download any files. just report\n"
" -x   --proxy <host[:port]>  use proxy. (default port is 1080)\n"
" -U <user[:password]>        specify proxy authentication\n"
" -f filename                 load settings from file\n"

"\n"
"Send bugs to <juam at users dot sourceforge dot net>\n"
"\n"),rs_program_name);

	exit( EXIT_SUCCESS );
}

static void 
usage ( void )
{
	printf("Usage: %s [-hnV] [--help] [--version] [--dry-run] [-u username]"
	       " [-f filename] [-x <host[:port]>] [-U <username[:port]]>\n",
	       rs_program_name);

	exit( EXIT_SUCCESS );
}

static void
version( void )
{
	printf( "%s %s\n"
		"\n"
		"This is free software:\n"
		" There is NO warranty; not even for MERCHANTABILITY or\n"
		" FITNESS FOR A PARTICULAR PURPOSE\n",rs_program_name,VERSION);

	exit( EXIT_SUCCESS );
}

static int
parseOptions( int argc, char * const * argv, struct opt *opt)
{	int i;
	const char *user = 0, *configfile = 0;
	static optionT lopt[]=
	{/*00*/	{"help",	OPT_NORMAL, 0,	OPT_T_FUNCT, (void *) help },
	 /*01*/	{"h",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) help },
	 /*02*/	{"version",	OPT_NORMAL, 0,  OPT_T_FUNCT, (void *) version},
	 /*03*/	{"V",		OPT_NORMAL, 1,  OPT_T_FUNCT, (void *) version},
	 /*04*/ {"u",      	OPT_NORMAL, 1,  OPT_T_GENER, NULL },
	 /*05*/ {"user",	OPT_NORMAL, 0,  OPT_T_GENER, NULL },
	 /*06*/ {"f",   	OPT_NORMAL, 1,	OPT_T_GENER, NULL },
	 /*07*/ {"U",           OPT_NORMAL, 1,  OPT_T_GENER, NULL },
	 /*08*/ {"x",           OPT_NORMAL, 1,  OPT_T_GENER, NULL },
	 /*09*/ {"proxy",       OPT_NORMAL, 0,  OPT_T_GENER, NULL },
	 /*10*/ {"n",           OPT_NORMAL, 1,  OPT_T_FLAG,  NULL },
	 /*11*/ {"dry-run",     OPT_NORMAL, 0,  OPT_T_FLAG,  NULL },
	 	{NULL,          OPT_NORMAL, 0,  OPT_T_FUNCT, 0 }
	}; lopt[4].data = lopt[5].data = (void *) &user;
	   lopt[6].data = (void *) &configfile;
	   lopt[7].data = &(opt->proxy_user);
	   lopt[8].data = lopt[9].data = &(opt->proxy);
	   lopt[10].data = lopt[11].data = &(opt->dry);

	assert( argv && opt );
	memset(opt,0,sizeof(*opt) );
	i = GetOptions( argv, lopt, 0, NULL);
	if( i < 0 )
	{	rs_log_error("parsing options");
		usage();
		return -1;
	}

	if( user )
		strncpy(opt->username, user, sizeof(opt->username)-1);
	if( configfile )
		strncpy(opt->configfile, configfile, sizeof(opt->configfile)-1);

	if( opt->proxy_user )
		opt->proxy_user = strdup(opt->proxy_user);
	if( opt->proxy )
		opt->proxy = strdup(opt->proxy_user);
	/** \todo support strdup error condition */

	return 0;
}

void free_options( struct opt *opt )
{
	if( opt )
	{	free(opt->proxy);
		free(opt->proxy_user);
	}
}

int
main( int argc, char **argv )
{	struct opt opt;
	iol_t iol;
	
	rs_program_name = basename(argv[0]);
	rs_trace_to(rs_trace_stderr);

	if( parseOptions(argc, argv, &opt) == -1 )
		return EXIT_FAILURE;

	if( load_config_file(&opt) == -1 )
		return EXIT_FAILURE;

	iol = iol_new();
	if( iol == NULL )
	{	rs_log_error("creating IOL object. bye bye");
		return EXIT_FAILURE;
	}
	
	iol_set(iol, IOL_REPOSITORY, opt.repository);
	iol_set(iol, IOL_PROXY_HOST, opt.proxy);
	iol_set(iol, IOL_PROXY_USER, opt.proxy_user);

	rs_log_info(_("login on as `%s'"), opt.username);
	if( iol_login(iol, opt.username, opt.password) != E_OK )
	{	rs_log_error(_("login(): login failed"));
		return 0;
	}

	if( iol_resync_all(iol) != E_OK )
	{ 	rs_log_info("eeehhh!");
		return 0;
	}

	rs_log_info(_("logging off"));
	iol_logout(iol);
	iol_destroy(iol);

	free_options(&opt);
	
	return 0;
}
