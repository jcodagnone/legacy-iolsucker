/*
 * opt.c -- parse command line options
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
#ifdef HAVE_CONFIG_H
  #include <config.h>
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
" -v   --verbose              prints verbose information\n"
" -u   --user username        specify the login username\n"
" -n   --dry-run              dry-run: don't download any files. just report\n"
" -F   --forum                resync the forums\n"
" -W   --wait                 wait when changing context\n"
" -X   --xenofobe             prints forein files\n"
" -t   --proxy-type type      proxy type. http is the default (other: socks5)\n"
" -x   --proxy <host[:port]>  use proxy. (default port is 1080)\n"
"      --fancy                use fancy names as course directories\n"
"      --no-cache             don't use any file cache (if available)\n"
"      --dump <prefix>        dumps all the transfer to the directory <prefix>"
" -U <user[:password]>        specify proxy authentication\n"
" -f filename                 load settings from file\n"
" -r repository               sets the file  repository\n" 
" -H <host[:port]>            sets the server address\n" 
"\n"
"Send bugs to <juam at users dot sourceforge dot net>\n"
"\n"),rs_program_name);

	exit( EXIT_SUCCESS );
}

static void 
usage ( void )
{
	printf("Usage: %s [-hnVvWX] [--help] [--version] [--dry-run] [--fancy]"
	       " [-F] [--forum] [--wait] [--xenofobe] [--no-cache]"
	       " [--dump prefix] [-u username] [-r repository]"
	       " [-f filename] [-x <host[:port]>] [-U <username[:port]]>"
	       " [-t proxy-type ] [--proxy-type proxy-type] "
	       " [-H <server[port]]\n",
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

int
parseOptions( int argc, char * const * argv, struct opt *opt)
{	int i;
	const char *user = 0, *configfile = 0, *rep=0,  *proxy_type = "";
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
	 /*12*/ {"t",           OPT_NORMAL, 1,  OPT_T_GENER,  NULL },
	 /*13*/ {"proxy-type",  OPT_NORMAL, 0,  OPT_T_GENER,  NULL },
	 /*14*/ {"v",           OPT_NORMAL, 1,  OPT_T_FLAG,  NULL  },
	 /*15*/ {"verbose",     OPT_NORMAL, 0,  OPT_T_FLAG,  NULL  },
	 /*16*/ {"r",           OPT_NORMAL, 1,  OPT_T_GENER, NULL },
	 /*17*/ {"fancy",       OPT_NORMAL, 0,  OPT_T_FLAG, NULL},
	 /*18*/ {"F",           OPT_NORMAL, 1,  OPT_T_FLAG, NULL},
	 /*19*/ {"forum",       OPT_NORMAL, 0,  OPT_T_FLAG, NULL},
	 /*20*/ {"W",      	OPT_NORMAL, 1,  OPT_T_FLAG, NULL},
	 /*21*/ {"wait",      	OPT_NORMAL, 0,  OPT_T_FLAG, NULL},
	 /*22*/ {"H",      	OPT_NORMAL, 1,  OPT_T_GENER, NULL},
	 /*23*/ {"X",      	OPT_NORMAL, 1,  OPT_T_FLAG,  NULL},
	 /*24*/ {"xenofobe",   	OPT_NORMAL, 0,  OPT_T_FLAG,  NULL},
	 /*25*/ {"no-cache",    OPT_NORMAL, 0,  OPT_T_FLAG,  NULL},
	 /*26*/ {"dump",        OPT_NORMAL, 0,  OPT_T_GENER, NULL},
	 	{NULL,          OPT_NORMAL, 0,  OPT_T_GENER, 0 }
	}; lopt[4].data = lopt[5].data = (void *) &user;
	   lopt[6].data = (void *) &configfile;
	   lopt[7].data = &(opt->proxy_user);
	   lopt[8].data = lopt[9].data = &(opt->proxy);
	   lopt[10].data = lopt[11].data = &(opt->dry);
	   lopt[12].data = lopt[13].data = (char *)&(proxy_type);
	   lopt[14].data = lopt[15].data = &(opt->verbose);
	   lopt[16].data = (char *)&rep;
	   lopt[17].data = &(opt->fancy);
	   lopt[18].data = lopt[19].data = &(opt->forum);
	   lopt[20].data = lopt[21].data = &(opt->wait);
	   lopt[22].data = &(opt->server);
	   lopt[23].data = &(opt->xenofobe);
	   lopt[24].data = &(opt->xenofobe);
	   lopt[25].data = &(opt->no_cache);
	   lopt[26].data = &(opt->dump);
	   
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
	if( rep )
		strncpy(opt->repository, rep, sizeof(opt->repository)-1);
	if( !strcmp(proxy_type, "socks5") )
		opt->proxy_type = "socks5";
	else if( !strcmp(proxy_type, "http") )
		opt->proxy_type = "http";
	else if( *proxy_type == 0 )
		opt->proxy_type = "";
	else
	{	rs_log_error(_("unknown proxy-type `%s'"),proxy_type);
			return -1;
	}
	if( opt->proxy_user )
		opt->proxy_user = strdup(opt->proxy_user);
	if( opt->proxy )
		opt->proxy = strdup(opt->proxy);
	if( opt->server )
		opt->server = strdup(opt->server);

	return 0;
}

void
free_options( struct opt *opt )
{
	if( opt )
	{	free(opt->proxy);
		free(opt->proxy_user);
		free(opt->server);
	}
}
