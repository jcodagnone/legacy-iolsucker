/*
 * iol.c -- IOLsucker web robot implementation
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
  #include "../config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <glib.h>
#include <curl/curl.h>

#include <trace.h>
#include <strdup.h>
#include <basename.h>
#include <mkrdir.h>
#include <progress.h>

#include "i18n.h"
#include "iol.h"
#include "link.h"

/** User Agent string reported to the webserver */
#define USERAGENT	"Links (0.97; Unix; 80x25)"
#define COOKIEFILE	"cookieiol"

#define IOL_HOST        "silvestre.itba.edu.ar"
#define IOL_PATH        "itbaV"
#define IOL_LEVEL       "4" 

#define URL_BASE	"http://"IOL_HOST"/"IOL_PATH
#define URL_LOGIN	URL_BASE"/login.asp"
#define URL_LOGIN_1	URL_BASE"/mynav.asp"
#define URL_LOGIN_ARG	"txtdni=%s&txtpwd=%s&Submit=Conectar&cmd=login"
#define URL_LOGOUT	URL_BASE"/mynav.asp?cmd=logout"
#define URL_CHANGE 	URL_BASE"/mynav.asp?cmd=ChangeContext&nivel=4&snivel=%s"
#define URL_MATERIAL	URL_BASE"/newmaterialdid.asp" 
#define IOL_COURSE_PARAMETER	"nivel=4"

#define IOL_MATERIAL_FOLDER	"material"
#define IOL_MATERIAL_TMPFILE	".download.tmp"

struct buff 
{ 	char *data;
	size_t size;
};

struct course 
{       char *code;
	char *name;
};


/**
 * Concrete data type for the IOL object */
struct iolCDT
{ 	CURL *curl;		/**< curl handler */
	int bLogged;    	/**< already logged in ? */ 
	char *cookie_file;	/**< ehhhh */
	GSList *courses;	/**< loaded courses */ 
	char *current_course;
	char *repository;	/**< repository directory */
};

#define IS_IOL_T( iol ) ( iol != NULL  )

enum TP_FLAGS 
{	TP_FILE	= 1 << 0
};

/**
 * gets the path were we save the cookies */
static char *
get_cookies_file(void)
{	char *nRet,*p;
	size_t n;

	nRet = curl_getenv("TMP"); if( nRet  == NULL ) 
	{       if( SYSTEM == UNIX )
			nRet = strdup("/tmp");
		else if( SYSTEM == WINDOWS )
			nRet = strdup( ".\\");
	}

	if( nRet == NULL )
		return NULL;

	n = strlen(nRet) + strlen(COOKIEFILE) + 2; p = malloc( n );
	if( p != NULL )
	{ 	g_snprintf(p,n,"%s%c%s",nRet, 
		          SYSTEM == UNIX ? '/' : '\\', COOKIEFILE);
		p[n-1] = '\0'; free(nRet);
	}

	return p;
}


static size_t
write_data_to_memory (void *ptr, size_t size, size_t nmemb, void *data)
{	size_t realsize = size * nmemb;
	struct buff  *mem = (struct buff *)data;

	if( data == NULL )	/* the user don't want the input */
		return realsize;

	/**
	 * \todo write something that don't suck :) 
	 */
	mem->data = (char *)realloc(mem->data, mem->size + realsize + 1);
	if (mem->data)
	{	memcpy(mem->data + mem->size, ptr, realsize);
		mem->size += realsize; mem->data[mem->size] = 0;
	}

	return nmemb;
}

static size_t
write_data_to_file(void *ptr, size_t size, size_t nmemb, void *data)
{	FILE *fp = data;

	
	return fwrite(ptr, size, nmemb, fp);
}
                                            
/**
 *  wraper to libcurl. Transfer the url, and saves it in the buffer page
 */
static int
transfer_page( CURL *curl, const char *url, unsigned flags, void *data)
{	CURLcode res;
	FILE *fp = NULL;
	struct progress progress;

	if( curl == NULL || url == 0 || url[0]==0)
		return E_INVAL;

	/*rs_log_info(_("downloading %s"),url); 
	 */
	
	/*if( flags & TP_FILE )*/
	{	
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data_to_file);
		fp = fopen(/*data*/"/tmp/pepe","wb");	
		if( fp == NULL )
			return E_NETWORK;
		curl_easy_setopt(curl, CURLOPT_FILE, fp);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, 
		                  dot_progress_callback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progress);
		memset(&progress,0,sizeof(progress));
	}
	/*else
	{ 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
		                       write_data_to_memory);
		curl_easy_setopt(curl, CURLOPT_FILE, data); 
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, TRUE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
	}*/
	
	curl_easy_setopt(curl, CURLOPT_URL, /*url*/ "http://mini/1");
	res = curl_easy_perform(curl);

	if( fp )
		fclose(fp);
	if( progress.data )
		dot_finish(progress.data, time(NULL));

	return res == 0 ? E_OK : E_NETWORK;
}

