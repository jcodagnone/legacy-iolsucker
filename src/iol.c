/*
 * main.cpp -- IOLsucker web robot implementation
 *
 * Copyright (C) 2003 by Juan F. Codagnone <juam@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

#include "i18n.h"
#include "iol.h"
#include "link.h"

/** User Agent string reported to the webserver */
#define USERAGENT	"test"
#define COOKIEFILE	"cookieiol"

#define IOL_HOST        "silvestre.itba.edu.ar"
#define IOL_PATH        "itbaV"
#define IOL_LEVEL       "4" 

#define URL_BASE	"http://"IOL_HOST"/"IOL_PATH
#define URL_LOGIN	URL_BASE"/login.asp"
#define URL_LOGIN_1	URL_BASE"/mynav.asp"
#define URL_LOGIN_ARG	"txtdni=%s&txtpwd=%s&Submit=Conectar&cmd=login"
#define URL_LOGOUT	URL_BASE"/mynav.asp?cmd=logout"
#define URL_CHANGE 	URL_BASE"/mynav.asp?cmd=ChangeContext&amp;nivel=4&amp;snivel=%s"
#define URL_MATERIAL	URL_BASE"/newmaterialdid.asp" 
#define IOL_COURSE_PARAMETER	"nivel=4"

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
	char *repository;	 /**< repository directory */
};


#define IS_IOL_T( iol ) ( iol != NULL  )

/**
 * gets the path were we save the cookies */
static char *
get_cookies_file(void)
{	char *nRet,*p;
	size_t n;

	nRet = curl_getenv("TMP"); if( nRet  == NULL ) {       if(
	SYSTEM == UNIX )
			nRet = strdup("/tmp");
		else if( SYSTEM == WINDOWS )
			nRet = strdup( ".\\");
	}

	if( nRet == NULL )
		return NULL;

	n = strlen(nRet) + strlen(COOKIEFILE) + 2; p = malloc( n );
	if( p != NULL )
	{
		g_snprintf(p,n,"%s%c%s",nRet, 
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

	return realsize;
}

/**
 *  wraper to libcurl. Transfer the url, and saves it in the buffer page
 * */
static int
transfer_page( CURL *curl, const char *url, unsigned flags, struct buff *page ) {	    CURLcode res;

	if( curl == NULL || url == 0 || url[0]==0)
		return E_INVAL;

	rs_log_info(_("downloading %s"),url); 
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1); 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_memory);
	curl_easy_setopt(curl, CURLOPT_FILE, page); 
	curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_perform(curl);

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

int iol_login(iol_t iol, const char *user, const char *pass)
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
	g_free(s);

	return	ret;
}

/**/
static int
create_course_directory(const char *dir) 
{	struct stat buf;
	int ret = 0;

	if( stat(dir,&buf) == -1 )
	{	if( errno == ENOENT )
		{	if(mkdir(dir,0755) == -1 )
				ret = -1;
		}
		else
			ret = -1;
	}

	return ret;
}

static void
foreach_printdebug(char *file, void *user_data) 
{
	if( file )
		printf("%s",file);
}

static int get_current_file_list(iol_t iol, GSList **l);

int 
iol_resync(iol_t iol, const char *code)
{	int ret = E_OK;

	if ( !IS_IOL_T(iol)||iol->repository==NULL||!is_valid_course_code(code))
		ret = E_INVAL;
	else if( !iol->bLogged )
		ret = E_NLOGED;
	else if( iol_set_current_course(iol, code) != E_OK ) 
	{ 	rs_log_error(_("setting current course to `%s'"),code);
		ret = E_NETWORK;
	} 
	else
	{       char *s = g_strdup_printf("%s/%s", iol->repository, code);
		struct stat buf;

		/* try to create course folder */ 
		if( create_course_directory(s) == -1 ) 
		{
			rs_log_error("error creating dir `%s'",s);
			rs_log_error("%s",strerror(errno) ); ret = E_FS;
		}
		else
		{       GSList *files;

			get_current_file_list(iol, &files);
			g_slist_foreach(files, (GFunc)foreach_printdebug, NULL);
		}

		g_free(s);
	}

	return ret;
} 

/**/

/** comunication between #iol_resync_all and #foreach_resync() */ 
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


/* Download Material didactico secction
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
};

static void link_files_fnc( const char *link, const char *comment, void *d ) 
{	struct  tmp *t = (struct tmp *)d;
	int bFile;
	char *s;

	s = g_strdup_printf("%s/%s",URL_BASE,link);
	if( s == NULL )
		return;

	bFile = url_is_file(link);
	if( bFile )
		t->files = g_slist_prepend(t->files, s);
	else
		g_queue_push_head(t->pending, s);

}


static int
get_current_file_list(iol_t iol, GSList **l)
{	struct buff webpage = { NULL, 0 };
	struct tmp t;
	char *url = URL_MATERIAL;
	int bFile;
	int ret;

	t.pending = g_queue_new(); 
	t.files = NULL;
	g_queue_push_head(t.pending, url);
	bFile = url_is_file(url);
	if( bFile )
		t.files = g_slist_prepend(t.files, (void *)g_strdup(url));
	else
	{
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
	}

	assert(g_queue_is_empty(t.pending));
	g_queue_free(t.pending); 
	*l = t.files;

	return 0;
}
