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

#include <libmisc/trace.h>
#include <libmisc/strdup.h>
#include <libmisc/basename.h>
#include <libmisc/mkrdir.h>
#include <libmisc/dirname.h>
#include <libmisc/queue.h>
#include <libmisc/ftw_.h>
#include <libmisc/i18n.h>

#include "iol.h"
#include "link.h"
#include "progress.h"
#include "forum.h"
#include "stringset.h"
#include "common.h"
#include "cache.h"
#include "course.h"

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
#define IOL_FORUM  	"/foroDis.asp"
#define IOL_NEWS	"/novlistall.asp"
#define IOL_SHOWFILE	"/showfile.asp?fiid=%s&volvera="
#define URL_LOGIN_ARG	"txtdni=%s&txtpwd=%s&Submit=Conectar&cmd=login"

#define IOL_MATERIAL_FOLDER	"material"
#define IOL_MATERIAL_TMPFILE	".download.tmp"
#define IOL_FILE_DB        	"files.db"
#define IOLSUCKER_LOGFILE	"cambios.txt"

struct dump 
{	char *path;      /**< directory where dumps are dumped (duh!) */
	unsigned index;  /**< index -> unique file */
};

/**
 * Concrete data type for the IOL object */
struct iolCDT
{ 	CURL *curl;		/**< curl handler */
	char errorbuf[CURL_ERROR_SIZE+1];	/**< CURLOPT_ERRORBUFFER */
	double transfered_bytes;/**< bytes downloaded */
	course_t courses;
	const struct course *current_course;

	int bLogged;    	/**< already logged in ? */ 
	char *repository;	/**< repository directory */
	const char *host; 	/**< host part in the URL */

	int dry;		/**< dry run ? */
	int verbose;		/**< print lots of information? */
	int fancy;		/**< use fancy names */
	int wait;		/**< seconds to wait when changing context */
	int xenofobe;           /**< show forein files (the ones that aren't 
	                             from iol's repository) */
	int no_cache;           /**< use cache feature? */
	struct dump dump;

	FILE *logfp;		/**< logfile filepointer */

	char url_tmp[256];
	char *url_tmp_pt;
	
	stringset_t repfiles;

