/*
 * iolunch.c -- program luncher for console apps
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
  #ifdef WIN32
    #include "../configwin.h"
  #else
    #include "../config.h"
  #endif
#endif

#include <windows.h>
#include <stdio.h>

struct tmp
{ 	const char *program;
	HANDLE lock;
};

static void
press_any_key(void)
{
	printf("press ENTER to exit");
	getchar();
}

static DWORD WINAPI
thread_exec( void *d )
{	struct tmp *data = d;
	PROCESS_INFORMATION pi;
	STARTUPINFO a;

	ZeroMemory( &a, sizeof(STARTUPINFO) );
	a.cb = sizeof(STARTUPINFO); 
	if(!CreateProcess(NULL, data->program, 0,0,FALSE,0,NULL,NULL,&a,&pi))
		fprintf(stderr,"error executing '%s'\n", data->program);
	else
	{	if( pi.hProcess != INVALID_HANDLE_VALUE )
		 	WaitForSingleObject(pi.hProcess, INFINITE);
		
	}	

	SetEvent(data->lock);

	return 0;
}

int
main(int argc, char **argv)
{ 	DWORD id;
	int ret = 0;
	struct tmp t;

	if( argc == 1 )
		ret = 1;
	else
	{ 	t.program = argv[1];
		t.lock = CreateEvent(NULL,FALSE,FALSE,NULL);
		CreateThread(NULL, 0, thread_exec , &t, 0, &id);
		WaitForSingleObject(t.lock, INFINITE);
	}

	press_any_key();
	return ret;
}

