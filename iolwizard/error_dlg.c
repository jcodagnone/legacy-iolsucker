/*
 * error_dlg.c -- 
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
#include <gtk/gtk.h>

#include <libmisc/i18n.h>

#include "error_dlg.h"

struct tmp
{
	GtkWidget *hwnd;
};


static void
ok_fnc( GtkWidget *widget, struct tmp *tmp)
{	
	gtk_widget_destroy(tmp->hwnd);
}


static gint
delete_fnc( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
	ok_fnc(widget,data);
	return FALSE;
}

void
show_error(const char *message)
{	GtkWidget *hwnd;
	GtkWidget *ok;
	GtkWidget *hbox;
	GtkWidget *label;
	struct tmp *tmp;

	tmp = g_malloc(sizeof(*tmp));

	hwnd = gtk_dialog_new();
	gtk_window_set_title (GTK_WINDOW(hwnd), _("Error"));
	gtk_container_set_border_width (GTK_CONTAINER(hwnd), 0);
	gtk_window_set_position( GTK_WINDOW(hwnd), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(hwnd),TRUE);

	ok = gtk_button_new_with_label(_("Ok"));
	hbox = gtk_hbox_new(FALSE, 10);

	label = gtk_label_new(message);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(hwnd)->vbox),hbox);
	GTK_WIDGET_SET_FLAGS(label,GTK_CAN_FOCUS);
	
	tmp->hwnd = hwnd;

	gtk_container_add(GTK_CONTAINER(hbox),label);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(hwnd)->action_area),ok);

        gtk_signal_connect(GTK_OBJECT(ok),"clicked",
                           (GtkSignalFunc) ok_fnc,
                           tmp);
        gtk_signal_connect(GTK_OBJECT(hwnd),"delete_event",
                           (GtkSignalFunc) delete_fnc,
                           tmp);

	gtk_widget_show_all(hwnd);
}
