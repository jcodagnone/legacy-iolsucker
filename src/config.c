/*
 *  config.cpp -- config file loader
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
/**
 * \todo XSD or DTD!
 */
 
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#ifdef  HAVE_FCNTL_H 
  #include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
  #include <unistd.h>
#endif

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include <strdup.h>
#include <trace.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main.h"
#include "i18n.h"
#include "win32.h"

#define IOL_RC ".iolrc"

#define NELEMS(q)	(sizeof(q)/sizeof(*(q)))

enum config_type
{ 	CT_SZ,
	CT_FLAG
};

struct config
{	const unsigned char *key;
	void *data;
	size_t size;	/* CT_SZ: size of string */
	enum config_type type;
};

static const struct config *
config_find_entry( const unsigned char *name, const struct config *table,
                   size_t n)
{	const struct config *ret = NULL;
	size_t i;

	for( i=0 ; i< n && !ret; i++)
	{	if( !strcmp( (char *)name, (char *)table[i].key) )
			ret = table + i;
	}
	
	return ret;
}

static void
config_load_table( xmlDocPtr doc, xmlNodePtr cur,
                   const struct config *table, size_t n)
{ 	const struct config *current;
	xmlChar *key;
	unsigned char *t;
	char **s;
	int *p;

	for( cur = cur->xmlChildrenNode ; cur ; cur = cur->next )
	{
		current = config_find_entry(cur->name, table, n);
		key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
		if( current && key )
		{	if( current->type == CT_SZ )
			{	
				if( current->size ) 
				{ 	t = current->data;
					strncpy(t, key, current->size);
					t[current->size-1] = 0;
				}
				else
				{	s = current->data;
					*s = strdup(key);
				}
			}
			else if( current->type == CT_FLAG )
			{	p = current->data;
				*p = 1;
			}
			else 
				assert(0);
		}
	 	xmlFree(key);
	}
}
                       
static void
parse_login(xmlDocPtr doc, xmlNodePtr cur, struct opt *opt)
{	struct config login[]=
	{	{  "user", &(opt->username), sizeof(opt->username), CT_SZ },
		{  "pass", &(opt->password), sizeof(opt->password), CT_SZ },
		{  "rep",  &(opt->repository), sizeof(opt->repository), CT_SZ },
		{  "fancy",&(opt->fancy), 0, CT_FLAG },
		{  "wait", &(opt->wait),  0, CT_FLAG },
		{  "host", &(opt->server),  0, CT_SZ },
	};

	config_load_table(doc, cur, login, NELEMS(login));
}

static void
parse_proxy(xmlDocPtr doc, xmlNodePtr cur, struct opt *opt)
{	char *ptype = NULL;
	struct config proxy[]=
	{	{ "host", &(opt->proxy),      0, CT_SZ },
		{ "user", &(opt->proxy_user), 0, CT_SZ },
		{ "type", &(ptype),           0, CT_SZ },
	};

	config_load_table(doc, cur, proxy, NELEMS(proxy));

	if( ptype )
	{ 	if( !strcmp(ptype, "socks5") )
			opt->proxy_type ="socks5";
		else if( !strcmp(ptype, "http" ) )
			opt->proxy_type = "http";
		else
			opt->proxy_type = "";
	}
}

static int
parse_config(const char *docname, struct opt *opt)
{ 	xmlDocPtr doc;
	xmlNodePtr cur;
	
	doc = xmlParseFile(docname);

	if (doc == NULL ) 
	{ 	rs_log_warning(_("document not parsed successfully."));
		return -1;
	}
	
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) 
	{ 	rs_log_warning(_("empty document\n"));
		xmlFreeDoc(doc);
		return -1;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "iolsucker")) 
	{ 	rs_log_warning(_("root node != iolsucker"));
		xmlFreeDoc(doc);
		return -1;
	}

	
	for( cur = cur->xmlChildrenNode; cur != NULL ;  cur = cur->next)
	{ 	if ((!xmlStrcmp(cur->name, (const xmlChar *)"login")))
			parse_login (doc, cur, opt);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"proxy")))
			parse_proxy (doc, cur, opt);
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
			;
		else
			rs_log_warning(_("unknown tag: `%s'"),cur->name);

	}

	xmlFreeDoc(doc);
	return 0;
}

