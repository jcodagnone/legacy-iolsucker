/*
 * ui.c -- ui creation for iolwizard
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Yes! you will see the function and say: `WTF! this is horrible!' and i will
 * answer you: `yeah, but is linear. give me some TNT!'
 */

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifdef HAVE_UNISTD_H
	#include <unistd.h>
#else
	#include <unix.h>
#endif

#include <libmisc/i18n.h>
#include <libmisc/trace.h>

#include "../src/main.h"
#include "../src/fconfig.h"
#include "error_dlg.h"

#define SIZE_X	320
#define SIZE_Y	320

#define STYPE_HTTP	"http"
#define STYPE_SOCK5	"socks5"

#define NELEMS(a) (sizeof(a)/sizeof(*(a)))

static int exec_iolwizard(void);

struct tmp 
{ 	struct opt *opt;
	GtkWidget *hwnd, *frame_proxy;
	GtkWidget *edtUser, *edtPass, *edtRep, *edtSHost;
	GtkWidget *edtHost, *spnPort, *edtPUser, *edtPPass, *cmbType;
	GtkWidget *chkDry, *chkFancy, *chkForum, *chkVerbose, *chkWait, 
	          *chkXenofobe, *chkNoCache, *chkAsk;
	char *msg;
};

static void 
sync_data(struct tmp *tmp)
{	G_CONST_RETURN char *s;
	G_CONST_RETURN char *p;
	int l;

	strncpy(tmp->opt->username,
	        gtk_entry_get_text(GTK_ENTRY(tmp->edtUser)), 
	        sizeof(tmp->opt->username));
	strncpy(tmp->opt->password,
	        gtk_entry_get_text(GTK_ENTRY(tmp->edtPass)),
	        sizeof(tmp->opt->password));
	strncpy(tmp->opt->repository,
	        gtk_entry_get_text(GTK_ENTRY(tmp->edtRep)),
	        sizeof(tmp->opt->repository));
	s=gtk_entry_get_text(GTK_ENTRY(tmp->edtSHost));
	tmp->opt->server = s && *s ? strdup(s) : NULL;

	tmp->opt->username[sizeof(tmp->opt->username)-1] = 0;
	tmp->opt->password[sizeof(tmp->opt->password)-1] = 0;
	tmp->opt->repository[sizeof(tmp->opt->repository)-1] = 0;

	s = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(tmp->cmbType)->entry));
	if( !strcmp(s, STYPE_HTTP) )
		tmp->opt->proxy_type = STYPE_HTTP;
	else if (!strcmp(s, STYPE_SOCK5) )
		tmp->opt->proxy_type = STYPE_SOCK5;
	else 
	{	tmp->opt->proxy_type = "";
		/* if the user typed something invalid we clean his shit */
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(tmp->cmbType)->entry),"");
	}

	s = gtk_entry_get_text(GTK_ENTRY(tmp->edtHost));
	if( *s )
	{	l = gtk_spin_button_get_value_as_int(
		                         GTK_SPIN_BUTTON(tmp->spnPort));
	
		free(tmp->opt->proxy);
		tmp->opt->proxy = malloc(strlen(s) + 1 + 7 + 1 );
		if( tmp->opt->proxy )
		{	sprintf(tmp->opt->proxy,"%s:%d",s,l);
		}
	}
	else
	{	free(tmp->opt->proxy);
		tmp->opt->proxy =NULL;
	}
		
	
	s   = gtk_entry_get_text(GTK_ENTRY(tmp->edtPUser));
	p  = gtk_entry_get_text(GTK_ENTRY(tmp->edtPPass));

	if( *s )
	{	free(tmp->opt->proxy_user);
		if( *p )
		{	tmp->opt->proxy_user=malloc(strlen(s)+1+ strlen(p) +1);
			if( tmp->opt->proxy_user )
				sprintf(tmp->opt->proxy_user,"%s:%s", s, p);
		}
		else
			tmp->opt->proxy_user = strdup(s);		
	}
	else
	{	free(tmp->opt->proxy_user);
		tmp->opt->proxy_user = NULL;
	}
} 

