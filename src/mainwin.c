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
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

// Begin of GObject-related stuff
G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW)

/* signal handlers */
static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static void on_prev(MainWin* mw );
static void on_next(MainWin* mw);
static void zoom_in();
static void zoom_out();
static void fit();
static void normal_size();
static void rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle);
static void rotate_cw(MainWin *mw);
static void rotate_ccw(MainWin *mw);
static void full_screen();

gchar *ui_info =
      "<ui>"
        "<toolbar name = 'ToolBar'>"
           "<toolitem action='Go Back'/>"
           "<toolitem  action='Go Forward'/>"
             "<separator action='Sep1'/>"
           "<toolitem  action='Zoom out'/>"
           "<toolitem  action='Zoom in'/>"
           "<toolitem  action='ZoomFit'/>"
           "<toolitem  action='ZoomNormal'/>"
           "<toolitem  action='FullScreen'/>"
             "<separator action='Sep2' />"
           "<toolitem  action='ImageRotate1'/>"
           "<toolitem  action='ImageRotate2'/>"
        "</toolbar>"
      "</ui>";


static ImageList* image_list;
static GtkAnimView* aview;
static GdkPixbufLoader* loader;

void
gtk_anim_view_set_static (GtkAnimView *aview, GdkPixbuf * pixbuf)
{
    GdkPixbufSimpleAnim *s_anim;

    s_anim = gdk_pixbuf_simple_anim_new (gdk_pixbuf_get_width(pixbuf),
                                         gdk_pixbuf_get_height(pixbuf),
                                         -1);
    gdk_pixbuf_simple_anim_add_frame(s_anim, pixbuf);

    /* Simple version of uni_anim_view_set_anim */
    if (aview->anim)
        g_object_unref (aview->anim);

    aview->anim = (GdkPixbufAnimation*)s_anim;

    g_object_ref (aview->anim);
    if (aview->iter)
        g_object_unref (aview->iter);

    gtk_image_view_set_pixbuf (GTK_IMAGE_VIEW (aview), pixbuf, TRUE);
    gtk_anim_view_set_is_playing (aview, FALSE);
    aview->delay = -1;
    aview->iter = NULL;

    g_object_unref(pixbuf);
}

void 
main_win_class_init( MainWinClass* klass )
{
    GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;
	widget_class->delete_event = on_delete_event;
}

GtkWindow* 
main_win_new()
{
	aview  =    gtk_anim_view_new();
	image_list = image_list_new();
	return (GtkWindow*)g_object_new (MAIN_WIN_TYPE, NULL);
}


static const GtkActionEntry entries[] = {
	{
	  "Go Back",
	  GTK_STOCK_GO_BACK,
	  "Go Back",
	  "<control>b",
	  "Go Back",
      G_CALLBACK(on_prev)
	},
	{
	 "Go Forward",
	 GTK_STOCK_GO_FORWARD,
	 "Go Forward",
	 "<control>g",
	 "Go Forward",
	 G_CALLBACK(on_next)
	},
	{
	 "Zoom out",
	 GTK_STOCK_ZOOM_OUT,
	 "Zoom out",
	 "<control>z",
	 "Zoom out",
	  G_CALLBACK(zoom_out)
	},
	{
     "Zoom in",
	 GTK_STOCK_ZOOM_IN,
	 "Zoom in",
	 "<control>i",
	 "Zoom in",
	  G_CALLBACK(zoom_in)
    },
	{
	  "ZoomFit",
	  GTK_STOCK_ZOOM_FIT,
	  "Fit",
	  "<control>f",
	  "Adapt zoom to fit image",
	   G_CALLBACK(fit)
	},
	{
	  "ZoomNormal",
	  GTK_STOCK_ZOOM_100, 
	  "_Normal Size",
	  "<control>0",
      "Show the image at its normal size",
      G_CALLBACK(normal_size)
	},
	{
	  "FullScreen",
	   GTK_STOCK_FULLSCREEN, 
	  "Full screen",
	  "<control>r",
      "Show the image in FULL SCREEN",
      G_CALLBACK(full_screen)
	},
	{
	   "ImageRotate1",
		GTK_STOCK_REDO,
		"Rotate Clockwise",
		"<control>R",
		"Rotate image",
		G_CALLBACK(rotate_cw)
	},
    {
	    "ImageRotate2",
		GTK_STOCK_UNDO,
		"Rotate Counter Clockwise",
		"<control>C",
		"Rotate image counter clockwise",
		G_CALLBACK(rotate_ccw)
	},
};

static guint n_entries = G_N_ELEMENTS (entries);

