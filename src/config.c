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
/*
 * TODO: XSD or DTD!
 */
#ifdef HAVE_CONFIG_H
  #ifdef WIN32
    #include "../configwin.h"
  #else
    #include "../config.h"
  #endif
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <glib.h>

#include <trace.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "main.h"
#include "i18n.h"
#include "win32.h"

#define IOL_RC ".iolrc"

static void
parse_login(xmlDocPtr doc, xmlNodePtr cur, struct opt *opt)
{ 	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) 
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"user"))) 
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			if( opt->username[0] == 0 && key )
			{  strncpy(opt->username,key,sizeof(opt->username));
				opt->username[sizeof(opt->username)-1] = 0;
	 		}
	 		xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pass")))
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1); 
			if( key )
			{ strncpy(opt->password,key,sizeof(opt->password));
				opt->password[sizeof(opt->password) -1 ] = 0;
			}
			xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rep")))
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
			if( opt->repository[0]==0 &&  key )
			{  strncpy(opt->repository,key,sizeof(opt->repository));
				opt->repository[sizeof(opt->repository)-1] = 0;
			}
			xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"fancy")))
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
			opt->fancy = 1;
			xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"forum")))
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
			opt->forum = 1;
			xmlFree(key);
		}
		cur = cur->next;
	}

	return;
}

static void
parse_proxy (xmlDocPtr doc, xmlNodePtr cur, struct opt *opt)
{ 	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) 
	{ 	if ((!xmlStrcmp(cur->name, (const xmlChar *)"host"))) 
		{	key = xmlNodeListGetString(doc,cur->xmlChildrenNode, 1);
			if( key )
				opt->proxy = strdup(key);
	 		xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"user")))
		{ 	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1); 
			if( key )
				opt->proxy_user = strdup(key);
			xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"type")))
		{	key = xmlNodeListGetString(doc,cur->xmlChildrenNode,1);
			if( key )
			{	if( !strcmp(key, "socks5") )
					opt->proxy_type ="socks5";
				else if( !strcmp(key, "http" ) )
					opt->proxy_type = "http";
				else
					opt->proxy_type = "";
			}
			xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
			;
		else
			rs_log_warning(_("unknown tag: `%s'"),cur->name);

		cur = cur->next;
	}
	
	return;
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

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"login")))
			parse_login (doc, cur, opt);
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"proxy")))
			parse_proxy (doc, cur, opt);
		else if((!xmlStrcmp(cur->name, (const xmlChar *)"text")))
			;
		else
			rs_log_warning(_("unknown tag: `%s'"),cur->name);

		cur = cur->next;
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
