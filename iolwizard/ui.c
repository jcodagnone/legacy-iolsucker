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

#include <i18n.h>
#include <trace.h>

#include "../src/main.h"
#include "../src/config.h"
#include "dirbrowser.h"
#include "error_dlg.h"

#ifdef HAVE_CONFIG_H
  #ifdef WIN32
    #include "../configwin.h"
  #else
    #include <config.h>
  #endif
#endif

#define SIZE_X	320
#define SIZE_Y	320

static int exec_iolwizard(void);

struct tmp 
{ 	struct opt *opt;
	GtkWidget *hwnd, *frame_proxy;
	GtkWidget *edtUser, *edtPass, *edtRep;
	GtkWidget *edtHost, *spnPort, *edtPUser, *edtPPass;
	GtkWidget *chkDry;
	char *msg;
};

static void 
hwndMain_quit( GtkWidget *widget, struct tmp *tmp)
{
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

/*
 * BUTTONS
 */ 
static void 
sync_data(struct tmp *tmp)
{	char *s, *ss;
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

	tmp->opt->username[sizeof(tmp->opt->username)-1] = 0;
	tmp->opt->password[sizeof(tmp->opt->password)-1] = 0;
	tmp->opt->repository[sizeof(tmp->opt->repository)-1] = 0;

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
	ss  = gtk_entry_get_text(GTK_ENTRY(tmp->edtPPass));

	if( *s )
	{	free(tmp->opt->proxy_user);
		if( *ss )
		{	tmp->opt->proxy_user=malloc(strlen(s)+1+ strlen(ss) +1);
			if( tmp->opt->proxy_user )
				sprintf(tmp->opt->proxy_user,"%s:%s",s,ss);
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
resync_fnc( GtkWidget *widget, struct tmp *tmp )
{

	sync_data(tmp);
	if( access(tmp->opt->repository, W_OK|X_OK) == -1 )
		show_error(_("el directorio del repositorio no existe,\n"
		             "o usted no tiene suficientes permisos sobre el"));
	else if( tmp->opt->username[0]==0 || tmp->opt->password[0]==0 )
		show_error(
		_("el nombre de usuario o la password estan vacias"));
	else
	{
		sync_data(tmp);
	 	if( save_config_file(tmp->opt) == -1 )
			show_error(_("no se ha podido guardar esta nueva "
			             "informacion"));
		else
			exec_iolwizard();
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
	gtk_entry_set_text(GTK_ENTRY(tmp->edtHost),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtPUser),"");
	gtk_entry_set_text(GTK_ENTRY(tmp->edtPPass),"");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(tmp->spnPort),(double)1080);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp->chkDry), 0);

}


static void handler( gchar *g, struct tmp *tmp)
{

	
}

static void
repbrowse_fnc( GtkWidget *widget, struct tmp *tmp )
{ 	GtkWidget *browse;
	gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(tmp->edtRep));	
	browse = xmms_create_dir_browser(_("Select repository directory"),s,
	         GTK_SELECTION_SINGLE, (void *)handler, tmp);
	gtk_widget_show(browse);
}

/*  CHECK-BOXES
 */

static void
dryrun_fnc( GtkWidget *widget, struct tmp *tmp )
{
	tmp->opt->dry =
	gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp->chkDry));
}

/*
 * EDIT CONTROLS
 */
static void
edtUser_changed( GtkWidget *widget, struct tmp *tmp )
{	gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(widget));
	gtk_widget_set_sensitive(tmp->edtPass, ( s && *s ) );
	
}