static void 
hwndMain_quit( GtkWidget *widget, struct tmp *tmp)
{
	sync_data(tmp);
	if( save_config_file(tmp->opt) == -1 )
		show_error(_("no se ha podido guardar esta nueva "
		             "informacion"));
	g_free(tmp->msg);
	g_free(tmp);
	gtk_main_quit();
}

static gint
hwndMain_delete( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
        hwndMain_quit(widget,data);
        
        return TRUE;
}

static void 
resync_fnc( GtkWidget *widget, struct tmp *tmp )
{

	sync_data(tmp);
	if( access(tmp->opt->repository, W_OK|X_OK) == -1 )
		show_error(_("el directorio del repositorio no existe,\n"
		             "o usted no tiene suficientes permisos sobre el"));
	else if( tmp->opt->username[0]==0 )
		show_error( _("el nombre de usuario es nulo!"));
	else
	{
		sync_data(tmp);
	 	if( save_config_file(tmp->opt) == -1 )
			show_error(_("no se ha podido guardar esta nueva "
			             "informacion"));
		else if( exec_iolwizard() == -1 )
			show_error(_("no se pudo ejecutar el iolsucker"));
	}
}

static void
clear_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->username[0]=0;
	memset(tmp->opt->password,0,sizeof(tmp->opt->password));
	free(tmp->opt->proxy);
	free(tmp->opt->proxy_user);
	tmp->opt->proxy = tmp->opt->proxy_user = NULL;
	tmp->opt->dry = 0;
	if( save_config_file(tmp->opt) == -1 )
		show_error(_("no se ha podido guardar esta nueva informacion"));

	gtk_entry_set_text(GTK_ENTRY(tmp->edtUser),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtPass),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtRep),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtSHost),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtHost),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtPUser),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtPPass),"");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmp->spnPort),(double)1080);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkDry), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkFancy), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkForum), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkVerbose), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkWait), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkXenofobe), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkNoCache), 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkAsk), 0);

}

struct  repbrowse
{	GtkWidget *fs;
	struct tmp *tmp;	
};


static void
repbrowse_cancel( GtkWidget *widget, struct repbrowse *rb)
{
	gtk_widget_destroy(GTK_WIDGET(rb->fs));
	free(rb);
}


static void
repbrowse_ok( GtkWidget *widget, struct repbrowse *rb)
{	G_CONST_RETURN char *s;

	s = gtk_file_selection_get_filename(GTK_FILE_SELECTION(rb->fs));
	gtk_entry_set_text(GTK_ENTRY(rb->tmp->edtRep), s);
	repbrowse_cancel(widget, rb);
}

static void
repbrowse_fnc( GtkWidget *widget, struct tmp *tmp )
{ 	GtkWidget *browse;
	struct repbrowse *rb;
	G_CONST_RETURN gchar *s;

	rb = malloc(sizeof(*rb));
	if( rb == NULL )
		return ;
		
	
	browse = gtk_file_selection_new(_("Seleccione donde se almacenan los "
	                                  "archivos"));
	rb->fs = browse;
	rb->tmp = tmp;
	
	s = gtk_entry_get_text(GTK_ENTRY(tmp->edtRep));	
	gtk_file_selection_set_filename(GTK_FILE_SELECTION(browse),s);

        gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(rb->fs)->ok_button),
                           "clicked", GTK_SIGNAL_FUNC(repbrowse_ok), rb);
        gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(rb->fs)->cancel_button)
                           ,"clicked",GTK_SIGNAL_FUNC(repbrowse_cancel), rb);

	gtk_widget_show(browse);
	gtk_file_selection_get_filename(GTK_FILE_SELECTION(browse));
}

/* COMBOS
 */
static void
changed_combo_fnc( GtkCombo *widget, struct tmp *tmp)
{	G_CONST_RETURN gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(tmp->cmbType)->entry));

	if( !strcmp(s,STYPE_HTTP) )
		tmp->opt->proxy_type = STYPE_HTTP;
	else if( !strcmp(s,STYPE_SOCK5) )
		tmp->opt->proxy_type = STYPE_SOCK5;
	else if( *s == 0 )
		tmp->opt->proxy_type = "";
}

