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

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curl/curl.h>
#include <trace.h>
#include <strdup.h>

#include "i18n.h"
#include "iol.h"
#include "link.h"

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
#define URL_FILE	URL_BASE"/newmaterialdid.asp"
#define IOL_COURSE_PARAMETER	"nivel=4"

struct buff {
	char *data;
	size_t size;
};

struct course
{	char *code;
	char *name;
};

/* hidden:
 *	bLogged:	Already logged in?
 *	szCookie:	The servers cookie if any
 */
struct hidden
{
	CURL *u;
	bool bLogged;
	char *szCookie;
	char *buff;
	size_t buffsize;
	char *cookietmp;
	
	std::list<struct course > courses;
	char *current_course;

	char *prefix;
};


/* Gets a temporary filename for the cookies files 
 */
static char *
getCookieTmp(void)
{ 	char *nRet,*p;
	size_t n;

	nRet = curl_getenv("TMP");
	if( nRet  == NULL )
	{ 	if( SYSTEM == UNIX )
			nRet = strdup("/tmp");
		else if( SYSTEM == WINDOWS )
			nRet = strdup( ".\\");
	}

	if( nRet == NULL )
		return NULL;

	n = strlen(nRet) + strlen(COOKIEFILE) + 2;
	p = (char *)malloc( n );
	if( p != NULL )
	{
		snprintf(p,n,"%s%c%s",nRet,
				SYSTEM == UNIX ? '/' : '\\',
				COOKIEFILE);
		p[n-1] = '\0';
		free(nRet);
	}

	return p;
}

IOL::IOL( void )
{
	rs_log_info("init");
	
	this->cdt = new struct hidden;
	cdt->bLogged = 0;
	cdt->szCookie = 0;
	cdt->buff = 0;
	cdt->buffsize = 0;
	cdt->cookietmp = getCookieTmp();
	cdt->current_course = 0;
	if( cdt->cookietmp == 0 )
	{	delete cdt;
		rs_log_error(_("locating a temporary file"));
		return;
	}
	curl_global_init(CURL_GLOBAL_ALL);
	cdt->u = curl_easy_init( );
	curl_easy_setopt(cdt->u,CURLOPT_COOKIEJAR, cdt->cookietmp);
	curl_easy_setopt(cdt->u,CURLOPT_USERAGENT,USERAGENT);
	
}

IOL::~IOL( void )
{	std::list <struct course>::iterator iter;

	if( cdt == NULL )
		return ;

	if( cdt->bLogged )
		logout();
	
	curl_easy_cleanup(cdt->u);
	curl_global_cleanup();

	free( cdt->cookietmp );
	if( cdt->buffsize )
		delete cdt->buff;
		
	for (iter = cdt->courses.begin(); iter != cdt->courses.end(); iter++)
	{ 	free( (*iter).code );
		free( (*iter).name );
	}

	free( cdt->current_course );
	free (cdt->prefix);

	delete cdt;
}

int IOL::set_repository( const char *path )
{	int ret = E_OK;

	if( cdt == NULL || path == NULL )
		ret = E_INVAL;
	else
	{
		cdt->prefix = strdup(path);
		if( cdt->prefix == 0 )
			ret = E_MEMORY;
	}
	
	return ret;
}

static bool
is_valid_code( const char *code )
{	bool b = true;
	unsigned i;
	
	if( code == 0 )
		b = false;
	else
	{
		for( i=0; b && code[i] ; i++ )
		{	/* dangerous characters for the filesystem */
			if( code[i]=='/' || code[i] == '\\' || code[i] == '.' ||
			    code[i]==':' || code[i] == '>' /* win32 */ )
			    	b = false;
		}

		if( i == 0 )
			b = false;
	}

	return b;
}

