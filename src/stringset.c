#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "stringset.h"

#define IS_STRINGSET(s) ((s)!=NULL)

struct stringsetCDT {
	GTree *tree;
};


stringset_t 
stringset_new(void)
{	stringset_t s;

	s = malloc(sizeof(*s));
	if( s )
	{
		memset(s,0,sizeof(*s));
		s->tree = g_tree_new_full( strcmp, NULL, (GDestroyNotify)free, 
		                           (GDestroyNotify) free);
		if( s->tree == NULL )
		{	free(s);
			s = NULL;
		}
	}

	return s;
}

stringset_t
stringset_destroy(stringset_t set)
{
	if( IS_STRINGSET(set) )
	{
		g_tree_destroy(set->tree);
		free(set);
	}

	return set;
}

int
stringset_is_valid(stringset_t set)
{
	return IS_STRINGSET(set);
}

stringset_error_t
stringset_add(stringset_t set, const char *string)
{	stringset_error_t ret = E_STRINGSET_OK;

	if( IS_STRINGSET(set) )
	{
		if( stringset_look(set, string) == E_STRINGSET_NOTFOUND )
			g_tree_insert(set->tree, strdup(string), strdup(string) );
		else
			ret = E_STRINGSET_EXISTS;
	}
	else
		ret = E_STRINGSET_INVALID;

	return ret;
}


stringset_error_t 
stringset_look(stringset_t set, const char *string)
{	stringset_error_t ret;
	char *s;
	
	if( IS_STRINGSET(set) )
	{
		if( (s=g_tree_lookup(set->tree, string)) )
			ret = E_STRINGSET_EXISTS;
		else
			ret = E_STRINGSET_NOTFOUND;
	}
	else
		ret = E_STRINGSET_INVALID;
		
	return ret;
}

#ifdef TEST_DRIVER_STRINGSET
#include <stdio.h>
int main(void)
{	unsigned i, j;
	char b[2]={'a',0};
	stringset_t  set = stringset_new();

	for( j=0; j<2 ; j++) {
		for( i = 0 ; i<10 ; i++ ) {
			b[0]++;
			printf("insert \"%s\"->  %d\n", b, stringset_add(set, b));
		}
		b[0]='a';
	}

	b[0]='a';
	stringset_destroy(set);
	
	return 0;
}
#endif