/*  CHECK-BOXES
 */

static void
dryrun_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->dry =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkDry));
}

static void
fancy_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->fancy = 
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkFancy));
}

static void
forum_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->forum =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkForum));
}

static void
verbose_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->verbose =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkVerbose));
}

static void
wait_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->wait =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkWait));
}

static void
xenofobe_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->xenofobe =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkXenofobe));
}

static void
no_cache_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->no_cache =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkNoCache));
}

static void
ask_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->ask = 
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkAsk));
}



/*
 * EDIT CONTROLS
 */
static void
edtUser_changed( GtkWidget *widget, struct tmp *tmp )
{	G_CONST_RETURN gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(widget));
	gtk_widget_set_sensitive(tmp->edtPass, ( s && *s ) );
	
}


static void
edtHost_changed( GtkWidget *widget, struct tmp *tmp )
{	G_CONST_RETURN gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(widget));
	gtk_widget_set_sensitive(tmp->spnPort, ( s && *s ) );
	gtk_widget_set_sensitive(tmp->edtPUser, ( s && *s ) );
	gtk_widget_set_sensitive(tmp->edtPPass, ( s && *s ) );
	gtk_widget_set_sensitive(tmp->cmbType, ( s && *s ) );
}

/* dont allow spaces inputs
 */
static void
entry_nospaces_insert(GtkEditable *editable, const gchar *text, gint length,
                      gint *position, gpointer data)
{	
	gint i, j;
	gchar *result = g_new (gchar, length); 

	for (i=j=0; i<length; i++)
	{	if( !isspace(text[i]) )
			result[j++] = text[i];
	}
		

	gtk_signal_handler_block_by_func (GTK_OBJECT (editable),
	                         GTK_SIGNAL_FUNC (entry_nospaces_insert), data);
	gtk_editable_insert_text (editable, result, j, position);
	gtk_signal_handler_unblock_by_func (GTK_OBJECT (editable), 
	                         GTK_SIGNAL_FUNC (entry_nospaces_insert), data);
	gtk_signal_emit_stop_by_name (GTK_OBJECT (editable), "insert_text");
	g_free (result);
}


/* UI creation */

static void
create_ui_login( struct tmp *tmp, GtkWidget *parent, GtkTooltips *tips)
{ 	GtkWidget *edtUser, *edtPass, *edtRep, *btnRep, *edtSHost;
	GtkWidget *table, *hbox;
	GtkWidget *label;
	
	/* login frame */
	edtUser    = gtk_entry_new();
	edtPass    = gtk_entry_new();
	edtRep     = gtk_entry_new();
	btnRep     = gtk_button_new_with_label(_("..."));
	edtSHost    = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(edtPass), 0);
	gtk_widget_set_sensitive(GTK_WIDGET(edtPass), 0);
	
	table = gtk_table_new (4, 2, FALSE);
	hbox  = gtk_hbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(table));
	
	label = gtk_label_new(_("User:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	label = gtk_label_new(_("Password:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	label = gtk_label_new(_("Repository:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	label = gtk_label_new(_("Host:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,3,4,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	
	gtk_box_pack_start(GTK_BOX(hbox), edtRep, TRUE,  TRUE,  0);
	gtk_box_pack_start(GTK_BOX(hbox), btnRep, FALSE, FALSE, 0);

	gtk_table_attach(GTK_TABLE(table), edtUser, 1, 2, 0, 1,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), edtPass, 1, 2, 1, 2,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 2, 3,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), edtSHost, 1, 2, 3, 4,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->username);
	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->password);
	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->repository);
	if( tmp->opt->server )
		gtk_entry_set_text(GTK_ENTRY(edtSHost), tmp->opt->server);

	/* signals */
	gtk_signal_connect(GTK_OBJECT(edtUser), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtUser), "changed", 
	                   GTK_SIGNAL_FUNC(edtUser_changed), tmp);
	gtk_signal_connect(GTK_OBJECT(btnRep),"clicked",
	                   GTK_SIGNAL_FUNC(repbrowse_fnc), tmp);
	                   
	/* tooltips */
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtUser, 
	                    _("(*) numero de DNI"), NULL );
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtPass, 
	                    _("password de iol. NO ES NECESARIO QUE LA INGRESE"
	                      ". si no la ingresa el programa se la "
	                      "preguntara."), NULL );
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtRep,
	                    _("(*) directorio donde guardar los archivos que "
	                      "se descargan"), NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), btnRep, 
	                    _("Buscar directorio"),NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtSHost, 
	                    _("Especifica donde hay que conectarse para "
	                      "acceder a IOL. Ej: silvestre.itba.edu.ar.\n"
	                      "La opcion sirve para redirigir los pedidos por "
	                      "un proxy de SSH. Ej. localhost:8080"),NULL);
	/* save data */
	tmp->edtUser = edtUser;
	tmp->edtPass = edtPass;
	tmp->edtRep  = edtRep;
	tmp->edtSHost= edtSHost;

	/* load data */
	gtk_entry_set_text(GTK_ENTRY(edtUser),tmp->opt->username);
	gtk_entry_set_text(GTK_ENTRY(edtPass),tmp->opt->password);
	gtk_entry_set_text(GTK_ENTRY(edtRep) ,tmp->opt->repository);
	gtk_entry_set_text(GTK_ENTRY(edtSHost),tmp->opt->server ?
	                                              tmp->opt->server:"");
}