static void
link_courses_fnc( const char *link, const char *comment, void *d )
{ 	std::list<struct course> *courses = (std::list<struct course> *)d;
	struct course course;
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

		course.code = strdup(s);
		course.name = strdup(ss);

		if( course.code && course.name )
			courses->push_back(course);
		else
		{	free(course.code);
			free(course.name);
		}
		
	}
}


unsigned 
IOL::load_courses(struct buff *page)
{	link_parser_t parser;
	unsigned i;	
	
	parser = link_parser_new();
	if( parser == NULL )
		return 0;
		
	link_parser_set_link_callback(parser,link_courses_fnc, &(cdt->courses));
	for( i = 0 ; i< page->size && 
	     link_parser_proccess_char(parser,page->data[i])==0 ; i++ )
	     ;
	link_parser_destroy(parser);

	return cdt->courses.size();
}

int
IOL::login( const char *user, const char *pass )
{	 	
	int nRet = E_OK;

	if( cdt == NULL || user == NULL || pass == NULL || !*user || !*pass )
		return E_INVAL;
		
	if( cdt->bLogged )
	{	rs_log_warning(_("login(): ya estamos logueados"));
		nRet = E_ALOGED ;
	}
	else
	{	struct buff buf;

		/* yummm!!! get a cookie */
		rs_log_info(_("login on as `%s'"),user);
		if( transfer_page(URL_LOGIN,0,NULL)!= E_OK )
		{	rs_log_error(_("login(): network error"));
			nRet = E_NETWORK;
		}
		else
		{	size_t len = sizeof(URL_LOGIN_ARG) + 2 +  strlen(user) 
                                    + strlen(pass);
			char *s;
			s = new char [len];

			/* login */
			sprintf(s,URL_LOGIN_ARG,user,pass);
			curl_easy_setopt(cdt->u, CURLOPT_POSTFIELDS, s);
			if( transfer_page(URL_LOGIN_1,0,&buf)
			   != E_OK )
                	{	rs_log_error(_("login(): network error"));
				nRet = E_NETWORK;
                	} 
                	else
                	{	if( load_courses(&buf) == 0)
                		 rs_log_error(_("login(): login failed"));
                		else
                			cdt->bLogged = 1;
                	}

			curl_easy_setopt(cdt->u,CURLOPT_HTTPGET,1L);
			delete []s;
		}
	}

	return nRet;
}

int
IOL::set_current_course(const char *course)
{ 	char *s;
	size_t len;
	int ret = E_OK;

	if( cdt == NULL )
		return E_INVAL;

	/* i dont check whether the course exists in the list because is not 
	 * catastrofic. ( at least not for the client side :^)  )
	 */

	if( cdt->current_course )
		free(cdt->current_course);
	cdt->current_course = strdup(course);
	if( cdt->current_course == NULL )
		return E_MEMORY;

	len = sizeof(URL_CHANGE) + strlen(course) + 1;
	s = new char [len];
	sprintf(s,URL_CHANGE,course);
	
	if( transfer_page(s,0,0) != E_OK )
		ret = E_NETWORK;
	delete []s;

	return  ret; 
}


struct tmp {
	std::queue<char *> *pending;
	std::list<char *> *files;
	char *prefix;
};

static bool
url_is_file(const char *url)
{	char *p,*q;

	p = basename(url);
	q = basename(URL_FILE);

	return strncmp(p,q, strlen(q));
}

static void
link_files_fnc( const char *link, const char *comment, void *d )
{	struct  tmp *t = (struct tmp *)d;
	bool bFile;

	bFile = url_is_file(link);
	
	assert(d);
}

