/*
 * cache.c -- cache database for `material didacticos files'
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
#include <assert.h>

#include <sys/types.h>
#include <db.h>

#include <trace.h>
#include <strdup.h>

#include "cache.h"

struct cache
{ 	DB *dbp;
};


static void
db_errcall_fcn(const char *errpfx, char *msg)
{
	rs_log_error("%s", msg);
}

cache_t
cache_new(const char *dbpath)
{	cache_t cdt = NULL;
	DB *dbp;
	int ret;

	if( dbpath == NULL )
		;
	else if ((ret = db_create(&dbp, NULL, 0)) != 0)
	 	rs_log_error("db_create: %s", db_strerror(ret));
	else if( dbp->set_errcall(dbp, db_errcall_fcn),0 )  /* C hack */
		;
	else if ((ret = dbp->open(dbp, NULL, dbpath, NULL, DB_BTREE, 
	                          DB_CREATE, 0664)) != 0)
	 	dbp->err(dbp, ret, "%s", dbpath);
	else
	{	cdt = malloc(sizeof(*cdt));
		if( cdt )
		{	cdt->dbp = dbp;
		}
		else	
			dbp->close(dbp, 0);
			
	}

	return cdt;
}

void
cache_destroy(cache_t cdt)
{
	if( cdt == NULL )
		return;

	cdt->dbp->close(cdt->dbp, 0);	
}

int
cache_add_file( cache_t cdt, const char *id, const char *file )
{ 	DBT key, data;
	int ret = 0;

	if( cdt == NULL )
		ret = -1;
	else
	{
		memset(&key,  0, sizeof(key));
		memset(&data, 0, sizeof(data));

		key.data  = (void *) id;
		key.size  = strlen(id) + 1;
		data.data = (void *) file;
		data.size = strlen(file) + 1;
		
		ret = cdt->dbp->put(cdt->dbp, NULL, &key, &data, 0);
		if ( ret == 0)
			;
		else
		{ 	cdt->dbp->err(cdt->dbp, ret, "DB->put");
			ret = -1;
		}
	}

	return ret;
}

char *
cache_get_file( cache_t cdt, const char *id)
{ 	DBT key, data;
	char *s = NULL;
	int ret;

	if( cdt == NULL )
		;
	else
	{ 	memset(&key,  0, sizeof(key));
		memset(&data, 0, sizeof(data));
		key.data = (void *)id;
		key.size = strlen(id) + 1;
	
		ret = cdt->dbp->get(cdt->dbp, NULL, &key, &data, 0);
		if ((ret = cdt->dbp->get(cdt->dbp, NULL, &key, &data, 0)) == 0)
			/* yes. use g_strdup. the rest of the program
			 * is made for a g_ */
			s = g_strdup(data.data);
			;
		else
			cdt->dbp->err(cdt->dbp, ret, "DB->get");
	}
	
	return s;
}

#ifdef TEST_DB_DRIVER
const char *rs_program_name = "test";
int
main(void)
{	cache_t c;

	c = cache_new("1.db");
	if( c )
	{	cache_add_file(c, "123123", "http://bASURA/AAA");
		printf("%s\n",cache_get_file(c, "123123"));
		cache_destroy(c);
	}
	
}
#endif