static void
create_ui_extra( struct tmp *tmp, GtkWidget *parent, GtkTooltips *tips)
{ 	GtkWidget *table;
	struct {
		GtkWidget  **widget;
		const char *name;

		/* for gtk_table_attach  */
		guint left_attach;
		guint right_attach;
		guint top_attach;
		guint bottom_attach;

		void *fnc;     /* for connect() */
		int enabled;
		const char *tooltip;

	} chkbox[] = 
	/* this isn't ANSI, but hell, it saves me from lots of bugs */
	{ 	{ &tmp->chkDry, "Dry Run", 0,1,0,1, (void *) dryrun_fnc, 
		  tmp->opt->dry, 
		  "activar la ejecucion en seco: no se descarga ningun archivo"
		},
		{ &tmp->chkFancy, "Fancy Names",   1,2,0,1, (void *) fancy_fnc,
                  tmp->opt->fancy, 
		  "usar el nombre de la materia, como nombre de directorio"
		},
		{ &tmp->chkForum, "Resync Foros",  2,3,0,1, (void *)forum_fnc,
		  tmp->opt->forum,
		  "resincronizar los foros? por ahora solo advierte de cambios"
		  " en los foros. (no baja los mensajes)"
		},
		{ &tmp->chkVerbose,  "Verbose", 0,1,1,2, (void *) verbose_fnc,
                  tmp->opt->verbose,
		  "imprime informacion extra sobre la conexion. util cuando"
		  " las cosas no funcionan" 
		},
		{ &tmp->chkWait,     "Wait", 1,2,1,2, (void *) wait_fnc,
		  tmp->opt->wait,
		  "esperar unos segundos en el cambio de contexo de materias. "
		  "Util cuando el programa imprime el mensaje de que el server"
		  " tiene problemas con los contextos"
		},
		{ &tmp->chkXenofobe, "Xenofobo", 2,3,1,2, (void *)xenofobe_fnc,
		  tmp->opt->xenofobe,
		  "Modo xenofobia: al terminar, lista los archivos que se "
		  "encuentran en nuestro repositorio y que no existen en iol"
		},
		{&tmp->chkNoCache,"No usar Caches",0,1,2,3,(void *)no_cache_fnc,
		  tmp->opt->no_cache, "No utilizar ningun cache"
		},
		{&tmp->chkAsk, "Confirmar Transferencias",2,3,2,3,
		(void *)ask_fnc, tmp->opt->ask, "Le consulta al usuario si "
		"desea descargar los la pagina web"
		}

	};
	unsigned i;
	table = gtk_table_new (4, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(table));
	for( i = 0 ; i < NELEMS(chkbox) ; i ++ )
	{	*(chkbox[i].widget)=gtk_check_button_new_with_label(chkbox[i].name); 
		gtk_table_attach(GTK_TABLE(table),*(chkbox[i].widget),
		                                  chkbox[i].left_attach,
		                                  chkbox[i].right_attach,
		                                  chkbox[i].top_attach,
		                                  chkbox[i].bottom_attach,
		                                  GTK_FILL, 0, 4, 0); 
		gtk_signal_connect(GTK_OBJECT(*(chkbox[i].widget)),"toggled",
	                           GTK_SIGNAL_FUNC(chkbox[i].fnc), tmp);

		gtk_toggle_button_set_active(
		                   GTK_TOGGLE_BUTTON(*(chkbox[i].widget)), 
		                   chkbox[i].enabled);
		gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), *(chkbox[i].widget),
		                   chkbox[i].tooltip, NULL );
	}
}


