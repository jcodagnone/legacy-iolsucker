/*
 * demo.c -- curl wrapper to localdisk
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
#ifdef IOLDEMO
   #warning "IOLDEMO activado"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <curl/curl.h>
#include "progress.h"

struct global
{	char *url;
	void *write_data;
	void *progress;
} g;

struct buff
{	char *data;
	unsigned size;
};

struct url_table
{	
	const char *file;
} url_table[] =
{
	{"demo/000_http:__silvestre.itba.edu.ar_itbaV_login.asp" },
	{"demo/001_http:__silvestre.itba.edu.ar_itbaV_mynav.asp" },
	{"demo/002_http:__silvestre.itba.edu.ar_itbaV_mynav.asp?cmd=ChangeContext&nivel=4&snivel=71.42" },
	{"demo/003_http:__silvestre.itba.edu.ar_itbaV_newmaterialdid.asp" },
};

static int
get_file_content( const char *file, struct buff *buf )
{	struct stat st;
	FILE *fp;
	int ret = 0;
	
	if( stat(file, &st) == 0 && (fp=fopen(file,"rb")) )
	{
		if( buf )
		{
			buf->data  = malloc( st.st_size );
			fread(buf->data, st.st_size, 1, fp);
			buf->size  = st.st_size;
		}
		fclose(fp);
	}
	else
		ret = -1;

	return ret;
}

int 
get_file_from_url( const char *url, struct buff *buf )
{	static unsigned i = 0;

	
	if( i >= sizeof(url_table)/sizeof(url_table[0]) )
		return -1;
	/* printf("---> %d %s %s\n",i, url,url_table[i].file); */
	return get_file_content(url_table[i++].file, buf);
}

static int
simulate( const char *url )
{	int ret = 1;	
	struct buff *buf;
	
	buf = g.write_data;
	ret = get_file_from_url(url, buf);
	
	return ret;
}

int
ioldemo_curl_easy_setopt(CURL *c, unsigned f, void *d, char *filename)
{
	if( f == CURLOPT_URL )
		g.url = d;
	else if( f == CURLOPT_WRITEDATA )
		g.write_data = d;
	else if ( f == CURLOPT_PROGRESSFUNCTION )
		g.progress = d;
	else
		;

	curl_easy_setopt(c, f, d );

	return 0;
}

CURLcode 
curl_easy_perform(CURL *curl)
{	int ret = 0;

	if( g.url )
		ret = simulate(g.url);
		
	g.url = NULL;
	return ret;
}

#endif