iol_t 
iol_new(void) 
{	iol_t cdt;

	cdt = malloc( sizeof(*cdt) );
	if( cdt == NULL )
		return NULL;
		
	memset( cdt, 0, sizeof(*cdt));
	cdt->cookie_file = get_cookies_file();
	if( cdt->cookie_file == NULL ) 
	{ 	free(cdt);
		rs_log_error(_("locating a temporary file")); return NULL;
	}

	curl_global_init(CURL_GLOBAL_ALL);
	cdt->curl = curl_easy_init();
	curl_easy_setopt(cdt->curl,CURLOPT_COOKIEJAR, cdt->cookie_file);
	curl_easy_setopt(cdt->curl,CURLOPT_USERAGENT,USERAGENT);

	return cdt;
}

static void
free_courses_list( struct course *data, gpointer user_data ) 
{
	if( data )
	{	free(data->code);
		free(data->name); free(data);
	}
}

void
iol_destroy(iol_t iol) 
{
	if( !IS_IOL_T(iol) )
		return;

	if( iol->bLogged )
		iol_logout(iol);

	curl_easy_cleanup(iol->curl); 
	curl_global_cleanup();

	g_slist_foreach(iol->courses, (GFunc)free_courses_list ,NULL);
	g_slist_free(iol->courses);

	free(iol->cookie_file);
	free(iol->current_course);
	free(iol->repository);

	free(iol);
}

int
iol_set_repository(iol_t cdt, const char *path)
{	int ret = E_OK;

	if( !IS_IOL_T(cdt) || cdt == NULL || path == NULL )
		ret = E_INVAL;
	else 
	{ 	cdt->repository= strdup(path); 
		if( cdt->repository == NULL )
			ret = E_MEMORY;
	}

	return ret;
}

static int
is_valid_course_code( const char *code )
{	int b = 1;
	unsigned i;

	if( code == NULL )
		b = 0;
	else {
		for( i=0; b && code[i] ; i++ )
		{       /* dangerous characters for the filesystem */
			if( code[i]=='/' || code[i] == '\\' ||
			    code[i]==':' || code[i] == '>' /* win32 */ )
				b = 0;
		}

		if( i == 0 )
			b = 0;
	}

	return b;
}

/**
 * callback called by the html parser when procesing newmaterial.asp 
 */
static void 
link_courses_fnc( const char *link, const char *comment, void *d )
{	GSList **listptr =  d;
	struct course *course; 
	unsigned nivel;
	char *s;

	assert(d);

	s = strstr(link,IOL_COURSE_PARAMETER);
	if( s == NULL )
		return;

	/* get code */
	s =  strstr(link,"snivel=" );
	if( s )
	{	char *ss;

		s += sizeof("snivel=") -1 ;
		for( ss = s; *ss && *ss != ';' ; ss++ )
			;
		if( *ss == ';' )
			*ss = 0;

		for( ss = (char *)comment; *ss && isspace(*ss) ; ss++)
			;

		course = malloc(sizeof(*course));
		if( course ) 
		{ 	course->code = strdup(s);
			course->name = strdup(ss);
			if( course->code && course->name )
				*listptr = g_slist_prepend(*listptr, course);
			else
			{	free(course->code);
				free(course->name);
				free(course);
			}
		}
	}
}

/**
 * gets the list of courses that are linked at page */
static unsigned 
parse_courses(GSList **listptr, struct buff *page) 
{	link_parser_t parser;
	unsigned i;

	parser = link_parser_new();
	if( parser == NULL )
		return 0;

	link_parser_set_link_callback(parser,link_courses_fnc, listptr);
	for( i = 0 ; i< page->size &&
	     link_parser_proccess_char(parser,page->data[i])==0 ; i++ ) 
	     ;
	link_parser_destroy(parser);

	return (*listptr)== NULL ? -1 : 0;
}