static void
fill_proxy_type_combo( GtkCombo *combo )
{	GList *l = NULL;

	l = g_list_append(l, "");
	l = g_list_append(l, STYPE_HTTP);
	l = g_list_append(l, STYPE_SOCK5);

	 gtk_combo_set_popdown_strings(combo, l);
	 g_list_free(l);
}

static void
create_ui_proxy( struct tmp *tmp, GtkWidget *parent, GtkTooltips *tips)
{ 	GtkWidget *edtHost, *spnPort, *edtPUser, *edtPPass, *cmbType;
	GtkWidget *hbox, *label, *table;
	GtkObject *adj;
	
	adj      = gtk_adjustment_new(1080,1,0xffff,1,10,10); 
	edtHost  = gtk_entry_new();
	spnPort  = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.5, 0);
	edtPUser = gtk_entry_new();
	edtPPass = gtk_entry_new();
	cmbType  = gtk_combo_new();
	gtk_entry_set_visibility(GTK_ENTRY(edtPPass), 0);
	fill_proxy_type_combo(GTK_COMBO(cmbType));

	table = gtk_table_new (4, 2, FALSE);
	hbox  = gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(table));
	
	label = gtk_label_new(_("Host:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	label = gtk_label_new(_("Type:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	label = gtk_label_new(_("User:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	label = gtk_label_new(_("Pass:"));
	gtk_table_attach(GTK_TABLE(table),label,0,1,3,4,GTK_FILL, 0, 4,0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	gtk_box_pack_start(GTK_BOX(hbox),edtHost, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox),spnPort, TRUE, TRUE, 0);
	gtk_table_attach(GTK_TABLE(table), hbox, 1, 2, 0, 1,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), cmbType, 1, 2, 1, 2,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), edtPUser, 1, 2, 2, 3,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	gtk_table_attach(GTK_TABLE(table), edtPPass, 1, 2, 3, 4,
	                 (GTK_EXPAND | GTK_FILL), 0, 4, 0);
	


	/* signals */
	gtk_signal_connect(GTK_OBJECT(edtPUser), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtHost), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtHost), "changed", 
	                   GTK_SIGNAL_FUNC(edtHost_changed), tmp);
	gtk_signal_connect(GTK_OBJECT(GTK_COMBO(cmbType)->entry),"changed", 
	                   GTK_SIGNAL_FUNC(changed_combo_fnc),tmp);
	                   
	/* tooltips */
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtHost,
	    _("ip o nombre de la maquina donde se encuentra el proxy"), NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), cmbType,
	    _("tipo de proxy: http|socks5"), NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtPUser,
	    _("usuario para autenticarse con el proxy"), NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtPPass,
	    _("password del usuario..."), NULL);

	/* save data */
	tmp->edtHost = edtHost;
	tmp->spnPort = spnPort;
	tmp->edtPUser = edtPUser; 
	tmp->edtPPass = edtPPass;
	tmp->cmbType = cmbType;
	
	/* default values */
	if( tmp->opt->proxy) 
	{	char *s;
	
		s = strrchr(tmp->opt->proxy,':');
		if( s )
		{	int port;
		
			*s = 0;
			gtk_entry_set_text(GTK_ENTRY(edtHost), tmp->opt->proxy);
			port = atoi(s+1);
			if( port != 0 )
				gtk_spin_button_set_value(
				         GTK_SPIN_BUTTON(spnPort),(gfloat)port);
			*s = ':';
		}
		else
			gtk_entry_set_text(GTK_ENTRY(edtHost), tmp->opt->proxy);

	}
	else
		gtk_entry_set_text(GTK_ENTRY(edtHost),"");

	if( !strcmp(tmp->opt->proxy_type, STYPE_HTTP ) )
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(cmbType)->entry),
		                   STYPE_HTTP);
	else if( !strcmp(tmp->opt->proxy_type, STYPE_SOCK5) )
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(cmbType)->entry),
		                   STYPE_SOCK5);
	else
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(cmbType)->entry), "");
		

	if( tmp->opt->proxy_user )
	{	char *s;
		s = strrchr(tmp->opt->proxy_user,':');
		if( s )
		{
			*s = 0;
			gtk_entry_set_text(GTK_ENTRY(edtPUser),
			                   tmp->opt->proxy_user);
			gtk_entry_set_text(GTK_ENTRY(edtPPass),s+1);
			*s = ':';
		}
		else
			gtk_entry_set_text(GTK_ENTRY(edtPUser),
			                   tmp->opt->proxy_user);

	}
}

