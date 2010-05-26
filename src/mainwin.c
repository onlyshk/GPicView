/***************************************************************************
 *   Copyright (C) 2007 by PCMan (Hong Jen Yee)  pcman.tw@gmail.com        *
 *                 2010 by shk (Kuleshov Alexander kuleshovmail@gmail.com  *  
 * 																		   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "mainwin.h"

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

/* For drag & drop */
static GtkTargetEntry drop_targets[] =
{
    {"text/uri-list", 0, 0},
    {"text/plain", 0, 1}
};

/* main window initialize and finalize*/
static void main_win_init(MainWin *mw);
static void main_win_finalize(GObject* obj);

// Begin of GObject-related stuff
G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW)

void main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;

    obj_class = ( GObjectClass * ) klass;
    
	//    obj_class->set_property = _set_property;
    //   obj_class->get_property = _get_property;
    
	obj_class->finalize = main_win_finalize;

    widget_class = GTK_WIDGET_CLASS ( klass );
    
	//widget_class->delete_event = on_delete_event;
    // widget_class->size_allocate = on_size_allocate;
    // widget_class->key_press_event = on_key_press_event;
    // widget_class->window_state_event = on_win_state_event;
}

void main_win_finalize( GObject* obj )
{
    gtk_main_quit();
}

GtkWidget* main_win_new()
{
    return (GtkWidget*)g_object_new (MAIN_WIN_TYPE, NULL);
}

// End of GObject-related stuff
void main_win_init( MainWin*mw )
{
	// main window color
	GdkColor color;
	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	
    gtk_window_set_title( (GtkWindow*)mw, "Image Viewer");
    gtk_window_set_default_size( (GtkWindow*)mw, 640, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);
	gtk_widget_modify_bg((GtkWindow*)mw, GTK_STATE_NORMAL, &color);
}

gboolean main_win_open( MainWin* mw, const char* file_path, ZoomMode zoom )
{
	GtkWidget* box = gtk_vbox_new( FALSE, 0 );
	gtk_container_add((GtkContainer*)mw, box);
	
    GError* err = NULL;
    GdkPixbufFormat* info;
	
    info = gdk_pixbuf_get_file_info( file_path, NULL, NULL );
    char* type = ((info != NULL) ? gdk_pixbuf_format_get_name(info) : "");
	
    mw->view = gtk_image_view_new ();
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (file_path, NULL);
    gtk_image_view_set_pixbuf(GTK_IMAGE_VIEW(mw->view), pixbuf, TRUE);
    mw->scroll = gtk_image_scroll_win_new(GTK_IMAGE_VIEW(mw->view));
	gtk_box_pack_start(GTK_BOX(box), mw->scroll, TRUE, TRUE,0);
	
    gtk_widget_show_all( box );
}

// error window
void main_win_show_error( MainWin* mw, const char* message )
{
    GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "%s", message );
    gtk_dialog_run( (GtkDialog*)dlg );
    gtk_widget_destroy( dlg );
}

