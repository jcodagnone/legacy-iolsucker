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
		if( stat(opt->configfile, &buff) == -1 )
			opt->configfile[0]=0;
	}


	if( opt->configfile[0] && parse_config(opt->configfile,opt) )
		ret = -1;
	return ret;
}

int
save_config_file( const struct opt *opt)
{ 	FILE *fp;
	char *s;

	
	if( ! opt->configfile[0] )
		return -1;

	s = g_strdup_printf("%s_",opt->configfile);
	fp = fopen(s, "wb");
	if( fp == NULL )
	{	g_free(s);
		return  -1;
	}
	
	fprintf(fp,"<?xml version=\"1.0\"?>\n" );
	fprintf(fp,"<iolsucker>\n");
	fprintf(fp,"\t<login>\n");

	if( opt->username[0] )
		fprintf(fp,"\t\t<user>%s</user>\n",opt->username);
	if( opt->password[0] )
		fprintf(fp,"\t\t<pass>%s</pass>\n",opt->password);
	if( opt->repository[0] )
		fprintf(fp,"\t\t<rep>%s</rep>\n", opt->repository);
	if( opt->fancy )
		fprintf(fp,"\t\t<fancy></fancy>\n");
	if( opt->forum )
		fprintf(fp,"\t\t<forum></forum>\n");
	if( opt->verbose )
		fprintf(fp,"\t\t<verbose></verbose>\n");
	if( opt->wait )
		fprintf(fp,"\t\t<wait></wait>\n");
	if( opt->server )
		fprintf(fp,"\t\t<host>%s</host>\n",opt->server);
        fprintf(fp,"\t</login>\n");
        fprintf(fp,"\t<proxy>\n");
        if( opt->proxy_type[0] )
        	fprintf(fp,"\t\t<type>%s</type>\n",opt->proxy_type);
        if( opt->proxy ) 
        	fprintf(fp,"\t\t<host>%s</host>\n",opt->proxy);
        if( opt->proxy_user ) 
        	fprintf(fp,"\t\t<user>%s</user>\n",opt->proxy_user);
	fprintf(fp,"\t</proxy>\n");
	fprintf(fp,"</iolsucker>\n");

	if( fclose(fp) == -1 ) 	/* BTW, this can't happen in linux AFAIK */ 
		remove(s);
	else
		rename(s, opt->configfile);

	g_free(s);
	return 0;
}
