/*
 * ask_pass.c -- ask a password in a portable way
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
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include <trace.h>

#include "i18n.h"
#include "main.h"
#include "ask_pass.h"

int
ask_password( struct opt *opt )
{	int attr;
	struct termios old,new;

	printf(_("\nEnter the password for `%s'> "),opt->username );
	if( tcgetattr(0, &old) == -1 )
	{	rs_log_warning(_("error getting terminal attributes"));
		return -1;
	}

	new = old;
	new.c_lflag &= ~ECHO;
	new.c_lflag |= ECHONL;

	if( tcsetattr(fileno(stdin), TCSANOW, &new) == -1 || 
	    tcsetattr(fileno(stdin), TCSANOW, &new) == -1 ||
	    new.c_lflag & ECHO ) 
	{	rs_log_warning(_("error setting terminal attributes"));
		return -1;
	}
	fgets(opt->password, sizeof(opt->password), stdin);
	opt->password[sizeof(opt->password)] = 0;
	tcsetattr(0, TCSANOW, &old);

	if( opt->password[0] == 0 )
		return -1;

	return 0;
}

