/*
 * $Id: strdup.c,v 1.2 2003/02/23 18:49:58 juam Exp $
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "strdup.h"
        
char *
strdup(const char *s)
{	char *ptr;
        extern int errno;

	if( s == NULL)
	{	errno = EINVAL;
		return NULL;
	}
	ptr=malloc( strlen(s) + 1 );
	if( ! ptr )
	{	errno = ENOMEM ;
		return NULL;
	}

	return strcpy(ptr,s);
}

