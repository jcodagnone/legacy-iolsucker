/*
 * main.cpp -- IOLsucker web robot implementation
 *
 * Copyright (C) 2003 by Juan F. Codagnone <juam@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

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

	if( iol.resync_all() != E_OK )
	{
		rs_log_info("eeehhh!");
		return 0;
	}

	iol.logout();

	return 0;
}