int
iol_login(iol_t iol, const char *user, const char *pass)
{	int nRet = E_OK;

	if( !IS_IOL_T(iol) || user == NULL || pass == NULL || !*user || !*pass )
		nRet = E_INVAL;
	else if( iol->bLogged )
	{	rs_log_warning(_("login(): ya estamos logueados"));
		nRet = E_ALOGED ;
	}
	else
	{       struct buff buf = {NULL, 0};

		/* yummm!!! get a cookie */ if(
		transfer_page(iol->curl, URL_LOGIN, 0, NULL)!= E_OK )
		{	rs_log_error(_("login(): network error"));
			nRet = E_NETWORK;
		} 
		else
		{       char *s = g_strdup_printf(URL_LOGIN_ARG,user,pass);

			/* login */
			curl_easy_setopt(iol->curl, CURLOPT_POSTFIELDS, s);

			if(transfer_page(iol->curl, URL_LOGIN_1,0,&buf) != E_OK)
			{	rs_log_error(_("login(): network error"));
				nRet = E_NETWORK;
			} 
			else if( parse_courses(&(iol->courses),&buf) == 0)
					iol->bLogged = 1;

			curl_easy_setopt(iol->curl,CURLOPT_HTTPGET,1L);
			g_free(s);
		}
		free(buf.data);
	}

	return nRet;
}

int 
iol_logout(iol_t iol)
{	int nRet = E_OK;

	if( !IS_IOL_T(iol)  )
		nRet = E_INVAL;
	else if( iol->bLogged  )
	{	 transfer_page(iol->curl, URL_LOGOUT,0,NULL);
		iol->bLogged = 0;
	}
	else
	{       rs_log_warning(_("logout(): no estamos logueados"));
		nRet = E_NLOGED;
	}


	return nRet;
}

static int 
iol_set_current_course(iol_t iol, const char *course) 
{	char *s;
	int ret = E_OK;

	/**
	 * \todo i dont check whether the course exists in the list because
	 * is not catastrofic. ( at least not for the client side :^)  ) 
	 */

	if( !IS_IOL_T(iol) )
		return E_INVAL;

	if( iol->current_course )
		free(iol->current_course);
	iol->current_course = strdup(course);
	if( iol->current_course == NULL )
		return E_MEMORY;

	s = g_strdup_printf(URL_CHANGE,course);

	if( transfer_page(iol->curl, s, 0, NULL) != E_OK )
		ret = E_NETWORK;

	return	ret;
}

/**/


/* Download Material didactico section
 */

static int url_is_file(const char *url) 
{	const char *p,*q;

	p = g_basename(url);
	q = g_basename(URL_MATERIAL);

	return strncmp(p,q, strlen(q));
}

struct tmp 
{ 	GQueue	*pending;
	GSList  *files;
	char *prefix;
	char *url_prefix;
};

static int
is_external_link( const char *link )
{
	return strstr(link,"://") !=NULL;
}

static int
is_javascript_link( const char *link)
{	
	return !strncmp(link,"javascript:",sizeof("javascript:"));
}

/* this is no very serius. just call curl_escape() */
static char *
my_url_escape(const char *url)
{	char *s;
	unsigned i=0,len=0;
	
	for(s=(char *)url; *s ; s++,len ++)
		if( *s == ' ' )
			i++;
	s = malloc(len + i*2 + 1);
	if( s == NULL )
		return NULL;
	for( i=0; *url; url++ , i++)
		if( *url == ' ' )
		{	s[i++] = '%';
			s[i++] = '2';
			s[i]   = '0';
		}
		else
			s[i] = *url;
	s[i]=0;

	return s;
}

static int
is_father_folder( const char *child, const char *father)
{
	return strstr(father, child)!=NULL;
}

static void
link_files_fnc( const char *link, const char *comment, void *d ) 
{	struct  tmp *t = (struct tmp *)d;
	int bFile;
	char *s, *p;

	if( is_external_link(link) || is_javascript_link(link) )
		return ;

	p = my_url_escape(link);
	if( p == NULL )
		return;
	s = g_strdup_printf("%s/%s",URL_BASE,p);
	free(p);
	if( s == NULL )
		return;
	
	bFile = url_is_file(link);
	if( bFile )
	{ 	if( t->url_prefix == NULL )
			t->url_prefix = g_path_get_dirname(link);
		t->files = g_slist_prepend(t->files, s);
	}
	else
	{ 	if( is_father_folder(link,t->prefix) )
			free(s);
		else
			g_queue_push_head(t->pending, s);
	}
}

