#include <gtk/gtk.h>

#include <i18n.h>

#define SIZE_X	320
#define SIZE_Y	320

static void 
hwndMain_quit( GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static gint
hwndMain_delete( GtkWidget *widget, GdkEvent  *event, gpointer   data )
{
        hwndMain_quit(widget,data);
        return TRUE;
}

void 
create_ui(void *data)
{ 	GtkWidget *hwnd;

	/* main window */
	hwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(hwnd), _("iolwizard"));
	gtk_container_set_border_width(GTK_CONTAINER(hwnd), 0);
	gtk_window_set_default_size( GTK_WINDOW(hwnd), SIZE_X,SIZE_Y );
	gtk_window_set_position( GTK_WINDOW(hwnd), GTK_WIN_POS_CENTER);


	/* signals */
	gtk_signal_connect (GTK_OBJECT (hwnd), "delete_event",
	                    GTK_SIGNAL_FUNC(hwndMain_delete), data);

 
	/* THE instruction */
        gtk_widget_show_all(hwnd);


}
