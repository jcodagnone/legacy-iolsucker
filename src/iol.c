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
 #include <config.h>
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
#ifdef HAVE_UNISTD_H
  #include <unistd.h>
#else
/*  #include <unix.h>*/
#endif



#include <trace.h>
#include <strdup.h>
#include <basename.h>
#include <mkrdir.h>
#include <dirname.h>
#include <queue.h>

#include "i18n.h"
#include "iol.h"
#include "link.h"
#include "progress.h"
#include "cache.h"
#include "forum.h"

#ifndef CURLOPT_WRITEDATA
  #define CURLOPT_WRITEDATA	CURLOPT_FILE	/* libcurl < 7.9.7 */
#endif

#ifdef IOLDEMO
  #define curl_easy_setopt(c,p,q) ioldemo_curl_easy_setopt(c,p,(void *)q)
  int ioldemo_curl_easy_setopt(CURL *c, unsigned f, void *q); 
#endif

/** User Agent string reported to the webserver */
#ifdef WIN32
	#define USERAGENT	"iolsucker ("VERSION"; Windows )"
#else
	#define USERAGENT	"iolsucker ("VERSION"; Linux )"
#endif

#define IOL_HOSTN	"silvestre.itba.edu.ar"
#define IOL_PATH        "itbaV"
#define IOL_LEVEL       "4" 

#define URL_BASE	"http://"IOL_HOSTN"/"IOL_PATH
#define URL_LOGIN	"/login.asp"
#define URL_LOGIN_1	"/mynav.asp"
#define URL_LOGOUT	"/mynav.asp?cmd=logout"
#define URL_CHANGE_COU 	"/mynav.asp?cmd=ChangeContext&nivel=4&snivel=%s"
#define URL_CHANGE_DPT	"/mynav.asp?cmd=ChangeContext&nivel=3&snivel=%s"
#define URL_MATERIAL	"/newmaterialdid.asp" 
#define URL_DOWNLOAD	"/download.asp" 
#define IOL_FORUM  	"/foroDis.asp"
#define IOL_NEWS	"/novlistall.asp"

#define URL_LOGIN_ARG	"txtdni=%s&txtpwd=%s&Submit=Conectar&cmd=login"
#define IOL_COURSE_PARAMETER	"nivel=4"
#define IOL_DEPART_PARAMETER	"nivel=3"

#define IOL_MATERIAL_FOLDER	"material"
#define IOL_MATERIAL_TMPFILE	".download.tmp"
#define IOL_FILE_DB        	"files.db"
#define IOLSUCKER_LOGFILE	"cambios.txt"

struct buff 
{ 	char *data;
	size_t size;
};

enum course_type {
	CT_COURSE,
	CT_DEPART
};

struct course 
{       char *code;	/**< code name: eg: "21.71" */
	char *name;     /**< human name: Base de datos I */
	enum course_type type; 
	enum resync_flags flags;
};

/**
 * Concrete data type for the IOL object */
struct iolCDT
{ 	CURL *curl;		/**< curl handler */
	char errorbuf[CURL_ERROR_SIZE+1];	/**< CURLOPT_ERRORBUFFER */
	double transfered_bytes;/**< bytes downloaded */

	const struct course *current_course;
	GSList *courses;	/**< loaded courses */ 
	int bLogged;    	/**< already logged in ? */ 
	char *repository;	/**< repository directory */
	const char *host; 	/**< host part in the URL */

	int dry;		/**< dry run ? */
	int verbose;		/**< print lots of information? */
	int fancy;		/**< use fancy names */
	int wait;		/**< seconds to wait when changing context */

	cache_t fcache;		/**< cache of files for download.asp **/
	FILE *logfp;		/**< logfile filepointer */

	char url_tmp[256];
	char *url_tmp_pt;
	
};

#define IS_IOL_T( iol ) ( iol != NULL  )

enum TP_FLAGS 
{	TP_FILE	= 1 << 0
};

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

#ifdef HAVE_CURLOPT_DEBUGDATA
/**
 *  prints the debug information of the curl library
 */
static int 
curl_debug_fnc(CURL *curl, curl_infotype type, char  *ptr, size_t size,
               iol_t iol)
{	
	/* stolen from libcurl */
	static const char * const s_infotype[CURLINFO_END] =
        { "* ", "< ", "> ", "{ ", "} " };

	if( iol->verbose )
	{
		switch(type) 
		{	case CURLINFO_TEXT:
			case CURLINFO_HEADER_OUT:
				fwrite(s_infotype[type], 
				       sizeof(s_infotype[type])-1, 1, stderr);
		       		fwrite(ptr, size, 1, stderr);
		       		if( ptr[size - 1]!='\n' )
		       			fwrite("\n", 1, 1, stderr);
		     		break;
		        default:
		        	break;
		}
	}

	return 0;
}
#endif

/**
 * acumulate transfered bytes
 */
