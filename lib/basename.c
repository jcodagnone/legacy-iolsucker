/*
 * $Id: basename.c,v 1.1 2003/02/19 19:59:38 juam Exp $
 *
 * basename(1) implementation
 */
#include <string.h>
#include "basename.h"

#ifdef WIN32
const char *
basename( const char *path )
{	char *back,*slash,*gr;
	char *r;
	
	back  = strrchr(path,'\\');
	slash = strrchr(path,'/');

	gr = back > slash ? back : slash;

	if( gr == NULL )
		r=NULL;
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
