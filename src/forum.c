/*
 * forum.c -- foro managment function
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef HAVE_CONFIG_H
  #ifdef WIN32
    #include "../configwin.h"
  #else
    #include "../config.h"
  #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
forum_parse( char *data, unsigned n)
{	char *s;
	
	/* WTF! ;^)-|< */
	data[n-1] = 0;
	n=0;
	/* this is nasty..but the dark forces of javascript, and lazyness
	 * are stroger. 
	 */
	s = data; 
	while( (s = strstr(s,"javascript:foro")) )
	{	s+=sizeof("javascript:foro");
		n++;
	}

	return n;
}


#ifdef TEST_FORUM_DRIVER
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

const char *rs_program_name = "test";
int
main(int argc, char **argv)
{	struct stat st;
	FILE *fp;

	if( argc != 1 )
	{
		if( stat(argv[1], &st) == 0 && (fp=fopen(argv[1],"rb")) )
		{	char *s;

			s = malloc( st.st_size + 1);
			if( s )
			{	fread(s, st.st_size, 1, fp);
				s[st.st_size] = 0;
				printf("msg = %d\n",forum_parse(s,st.st_size));
				free(s);
			}
			
			fclose(fp);
		}
	}
	else
		fprintf(stderr,"%s foro.html\n",argv[0]);
	
	return 0;
}
#endif
