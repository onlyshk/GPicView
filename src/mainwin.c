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
#include "utils.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <glib/gstdio.h>

#define LOAD_BUFFER_SIZE 65536 

static ImageList* image_list;
static GdkPixbufAnimation* animation;
static GtkAnimView* aview;
static GdkPixbufLoader* loader;
static GCancellable* generator_cancellable = NULL;

static void main_win_init( MainWin*mw );
static void main_win_finalize( GObject* obj );

static void on_prev(MainWin* mw );
static void on_next(MainWin* mw);
static void zoom_in();
static void zoom_out();
static void fit();
static void normal_size();
static void rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle);
static void flip_pixbuf(MainWin *mw, gboolean horizontal);
static void rotate_cw(MainWin *mw);
static void rotate_ccw(MainWin *mw);
static void full_screen(GtkWidget *button, MainWin *mw);
static void flip_v(MainWin *mw);
static void flip_h(MainWin *mw);
static void open_dialog();
static void on_save(MainWin* mw);
static void on_delete(MainWin* mw);
static gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm );
static void on_save_as(MainWin* mw);
static gboolean save_confirm( MainWin* mw, const char* file_path );
void on_preference(MainWin* mw );


/* signal handlers */
static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state );

// Begin of GObject-related stuff
G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW)

void 
main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;
	
	obj_class = ( GObjectClass * ) klass;
	obj_class->finalize = main_win_finalize;
	
	widget_class = GTK_WIDGET_CLASS ( klass );
	widget_class->delete_event = on_delete_event;
	widget_class->window_state_event = on_win_state_event;
}

void main_win_finalize( GObject* obj )
{
    MainWin *mw = (MainWin*)obj;
    main_win_close(mw);

    if( G_LIKELY(image_list) )
        image_list_free(image_list );
	
    gtk_main_quit();
}

GtkWindow* 
main_win_new()
{
	return (GtkWindow*)g_object_new (MAIN_WIN_TYPE, NULL);
}

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
           "<toolitem  action='ImageRotate3'/>"
           "<toolitem  action='ImageRotate4'/>"
             "<separator action='Sep3' />"
           "<toolitem  action='Open File'/>"
           "<toolitem  action='Save File'/>"
           "<toolitem  action='Save as File'/>"
           "<toolitem  action='Delete File'/>"
             "<separator  action='Sep4' />"
           "<toolitem   action='Preferences'/>"
           "<toolitem   action='Quit' />"
        "</toolbar>"
      "</ui>";

static const GtkActionEntry entries[] = {
	{"Go Back",GTK_STOCK_GO_BACK, "Go Back",
	  "<control>b","Go Back", G_CALLBACK(on_prev)
	},
	{"Go Forward",GTK_STOCK_GO_FORWARD,"Go Forward",
	 "<control>g","Go Forward",G_CALLBACK(on_next)
	},
	{"Zoom out",GTK_STOCK_ZOOM_OUT,"Zoom out",
	 "<control>z","Zoom out", G_CALLBACK(zoom_out)
	},
	{"Zoom in",GTK_STOCK_ZOOM_IN,"Zoom in",
	 "<control>i","Zoom in",G_CALLBACK(zoom_in)
    },
	{"ZoomFit",GTK_STOCK_ZOOM_FIT,"Fit",
	  "<control>f","Adapt zoom to fit image",G_CALLBACK(fit)
	},
	{"ZoomNormal",GTK_STOCK_ZOOM_100, "_Normal Size","<control>0",
     "Show the image at its normal size",G_CALLBACK(normal_size)
	},
	{"FullScreen",GTK_STOCK_FULLSCREEN, "Full screen",
	 "<control>r","Show the image in FULL SCREEN",G_CALLBACK(full_screen)
	},
	{"ImageRotate1","object-rotate-left","Rotate Clockwise",
	"<control>R","Rotate image",G_CALLBACK(rotate_cw)
	},
    {"ImageRotate2","object-rotate-right","Rotate Counter Clockwise",
	"<control>C","Rotate image counter clockwise",G_CALLBACK(rotate_ccw)
	},
	{"ImageRotate3","object-flip-vertical","Flip Vertical",
	"<control>v","Flip Horizontal",G_CALLBACK(flip_v)
	},
    {"ImageRotate4","object-flip-horizontal","Flip Vertical",
	"<control>C","Flip Horizontal",G_CALLBACK(flip_h)
	},
	{"Open File",GTK_STOCK_OPEN,"Open File",
	"<control>O","Open File",G_CALLBACK(open_dialog)
	},
	{"Save File",GTK_STOCK_SAVE,"Save File",
	"<control>s","Save File",G_CALLBACK(on_save)
	},
	{"Save as File",GTK_STOCK_SAVE_AS,"Save as File",
	  NULL,"Save as File",G_CALLBACK(on_save_as)
	},
	{"Delete File",GTK_STOCK_DELETE,"Delete File",
     "<control>r","Delete File", G_CALLBACK(on_delete)
	},
	{"Preferences",GTK_STOCK_PREFERENCES,"Preferences",
	 "<control>p", "Preferences", G_CALLBACK(on_preference)
	},
	{
	  "Quit",GTK_STOCK_QUIT,"Quit",
	   "<control>q", "Quit",G_CALLBACK(gtk_main_quit)
	}
};