static void
count_bytes(CURL *curl)
{
#ifdef HAVE_CURLOPT_PRIVATE	/* libcurl >= 7.10.3 */
	double d=0 ;
	double *acc=0;
	long header, request;

	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &d);
	curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &request);
	curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &header);
	curl_easy_getinfo(curl, CURLINFO_PRIVATE, &acc);

	*acc += d + request + header;
#endif
}

/**
 *  wraper to libcurl. Transfer the url, and saves it in the buffer page or 
 *  in a file.
 */
static int
transfer_page( CURL *curl, const char *url, unsigned flags, void *data)
{	CURLcode res;
	FILE *fp = NULL;
	struct progress *progress = NULL;

	if( curl == NULL || url == 0 || url[0]==0)
		return E_INVAL;

	/* rs_log_info(_("downloading %s"),url);  */
	
	if( flags & TP_FILE )
	{	void *ptr;

		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data_to_file);
		fp = fopen(data, "wb");	
		if( fp == NULL )
			return E_INVAL;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);

		if( isatty(fileno(stdout)) )
			ptr = bar_progress_callback;
		else
			ptr = dot_progress_callback;
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ptr);
		progress = new_progress_callback(ptr);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress);
	}
	else
	{ 	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
		                       write_data_to_memory);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data); 
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, TRUE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_perform(curl);
	count_bytes(curl);

	if( fp )
		fclose(fp);
	if( progress )
		destroy_progress_callback(progress);
	curl_easy_setopt(curl, CURLOPT_URL, "");
	return res == 0 ? E_OK : E_NETWORK;
}

/**
 * given url_part creates the url.
 *
 * \return NULL if fails.
 */
static char *
iol_get_url(iol_t iol, const char *url_part)
{	static const char *http = "http://";
	size_t len;
	char *pt;
	int slash = *url_part != '/';
	
	free(iol->url_tmp_pt);

	len = strlen(http) + strlen(url_part) + strlen(iol->host) + 1 + slash;
	if( len < sizeof(iol->url_tmp) )
		pt = iol->url_tmp;
	else
		pt = iol->url_tmp_pt = malloc(len);

	if( pt )
	{	strcpy(iol->url_tmp, http);	
		strcat(iol->url_tmp, iol->host);
		strcat(iol->url_tmp, "/");
		strcat(iol->url_tmp, IOL_PATH);
		if( slash )
			strcat(iol->url_tmp, "/");
		
		strcat(iol->url_tmp, url_part);
	}

	return pt;
}