static int
get_current_file_list(iol_t iol, GSList **l, char **url_prefix )
{	struct buff webpage = { NULL, 0 };
	char *url; 
	struct tmp t;
	int ret;

	t.pending = g_queue_new(); 
	t.files = NULL;
	t.url_prefix = NULL;
	g_queue_push_head(t.pending, strdup(URL_MATERIAL));

	while( ! g_queue_is_empty(t.pending) )
	{	
		url = g_queue_pop_head(t.pending);
		t.prefix = url;
		webpage.data = NULL;
		webpage.size = 0;

		if( transfer_page(iol->curl, url,0,&webpage) != E_OK )
			ret = E_NETWORK;
		else
		{	link_parser_t parser;
			unsigned i;

			/* get and analize links */
			parser = link_parser_new();
			if( parser == NULL )
				return 0;
			
			link_parser_set_link_callback(parser,link_files_fnc,&t);
			for( i = 0 ; i< webpage.size && 
			   link_parser_proccess_char(parser,webpage.data[i])==0;
			   i++ )
			     ;
			link_parser_destroy(parser);
		}
		*url_prefix = t.url_prefix;
		free(url);
		free(webpage.data);
	}
	
	assert(g_queue_is_empty(t.pending));
	g_queue_free(t.pending); 
	*l = t.files;

	return 0;
}

static int
create_course_directory(const char *dir) 
{	struct stat buf;
	int ret = 0;

	if( stat(dir,&buf) == -1 )
	{	if( errno == ENOENT )
		{	if(mkrdir(dir,0755) == -1 )
				ret = -1;
		}
		else
			ret = -1;
	}

	return ret;
}

/** structure for passing data across the  g_slist_foreach function
 */
struct tmp_resync_getfile
{	char *prefix;
	char *url_prefix;
	iol_t iol;
};

static void
foreach_getfile(char *file, struct tmp_resync_getfile *d)
{	size_t len;
	char *local, *dirname, *download, *unquote, *q;
	struct stat st;

	q = URL_BASE;
	if( file && d && d->url_prefix )
	{	size_t len_q = strlen(q);
		size_t len_url = strlen(d->url_prefix);
		
		len =  len_q   + (q[len_q-1]!='/')  + 
		       len_url + (d->url_prefix[len_url - 1] != '/');
		assert( strlen(file) > len );

		unquote = curl_unescape(file+len,0);
		if( !unquote )
			return;

		local = g_strdup_printf("%s/%s", d->prefix, unquote);
		dirname = g_path_get_dirname(local);
		curl_free(unquote);

		if( stat(local,&st) == -1 )
		{	/** \todo 
		 	 * if a deeper directory  has been created
		 	 *  recently, don't try to create the dir
		 	 */
		 	errno = 0;
			if( mkrdir(dirname,0755) == 0 || errno == EEXIST)
			{
				download = g_strdup_printf("%s/%s", dirname, 
				                          IOL_MATERIAL_TMPFILE);
				if( transfer_page(d->iol->curl, file, TP_FILE,
				             download) == 0 )
					rename(download,local);
				else
				{	remove(download);
					rs_log_error("downloading %s",file);
				}

				g_free(download);
			}
		}
		
		g_free(dirname);
		g_free(local);
	}
}

int 
iol_resync(iol_t iol, const char *code)
{	struct tmp_resync_getfile tmp;
	int ret = E_OK;

	if ( !IS_IOL_T(iol)||iol->repository==NULL||!is_valid_course_code(code))
		ret = E_INVAL;
	else if( !iol->bLogged )
		ret = E_NLOGED;
	else if( iol_set_current_course(iol, code) != E_OK ) 
	{ 	rs_log_error(_("setting current course to `%s'"),code);
		ret = E_NETWORK;
	} 
	else
	{       char *s = g_strdup_printf("%s/%s/%s", iol->repository, code,
	                                  IOL_MATERIAL_FOLDER);
		struct stat buf;

		/* try to create course folder */ 
		if( create_course_directory(s) == -1 ) 
		{
			rs_log_error("error creating dir `%s'",s);
			rs_log_error("%s",strerror(errno) ); ret = E_FS;
		}
		else
		{       GSList *files;
		
			tmp.prefix = s;
			tmp.iol = iol;
			tmp.url_prefix = NULL;
			get_current_file_list(iol, &files, &(tmp.url_prefix));
			g_slist_foreach(files, (GFunc)foreach_getfile, &tmp);
			g_free(tmp.url_prefix);
		}

		g_free(s);
	}

	return ret;
} 

/**/

/** comunication between #iol_resync_all() and #foreach_resync() */ 
struct foreach_resync 
{	int ret;
	iol_t iol;
};

/* nice for a lambda function */
static void
foreach_resync(struct course *course, struct foreach_resync *r) 
{
	if( course && r )
		r->ret |= iol_resync(r->iol, course->code);
}

int
iol_resync_all(iol_t iol) 
{	struct foreach_resync r;

	if( !IS_IOL_T(iol) )
		return -1;

	r.ret = 0;
	r.iol = iol;
	g_slist_foreach(iol->courses,(GFunc)foreach_resync,&r);

	return r.ret;
}
