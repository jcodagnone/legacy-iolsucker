/*
 * win32.c --  Windows specific rutins
 *
 * Copyright (C) 2000, 2003 by Juan F. Codagnone <juam@users.sourceforge.net>
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#define	 STRICT
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "main.h"

/* returs false on error 
 */
static BOOL
registry_get_string( HKEY root,		/* root key */
                     const char *path,	/* key path */
                     const char *item,  /* the item we want in root/path */
                     char *buff,
                     DWORD nBuff)
{	HKEY key;
	DWORD dwType, cbData;
	int ret = FALSE;

	buff[0]=0;
	if( RegOpenKeyEx(root, path, 0, KEY_READ, &key) == ERROR_SUCCESS )
	{	if(RegQueryValueEx(key,item,0,&dwType,0,&cbData)==ERROR_SUCCESS)
		{	if(nBuff<cbData)
				;
			else if(dwType==REG_SZ)
			{	if( RegQueryValueEx(key, item, 0, &dwType, buff,
				                     &cbData)==ERROR_SUCCESS)
					ret = TRUE;
			}
		}
		RegCloseKey(key);
	}

	return ret;
}

/* changes the value of root/path/item
 * creates it if it doesn't exists
 */
static BOOL
registry_change_string( HKEY root, const char *path, const char *item, 
                        const char *buff)
{	HKEY key;
	DWORD n;
	BOOL ret = FALSE;
	DWORD nBuff = lstrlen(buff) + 1;

	if( RegCreateKeyEx(root, path, 0, NULL, 0, KEY_SET_VALUE, NULL, &key,&n)
           == ERROR_SUCCESS)
	{	if(RegSetValueEx(key,item,0,REG_SZ,buff,nBuff)==ERROR_SUCCESS)
			ret = TRUE;

		RegCloseKey(key);
	}

	return ret;
}

/*
 * Nobody likes to find passwords while looking values at the registry.
 * 
 * A fast implementation of rot13 algorithm (not mine)
 */
static char *
rot13(char *data)
{	char *s;
	char cap;
	
	for( s=data; *s ; s++ )
	{
		 cap = *s & 32;
		 *s &= ~cap;
		 *s= ((*s>= 'A') && (*s<= 'Z') ?
		         ((*s- 'A' + 13) % 26 + 'A') :
		         *s) | cap;
	}
	return data;
}

#define IOL_ROOT	HKEY_CURRENT_USER
#define IOL_PATH	"Software\\Embryo Software\\iolsucker"
#define IOL_USER	"username"
#define IOL_PASS	"password"
#define IOL_REPO	"repository"
#define IOL_DRY 	"dry_run"
#define IOL_PROXY_TYPE  "proxy_type"
#define IOL_PROXY_HOST	"proxy"
#define IOL_PROXY_USER	"proxy_user"
#define IOL_FANCY    	"fancy"
#define IOL_FORUM    	"forum"
#define IOL_WAIT	"wait"

int
save_config_file(struct opt *opt)
{	int r = 1;
	char *t = "1", *f = "0";

	assert(opt->proxy_type);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_USER,opt->username); 
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_REPO,opt->repository);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_PROXY_TYPE,
	                          opt->proxy_type);
	rot13(opt->password);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_PASS,opt->password); 
	rot13(opt->password);

	r&=registry_change_string(IOL_ROOT,IOL_PATH, IOL_PROXY_HOST,
	                          opt->proxy ? opt->proxy : "" );
	if(opt->proxy_user )
	{	rot13(opt->proxy_user);
		r&=registry_change_string(IOL_ROOT,IOL_PATH,
		                           IOL_PROXY_USER, opt->proxy_user); 
		rot13(opt->proxy_user);
	}
	else
	{	r&=registry_change_string(IOL_ROOT,IOL_PATH,
		                           IOL_PROXY_USER, ""); 
	}
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_DRY, opt->dry ? t : f);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_FANCY, 
	                          opt->fancy ? t : f);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_FORUM, 
	                          opt->forum? t : f);
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_WAIT, 
	                          opt->wait ? t : f);
	return r == 1 ? 0 : -1;
}