int IOL::get_file_list_recursive(const char *url, std::list<char *> *files,
                                 std::queue<char *> *pending)
{ 	struct buff webpage;
	bool bFile;
	int ret;

	bFile = url_is_file(url);
	if( bFile )
		files->push_front((char *)url);
	else
	{
		if( transfer_page(url,0,&webpage) != E_OK )
			ret = E_NETWORK;
		else
		{	link_parser_t parser;
			struct tmp tmp;
			unsigned i;

			tmp.pending = pending;
			tmp.files = files;
		
			/* get and analize links */
			parser = link_parser_new();
			if( parser == NULL )
				return 0;
				
			link_parser_set_link_callback(parser,link_courses_fnc,
			                              &(cdt->courses));
			for( i = 0 ; i< webpage.size && 
			   link_parser_proccess_char(parser,webpage.data[i])==0;
			   i++ )
			     ;
			link_parser_destroy(parser);

			}
	}
	return 0;
}

int IOL::get_file_list(std::list<char *> &l)
{	std::queue<char *> pending;

	/* queue is passed so each time i dont need to create a new queue */
	get_file_list_recursive(URL_FILE,&l,&pending);

	return 0;
}

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

int
IOL::resync(const char *code)
{	int ret = E_OK;

	if( ! cdt->bLogged )
		ret = E_NLOGED;
	else if ( cdt->prefix == NULL || !is_valid_code(code) )
		ret = E_INVAL;
	else if( set_current_course(code) != E_OK )
	{	rs_log_error(_("setting current course to `%s'"),code);
		ret = E_NETWORK;
	}
	else
	{	char *s;
		size_t len = strlen(cdt->prefix) + strlen(code)  + 2;
		struct stat buf;
		
		/* try to create course folder */
		s = new char[len];
		snprintf(s,len,"%s/%s", cdt->prefix, code);
		s[len-1]=0;

		if( create_course_directory(s) == -1 )
		{	rs_log_error("error creating dir `%s'",s);
			rs_log_error("%s",strerror(errno) );
			ret = E_FS;
		}
		else
		{	std::list<char *> files;
			std::list <char *>::iterator iter;

			get_file_list(files);
			for (iter=files.begin(); iter != files.end(); iter++)
				rs_log_info("%s\n",*iter);
		}
		
		delete []s;
	}

	return ret;
}

int
IOL::resync_all()
{	std::list <struct course>::iterator iter;
	struct course c;
	int nRet = 0;
	
	if( cdt== NULL )
		return -1;

	for (iter = cdt->courses.begin(); iter != cdt->courses.end(); iter++)
		nRet |= resync((*iter).code);

	return 0;
}

int
IOL::logout()
{	int nRet = E_OK;

	if( cdt == NULL )
		return E_INVAL;

	if( cdt->bLogged  )
	{
		rs_log_info(_("logging off"));
		cdt->bLogged = false;
	}
	else
	{	rs_log_warning(_("logout(): no estamos logueados"));
		nRet = E_NLOGED;
	}
	

	return nRet;
}

static size_t
write_data_to_memory (void *ptr, size_t size, size_t nmemb, void *data)
{ 	size_t realsize = size * nmemb;
	struct buff  *mem = (struct buff *)data;

	if( data == NULL )	/* the user don't want the input */
		return realsize;
	
	mem->data = (char *)realloc(mem->data, mem->size + realsize + 1);
	if (mem->data) 
	{ 	memcpy(mem->data + mem->size, ptr, realsize);
		mem->size += realsize;
		mem->data[mem->size] = 0;
	}

	return realsize;
}
           

int
IOL::transfer_page( const char *url, unsigned flags, struct buff *page )
{	CURLcode res;

	if( page ) 
	{	page->data= NULL;
		page->size = 0;
	}

	if( cdt == NULL || url == 0 || url[0]==0)
		return E_INVAL;

	rs_log_info(_("downloading %s"),url);

	curl_easy_setopt(cdt->u, CURLOPT_NOPROGRESS, 1);
	
	curl_easy_setopt(cdt->u, CURLOPT_WRITEFUNCTION, write_data_to_memory);
	curl_easy_setopt(cdt->u, CURLOPT_FILE, page);
	curl_easy_setopt(cdt->u, CURLOPT_URL, url);
	res = curl_easy_perform(cdt->u);

	return res == 0 ? E_OK : E_NETWORK;
}