iol_t 
iol_new(void) 
{	iol_t cdt;

	cdt = malloc( sizeof(*cdt) );
	if( cdt == NULL )
		return NULL;
		
	memset( cdt, 0, sizeof(*cdt));
	cdt->host = IOL_HOSTN;

	curl_global_init(CURL_GLOBAL_ALL);
	cdt->curl = curl_easy_init();
	curl_easy_setopt(cdt->curl,CURLOPT_COOKIEJAR, "");
	curl_easy_setopt(cdt->curl,CURLOPT_USERAGENT,USERAGENT);
	curl_easy_setopt(cdt->curl,CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(cdt->curl,CURLOPT_ERRORBUFFER, cdt->errorbuf);
	#ifdef HAVE_CURLOPT_PRIVATE /* libcurl >= 7.10.3 */
	  curl_easy_setopt(cdt->curl,CURLOPT_PRIVATE, &(cdt->transfered_bytes));
	#endif  

	#ifdef HAVE_CURLOPT_DEBUGDATA
	  curl_easy_setopt(cdt->curl,CURLOPT_DEBUGFUNCTION, curl_debug_fnc);
	  curl_easy_setopt(cdt->curl,CURLOPT_DEBUGDATA, cdt);
	#endif

	return cdt;
}

static void
free_courses_list( struct course *data, gpointer user_data ) 
{
	if( data )
	{	free(data->code);
		free(data->name);
		free(data);
	}
}

static void
show_curl_stats( CURL *curl)
{	
#ifdef HAVE_CURLOPT_PRIVATE	/* libcurl >= 7.10.3 */
	static const char *unit[]={"bytes","KB","MB","GB","TB"};
	double *d=0 ;
	unsigned i;
	const char *u;
	

	curl_easy_getinfo(curl, CURLINFO_PRIVATE, &d);
	assert(d);
	for( i=0; *d > 1024; i++ )
		*d/=1024;

	if( i >= sizeof(unit)/sizeof(*unit) )
		u = "unknown";
	else
		u = unit[i];

	*d = ((int)(*d+0.005)) + ((int)((*d+0.005)*100))/100.0;
	rs_log_info(_("%.2f %s transfered"),*d,u);
#endif
}

void
iol_destroy(iol_t iol) 
{
	if( !IS_IOL_T(iol) )
		return;

	if( iol->bLogged )
		iol_logout(iol);

	show_curl_stats(iol->curl);
	curl_easy_cleanup(iol->curl); 
	curl_global_cleanup();

	g_slist_foreach(iol->courses, (GFunc)free_courses_list ,NULL);
	g_slist_free(iol->courses);

	free(iol->repository);
	free(iol->url_tmp_pt);
	
	if( iol->fcache )
		cache_destroy(iol->fcache);
	free(iol);
}

static int
iol_set_repository(iol_t cdt, const char *path)
{	int ret = E_OK;
	char *s;
	
	if( path == NULL )
		ret = E_INVAL;
	else 
	{ 	cdt->repository= strdup(path); 
		if( cdt->repository == NULL )
			ret = E_MEMORY;
		else
		{	if( cdt->fcache )
			{	/** \todo  close and reopen the db ? 
			 	  * mmmm
			 	  */
			 	  abort();
			}
			else
			{	s = g_strdup_printf("%s/%s", cdt->repository,
				                         IOL_FILE_DB);
				mkrdir(cdt->repository, 0755);
				cdt->fcache = cache_new(s);
				g_free(s);
			}

			if( cdt->logfp )
				fclose(cdt->logfp);
			s = g_strdup_printf("%s/%s", cdt->repository,
			                    IOLSUCKER_LOGFILE);
			if( !s || (cdt->logfp = fopen(s,"a")) == NULL)
			{	rs_log_warning(_("could not open log file"));
				rs_log_warning("%s:%s", s, strerror(errno));
			}
			g_free(s);
		}
	}

	return ret;
}

static int 
iol_set_proxy_type(iol_t cdt, const char *type)
{	
#ifdef HAVE_PROXYTYPE
	if( !strcmp(type, "http" ) )
		curl_easy_setopt(cdt->curl,CURLOPT_PROXYTYPE,CURLPROXY_HTTP);
	else if( !strcmp(type, "socks5") )
		curl_easy_setopt(cdt->curl,CURLOPT_PROXYTYPE,CURLPROXY_SOCKS5);

	return E_OK;
#else
	return E_INVAL;
#endif
}

static int
iol_set_proxy_host(iol_t cdt,  const char *proxy)
{	int ret = E_OK;
	int b;

	if( proxy )
	{	b = curl_easy_setopt(cdt->curl, CURLOPT_PROXY, proxy);
		if( b != CURLE_OK )
			ret = E_INVAL;
	}
	else
		ret = E_INVAL;

	return ret;
}

static int
iol_set_proxy_user(iol_t cdt, const char *proxy_user)
{	int ret = E_OK;
	int b;
	
	if( proxy_user )
	{	b = curl_easy_setopt(cdt->curl,CURLOPT_PROXYUSERPWD,proxy_user);
		if( b != CURLE_OK )
			ret = E_INVAL;
	}
	else
		ret = E_INVAL;
		
	return ret;
}

static int 
iol_set_verbose( iol_t cdt, int *b)
{	int ret = E_OK;

	if( b )
	{	cdt->verbose = *b !=0;
		curl_easy_setopt(cdt->curl,CURLOPT_VERBOSE, cdt->verbose);
	}
	else
		ret = E_INVAL;
	
	return ret;
}

static int
iol_set_download(iol_t cdt, int *download)
{	int ret = E_OK;

	if( download )
		cdt->dry = *download;
	else
		ret = E_INVAL;
		
	return ret;
}

static int
iol_set_fancy_names(iol_t cdt, int *fancy)
{	int ret = E_OK;

	if( fancy )
		cdt->fancy = *fancy;
	else
		ret = E_INVAL;

	return ret;
}


static int
iol_set_wait(iol_t cdt, int *wait)
{	int ret = E_OK;

	if( wait )
		cdt->wait = *wait != 0;
	else
		ret = E_INVAL;
		
	return ret;
}

static int
iol_set_host(iol_t cdt, const char *host)
{	int ret = E_INVAL;
	int valid = 1;
	const char *s;
	
	if( host )
	{	
		for( s = host; valid && *s ; *s ++ )	
			valid = isalnum(*s) || *s == '.' || *s == '-' ||
			        *s == '_' || *s == ':';

		if( valid )
		{	cdt->host = host;
			ret = E_OK;
		}
	}

	return ret;
}

int
iol_set(iol_t iol, enum iol_settings set, void *data)
{	unsigned i;
	int ret = E_OK;
	typedef int (* iol_set_fnc) (iol_t, void *d);
	struct table 
	{	enum iol_settings id;
		iol_set_fnc fnc;
	}  table [] = 
	{	{	IOL_REPOSITORY,	(iol_set_fnc) iol_set_repository },
		{	IOL_PROXY_TYPE, (iol_set_fnc) iol_set_proxy_type },
		{	IOL_PROXY_HOST, (iol_set_fnc) iol_set_proxy_host },
		{	IOL_PROXY_USER, (iol_set_fnc) iol_set_proxy_user },
		{	IOL_DRY,        (iol_set_fnc) iol_set_download   },
		{	IOL_VERBOSE,    (iol_set_fnc) iol_set_verbose    },
		{	IOL_FANCY_NAMES,(iol_set_fnc) iol_set_fancy_names},
		{	IOL_WAIT,       (iol_set_fnc) iol_set_wait       },
		{	IOL_HOST,       (iol_set_fnc) iol_set_host       }
	};

	if( !IS_IOL_T(iol) || set <0 || set >= IOL_MAX )
		ret = E_INVAL;
	else
	{	for( i=0; i< sizeof(table)/sizeof(*table) ; i++)
			if( table[i].id == set )
			{	ret = table[i].fnc(iol,data);
				break;
			}
		/* the user think he is funny ... */
		if( i == sizeof(table)/sizeof(*table) )
		{	rs_log_warning("iol_set(): unknown id `%d'",set);
			ret = E_INVAL;
		}
	}

	return ret;
}
/** validates weather ::code is in the parameters in server's list */
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

/* one_space ( touper ( trim( name ) ) ) */
static char *
normalize_course_name( char *name )
{       char *p, *q;

        for( q = p = name ; *p && isspace(*p) ; p++ )
                ;
        while( *p )
        {
                for( ; *p && !isspace(*p) ; p++ )
                        *(q++) = toupper(*p);

                if( isspace(*p) )
                {       *(q++) = *p++;
                        for( ; *p && isspace(*p) ;  p++ )
                                ;
                }
        }

        *q = 0;
        if( *(q-1) == ' ' )
        	*(q-1) = 0;

        return name;
}

/**
 * callback called by the html parser when procesing newmaterial.asp 
 */
static void 
link_courses_fnc( unsigned const char *link, 
                  unsigned const char *comment, void *d )
{	GSList **listptr =  d;
	struct course *course; 
	char *s;
	enum course_type type;
	assert(d);

	s = strstr(link, IOL_COURSE_PARAMETER);
	if( s == NULL )
	{	s = strstr(link, IOL_DEPART_PARAMETER);
		if( s == NULL )
			return;
		else
			type = CT_DEPART;
	}
	else
		type = CT_COURSE;
	
	
	/* get course code */
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
			course->name = normalize_course_name(strdup(ss));
			course->type = type;
			course->flags = 0;
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
 * gets the list of courses that are linked from the html at ::page 
 */
static unsigned 
parse_courses(GSList **listptr, struct buff *page) 
{	link_parser_t parser;
	unsigned i;

	parser = link_parser_new();
	if( parser == NULL )
		return 0;

	link_parser_set_link_callback(parser,link_courses_fnc, listptr);
	for( i = 0 ; i< page->size &&
	     link_parser_process_char(parser,page->data[i])==0 ; i++ ) 
	     ;
	link_parser_end(parser);
	link_parser_destroy(parser);

	return (*listptr)== NULL ? -1 : 0;
}

int
iol_login(iol_t iol, const char *user, const char *pass)
{	int nRet = E_OK;
	char *url = NULL;
	
	if( !IS_IOL_T(iol) || user == NULL || pass == NULL || !*user || !*pass )
		nRet = E_INVAL;
	else if( iol->bLogged )
	{	rs_log_warning(_("login(): ya estamos logueados"));
		nRet = E_ALOGED ;
	}
	else if( (url = iol_get_url(iol, URL_LOGIN)) == NULL )
		nRet = E_MEMORY;
	else
	{       struct buff buf = {NULL, 0};
		
		/* yummm!!! get a cookie */ 
		if( transfer_page(iol->curl, url, 0, NULL)!= E_OK )
			nRet = E_NETWORK;
		else if( (url = iol_get_url(iol, URL_LOGIN_1)) == NULL) 
			nRet = E_MEMORY;
		else
		{       char *s = g_strdup_printf(URL_LOGIN_ARG, user,pass);

			/* login */
			curl_easy_setopt(iol->curl, CURLOPT_POSTFIELDS, s);

			/* hack:  iol scripts break if the login is not
			 * a number. i won't force the user to enter 
			 * numbers.... if it fails, just ignore the error
			 * the parser won't find any course
			 */
			curl_easy_setopt(iol->curl,CURLOPT_FAILONERROR, 0);
			
			if(transfer_page(iol->curl, url,0,&buf) != E_OK)
				nRet = E_NETWORK;
			else if( parse_courses(&(iol->courses),&buf) == 0)
					iol->bLogged = 1;
			else
				nRet = E_LOGINTUPLE;

			curl_easy_setopt(iol->curl,CURLOPT_FAILONERROR, 1);
			curl_easy_setopt(iol->curl, CURLOPT_POSTFIELDS, "");
			curl_easy_setopt(iol->curl,CURLOPT_POST,0L);
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
	char *url;
	
	if( !IS_IOL_T(iol)  )
		nRet = E_INVAL;
	else if( (url = iol_get_url(iol, URL_LOGOUT) ) == NULL )
		nRet = E_MEMORY;
	else if( iol->bLogged  )
	{	transfer_page(iol->curl, url, 0, NULL);
		iol->bLogged = 0;
	}
	else
	{       rs_log_warning(_("logout(): no estamos logueados"));
		nRet = E_NLOGED;
	}


	return nRet;
}


struct tmp_cname
{	const char *code;
	struct course *ret;
};

static void
foreach_find_cname( struct course *course, struct tmp_cname *t)
{
	if( t && !t->ret )
	{	
		if( !strcmp(t->code, course->code) )
			t->ret = course;
	
	}
}

static struct course *
get_course_by_name( iol_t iol, const char *code )
{	struct tmp_cname tmp;

	tmp.code = code;
	tmp.ret = NULL;
	
	g_slist_foreach(iol->courses, (void *)foreach_find_cname, (void *)&tmp);
	
	return tmp.ret;
}

static void 
link_context_fnc( const unsigned char *link,
                  const unsigned char *comment, enum resync_flags *flags)
{	static const struct 
	{	const char *link;
		enum resync_flags flag;
	} table[]=
	{	{ "newmaterialdid.asp", IOL_RF_FILE  },
		{ "reglamentacion.asp", IOL_RF_RULES },
		{ "novlist.asp",        IOL_RF_NEWS  },
		{ "foroDis.asp",        IOL_RF_FORUM },
		{ "AluList.asp",        IOL_RF_ALULIST},
	};
	unsigned i;
	int found=0;

	assert(flags);
	
	for(i=0; !found && i<sizeof(table)/sizeof(*table); i++)
	{
		if( !strcmp(link,table[i].link) )
			found=1;
	}

	if( found )
		*flags |=table[i-1].flag;
}

static enum resync_flags
get_course_capabilities( struct buff *page )
{ 	link_parser_t  parser;
	enum resync_flags flags = 0;
	unsigned i;
	
	parser = link_parser_new();
	if( parser )
	{
		link_parser_set_link_callback(parser,
				(link_callback)link_context_fnc,
				&flags);
		for( i = 0 ; i< page->size &&
		           link_parser_process_char(parser,page->data[i])==0;
		     i++ ) 
		     	;
		link_parser_end(parser);
		link_parser_destroy(parser);
	}
	
	
	return flags;
}

static int 
iol_set_current_course(iol_t iol, const char *course) 
{	struct course *c;
	int ret = E_OK;
	char *s, *burl;
	
	if( !IS_IOL_T(iol) )
		ret = E_INVAL;
	else if( (c=get_course_by_name(iol,course)) == NULL)
		return E_INVAL;
	else if( iol->current_course && 
	         !strcmp(iol->current_course->code, c->code) )
		ret = E_OK;	
	else if( (burl=iol_get_url(iol, c->type == CT_COURSE ? 
	                           URL_CHANGE_COU : URL_CHANGE_DPT)) == NULL  )
		ret = E_MEMORY;
	else
	{	struct buff page = {NULL, 0};
		iol->current_course = c;

		s = g_strdup_printf(burl, course);
			
		if( iol->wait )
			sleep(5); 
		
		if( transfer_page(iol->curl, s, 0, &page) != E_OK )
			ret = E_NETWORK;
		else
			c->flags = get_course_capabilities(&page);

		free(page.data);
		g_free(s);
	}

	return	ret;
}

/**/


/* Download Material didactico section
 */

static int
link_is_iol_file(const char *url) 
{	const char *p, *q, *r;

	p = url;
	q = "download.asp";
	r = "newmaterialdid.asp";

	return (strncmp(p,q, strlen(q))==0) ? 1 : (strncmp(p,r,strlen(r))!=0) * 2;
}


static int
link_is_sort_link( const char *url)
{
	return !link_is_iol_file(url) && (strstr(url,"&ordenX="));
}

static int
is_father_folder( const char *child, const char *father)
{
	return strstr(father, child)!=NULL;
}

struct tmp 
{ 	queue_t pending;
	GSList  *files;
	char *prefix;
	char *url_prefix;
	iol_t iol;
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

static char *
my_url_escape(const char *url)
{	char *s;
	unsigned i=0, j,len=0;
	
	for(s=(char *)url; *s ; s++,len ++)
	{	if( *s == ' ' )
			i++;
	}

	s = malloc(len + i*2 + 100);
	if( s == NULL )
		return NULL;
	for( j=0; *url; url++ , j++)
		if( *url == ' ' )
		{	s[j++] = '%';
			s[j++] = '2';
			s[j]   = '0';
		}
		else
			s[j] = *url;
	s[j]=0;
	assert(j == len + i*2);

	return s;
}

/**
 * \returns true if the url contains and file id (download.asp scheme).
 * Also copy the fid to ::buf
 */
static int
url_has_fid(const char *url, char *buf, size_t size)
{	size_t i;
	char *p, *q;

	p = strchr(url,'?');
	if( p )
	{	/** \todo a url_crack function  */
		p++;
		p = strstr(p,"id=");
		p += sizeof("id=") -1;
		q = strchr(p,'&');
		if( q == NULL )
			for( q=p; *q; q++)
				;

		for( i=0 ; i < size - 1 && p != q && isdigit(p[i]) ; i++ )
			buf[i] = p[i];
		buf[i]=0;
	}

	return -(p!=q);
}

/*
 * #    #    ##    #####   #    #     #    #    #   ####
 * #    #   #  #   #    #  ##   #     #    ##   #  #    #
 * #    #  #    #  #    #  # #  #     #    # #  #  #
 * # ## #  ######  #####   #  # #     #    #  # #  #  ###
 * ##  ##  #    #  #   #   #   ##     #    #   ##  #    #
 * #    #  #    #  #    #  #    #     #    #    #   ####
 * Mon, 24 Mar 2003 14:21:36 -0300
 * this is a tempory ugly hack, until i get a javascript parser
 *
 * get the filename url from the redirect page (download.asp scheme)
 */
static char *
javascript_get_refresh_link( iol_t iol,  struct buff *page )
{	char *p, *q, *ret = NULL;

	/* como diria un amigo.....a lo cabeza!!! 
	 * niños, no intenten esto en sus casas
	 */
	page->data[page->size-1] = 0 ;
	p = strstr(page->data,"javascript:window.open"); 
	if( p )
	{ 	p += sizeof("javascript:window.open");
		p = strchr(p, '"' );
		if( p )
		{	p++;
			q = strchr(p, '"' );
			if( q )
			{	*q = 0;
				ret = g_strdup( iol_get_url(iol, p) );
			}
		}	
	}

	return ret;
}

/** 
 * returns the url for the (real) file that we need to download in the 
 * download.asp scheme.
 */
static char *
get_real_download_file( iol_t iol,  const char *url )
{	char buff[12]={0};
	char *file;
	char *ret = NULL;
	
	if( url_has_fid(url, buff, sizeof(buff)) == 0 )
		;
	else
	{	
		if( (file = cache_get_file(iol->fcache, buff)) )
			ret = file;
		else
		{	struct buff page = {NULL, 0}; 

			if( transfer_page(iol->curl, url, 0, &page) != E_OK)
				;
			else
			{ 	ret = javascript_get_refresh_link(iol, &page);
				if( ret )
					cache_add_file(iol->fcache, buff,ret);
				free(page.data);
			}
		}
	}

	return ret;
}

static int
link_is_rare_for_file( const char *link )
{
	return !strncmp(link, "./",  2) || !strncmp(link, "../", 3) ;
}

/**
 * callback called for every link found in `material didacticos` pages
 * creates the list of files to download
 */
static void
link_files_fnc( const unsigned char *link, 
                const unsigned char *comment, void *d ) 
{	struct  tmp *t = (struct tmp *)d;
	int bFile;
	char *s, *p, *q ;

	if( is_external_link(link) || is_javascript_link(link) )
		return ;

	if( link_is_rare_for_file(link) )
	{
		rs_log_warning("hum. the link is rare. skiping: `%s'", link);
		rs_log_warning("please update to the lastest version "
		               "or contact the author");
		return;
	}
	
	p = my_url_escape(link); 
	if( p == NULL )
		return;
	s = g_strdup(iol_get_url(t->iol, p) );
	free(p);
	if( s == NULL )
		return;

	bFile = link_is_iol_file(link);
	if( bFile )
	{
		q = NULL;
		if( bFile == 1 )	/* download.asp scheme */
		{	q = get_real_download_file(t->iol, s);
			if( q )
			{	if( t->url_prefix == NULL )
					t->url_prefix = my_path_get_dirname(
					   q+strlen(iol_get_url(t->iol,"")));
			}
		}
		else if( bFile == 2 )	/* direct link scheme */
		{	if( t->url_prefix == NULL )
				t->url_prefix = my_path_get_dirname(link);
			q = g_strdup_printf("%s/%s",URL_BASE,link);
		}
		else
			assert(0);

		if( q )
			t->files = g_slist_prepend(t->files, q);
		g_free(s);
	}
	else if( link_is_sort_link(link) )
		g_free(s);
	else
	{ 	if( is_father_folder(s,t->prefix) )
			g_free(s);
		else
			queue_enqueue(t->pending, s);
	}
}

/**
 * retrives all the url for the available files in the current course.
 * They are stored in the list ::l.
 */
static int
get_file_list_from_current(iol_t iol, GSList **l, char **url_prefix )
{	struct buff webpage; 
	int ret = E_OK;
	struct tmp t;
	char *url; 
	
	t.pending = queue_new(); 
	if( t.pending == NULL || (url=iol_get_url(iol, URL_MATERIAL))== NULL )
		return E_MEMORY;
	t.files = NULL;
	t.url_prefix = NULL;
	t.iol = iol;
	queue_enqueue(t.pending, g_strdup(url));

	while( !queue_is_empty(t.pending) && ret == E_OK )
	{	
		url = queue_dequeue(t.pending);
		t.prefix = url;
		webpage.data = NULL;
		webpage.size = 0;

		if(transfer_page(iol->curl, url, 0, &webpage)!=E_OK)
			ret = E_NETWORK;
		else
		{	link_parser_t parser;
			unsigned i;

			parser = link_parser_new();
			if( parser == NULL )
				return 0;
			
			link_parser_set_link_callback(parser,link_files_fnc,&t);
			for( i = 0 ; i< webpage.size && 
			   link_parser_process_char(parser,webpage.data[i])==0;
			   i++ )
			     ;
			link_parser_end(parser);    
			link_parser_destroy(parser);
		}
		*url_prefix = t.url_prefix;
		g_free(url);
		free(webpage.data);
	}

	/* in case something has failed, we clean the memory */
	while( !queue_is_empty(t.pending) )
		g_free( queue_dequeue(t.pending) );
	
	queue_free(t.pending); 
	*l = t.files;

	return ret;
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

static unsigned
get_tty_columns( void )
{	const char *env = getenv("COLUMNS");
	unsigned columns = 80;
	
	if( env )
	{	columns = atoi(env);
		if( columns == 0 )
			columns = 80;
	}

	return columns;
}

/** print some information of the url we are downloading to a log/tty */
static void
inform_url_and_date( FILE *fp, const char *url )
{	time_t now =  time(NULL);
	struct tm *tm = localtime(&now);
	static unsigned columns;
	unsigned n = 0, len = strlen(url), off=0;
	int bTTY = isatty(fileno(fp)) != 0;
	
	/* nice printing:
	 *   don't go ahead of $COLUMNS or 80 chars
	 */
	if( fp == NULL || url == NULL )
		return;

	if( columns == 0 )
		columns = get_tty_columns();

	fprintf(fp,"--");
	if( !bTTY )
		fprintf(fp, "%02d/%02d/%d ",
		        tm->tm_mday, tm->tm_mon+1, tm->tm_year + 1900);
		
	fprintf(fp, "%02d:%02d:%02d-- ", tm->tm_hour, tm->tm_min,tm->tm_sec);
	if( bTTY )
	{
		n = 2 + 2 + 2 + 2 + 4 + 1; 

		if( n + len + 1 > columns )
		{	off = len - (columns - 1 - n - 4);
			fprintf(fp, "... ");
		}
	}
	
	fprintf(fp, "%s\n",url + off);
}

/** tries to download ::file if it doesn't exist */
static void
foreach_getfile(const char *file, struct tmp_resync_getfile *d)
{	size_t len;
	char *local, *dirname, *download;
	const char *q, *unquote;;
	struct stat st;

	q = URL_BASE;
	if( file && d && d->url_prefix )
	{	size_t len_q = strlen(q);
		size_t len_url = strlen(d->url_prefix);

		/* server race happenend ? */
		if(strstr(file,d->iol->current_course->code) == NULL)
		{	rs_log_error(_("No se ha podido cambiar de materia. "
		                       "La teoria del autor es que existe una "
		                       "race en el servidor. Si bajase los "
		                       "archivos, estaría mezclando carpetas"));
		        return ;
		}
		
		assert(len_url>0);
		len =  len_q   + (q[len_q-1]!='/')  + 
		       len_url + (d->url_prefix[len_url - 1] != '/');
		assert( strlen(file) > len );

		/* IOL has files without escape 
		 *
		 *unquote = curl_unescape(file+len,0);
		 *if( !unquote )
		 *	return;
		 */
		unquote = file + len;
		local = g_strdup_printf("%s/%s", d->prefix, unquote);
		dirname = my_path_get_dirname(local);
		/*curl_free(unquote);*/

		if( stat(local,&st) == -1 )
		{	/** \todo 
		 	 * if a deeper directory  has been created
		 	 *  recently, don't try to create the dir
		 	 */
		 	errno = 0;
			if( mkrdir(dirname,0755) == 0 || errno == EEXIST)
			{	char *f;
				
				f = my_url_escape(file);
				inform_url_and_date(stdout, file);
				inform_url_and_date(d->iol->logfp, file);
				download = g_strdup_printf("%s/%s", dirname, 
				                          IOL_MATERIAL_TMPFILE);
				if( d->iol->dry )
					;
				else if( transfer_page(d->iol->curl, f,
				         TP_FILE, download) == 0 )
					rename(download,local);
				else
				{	remove(download);
					rs_log_error(_("downloading: %s"),
					 iol_get_network_error(d->iol));
				}

				g_free(download);
				free(f);
			}
		}
		
		free(dirname);
		g_free(local);
	}
}

static int
convertRepository( const char *from, const char *to)
{	char *f, *t;
	int ret;


	f = my_path_get_dirname(from);
	t = my_path_get_dirname(to);
	ret = rename(f, t);
	/** \todo detect EISDIR EEXIST ENOTDIR ? */

	g_free(f);
	g_free(t);
	
	return ret;
}

static int 
iol_resync_download(iol_t iol, const struct course *course)
{	int ret = E_OK;
	struct stat st;
	struct tmp_resync_getfile tmp;
	char *s, *r;

	s = g_strdup_printf("%s/%s/%s", iol->repository, course->code,
				  IOL_MATERIAL_FOLDER);
	r = g_strdup_printf("%s/%s/%s", iol->repository, course->name,
				  IOL_MATERIAL_FOLDER);

	if( iol->fancy )	/* convert repository ? */
	{	if( stat(s,&st)==0 && convertRepository(s,r) )
		{	rs_log_error(
			   _("error renaming repository. using old"));
			g_free(r);
		}
		else
		{	g_free(s);
			s = r;
		}
	}
	else 
	{ 	if( stat(r,&st)==0 && convertRepository(r,s) )
		{	rs_log_error(
			    _("error renaming repository. using old"));
			g_free(s);
			s = r;
		}
		else
			g_free(r);	
	}

	/* try to create course folder */ 
	if( create_course_directory(s) == -1 ) 
	{
		rs_log_error(_("error creating dir `%s'"),s);
		rs_log_error("%s",strerror(errno) ); ret = E_FS;
	}
	else
	{       GSList *files = NULL;

		tmp.prefix = s;
		tmp.iol = iol;
		tmp.url_prefix = NULL;
		
		ret=get_file_list_from_current(iol, &files, &(tmp.url_prefix));
		g_slist_foreach(files, (GFunc)foreach_getfile, &tmp);

		free(tmp.url_prefix);
	}

	g_free(s);

	return ret;
}


static int 
iol_resync_news(iol_t iol, const struct course *course)
{
	return E_INVAL;
}

static int 
iol_resync_forum(iol_t iol, const struct course *courses)
{	struct buff webpage = { NULL, 0};
	char *url = iol_get_url(iol, IOL_FORUM);
	int ret = E_OK;
	
	if(transfer_page(iol->curl,url,0,&webpage)!=E_OK)
		ret = E_NETWORK;
	else
	{	
		rs_log_info("forum: %d",forum_parse(webpage.data,webpage.size));

	}
	
	free(webpage.data);
	
	return ret;
}

int 
iol_resync(iol_t iol, const char *code, enum resync_flags flags)
{	
	int ret = E_OK;
	struct course *c;

	if ( !IS_IOL_T(iol)||iol->repository==NULL||!is_valid_course_code(code))
		ret = E_INVAL;
	else if( !iol->bLogged )
		ret = E_NLOGED;
	else if( (c=get_course_by_name(iol,code)) == NULL )
	{	ret = E_INVAL;
		rs_log_error(_("invalid course `%s'"), code);
	}
	else if( c->name == NULL )
	{	ret = E_INVAL;
		rs_log_error(_("course `%s' has no name! (TODO)"),code);
	}
	else if( rs_log_info(_("%s (%s)"), code, c->name) ,
	         iol_set_current_course(iol, code) != E_OK )  /* C hack */
	{ 	rs_log_error(_("setting current course to `%s'"),code);
		ret = E_NETWORK;
	} 
	else
	{	/* only do what the servers capabilities says */
		flags &= iol->current_course->flags;
		if( flags & IOL_RF_FILE && ret == E_OK )
			ret = iol_resync_download(iol, c);
		if( flags & IOL_RF_NEWS && ret == E_OK )
			ret = iol_resync_news(iol, c);
		if( flags & IOL_RF_FORUM && ret == E_OK )
			ret = iol_resync_forum(iol, c);
	}

	return ret;
} 

/**/

/** comunication between #iol_resync_all() and #foreach_resync() */ 
struct foreach_resync 
{	int ret;
	iol_t iol;
	unsigned flags;
};

static void
foreach_resync(struct course *course, struct foreach_resync *r) 
{
	if( course && r )
		r->ret |= iol_resync(r->iol, course->code, r->flags);
}

int
iol_resync_all(iol_t iol, enum resync_flags flags) 
{	struct foreach_resync r;

	if( !IS_IOL_T(iol) )
		return -1;

	r.ret = 0;
	r.iol = iol;
	r.flags = flags;
	
	g_slist_foreach(iol->courses,(GFunc)foreach_resync,&r);

	return r.ret;
}

const char * curl_strerror(CURLcode code);

const char *
iol_get_network_error(iol_t iol)
{	const char *ret = NULL;

	if( IS_IOL_T(iol) )
		ret = iol->errorbuf;
	
	return ret ;
}

/***/
static void
link_news_fnc( unsigned const char *link,
               unsigned const char *comment, void *d )
{	unsigned *i=d;

	(*i)++;
}

int
iol_get_new_novedades( iol_t iol, unsigned *n )
{ 	struct buff page = { NULL, 0};
	unsigned j = 0;
	int ret = 0;
	char *url;
	
	if( !IS_IOL_T(iol) || !n )
		ret = E_INVAL;
	else if( (url=iol_get_url(iol, IOL_NEWS))== NULL )
		ret = E_MEMORY;
	else
	{	if( transfer_page(iol->curl, url, 0, &page)!= E_OK )
	        	ret = E_NETWORK;
	        else
	        {	link_parser_t parser;
			unsigned i;

			parser = link_parser_new();
			if( parser == NULL )
			 	return 0;
			link_parser_set_link_callback(parser,link_news_fnc, &j);

			for( i = 0 ; i< page.size &&
			     link_parser_process_char(parser,page.data[i])==0;
			     i++ ) ;
			link_parser_end(parser);
			link_parser_destroy(parser);
			*n = j;
		}
		free(page.data);
	}

	return ret; 
}
