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
}

int 
load_config_file(struct opt *opt)
{	char buf[512];

	if( opt->username[0] == 0 )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_USER, buf,
                                        sizeof(buf) ) == TRUE )
		{	strncpy(opt->username,buf,sizeof(opt->username));
			opt->username[sizeof(opt->username)-1] = 0;
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_USER,""); 
	}

	if( opt->password[0] == 0 )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PASS, buf,
                                        sizeof(buf) ) == TRUE )
		{	strncpy(opt->password,buf,sizeof(opt->password));
			opt->password[sizeof(opt->password)-1] = 0;
			rot13(opt->password);
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_PASS,""); 

	}
	if( opt->repository[0] == 0 )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_REPO, buf,
                                        sizeof(buf) ) == TRUE )
		{	strncpy(opt->repository,buf,sizeof(opt->repository));
			opt->repository[sizeof(opt->repository)-1] = 0;
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_REPO,""); 
	}
	if( opt->proxy_type[0] == 0)
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PROXY_TYPE, buf,
		                        sizeof(buf) ) == TRUE )
		{
			if( !strcmp(buf,"http") )
				opt->proxy_type = "http";
			else if( !strcmp(buf, "socks5" ) )
				opt->proxy_type = "sock5";
			else
				opt->proxy_type = "";
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_PROXY_TYPE,
			                        "");

	}

	if( opt->proxy == NULL )
	{	buf[0]=0;
		if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PROXY_HOST, buf,
                                        sizeof(buf) ) == TRUE )
		{	opt->proxy = strdup(buf);
			if( opt->proxy && opt->proxy[0] == 0 )
			{	free(opt->proxy);
				opt->proxy  = NULL;
			}
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_PROXY_HOST,
			                       "");
	}

	if( opt->proxy_user == NULL )
	{	buf[0]=0;
		if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PROXY_USER, buf,
                                        sizeof(buf) ) == TRUE )
		{ 	opt->proxy_user = strdup(buf);
			if( opt->proxy_user && opt->proxy_user[0] == 0 )
			{	free(opt->proxy_user);
				opt->proxy_user = NULL;
			}
			else
				rot13(opt->proxy_user);
		}
		else
			registry_change_string(IOL_ROOT,IOL_PATH,IOL_PROXY_USER,
			                        "");
	}

	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_DRY, buf,
	                        sizeof(buf) ) == TRUE )
	{	if( isdigit(buf[0]) )
	 		opt->dry = buf[0] - '0' != 0;
		else
			opt->dry = 0;
	}
	else
		registry_change_string(IOL_ROOT,IOL_PATH,IOL_DRY,"");

	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_FANCY, buf,
	                        sizeof(buf) ) == TRUE )
	{	
		if( isdigit(buf[0]) )
			opt->fancy= buf[0] - '0' != 0;
		else
			opt->fancy = 0;
	}
	else
		registry_change_string(IOL_ROOT,IOL_PATH,IOL_FANCY,"");
	
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