static void
edtHost_changed( GtkWidget *widget, struct tmp *tmp )
{	gchar *s;

	s = gtk_entry_get_text(GTK_ENTRY(widget));
	gtk_widget_set_sensitive(tmp->spnPort, ( s && *s ) );
	gtk_widget_set_sensitive(tmp->edtPUser, ( s && *s ) );
	gtk_widget_set_sensitive(tmp->edtPPass, ( s && *s ) );
	
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
{ 	GtkWidget *edtUser, *edtPass, *edtRep, *btnRep;
	GtkWidget  *hbox, *label,  *vbox_label, *vbox_edit;;

	/* login frame */
	edtUser    = gtk_entry_new();
	edtPass    = gtk_entry_new();
	edtRep     = gtk_entry_new();
	hbox       = gtk_hbox_new(FALSE, 0);
	vbox_label = gtk_vbox_new(FALSE, 0);
	vbox_edit  = gtk_vbox_new(FALSE, 0);
	btnRep     = gtk_button_new_with_label(_("..."));
	gtk_entry_set_visibility(GTK_ENTRY(edtPass), 0);
	gtk_widget_set_sensitive(GTK_WIDGET(edtPass), 0);

	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(hbox));
	gtk_box_pack_start(GTK_BOX(hbox),vbox_label, FALSE,FALSE,2);
	gtk_box_pack_start(GTK_BOX(hbox),vbox_edit,  TRUE, TRUE, 2);

	label = gtk_label_new(_("User:"));
	gtk_box_pack_start(GTK_BOX(vbox_label), label, FALSE, FALSE, 2);
	label = gtk_label_new(_("Password:"));
	gtk_box_pack_start(GTK_BOX(vbox_label), label, FALSE, FALSE, 2);
	label = gtk_label_new(_("Repository:"));
	gtk_box_pack_start(GTK_BOX(vbox_label), label, FALSE, FALSE, 2);
	
	gtk_box_pack_start(GTK_BOX(vbox_edit),edtUser, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox_edit),edtPass, FALSE, FALSE, 2);
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox),edtRep, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox),btnRep, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_edit),hbox,  FALSE, FALSE, 2);

	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->username);
	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->password);
	gtk_entry_set_text(GTK_ENTRY(edtRep), tmp->opt->repository);

	/* signals */
	gtk_signal_connect(GTK_OBJECT(edtUser), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtUser), "changed", 
	                   GTK_SIGNAL_FUNC(edtUser_changed), tmp);
	gtk_signal_connect(GTK_OBJECT(btnRep),"clicked",
	                   GTK_SIGNAL_FUNC(repbrowse_fnc), tmp);
	/* tooltips */
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtUser, 
	                    _("numero de DNI"), NULL );
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtPass, 
	                    _("password de iol"), NULL );
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), edtRep,
	                    _("directorio donde guardar los archivos que se "
	                      "descargan"), NULL);
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), btnRep, 
	                    _("Buscar directorio"),NULL);

	/* save data */
	tmp->edtUser = edtUser;
	tmp->edtPass = edtPass;
	tmp->edtRep= edtRep;

	/* load data */
	gtk_entry_set_text(GTK_ENTRY(edtUser),tmp->opt->username);
	gtk_entry_set_text(GTK_ENTRY(edtPass),tmp->opt->password);
	gtk_entry_set_text(GTK_ENTRY(edtRep) ,tmp->opt->repository);
}

static void
create_ui_extra( struct tmp *tmp, GtkWidget *parent, GtkTooltips *tips)
{ 	GtkWidget *chkDry;
	GtkWidget *vbox;

	/* extra frame */
	vbox = gtk_vbox_new(FALSE, 0);
	chkDry   = gtk_check_button_new_with_label(_("Dry Run"));
	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(vbox));
	gtk_box_pack_start(GTK_BOX(vbox), chkDry,   FALSE, FALSE, 0);

	/* signals */
	gtk_signal_connect(GTK_OBJECT(chkDry),"toggled",
	                           GTK_SIGNAL_FUNC(dryrun_fnc), tmp);
	/* tooltips */
	gtk_tooltips_set_tip(GTK_TOOLTIPS(tips), chkDry,
	                    _("activar la ejecucion en seco: no se descarga "
	                      "ningun archivo"),NULL);
	/* save data */
	tmp->chkDry = chkDry;
	
	/* default values */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkDry), tmp->opt->dry);
}


static void
create_ui_proxy( struct tmp *tmp, GtkWidget *parent, GtkTooltips *tips)
{ 	GtkWidget *edtHost, *spnPort, *edtPUser, *edtPPass;
	GtkWidget *vbox, *hbox, *label;
	GtkObject *adj;

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(parent), GTK_WIDGET(vbox));

	adj      = gtk_adjustment_new(1080,1,0xffff,1,10,10); 
	edtHost  = gtk_entry_new();
	spnPort  = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.5, 0);
	edtPUser = gtk_entry_new();
	edtPPass = gtk_entry_new();
	gtk_entry_set_visibility(GTK_ENTRY(edtPPass), 0);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("Host:"));
	gtk_box_pack_start(GTK_BOX(hbox),label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox),edtHost, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(hbox),spnPort, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox, FALSE, FALSE, 2);

	hbox = gtk_hbox_new(FALSE, 0);
	label = gtk_label_new(_("User:"));
	gtk_box_pack_start(GTK_BOX(hbox),label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox),edtPUser, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox, FALSE, FALSE, 2);
	hbox = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new(_("Pass:"));
	gtk_box_pack_start(GTK_BOX(hbox),label, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox),edtPPass, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox),hbox, FALSE, FALSE, 2);

	/* signals */
	gtk_signal_connect(GTK_OBJECT(edtPUser), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtHost), "insert-text",
	                   GTK_SIGNAL_FUNC(entry_nospaces_insert), tmp);
	gtk_signal_connect(GTK_OBJECT(edtHost), "changed", 
	                   GTK_SIGNAL_FUNC(edtHost_changed), tmp);

	/* tooltips */

	/* save data */
	tmp->edtHost = edtHost;
	tmp->spnPort = spnPort;
	tmp->edtPUser = edtPUser; 
	tmp->edtPPass = edtPPass;

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
	gtk_window_set_title (GTK_WINDOW(hwnd), _("iolwizard"));
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

	ret = ShellExecute(NULL,"open", "iolsucker", "", NULL, SW_SHOW);
	
	return ret < 31 ? -1 : 0;
}
#else
static int
exec_iolwizard(void)
{	
	return -1;
}
#endif
