/*
 * $Id: basename.c,v 1.3 2003/03/01 18:09:38 juam Exp $
 *
 * basename(1) implementation
 */
#include <string.h>
#include "basename.h"

#ifdef WIN32
EXPORT const char *
basename( const char *path )
{	char *back,*slash,*gr;
	char *r;
	
	back  = strrchr(path,'\\');
	slash = strrchr(path,'/');

	gr = back > slash ? back : slash;

	if( gr == NULL )
		r=path;
	else 
		r = gr +1;

	return r;
}

#else
const char *
basename( const char *path )
{	const char *nRet;
	
	return (nRet = strrchr(path,'/')) ? nRet+1 : path;
}
#endif
