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

gchar *ui_info =
      "<ui>"
        "<toolbar name = 'ToolBar'>"
           "<toolitem action='Go Back'/>"
           "<toolitem  action='Go Forward'/>"
             "<separator action='Sep1'/>"
           "<toolitem  action='Zoom out'/>"
           "<toolitem  action='Zoom in'/>"
        "</toolbar>"
      "</ui>";

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

void 
main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;

    obj_class = ( GObjectClass * ) klass;
    
	//obj_class->set_property = _set_property;
    //obj_class->get_property = _get_property;
    
	obj_class->finalize = main_win_finalize;

    widget_class = GTK_WIDGET_CLASS ( klass );
    
	//widget_class->delete_event = on_delete_event;
    // widget_class->size_allocate = on_size_allocate;
    // widget_class->key_press_event = on_key_press_event;
    // widget_class->window_state_event = on_win_state_event;
}

void
main_win_finalize( GObject* obj )
{
    gtk_main_quit();
}

GtkWidget* 
main_win_new()
{
    return (GtkWidget*)g_object_new (MAIN_WIN_TYPE, NULL);
}

// End of GObject-related stuff
void
main_win_init( MainWin*mw )
{
	GError *error = NULL;
	
    gtk_window_set_title( (GtkWindow*)mw, "Image Viewer");
    gtk_window_set_default_size( (GtkWindow*)mw, 640, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);
	
	mw->box = gtk_vbox_new(FALSE, 0);	
	
	mw->view = gtk_image_view_new ();
	
	
	mw->img_box = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mw->box), mw->img_box, TRUE, TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->img_box),mw->view,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->box), gtk_hseparator_new(), FALSE, TRUE,0);
	
	//gtkuimanager
	mw->actions = gtk_action_group_new ("Actions");
	gtk_action_group_add_actions (mw->actions, entries, n_entries, NULL);
	mw->uimanager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group (mw->uimanager, mw->actions, 0);
	g_object_unref (mw->actions);
    gtk_window_add_accel_group (GTK_WINDOW (mw), 
				                gtk_ui_manager_get_accel_group (mw->uimanager));
	if (!gtk_ui_manager_add_ui_from_string (mw->uimanager, ui_info, -1, &error))
	{
	  g_message ("building menus failed: %s", error->message);
	  g_error_free (error);
	}
	gtk_box_pack_end(GTK_BOX (mw->box), gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), FALSE, TRUE, 0);
	gtk_toolbar_set_style(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), GTK_TOOLBAR_ICONS);
	//end gtuimanager 
	
	gtk_container_add((GtkContainer*)mw, mw->box);
	gtk_widget_show(mw->box);
	gtk_widget_show_all((GtkWindow*)mw);	
	g_object_unref(mw->uimanager);
}

gboolean 
main_win_open( MainWin* mw, const char* file_path, ZoomMode zoom )
{	
    GError* err = NULL;
    GdkPixbufFormat* info;
	
	GFileInputStream* ins;
	GFile* gf = g_file_new_for_path(file_path);

	ins = g_file_read(gf, NULL, NULL);
    
	mw->pix = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(ins), NULL, NULL);
    gtk_image_view_set_pixbuf (GTK_IMAGE_VIEW (mw->view), mw->pix, TRUE);
    g_input_stream_close(G_INPUT_STREAM(ins), NULL, NULL);
	
    info = gdk_pixbuf_get_file_info( file_path, NULL, NULL );
    char* type = ((info != NULL) ? gdk_pixbuf_format_get_name(info) : "");
	
	gtk_widget_show_all(mw->box);
}

// error window
void 
main_win_show_error( MainWin* mw, const char* message )
{
    GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "%s", message );
    gtk_dialog_run( (GtkDialog*)dlg );
    gtk_widget_destroy( dlg );
}




