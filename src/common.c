/*
 * common.c -- common functions
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
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include <dirname.h>

struct get_prefix {
	char *prefix;
	size_t len;
};

static void 
foreach_getprefix( const char *file, struct get_prefix *d)
{	unsigned i = 0;
	unsigned len;
	
	if( d->prefix ==  NULL ) {
		d->prefix = g_strdup(file);
		d->len = strlen(file);
	} else if( d->prefix[0] == 0 ) 
		;
	 else {
		len = strlen(file);
		for( i = 0 ; i < d->len && i< len  && d->prefix[i] == file[i];
		     i ++ )
		     ;
		d->len = i;
		d->prefix[i] = 0;
	}
}

char *
get_common_startpath(GSList *list)
{	struct get_prefix d;
	char *ret, *s;
	
	memset(&d, 0, sizeof(d));
	g_slist_foreach(list, (GFunc)foreach_getprefix, &d);
	
	if( d.prefix == 0 )
		ret = NULL;
	else if( d.prefix[0] == 0 )
		ret= d.prefix;
	else if( d.prefix[d.len]  == '/' )
		ret = d.prefix;
	else {
		ret = g_strdup_printf("%s/", (s=my_path_get_dirname(d.prefix)));
		free(s);
		g_free(d.prefix);
	}

	return  ret;
}

#ifdef TEST_DRIVER
/* gcc common.c -DTEST_DRIVER  `pkg-config glib-2.0 --cflags --libs` -L../lib
 * -lmisc
 */

#include <stdio.h>
#include <string.h>

int
main(int argc, char **argv)
{
	GSList *list = 0;
	int i = 0 ;
	
	for( i = 1 ; i < argc ; i++ ) 
		list = g_slist_prepend(list, argv[i]);

	puts(get_common_startpath(list));

	return 0;
}
#endif
