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
  #ifdef WIN32
    #include "../configwin.h"
  #else
    #include "../config.h"
  #endif
#endif

#include <stdlib.h>
#include <string.h>

#include <trace.h>
#include <basename.h>

#include "i18n.h"
#include "main.h"
#include "config.h"
#include "getpass.h"

const char *rs_program_name;

static int
validate_opt(struct opt* opt)
{	int ret = 0;

	if( opt->username[0] == 0 )
	{	rs_log_info("unknown username");
		ret = -1;
	}
	else if( opt->password[0] == 0 )
	{	char *s;
	
	        s =  getpass_r(_("iol password: "),opt->password, 
	                        sizeof(opt->password));
	        if( s == NULL || *s == 0 )
		{	rs_log_info("unknown password");
			ret = -1;
		}
	}
	
	if( opt->repository[0] == 0  )
	{ 	rs_log_info("unknown repository");
		ret = -1;
	}

	return ret;
}

/* Usually i include the function that parse the command line options
 * in the main.c, but this time the function is share with iolwizard
 */
int
main( int argc, char **argv )
{	struct opt opt;
	iol_t iol;
	int ret;
	
	rs_program_name = basename(argv[0]);
	rs_trace_to(rs_trace_stderr);

	if( parseOptions(argc, argv, &opt) == -1 )
		return EXIT_FAILURE;

	if( load_config_file(&opt) == -1 )
		return EXIT_FAILURE;

	if( validate_opt(&opt) == -1 )
		return EXIT_FAILURE;

	iol = iol_new();
	if( iol == NULL )
	{	rs_log_error("creating IOL object. bye bye");
		
		return EXIT_FAILURE;
	}
	
	iol_set(iol, IOL_REPOSITORY, opt.repository);
	iol_set(iol, IOL_PROXY_HOST, opt.proxy);
	iol_set(iol, IOL_PROXY_USER, opt.proxy_user);
	iol_set(iol, IOL_PROXY_TYPE, opt.proxy_type);
	iol_set(iol, IOL_VERBOSE,    &(opt.verbose));

	rs_log_info(_("login on as `%s'"), opt.username);
	if( (ret = iol_login(iol, opt.username, opt.password)) != E_OK )
	{	const char *p = ret == E_NETWORK ? "login(): %s: %s" :
	 	                                   "login(): %s";
	 	                                   
		rs_log_error(_("login(): login failed"));
		rs_log_error(p,iol_strerror(ret), iol_get_network_error(iol));
		return 0;
	}

	if( (ret=iol_resync_all(iol)) != E_OK )
	{	const char *p = ret == E_NETWORK ? "resync_all(): %s: %s" :
	 	                                   "resync_all(): %s";
	 	                                   
		rs_log_error(_("resync_all(): login failed"));
		rs_log_error(p,iol_strerror(ret), iol_get_network_error(iol));
		return 0;
	}
	else
	{
		rs_log_info(_("hay novedades?"));
		ret = iol_get_new_novedades(iol);
		if( ret != -1 )
		{ 	if( ret == 0 )
				rs_log_info(_("no hay noticias :^("));
			else
				rs_log_info(_("si. hay %d"),ret);
		}

	}

	rs_log_info(_("logging off"));
	iol_logout(iol);
	iol_destroy(iol);

	free_options(&opt);
	
	return 0;
}

