/*
 * cache.c -- cache database for `material didacticos files'
 *
 * Copyright (C) 2003, 2004 by Juan F. Codagnone <juam@users.sourceforge.net>
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
  #include <config.h>
#endif

#include <stdlib.h>

cache_t 
cache_new(const char *dbpath)
{
	return 0;
}

void
cache_destroy(cache_t cdt)
{
}

int
cache_is_valid(cache_t cdt)
{
	return 1;
}
int
cache_add_file( cache_t cdt, const char *id, const char *file )
{
	return 0;
}

char *
cache_get_file( cache_t cdt, const char *id)
{
	return NULL;
}

char *
cache_get_file( cache_t cdt, const char *id)
{
	return NULL;
}


char *
cache_version(char *buf, unsigned nbuf)
{
	snprintf(buf, nbuf, "\n");
	buf[nbuf-1]=0;

	return buf;
}