static guint n_entries = G_N_ELEMENTS (entries);

void 
main_win_close( MainWin* mw )
{
	gtk_main_quit ();
}

void
main_win_init( MainWin *mw )
{
	GError *error = NULL;
	
    aview  =    gtk_anim_view_new();
	image_list = image_list_new();
	
    gtk_window_set_title((GtkWindow*)mw, "Image Viewer");
    gtk_window_set_default_size((GtkWindow*)mw, 670, 480 );
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
	mw->uimanager = gtk_ui_manager_new();
	
	gtk_ui_manager_insert_action_group (mw->uimanager, mw->actions, 0);
	gtk_action_group_add_actions (mw->actions, entries, n_entries, GTK_WINDOW(mw));
    gtk_window_add_accel_group (GTK_WINDOW (mw), 
				                gtk_ui_manager_get_accel_group (mw->uimanager));
	if (!gtk_ui_manager_add_ui_from_string (mw->uimanager, ui_info, -2, &error))
	{
	  g_message ("building menus failed: %s", error->message);
	  g_error_free (error);
	}
	gtk_box_pack_end(GTK_BOX (mw->box), gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), FALSE, TRUE,0);
	gtk_toolbar_set_style(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), GTK_TOOLBAR_ICONS);
	//end gtuimanager 
	
	gtk_container_add(mw, mw->box);
	gtk_widget_show_all(mw);	
	g_object_unref(mw->uimanager);	
	
}

gboolean
main_win_open( MainWin* mw, const char* file_path)
{	    
	GError *error;
	GInputStream* input_stream;
	GFile *file = g_file_new_for_path(file_path);
	
    loader =    gdk_pixbuf_loader_new();
	animation = gdk_pixbuf_animation_new_from_file(file_path,error);
	
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
        name = image_list_get_first( image_list );
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
	if (!animation)
		return;
	
    GdkPixbuf *result = NULL;
	gdk_flush();
	result = gdk_pixbuf_rotate_simple(GTK_IMAGE_VIEW(aview)->pixbuf,angle);
	
	if(result == NULL)
        return;
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(aview), result);
	
	g_object_unref(result);
	
	mw->current_image_width = gdk_pixbuf_get_width (result);
    mw->current_image_height = gdk_pixbuf_get_height (result);
	
	if((mw->modifications & (4))^((angle==GDK_PIXBUF_ROTATE_CLOCKWISE)<<2))
        mw->modifications ^= 3;
	
	mw->modifications ^=4;
}

static void
flip_pixbuf(MainWin *mw, gboolean horizontal)
{
	if (!animation)
		return;
    
	GdkPixbuf *result = NULL;
    gdk_flush();
	
	result = gdk_pixbuf_flip(GTK_IMAGE_VIEW(aview)->pixbuf,horizontal);

	
	if(result == NULL)
        return;
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(aview), result);
	
	g_object_unref(result);
	
	mw->modifications ^= (mw->modifications&4)?1+horizontal:2-horizontal;
}

void
rotate_cw(MainWin *mw)
{
  	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
}

static void
rotate_ccw(MainWin *mw)
{
	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_CLOCKWISE);
}

static void
flip_v(MainWin *mw)
{
  flip_pixbuf(mw,FALSE);
}

static void
flip_h(MainWin * mw)
{
	flip_pixbuf(mw,TRUE);
}
/* end rotate ***************/

gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state )
{
    MainWin* mw = (GtkWindow*)widget;
    if( state->new_window_state == GDK_WINDOW_STATE_FULLSCREEN )
    {
        mw->full_screen = TRUE;
    }
    else
    {
        mw->full_screen = FALSE;
    }
    return TRUE;
}

static void
full_screen(GtkWidget *button, MainWin* mw)
{	
    if( ! mw->full_screen )
	{    
        gtk_window_fullscreen( (GtkWindow*)mw );
	}
	else
        gtk_window_unfullscreen( (GtkWindow*)mw );
	
	g_signal_connect(button, "clicked", G_CALLBACK(full_screen), mw);
}

static void
open_dialog(MainWin* mw)
{
   GtkWidget *dialog;
   dialog = gtk_file_chooser_dialog_new( ("Open Image"),
                          GTK_WINDOW(mw),
                          GTK_FILE_CHOOSER_ACTION_OPEN,
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                          NULL);

    gtk_window_set_modal (GTK_WINDOW(dialog), FALSE);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
		
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
	  gchar *filename;
	  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	  main_win_open (mw,filename);
	  gtk_widget_destroy (dialog);
	  g_free (filename);
	}
	else
	{
	  gtk_widget_destroy (dialog);
	}
}