void 
create_ui(struct opt *opt)
{ 	GtkWidget *frame_login, *frame_proxy, *frame_extra;
	GtkWidget *resync, *clear;
	GtkTooltips *tips;
	GtkWidget *hwnd;
	
	
	struct tmp *tmp = g_malloc(sizeof(*tmp));

	if( tmp == NULL )	/* can't happen, only for purists */
		return;
	memset(tmp,0,sizeof(*tmp));
	tmp->opt = opt;
	
	/* main window */
	hwnd = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(hwnd), _("iolwizard " VERSION));
	gtk_container_set_border_width(GTK_CONTAINER(hwnd), 0);
	gtk_window_set_default_size( GTK_WINDOW(hwnd), SIZE_X,SIZE_Y );
	gtk_window_set_position( GTK_WINDOW(hwnd), GTK_WIN_POS_CENTER);
	tips = gtk_tooltips_new();
	
	/* containers */
	frame_login  = gtk_frame_new("Login");
	frame_extra  = gtk_frame_new("Extra");
	tmp->frame_proxy = frame_proxy  = gtk_frame_new("Proxy");

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(hwnd)->vbox), frame_login,0,0,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(hwnd)->vbox), frame_extra,0,0,0);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(hwnd)->vbox),frame_proxy);

	/* frames */
	create_ui_login(tmp,frame_login, tips);
	create_ui_proxy(tmp,frame_proxy, tips);
	create_ui_extra(tmp,frame_extra, tips);
	

	/* buttons */
	clear = gtk_button_new_with_label(_("clear")); 
	resync = gtk_button_new_with_label(_("resync"));
	
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(hwnd)->action_area),clear);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(hwnd)->action_area),resync);
	
	/* signals */
	gtk_signal_connect (GTK_OBJECT (hwnd), "delete_event",
	                    GTK_SIGNAL_FUNC(hwndMain_delete), tmp);
	                    
	gtk_signal_connect(GTK_OBJECT(resync),"clicked",
	                   GTK_SIGNAL_FUNC(resync_fnc), tmp);
	gtk_signal_connect(GTK_OBJECT(clear),"clicked",
	                   GTK_SIGNAL_FUNC(clear_fnc), tmp);
	

	/* copy usefull data */
	tmp->hwnd = hwnd;
	tmp->frame_proxy = frame_proxy;


	/* THE instruction */
        gtk_widget_show_all(hwnd);


}

#ifdef WIN32
#include <windows.h>
static int
exec_iolwizard(void)
{	HINSTANCE ret;

	ret = ShellExecute(NULL,"open", "iolunch", "iolsucker", NULL, SW_SHOW);
	
	return ret < 31 ? -1 : 0;
}
#else
static int
exec_iolwizard(void)
{	pid_t pid;

	pid = fork();
	if( pid == -1 )
		return -1;
	else if( pid == 0 )
	{	system("xterm -e /bin/bash -c 'iolsucker; read -p \"press ENTER>\"'");
		exit(0);
	}
		
	return 0;
}
#endif