static void 
print_verbose(const struct opt *opt)
{	FILE *fp = stderr;

	fprintf(fp,"user: %s\n",opt->username);
	fprintf(fp,"rep: %s\n", opt->repository);
       	fprintf(fp,"proxy_type: %s\n",opt->proxy_type);
        if( opt->proxy ) 
        	fprintf(fp,"proxy: %s\n",opt->proxy);
        if( opt->proxy_user ) 
        	fprintf(fp,"puser: %s\n",opt->proxy_user);
	fprintf(fp,"dry: %d\n",opt->dry );
	fprintf(fp,"fancy: %d\n",opt->fancy);
	fprintf(fp,"forum: %d\n",opt->forum);
	fprintf(fp,"forum: %d\n",opt->wait);
}

struct config_sz
{	const char *key;
	void *data;	/* char * for arrays, char ** for malloc pts */
	size_t size;	/**< 0 means we need to malloc */
	int crypted;
};

struct config_bool
{	const char *key;
	int *value;
	int defaul;
};

static int
load_config_reg_sz(struct config_sz *table_sz, unsigned n)
{	const struct config_sz *entry;
	unsigned i;
	char buf[512];
	int ret=0;
	char **s;

	for( i = 0 ;  i< n; i++ )
	{	entry = table_sz+i;
		buf[0]=0;

		if( (entry->size && ((char *)entry->data)[0]==0) ||
		     entry->size == 0 && *((char **)entry->data)==NULL )
		{
			if( registry_get_string(IOL_ROOT, IOL_PATH,
                                    entry->key,buf,sizeof(buf) ) == TRUE )
			{	
				if( entry->size )
				{
					strncpy(entry->data,buf,entry->size);
					((char *)entry->data)[entry->size-1]=0;
					if( entry->crypted ) 
						rot13(entry->data);
				}
				else
				{	s = entry->data;
					*s = strdup(buf);
					if( *s && **s==0 )
					{	free(*s);
						*s = NULL;
					}
					else if( entry->crypted )
						rot13(*s);
				}
				
			}
			else
				registry_change_string(IOL_ROOT,IOL_PATH,
				                       entry->key,""); 
		}
	}
	
	return ret;
}

static int
load_config_reg_bool(struct config_bool *table_bool, unsigned n)
{	char buf[512];
	unsigned i;
	int ret=0;
	
	for( i = 0 ;  i< n; i++ )
	{	buf[0]=0;
	 	if( registry_get_string(IOL_ROOT, IOL_PATH, table_bool[i].key,
		                        buf, sizeof(buf) ) == TRUE )
		{ 	if( isdigit(buf[0]) )
				*(table_bool[i].value) = buf[0] - '0' != 0;
			else
				*(table_bool[i].value) = 0;
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,
			                       table_bool[i].key, "");
	}
	
	return ret;
}

int 
load_config_file(struct opt *opt)
{	char *proxy_type = NULL;
	struct config_sz table_sz[]=
	{	{ IOL_USER,	opt->username, sizeof(opt->username), 0 },
		{ IOL_PASS,	opt->password, sizeof(opt->password), 1 },
		{ IOL_REPO,	opt->repository, sizeof(opt->repository), 0},
		{ IOL_PROXY_HOST, &(opt->proxy),0, 0},
		{ IOL_PROXY_TYPE, &(proxy_type),0, 0},
		{ IOL_PROXY_USER, &(opt->proxy_user), 0, 1 }
	};
	struct config_bool table_bool[] =
	{	{ IOL_DRY,	&(opt->dry),	0},
		{ IOL_FANCY,	&(opt->fancy),	0},
		{ IOL_FORUM,	&(opt->forum),	0}
		{ IOL_WAIT,	&(opt->wait),	0}
	};

	#define NELEM(q) (sizeof(q)/sizeof(*(q)))

	load_config_reg_sz  (table_sz,NELEM(table_sz));
	load_config_reg_bool(table_bool,NELEM(table_bool));
	
	if( proxy_type  )
	{	if( !strcmp(proxy_type,"http") )
			opt->proxy_type = "http";
		else if( !strcmp(proxy_type, "socks5" ) )
			opt->proxy_type = "sock5";
		else
			opt->proxy_type = "";
		free(proxy_type);
	}
	else
		opt->proxy_type = "";

	if( opt->verbose )
		print_verbose(opt);
	
	return 0;
}

unsigned int 
sleep(unsigned int seconds)
{
	Sleep(seconds * 1000 );

	return 0;
}
