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
	DWORD nBuff = lstrlen(buff);

	if( RegCreateKeyEx(root, path, 0, NULL, 0, KEY_SET_VALUE, NULL, &key,&n)
           == ERROR_SUCCESS)
	{	if(RegSetValueEx(key,item,0,REG_SZ,buff,nBuff)==ERROR_SUCCESS)
			ret = TRUE;

		RegCloseKey(key);
	}

	return ret;
}

/*
 * Nobody likes to find password while looking at the registry.
 * yeah. this is nasty. the password should be save to disk at all
 *
 * We need an advanced rpc between iolsucker and iolwizard 
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
	
}

#define IOL_ROOT	HKEY_CURRENT_USER
#define IOL_PATH	"Software\\Embryo Software\\iolsucker"
#define IOL_USER	"username"
#define IOL_PASS	"password"
#define IOL_DRY 	"dry_run"
#define IOL_PROXY_HOST	"proxy"
#define IOL_PROXY_USER	"proxy_user"

int
save_config_file(struct opt *opt)
{	int r = 1;
	char *t = "1", *f = "0";
	
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_USER,opt->username); 
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_PASS,opt->password); 
	if( opt->proxy )
		 r&=registry_change_string(IOL_ROOT,IOL_PATH,
		                          IOL_PROXY_HOST,opt->proxy);
	if( opt->proxy_user )
		 r&=registry_change_string(IOL_ROOT,IOL_PATH,
		                           IOL_PROXY_USER, opt->proxy_user); 
	r&=registry_change_string(IOL_ROOT,IOL_PATH,IOL_DRY, opt->dry ? t : f);
	
	return r == 1 ? 0 : -1;
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
	}
	if( opt->password[0] == 0 )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PASS, buf,
                                        sizeof(buf) ) == TRUE )
		{	strncpy(opt->password,buf,sizeof(opt->password));
			opt->password[sizeof(opt->password)-1] = 0;
			rot13(opt->password);
		}
	}
	if( opt->proxy == NULL )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PROXY_HOST, buf,
                                        sizeof(buf) ) == TRUE )
			opt->proxy = strdup(buf);
			if( opt->proxy && opt->proxy[0] == 0 )
			{	free(opt->proxy);
				opt->proxy  = NULL;
			}
	}

	if( opt->proxy_user == NULL )
	{	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_PROXY_USER, buf,
                                        sizeof(buf) ) == TRUE )
			opt->proxy_user = strdup(buf);
			if( opt->proxy_user && opt->proxy_user[0] == 0 )
			{	if( opt->proxy_user[0] == 0 )
				{	free(opt->proxy_user);
					opt->proxy_user = NULL;
					rot13(opt->proxy_user);
				}
			}
	}

	if( registry_get_string(IOL_ROOT, IOL_PATH, IOL_DRY, buf,
	                        sizeof(buf) ) == TRUE )
	 	opt->dry = buf[0] - '0' != 0;

	
	return 0;
}