void on_delete(MainWin* mw )
{
	GError *error;
    char* file_path = image_list_get_current_file_path( image_list );

    if( file_path )
    {
        int resp = GTK_RESPONSE_YES;
        GtkWidget*  dlg = gtk_message_dialog_new(0,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_QUESTION,
                    GTK_BUTTONS_YES_NO,
                    ("Are you sure you want to delete current file?\n\nWarning: Once deleted, the file cannot be recovered.") );
            resp = gtk_dialog_run( (GtkDialog*)dlg );
            gtk_widget_destroy( dlg );
   
	if( resp == GTK_RESPONSE_YES )
    {
         const char* name = image_list_get_current( image_list );
		
		 if (g_unlink( file_path ) != 0)
		     printf("deleting error");
		
		 const char* next_name = image_list_get_next( image_list );
		
		if( ! next_name )
		    next_name = image_list_get_prev( image_list );

		if( next_name )
		{
		    char* next_file_path = image_list_get_current_file_path(image_list );
		    main_win_open( mw, next_file_path );
		    g_free( next_file_path );
		}

		image_list_remove (image_list, name );

		if ( ! next_name )
		{
		    main_win_close( mw );
		    image_list_close( image_list );
		    gtk_window_set_title( (GtkWindow*) mw, ("Image Viewer"));
		}
	}
    }
	g_free( file_path );
}

/* save and save_as */

gboolean save_confirm( MainWin* mw, const char* file_path )
{
    if( g_file_test( file_path, G_FILE_TEST_EXISTS ) )
    {
        GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO,
                ("The file name you selected already exists.\nDo you want to overwrite existing file?\n(Warning: The quality of original image might be lost)") );
        if( gtk_dialog_run( (GtkDialog*)dlg ) != GTK_RESPONSE_YES )
        {
            gtk_widget_destroy( dlg );
            return FALSE;
        }
        gtk_widget_destroy( dlg );
    }
    return TRUE;
}

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm )
{
    gboolean result1,gdk_save_supported;
    GSList *gdk_formats;
    GSList *gdk_formats_i;
    if( !aview )
        return FALSE;
	
	/* detect if the current type can be save by gdk_pixbuf_save() */
    gdk_save_supported = FALSE;
    gdk_formats = gdk_pixbuf_get_formats();
    for (gdk_formats_i = gdk_formats; gdk_formats_i;
         gdk_formats_i = g_slist_next(gdk_formats_i))
    {
        GdkPixbufFormat *data;
        data = gdk_formats_i->data;
        if (gdk_pixbuf_format_is_writable(data))
        {
            if ( strcmp(type, gdk_pixbuf_format_get_name(data))==0)
            {
                gdk_save_supported = TRUE;
                break;
            }
        }
    }
    g_slist_free (gdk_formats);
	
	GError* err = NULL;
    if (!gdk_save_supported)
    {
        main_win_show_error( mw, ("Writing this image format is not supported.") );
        return FALSE;
    }
    if( strcmp( type, "jpeg" ) == 0 )
    {
        char tmp[32];
        g_sprintf(tmp, "%d", pref.jpg_quality);
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(aview)),
								  file_path, type, &err, "quality", tmp, NULL );
    }
    else if( strcmp( type, "png" ) == 0 )
    {
        char tmp[32];
        g_sprintf(tmp, "%d", pref.png_compression);
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(aview)),
								  file_path, type, &err, "compression", tmp, NULL );
    }
    else
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(aview)),
								  file_path, type, &err, NULL );
    if( ! result1 )
    {
        main_win_show_error( mw, err->message );
        return FALSE;
    }
    return TRUE;
}

void on_save(MainWin* mw )
{
   if( !image_list )
       return;

    char* file_name = g_build_filename( image_list_get_dir( image_list ),
                                        image_list_get_current( image_list ), NULL );
    GdkPixbufFormat* info;
    info = gdk_pixbuf_get_file_info( file_name, NULL, NULL );
    char* type = gdk_pixbuf_format_get_name( info );

    /* Confirm save if requested. */
    if ((pref.ask_before_save) && ( ! save_confirm(mw, file_name)))
        return;

    if(strcmp(type,"jpeg")==0)
    {
		
#ifdef HAVE_LIBJPEG
        int status = rotate_and_save_jpeg_lossless(file_name,mw->rotation_angle);
	    if(status != 0)
            {
                main_win_show_error( mw, g_strerror(status) );
            }
#else
            main_win_save( mw, file_name, type, pref.ask_before_save );
#endif
        } 
    else 
    main_win_save( mw, file_name, type, pref.ask_before_save );
    g_free( file_name );
    g_free( type );
}

void on_save_as(MainWin* mw)
{
    char *file, *type;

    if( ! aview )
        return;
	
	file = get_save_filename( GTK_WINDOW(mw), image_list_get_dir(image_list), &type );
	
	if( file )
    {
	    char* dir;
        main_win_save( mw, file, type, TRUE );
        dir = g_path_get_dirname(file);
        const char* name = file + strlen(dir) + 1;
		
		if( strcmp( image_list_get_dir(image_list), dir ) == 0 )
        {
            image_list_add_sorted( image_list, name, TRUE );
        }
        else 
        {
            image_list_open_dir( image_list, dir, NULL );
        }
        //update_title( name, mw );
        g_free( dir );
        g_free( file );
        g_free( type );
	}
}

void on_preference(MainWin* mw )
{
    edit_preferences( (GtkWindow*)mw );
}
/* end save and save as */
