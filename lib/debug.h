/*
 *  $Id: debug.h,v 1.1 2003/02/19 04:44:56 juam Exp $
 */
#ifndef _DEBUG_H_
#define _DEBUG_H_

enum {  DEBUG_NONE,
	DEBUG_NORMAL,
	DEBUG_VERBOSE,
	DEBUG_VERY_VERBOSE,
	DEBUG_ALL
     };

void
debug(unsigned level,const char *fmt,...);


#endif 