int
load_config_file(struct opt *opt)
{	int ret = 0;
	
	if( ! opt->configfile[0] )
	{	char *home;
		struct stat buff;
		
		home = getenv("HOME");
		if( home == NULL )
			return -1;

		g_snprintf(opt->configfile, sizeof(opt->configfile), "%s/%s",
		          home,IOL_RC);
		opt->configfile[sizeof(opt->configfile)-1]=0;
		if( stat(opt->configfile, &buff) == -1||!S_ISREG(buff.st_mode))
			;
		else if( buff.st_mode & 0077 )
			rs_log_warning(
			    _("file `%s' has dangerous permition 0%o"),
			    opt->configfile, buff.st_mode & 0777);
	}


	if( opt->configfile[0] && parse_config(opt->configfile,opt) )
		ret = -1;
	return ret;
}

static int
dump_line(const char *line, int fd)
{	int r = 0;
	size_t n;
	
	if( line )
	{ 	n = strlen(line);
		r = write(fd, line, n);
		if( r !=  n )
		{	rs_log_error("saving configuration file: %s",
			             strerror(errno));
			r = -1;
		}
		else
			r = 0;
	}

	return r;
}

/* why do i use POSIX I/O instead of ANSI I/O? because is the only safe way 
 * to set mode_t to a file we are creating.
 *
 * this function sucks in style, but is pretty dumb
 */
int
save_config_file( const struct opt *opt)
{ 	char *s, *buf;
	int fd, err;

	if( ! opt->configfile[0] )
		return -1;

	s = g_strdup_printf("%s_",opt->configfile);
	fd = open(s, O_WRONLY|O_CREAT|O_TRUNC, 0600);
	if( fd == -1 )
	{	rs_log_error("creating `%s': %s", s, strerror(errno) );
		g_free(s);
		return  -1;
	}

	(err = dump_line("<?xml version=\"1.0\"?>\n", fd) )||
	(err = dump_line("<iolsucker>\n", fd)) ||
	(err = dump_line("\t<login>\n", fd));

	if( !err &&  opt->username[0] )
	{	buf = g_strdup_printf("\t\t<user>%s</user>\n", opt->username);
		err = dump_line(buf, fd);
		g_free(buf);
	}
	if( !err && opt->password[0] )
	{	buf = g_strdup_printf("\t\t<pass>%s</pass>\n",opt->password);
		err = dump_line(buf, fd);
		g_free(buf);
	}
	if( !err && opt->repository[0] )
	{	buf = g_strdup_printf("\t\t<rep>%s</rep>\n", opt->repository);
		err = dump_line(buf, fd);
		g_free(buf);
	}
	if( !err && opt->fancy )
		err = dump_line("\t\t<fancy></fancy>\n", fd);
	if( !err && opt->forum )
		err = dump_line("\t\t<forum></forum>\n", fd);
	if( !err && opt->verbose )
		err = dump_line("\t\t<verbose></verbose>\n", fd);
	if( !err && opt->wait )
		err = dump_line("\t\t<wait></wait>\n", fd);
	if( !err && opt->server )
	{	buf = g_strdup_printf("\t\t<host>%s</host>\n", opt->server);
		err = dump_line(buf, fd);
		g_free(buf);
	}
	if( !err )
		err = dump_line("\t</login>\n", fd);
	if( !err )
		err = dump_line("\t<proxy>\n",  fd);
        if( !err && opt->proxy_type[0] )
        {	buf = g_strdup_printf("\t\t<type>%s</type>\n",opt->proxy_type);
        	err = dump_line(buf, fd);
        	g_free(buf);
        }
        if( !err && opt->proxy ) 
        {	buf = g_strdup_printf("\t\t<host>%s</host>\n",opt->proxy);
        	err = dump_line(buf, fd);
        	g_free(buf);
        }
        
        if( !err &&  opt->proxy_user ) 
        {	buf = g_strdup_printf("\t\t<user>%s</user>\n",opt->proxy_user);
        	err = dump_line(buf, fd);
        	g_free(buf);
        }

        if( !err )
        	err = dump_line("\t</proxy>\n", fd);
        if( !err )
		err = dump_line("</iolsucker>\n", fd);

	if( close(fd) == -1 )
		remove(s);
	else
		rename(s, opt->configfile);

	g_free(s);
	return 0;
}
