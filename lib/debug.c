/* 
 * $Id: debug.c,v 1.1 2003/02/19 04:44:56 juam Exp $
 *
 * Funciones para debugear el programa
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"


#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 20
#endif

/* configuration ala K&P */
static int with_ansi_colors = 1 ;
static const char *ansi_colors[]=
{	"\x1b[31m",
	"\x1b[32m",
	"\x1b[33m",
	"\x1b[34m",
	"\x1b[35m",
	"\x1b[36m",
	"\x1b[37m",
};

#define NELEMS(x) (sizeof(x)/sizeof(*x))

static const char *ansi_normal= "\x1b[39m\x1b[49m";


void
debug(unsigned level,const char *fmt,...)
{	va_list ap;

	if( level <= DEBUG_LEVEL )
	{	va_start(ap,fmt);
		if( with_ansi_colors )
		 	 fprintf(stderr,"%s",
				ansi_colors[level%NELEMS(ansi_colors)]);
		fprintf(stderr,"%2.2d:",level);
		vfprintf(stderr,fmt,ap);
		if( with_ansi_colors )
        		fprintf(stderr,"%s",ansi_normal);

   		va_end(ap);  
	}

}
