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
	char *ret;
	
	memset(&d, 0, sizeof(d));
	g_slist_foreach(list, (GFunc)foreach_getprefix, &d);
	
	if( d.prefix[0] == 0 )
		ret= d.prefix;
	else if( d.prefix[d.len]  == '/' )
		ret = d.prefix;
	else {
		ret = g_strdup_printf("%s/", my_path_get_dirname(d.prefix));
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