	cache_t fid_cache;      /**< file id's cache (for material didactico */
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

struct dump_transfer
{	struct buff *mem;
	FILE *realfp;
	FILE *dumpfp;
};

static size_t
write_data_to_memory_and_dump(void *ptr, size_t size, size_t nmemb, void *data)
{	struct dump_transfer *d = data;
	size_t ret;
	
	ret = write_data_to_memory(ptr, size, nmemb, d->mem);
	write_data_to_file(ptr, size, nmemb, d->dumpfp);

	return ret;
}

static size_t
write_data_to_file_and_dump(void *ptr, size_t size, size_t nmemb, void *data)
{	struct dump_transfer *d = data;
	size_t ret;

	ret = write_data_to_file(ptr, size, nmemb, d->realfp);
	write_data_to_file(ptr, size, nmemb, d->dumpfp);

	return ret;
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
transfer_page( CURL *curl, const char *url, unsigned flags, void *data, 
               struct dump *dump)
{	struct dump_transfer dt = { 0 };
	struct progress *progress = NULL;
	void *fnc = 0, *fncdata = 0;
	CURLcode res; 
	
	if( curl == NULL || url == 0 || url[0]==0)
		return E_INVAL;

	if( dump->path )
	{	char *file = g_strdup_printf("%s/%03u_%s", dump->path, 
		                             dump->index++, url);
		char *s = file + strlen(dump->path) + 1;
		
		if( mkrdir(dump->path, 0700) == -1 && errno != EEXIST )
			rs_log_warning("creating dump dir `%s'", dump->path );

		for( ; *s ; s++ )
			if( *s == '/' )
				*s = '_';
				
		dt.dumpfp = fopen(file, "wb");
		if( dt.dumpfp == NULL )
			rs_log_warning("can't dump url `%s' to `%s' (ignoring)",
			                url, file);

		g_free(file);
	}
	
	if( flags & TP_FILE )
	{	void *ptr;
		
		dt.realfp  = fopen(data, "wb");	
		if( dt.realfp == NULL )
			return E_INVAL;

		if( dt.dumpfp )
		{ 	fnc  = write_data_to_file_and_dump;
			fncdata = &dt;
		}
		else
		{ 	fnc = write_data_to_file;
			fncdata = dt.realfp;
		}
		
		if( isatty(fileno(stdout)) )
			ptr = bar_progress_callback;
		else
			ptr = dot_progress_callback;
		
		progress = new_progress_callback(ptr);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ptr);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress);
	}
	else
	{ 	if( dt.dumpfp )
		{	fnc     = write_data_to_memory_and_dump;
			dt.mem = data;
			fncdata = &dt;
		}
		else
		{	fnc     = write_data_to_memory;
			fncdata = data;
		}
		
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, TRUE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,     NULL);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fncdata);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fnc);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	count_bytes(curl);

	if( dt.realfp )
		fclose(dt.realfp);
	if( dt.dumpfp)
		fclose(dt.dumpfp);

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
        
        #ifdef CURLVERSION_NOW
	curl_version_info_data *version;
        #endif

	cdt = malloc( sizeof(*cdt) );
	if( cdt == NULL )
		return NULL;
		
	memset( cdt, 0, sizeof(*cdt));
	cdt->repfiles = stringset_new();
	cdt->courses = course_new();
	if( !stringset_is_valid(cdt->repfiles)|| !course_is_valid(cdt->courses))
	{	
		course_destroy(cdt->courses);
		free(cdt);
		return NULL;
	}
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

	#ifdef CURLVERSION_NOW
	version = curl_version_info(CURLVERSION_NOW);
	if( version->version_num == 0x070a03 )
		/* avoid a bug when using libcurl-7.0.3 */
		curl_easy_setopt(cdt->curl,CURLOPT_FORBID_REUSE, 1L);
	#endif

	return cdt;
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

	cache_destroy(iol->fid_cache);
	show_curl_stats(iol->curl);
	curl_easy_cleanup(iol->curl); 
	curl_global_cleanup();

	course_destroy(iol->courses);

	free(iol->repository);
	free(iol->url_tmp_pt);
	stringset_destroy(iol->repfiles);

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
		else if( mkrdir(cdt->repository, 0755) != 0 && errno!=EEXIST)
			ret = E_FS;
		else
		{
			if( cdt->logfp )
			{	fclose(cdt->logfp);
				cache_destroy(cdt->fid_cache);
				cdt->fid_cache = 0;
			}
			s = g_strdup_printf("%s/%s", cdt->repository,
			                    IOLSUCKER_LOGFILE);
			if( !s || (cdt->logfp = fopen(s,"a")) == NULL)
			{	rs_log_warning(_("could not open log file"));
				rs_log_warning("%s:%s", s, strerror(errno));
			}
			g_free(s); 
			s=g_strdup_printf("%s/%s",cdt->repository, IOL_FILE_DB);
			if( s == 0 || !cache_is_valid((cdt->fid_cache =
			            cache_new(s)))  )
				rs_log_warning(_("opening fid cache"));
			g_free(s);
		}
	}

	return ret;
}

