/*
 * urihelper.c -- helper para parsear uris
 *
 * Copyright (C) 2005 by Juan F. Codagnone <juam@users.sourceforge.net>
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
#include "urihelper.h"

/* backport de ar.com.leak.iolsucker.impl.http.util.URIHelper
 */

struct urihelper {
	char *base;
	char *uri;
	GHashTable* map;
};

static void 
_urihelper_init(urihelper_t cdt) {
	char *p =  strchr(cdt->uri, '?');
	char *q;
	if( p == 0 ) {
		cdt->base = cdt->uri;
	} else {
		*p = 0;
		cdt->base = cdt->uri;
		p++;
	       	for( q = strchr(p, '&') ; q!=0 ;  ){
			char *eq;
			*q = 0;
			q++;
		       	eq = strchr(p, '=');
		       	if( eq != 0 ) {
				*(eq++)  = 0;
				g_hash_table_insert(cdt->map, p, eq);   
			}
			p = q;
			q = strchr(p, '&');
		}
		
		if( p ) {
			char *eq = strchr(p, '=');
		       	if( eq != 0 ) {
				*eq  = 0;
				eq++;
				g_hash_table_insert(cdt->map, p, eq);   
			}

		}
	}
}

urihelper_t 
urihelper_new(const char *name) {
	urihelper_t ret = 0;
	
	if( name ) {
		ret = malloc(sizeof(*ret));
		ret->base = 0;
		ret->uri = strdup(name);
		ret->map = g_hash_table_new(g_str_hash, g_str_equal);

		if(ret->map == 0 || ret->uri == 0) {
			urihelper_destroy(ret);
			ret = 0;
		} else {
			_urihelper_init(ret);
		}
	}

	return ret;
}

void 
urihelper_destroy(urihelper_t c) {
	if( c ) {
		free(c->uri);
		if( c->map ) {
			g_hash_table_destroy(c->map);
		}
		free(c);
	}
}

const char * 
urihelper_getbase(urihelper_t cdt) {
	return cdt->base;
}

int 
urihelper_has_param(urihelper_t cdt, const char *s) {
	return g_hash_table_lookup(cdt->map, s) != 0;
}


const char *
urihelper_get_param(urihelper_t cdt, const char *s) {
	return g_hash_table_lookup(cdt->map, s);
}


unsigned urihelper_size(urihelper_t cdt) {
	return g_hash_table_size(cdt->map);
}

#ifdef URIHELPER_DRIVER
#include <stdio.h>

int
main(int argc, char **argv) {
	if( argc != 1 ) {
		urihelper_t uh = urihelper_new(argv[1]);
		printf("%d\n",urihelper_has_param(uh, "keyy2"));
		printf("%s\n",urihelper_get_param(uh, "keyy2"));
		urihelper_destroy(uh);
	} else {
		puts("falta un parametro");
	}

	return 0;
}
#endif