// End of GObject-related stuff
void
main_win_init( MainWin*mw )
{
	GError *error = NULL;
	
    gtk_window_set_title( (GtkWindow*)mw, "Image Viewer");
    gtk_window_set_default_size( (GtkWindow*)mw, 640, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);
	
	mw->max_width = gdk_screen_width () * 0.7;
    mw->max_height = gdk_screen_height () * 0.7;
	
	mw->box = gtk_vbox_new(FALSE, 0);
	mw->img_box = gtk_vbox_new(FALSE, 0);

	mw->scroll = GTK_IMAGE_SCROLL_WIN (gtk_image_scroll_win_new (aview));
	
	gtk_box_pack_start(GTK_BOX(mw->box), mw->img_box, TRUE, TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->img_box),mw->scroll,TRUE,TRUE,0);
	
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
	gtk_widget_show_all(mw);	

	g_object_unref(mw->uimanager);
	
	g_signal_connect (G_OBJECT (mw), "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_widget_grab_focus(aview);
	
	main_win_open(mw,"/home/shk/a.gif");
		
}

gboolean
main_win_open( MainWin* mw, const char* file_path)
{	    
	GError *error;
	GInputStream* input_stream;
	GFile *file = g_file_new_for_path(file_path);
	
    loader =    gdk_pixbuf_loader_new();
	GdkPixbufAnimation* animation = gdk_pixbuf_animation_new_from_file(file_path,error);
	
	gssize n_read;
	gboolean res;
	guchar buffer[LOAD_BUFFER_SIZE];
	
	input_stream = g_file_read(file,generator_cancellable ,NULL);
	
	res = TRUE;
	while (1){
		n_read = g_input_stream_read(input_stream, buffer, sizeof (buffer),generator_cancellable,error);
		
		if (n_read < 0) {
                        res = FALSE;
                        error = NULL; 
                        break;
                }
	
	if (n_read == 0)
        break;
	
	if (!gdk_pixbuf_loader_write(loader, buffer, sizeof(buffer), error)){
	   res = FALSE;
       break;
	   }
	}
	
	if (res){		
		animation = gdk_pixbuf_loader_get_animation((loader));
	    gtk_anim_view_set_anim (aview,animation);	
		
		char* dir_path = g_path_get_dirname( file_path );
        image_list_open_dir(image_list, dir_path, NULL );
        image_list_sort_by_name( image_list, GTK_SORT_ASCENDING );
        image_list_sort_by_name( image_list, GTK_SORT_DESCENDING );
        g_free( dir_path );
        
        char* base_name = g_path_get_basename( file_path );
        image_list_set_current( image_list, base_name );

        char* disp_name = g_filename_display_name( base_name );
		
        g_free( base_name );
        g_free( disp_name );	
		
		gdk_pixbuf_loader_close(loader,NULL);
	}
    else
	{
        res = FALSE;
        error = NULL;
		//g_object_unref (loader);
		g_object_unref (input_stream);
        g_object_unref (file);
        g_object_unref (generator_cancellable);		
		return;
    }
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

gboolean
on_delete_event( GtkWidget* widget, GdkEventAny* evt )
{   
	gtk_widget_destroy( widget );
	return TRUE;
}

/* prev/next **************************/
void on_prev(MainWin* mw)
{
    const char* name;
   
	name = image_list_get_prev( image_list);
	
	if( !name && image_list_has_multiple_files( image_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_get_last( image_list );
    }
    if( name )
    {
        char* file_path = image_list_get_current_file_path( image_list );
        main_win_open( mw, file_path );
        g_free( file_path );
    }
    
}

void on_next(MainWin* mw)
{
        const char* name;
   
	name = image_list_get_next( image_list);
	
	if( !name && image_list_has_multiple_files( image_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_get_last( image_list );
    }
    if( name )
    {
        char* file_path = image_list_get_current_file_path( image_list );
        main_win_open( mw, file_path );
        g_free( file_path );
    }
}
/* end prev/next **********************/

/* zoom **********************/
void zoom_out()
{
  gtk_image_view_zoom_out(aview);
}

void zoom_in()
{
  gtk_image_view_zoom_in(aview);
}

void fit()
{
  gtk_image_view_set_fitting(aview, TRUE);
}

void normal_size()
{
  gtk_image_view_set_zoom((aview), 1);
}
/* end zoom *****************/

/* rotate *******************/

static void
rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle)
{
    GdkPixbuf *result = NULL;
	
	result = gdk_pixbuf_rotate_simple(GTK_IMAGE_VIEW(aview)->pixbuf,angle);
	
	if(result == NULL)
        return;
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(aview), result);
	
	g_object_unref(result);
	
	mw->current_image_width = gdk_pixbuf_get_width (result);
    mw->current_image_height = gdk_pixbuf_get_height (result);
	
    if (mw->modifications & (4))
        mw->modifications ^= 3;
	
	mw->modifications ^=0 ;
}

void
rotate_cw(MainWin *mw)
{
	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_CLOCKWISE);
}

static void
rotate_ccw(MainWin *mw)
{
	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
}
/* end rotate ***************/

static void
full_screen()
{}