static int 
iol_set_proxy_type(iol_t cdt, const char *type)
{	int ret = E_OK;
#ifdef HAVE_PROXYTYPE
	if( !strcmp(type, "http" ) )
		curl_easy_setopt(cdt->curl,CURLOPT_PROXYTYPE,CURLPROXY_HTTP);
	else if( !strcmp(type, "socks5") )
		curl_easy_setopt(cdt->curl,CURLOPT_PROXYTYPE,CURLPROXY_SOCKS5);
        else if( *type == 0 )
		;
	else
		ret = E_INVAL;
#else
	if( type && *type == 0)
		ret = E_OK;
	else
		ret = E_INVAL;
#endif
	return ret;
	
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

static int
iol_set_xenofobe(iol_t cdt, int *xenofobe)
{	int ret = E_OK;

	if( xenofobe )
		cdt->xenofobe = *xenofobe != 0;
	else
		ret = E_INVAL;
		
	return ret;
}

static int
iol_set_no_cache(iol_t cdt, int *no_cache)
{	int ret = E_OK;

	if( no_cache )
		cdt->no_cache = *no_cache != 0;
	else
		ret = E_INVAL;
		
	return ret;
}

static int
iol_set_dump(iol_t cdt, const char *dump)
{	int ret = E_OK;
	
	if( dump )
	{	cdt->dump.path  = strdup(dump);
		cdt->dump.index = 0;
		if( cdt->dump.path == NULL )
			ret = E_MEMORY;
	}
	else
		ret = E_INVAL;
		
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
		{	IOL_HOST,       (iol_set_fnc) iol_set_host       },
		{	IOL_XENOFOBE,   (iol_set_fnc) iol_set_xenofobe   },
		{	IOL_NO_CACHE,   (iol_set_fnc) iol_set_no_cache   },
		{	IOL_DUMP,       (iol_set_fnc) iol_set_dump       }
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
		
		/* yummm!!! cookiessss */ 
		if( transfer_page(iol->curl, url, 0, NULL, &iol->dump)!= E_OK )
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
			
			if(transfer_page(iol->curl,url,0,&buf,&iol->dump)!=E_OK)
				nRet = E_NETWORK;
			else if( course_load_from_page(iol->courses,&buf) == 0)
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
	{	transfer_page(iol->curl, url, 0, NULL, &iol->dump);
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
{	struct course *c;
	int ret = E_OK;
	char *s, *burl;
	
	if( !IS_IOL_T(iol) )
		ret = E_INVAL;
	else if( (c=course_get_by_name(iol->courses, course)) == NULL)
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
		
		if( transfer_page(iol->curl, s, 0, &page, &iol->dump) != E_OK )
			ret = E_NETWORK;
		else
			c->flags = course_get_capabilities_from_page(&page);

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
{	const char *p, *q;

	p = url;
	q = "newmaterialdid.asp";

	return (strncmp(p,q,strlen(q))!=0);
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
	return  strstr(link,"://") !=NULL;
}

static int
is_localhost_link( const char *host, const char *link)
{	char buff[1024];
	
	sprintf(buff,"http://%s/", host);
	
	return !strncmp(buff, link, strlen(buff)); 
}

static int
is_javascript_link( const char *link)
{	
	return !strncmp(link,"javascript:",sizeof("javascript:"));
}

/**
 * escape only spaces.
 *
 *  \returns NULL if there is nothing to escape
 */
static char *
my_url_escape(const char *url)
{	char *s;
	unsigned i=0, j,len=0;
	
	for( s=(char *)url; *s ; s++,len ++) /* how many spaces we have? */
	{	if( *s == ' ' )
			i++;
	}

	if( i == 0 )
		return NULL;

	s = g_malloc(len + i*2 + 100);
	if( s == NULL )
		return NULL;
	for( j=0; *url; url++ , j++)
	{	if( *url == ' ' )
		{	s[j++] = '%';
			s[j++] = '2';
			s[j]   = '0';
		}
		else
			s[j] = *url;
	}
	s[j]=0;
	assert(j == len + i*2);

	return s;
}

static int
link_is_rare_for_file( const char *link )
{
	return !strncmp(link, "./",  2) || !strncmp(link, "../", 3) ;
}

static int
link_is_special_download(const char *link)
{
	return link[0]=='#' && link[1]=='\0';
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
	char *s, *q ;

	if( (is_external_link(link) &&!is_localhost_link(t->iol->host, link))|| 
	    is_javascript_link(link) || link_is_special_download(link) )
		return ;

	if( link_is_rare_for_file(link) )
	{
		rs_log_warning("hum. the link is rare. skiping: `%s'", link);
		rs_log_warning("please update to the lastest version "
		               "or contact the author");
		return;
	}
	
	if( !is_localhost_link(t->iol->host, link) )
		s = g_strdup(iol_get_url(t->iol, link) );
	else
		s = strdup(link);

	if( s == NULL )
		return;

	bFile = link_is_iol_file(link);
	if( bFile )
	{
		q = NULL;
		if( bFile )	/* direct link scheme */
			q = s;
		else
			assert(0);

		if( q )
			t->files = g_slist_prepend(t->files, q);
		if( q != s ) 
			g_free(s);
	}
	else if( link_is_sort_link(link) )
		g_free(s);
	else if( is_father_folder(s,t->prefix) )
		g_free(s);
	else
		queue_enqueue(t->pending, s);
}

/** 
 * Retrives the real url for a iol's file id
 *
 * \returns NULL on error
 */
static char *
_iol_get_url_from_fid(iol_t iol, unsigned long fid, const char *fid_sz)
{	char * url, *s, *p, tmp[64];
	char *ret = 0;
	char needle[]="\"fileviewer\" SRC=\"";
	
	 if( !iol->no_cache && (s=cache_get_file(iol->fid_cache, fid_sz)) )
	 	ret = g_strdup(s);
	 else
	 {
	 	struct buff buf = {NULL, 0};
		snprintf(tmp, sizeof(tmp), IOL_SHOWFILE, fid_sz);
		tmp[sizeof(tmp)-1]=0;
		url = iol_get_url(iol, tmp);
		if( transfer_page(iol->curl, url, 0, &buf, &iol->dump) == E_OK )
		{
			if( buf.size && buf.data  )
			{	buf.data[buf.size - 1] = 0; /* hack */
				
				/* la pagina que descargamos, contiene
				 * dos frames. 
				 *    1. contiene la descripcion 
				 *       del archivo y un form con acciones
				 *       (imprimir, marcar como leido, volver);
				 *    2. contiene la direccion del archivo
				 *       El frame se llama fileviewer
				 * 
				 *  Solo nos interesa encontrar el segundo
				 */
				 s = strstr(buf.data, needle);
				 if( s )
				 {	s += sizeof(needle) - 1;
				 	p = strchr(s, '"');
					if( p )
						*p = '\0';

					ret = g_strdup(s);
					cache_add_file(iol->fid_cache,fid_sz,s);
				 }
				 else
				 	rs_log_warning("_iol_get_url_from_fid: "
					 "hmmm...no encontré el url hacia el "
					 "archivo en `%s'. Contactese con el "
					 "mantenedor del programa");
					 
			}
			free(buf.data);
		}
	}

	return ret;
}

/**
 * Second pass. 
 * Try to get the file_id's and theirs url
 *
 */
static void
link_files_fnc2( iol_t iol,  const char *webpage, GSList **_files )
{	const char needle[] = "javascript:BajaArch(", *s;
	char *err, *url;
	unsigned i;
	unsigned long fid, last_fid=0;
	char fid_sz[128];
	size_t nbuff = sizeof(fid_sz);
	GSList *files = *_files;
	
	for( s=webpage ;  (s=strstr(s, needle)) ; )
	{
		s += sizeof(needle) - 1;
		for( i=0 ; isdigit(*s) && i<nbuff ; s++)
			fid_sz[i++]=*s;
		fid_sz[i]=0;
		err = 0;
		fid = strtoul(fid_sz, &err, 10);
		if( *err )
			; /* error. fid is not an int */
		else
		{	/* hack: en general, en la pagina aparecen 2 veces
			 * seguidos los fids. No duele procesar los dos
			 * ya que _iol_get_url_from_fid va a tener un cache,
			 * y que la url se va a descargar una sola vez
			 * dado que el archivo ya va existir
			 */
			if( fid != last_fid )
			{	url = _iol_get_url_from_fid(iol, fid, fid_sz);
				if( url )
					files = g_slist_prepend(files, url);

				last_fid = fid;
			}
		}
	}
	*_files = files;
}

/**
 * retrives all the url for the available files in the current course.
 * They are stored in the list ::l.
 */
static int
get_file_list_from_current(iol_t iol, GSList **l)
{	struct buff webpage; 
	int ret = E_OK;
	struct tmp t;
	char *url, *eurl; 
	
	t.pending = queue_new(); 
	if( t.pending == NULL || (url=iol_get_url(iol, URL_MATERIAL))== NULL )
		return E_MEMORY;
	t.files = NULL;
	t.iol = iol;

	queue_enqueue(t.pending, g_strdup(url));

	while( !queue_is_empty(t.pending) && ret == E_OK )
	{	
		url  = queue_dequeue(t.pending);
		eurl = my_url_escape( url );
		eurl = eurl ? eurl : url;
		
		t.prefix = url;

		webpage.data = NULL;
		webpage.size = 0;

		if(transfer_page(iol->curl, eurl, 0, &webpage,&iol->dump)!=E_OK)
			ret = E_NETWORK;
		else
		{	link_parser_t parser;
			unsigned i;
		                        
			parser = link_parser_new();
			if( parser == NULL )
				return 0;
			
			/* Algoritmo de dos pasadas:
			 *
			 * la primer pasada es con la maquina de estados
			 * finitos que parsea links. Antes los downloads
			 * estaban como links. Ahora, están como javascript.
			 * La segunda pasada busca por javascripts, y resuelve
			 * los id de los archivos a urls
			 */
			link_parser_set_link_callback(parser,link_files_fnc,&t);
			for( i = 0 ; i< webpage.size && 
			   link_parser_process_char(parser,webpage.data[i])==0;
			   i++ )
			     ;
			link_parser_end(parser);    
			link_parser_destroy(parser);
			
			/* printf("After: %s\n", eurl);
			 * queue_print_debug(t.pending);
			 */

			/* segunda pasada */
			webpage.data[webpage.size-1]=0; 
			link_files_fnc2(iol, webpage.data, &(t.files) );
		}
		if( url != eurl )
			g_free(eurl);
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
		columns = determine_screen_width();

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

/** try to download ::file if it doesn't exist in the repository.
 *
 * ::file can contain spaces.
 */
static void
foreach_getfile(const char *file, struct tmp_resync_getfile *d)
{	size_t len;
	char *local, *dirname, *download;
	const char  *unquote;
	struct stat st;
	
	if( file && d )
	{
		/* server race happenend ? */
		if(strstr(file,d->iol->current_course->code) == NULL)
		{	rs_log_error(_("No se ha podido cambiar de materia. "
		                       "La teoria del autor es que existe una "
		                       "race en el servidor. Si bajase los "
		                       "archivos, estaría mezclando carpetas"));
		        return ;
		}
		
		len = strlen(d->url_prefix);
		if( *(file+len) == '/' )
			len++;
		unquote = file + len;
		local = g_strdup_printf("%s/%s", d->prefix, unquote);
		dirname = my_path_get_dirname(local);

		if( d->iol->xenofobe ) 
			stringset_remove(d->iol->repfiles, local);
		if( stat(local,&st) == -1 )
		{	/** \todo 
		 	 * if a deeper directory  has been created
		 	 *  recently, don't try to create the dir
		 	 */
		 	errno = 0;
			if( mkrdir(dirname,0755) == 0 || errno == EEXIST)
			{	const char *f;
				
				f = my_url_escape(file);
				f = f ? f : file;
				inform_url_and_date(stdout, file);
				inform_url_and_date(d->iol->logfp, file);
				download = g_strdup_printf("%s/%s", dirname, 
				                          IOL_MATERIAL_TMPFILE);
				if( d->iol->dry )
					;
				else if( transfer_page(d->iol->curl, f,
				                       TP_FILE, download,
						       &d->iol->dump) == 0 )
					rename(download,local);
				else
				{	remove(download);
					rs_log_error(_("downloading: %s"),
					         iol_get_network_error(d->iol));
				}

				g_free(download);
				if( f != file )
					g_free((char *)f);
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

		ret=get_file_list_from_current(iol, &files );

		tmp.url_prefix = get_common_startpath(files);
		if( tmp.url_prefix ) 
		{ 	g_slist_foreach(files, (GFunc)foreach_getfile, &tmp);
			g_free(tmp.url_prefix);
		}
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
	
	if(transfer_page(iol->curl,url,0,&webpage, &iol->dump)!=E_OK)
		ret = E_NETWORK;
	else
	{	
		rs_log_info("forum: %d",forum_parse(webpage.data,webpage.size));

	}
	
	free(webpage.data);
	
	return ret;
}

static int
_iol_fill_xenofobe_fnc(const char *file, struct stat *st, iol_t iol)
{
	stringset_add(iol->repfiles, file);
	return 0;
}

int 
iol_resync(iol_t iol, const char *code, enum resync_flags flags)
{	
	int ret = E_OK;
	struct course *c;

	if ( !IS_IOL_T(iol)||iol->repository==NULL||!course_name_is_valid(code))
		ret = E_INVAL;
	else if( !iol->bLogged )
		ret = E_NLOGED;
	else if( (c=course_get_by_name(iol->courses,code)) == NULL )
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
	{	
		if( iol->xenofobe )
		{	char *r;

			if( iol->fancy )
				r = g_strdup_printf("%s/%s/%s", iol->repository,
				             c->name, IOL_MATERIAL_FOLDER);
			else
				r = g_strdup_printf("%s/%s/%s", iol->repository,
			 	             c->code, IOL_MATERIAL_FOLDER);
			ftw_(r,(void *) _iol_fill_xenofobe_fnc, iol);
			g_free(r);
		}
		/* only do what the servers capabilities says */
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
	if( course && r && r->ret == E_OK )
		r->ret = iol_resync(r->iol, course->code, r->flags);
}

int
iol_resync_all(iol_t iol, enum resync_flags flags) 
{	struct foreach_resync r;

	if( !IS_IOL_T(iol) )
		return -1;

	r.ret = E_OK;
	r.iol = iol;
	r.flags = flags;
	
	course_foreach_run(iol->courses, foreach_resync, &r);

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
	{	if( transfer_page(iol->curl, url, 0, &page,&iol->dump)!= E_OK )
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

void 
iol_traverse_xenofobe_list( iol_t iol, int (*fn)(const char *file, void *data),
 void *data)
{

	if( IS_IOL_T(iol) && iol->xenofobe )
	{
		stringset_list(iol->repfiles, fn, data);
	}
}


#include <libxml/xmlversion.h>
char *
iol_version(char *buf, unsigned nbuf )
{	char *curl;
	char db[512];
	curl = curl_version();

	snprintf(buf, nbuf, "%s\n"
	                    "glib/%d.%d.%d\n"
			    "xmlib/%s\n"
			    "%s" ,
			    curl, GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, 
			    GLIB_MICRO_VERSION, LIBXML_DOTTED_VERSION,
			    cache_version(db,sizeof(db)));

	buf[nbuf-1] = 0;
	
	return buf;
}
