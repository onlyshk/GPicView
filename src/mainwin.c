/***************************************************************************
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
 *                                                                         *
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "mainwin.h"
#include "utils.h"

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "pref.h"

#include "image-list.h"
#include "ptk-menu.h"
#include "file-dlgs.h"
#include "jpeg-tran.h"


#define LOAD_BUFFER_SIZE 65536 

static GdkPixbufAnimation* animation;
static GtkAnimView* aview;
static GdkPixbufLoader* loader;
static GCancellable* generator_cancellable = NULL;
static GtkActionGroup *actions;
static GtkActionGroup *rotation_actions;

/* For drag & drop */
static GtkTargetEntry drop_targets[] =
{
    {"text/uri-list", 0, 0},
    {"text/plain", 0, 1}
};

typedef struct _Param
{
  GtkWidget* btn;
  MainWin*   mw;
}Param;

static void main_win_init( MainWin*mw );
static void main_win_finalize( GObject* obj );

static void on_prev( GtkWidget* btn, MainWin* mw );
static void on_next( GtkWidget* btn, MainWin* mw );
static void zoom_in();
static void zoom_out();
static void next_for_slide_show( MainWin* mw );
static void fit();
static void normal_size();
static void rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle);
static void flip_pixbuf(MainWin *mw, gboolean horizontal);
static void rotate_cw(MainWin *mw);
static void rotate_ccw(MainWin *mw);
static void flip_v(MainWin *mw);
static void flip_h(MainWin *mw);
static void open_dialog (MainWin* mw);

static void show_popup_menu( MainWin* mw, GdkEventButton* evt );

/* signal handlers */
static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key);
static void on_size_allocate( GtkWidget* widget, GtkAllocation    *allocation );
static gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state );
static void on_zoom_fit( GtkToggleButton* btn, MainWin* mw );
static void on_zoom_fit_menu( GtkMenuItem* item, MainWin* mw );
static void on_full_screen( GtkWidget* btn, MainWin* mw );
static void start_slideshow(GtkWidget* btn, MainWin* mw);
static void on_orig_size( GtkToggleButton* btn, MainWin* mw );
static void on_orig_size_menu( GtkToggleButton* btn, MainWin* mw );
static void on_save_as( GtkWidget* btn, MainWin* mw );
static void on_save( GtkWidget* btn, MainWin* mw );
static void on_open( GtkWidget* btn, MainWin* mw );
static void on_delete( GtkWidget* btn, MainWin* mw );
static void update_title(const char *filename, MainWin *mw );
static void on_preference(MainWin* mw);


// Begin of GObject-related stuff

G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW )

void main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;

    obj_class = ( GObjectClass * ) klass;
    obj_class->finalize = main_win_finalize;

    widget_class = GTK_WIDGET_CLASS ( klass );
    widget_class->delete_event = on_delete_event;
    widget_class->key_press_event = on_key_press_event;
    widget_class->window_state_event = on_win_state_event;
}

void main_win_finalize( GObject* obj )
{
    MainWin *mw = (MainWin*)obj;
    main_win_close(mw);
    gtk_main_quit();
}

GtkWidget* main_win_new()
{
    return (GtkWidget*)g_object_new ( MAIN_WIN_TYPE, NULL );
}

// End of GObject-related stuff

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
           "<toolitem   action='SlideShow' />"
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
	 "<control>g","Go Forward", G_CALLBACK(on_next)
	},
	{"Zoom out",GTK_STOCK_ZOOM_OUT,"Zoom out",
	 "<control>z","Zoom out", G_CALLBACK(zoom_out)
	},
	{"Zoom in",GTK_STOCK_ZOOM_IN,"Zoom in",
	 "<control>i","Zoom in",G_CALLBACK(zoom_in)
    },
	{"ZoomFit",GTK_STOCK_ZOOM_FIT,"Fit",
	  "<control>f","Adapt zoom to fit image", G_CALLBACK(fit)
	},
	{"ZoomNormal",GTK_STOCK_ZOOM_100, "_Normal Size","<control>0",
     "Show the image at its normal size", G_CALLBACK(normal_size)
	},
	{"FullScreen",GTK_STOCK_FULLSCREEN, "Full screen",
	 "<control>r","Show the image in FULL SCREEN", G_CALLBACK(on_full_screen)
	},
	{"SlideShow", GTK_STOCK_DND_MULTIPLE, "SlideShow",
	 "<control>w", "Slide show", G_CALLBACK(start_slideshow)
	},
	{"Open File",GTK_STOCK_OPEN,"Open File",
	"<control>O","Open File", G_CALLBACK(on_open)
	},
	{"Save File",GTK_STOCK_SAVE,"Save File",
	"<control>s","Save File", G_CALLBACK(on_save)
	},
	{"Save as File",GTK_STOCK_SAVE_AS,"Save as File",
	  NULL,"Save as File", G_CALLBACK(on_save_as)
	},
	{"Delete File",GTK_STOCK_DELETE,"Delete File",
     "<control>r","Delete File", G_CALLBACK(on_delete)
	},
	{"Preferences",GTK_STOCK_PREFERENCES,"Preferences",
	 "<control>p", "Preferences", G_CALLBACK(on_preference)
	},
	{
	  "Quit",GTK_STOCK_QUIT,"Quit",
	   "<control>q", "Quit", G_CALLBACK(gtk_main_quit)
	}
};

static const GtkActionEntry entries1[] = {
	{"ImageRotate1","object-rotate-left","Rotate Clockwise",
	"<control>R","Rotate image", G_CALLBACK(rotate_ccw)
	},
    {"ImageRotate2","object-rotate-right","Rotate Counter Clockwise",
	"<control>C","Rotate image counter clockwise", G_CALLBACK(rotate_cw)
	},
	{"ImageRotate3","object-flip-vertical","Flip Vertical",
	"<control>v","Flip Horizontal", G_CALLBACK(flip_v)
	},
    {"ImageRotate4","object-flip-horizontal","Flip Vertical",
	"<control>C","Flip Horizontal", G_CALLBACK(flip_h)
	},
};

static guint n_entries = G_N_ELEMENTS (entries);
static guint n_entries1 = G_N_ELEMENTS (entries1);

void 
main_win_close( MainWin* mw )
{
	gtk_main_quit ();
}

void main_win_init( MainWin*mw )
{	
	GError* error;
	aview  =    GTK_IMAGE_VIEW(gtk_anim_view_new());
	mw->img_list = image_list_new();
	
    gtk_window_set_title( (GtkWindow*)mw, _("Image Viewer"));
    gtk_window_set_icon_from_file( (GtkWindow*)mw, PACKAGE_DATA_DIR"/pixmaps/gpicview.png", NULL );
    gtk_window_set_default_size( (GtkWindow*)mw, 700, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);

	mw->box = gtk_vbox_new(FALSE, 0);
	mw->img_box = gtk_vbox_new(FALSE, 0);
    mw->thumb_box = gtk_vbox_new (FALSE,0);
	
	mw->scroll = GTK_IMAGE_SCROLL_WIN (gtk_image_scroll_win_new (aview));
	gtk_box_pack_start(GTK_BOX(mw->box), mw->img_box, TRUE, TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->img_box),mw->scroll,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->box), gtk_hseparator_new(), FALSE, TRUE,0);

	//gtkuimanager
	actions = gtk_action_group_new ("Actions");
	rotation_actions = gtk_action_group_new("Rotate_Actions");
	
	mw->uimanager = gtk_ui_manager_new();
	
	gtk_ui_manager_insert_action_group (mw->uimanager, actions, 0);
	gtk_ui_manager_insert_action_group (mw->uimanager, rotation_actions,1);
	
	gtk_action_group_add_actions (actions, entries, n_entries, GTK_WINDOW(mw));
    gtk_action_group_add_actions (rotation_actions, entries1, n_entries1, GTK_WINDOW(mw));
	
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
    gtk_widget_show_all( mw->box );
}

gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt )
{
    gtk_widget_destroy( widget );
    return TRUE;
}

static void update_title(const char *filename, MainWin *mw )
{
    static char fname[50];
    static int wid, hei;

    char buf[100];

    if(filename != NULL)
    {
      strncpy(fname, filename, 49);
      fname[49] = '\0';

	  wid = gdk_pixbuf_get_width(  gtk_image_view_get_pixbuf (GTK_IMAGE_VIEW(aview)) );
      hei = gdk_pixbuf_get_height( gtk_image_view_get_pixbuf ( GTK_IMAGE_VIEW(aview)) );
    }

    snprintf(buf, 100, "%s (%dx%d) %d%%", fname, wid, hei, (int)(mw->scale * 100));
    gtk_window_set_title( (GtkWindow*)mw, buf );

    return;
}

gboolean main_win_open( MainWin* mw, const char* file_path, ZoomMode zoom )
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

      if ( !gdk_pixbuf_animation_is_static_image( animation ) )
		gtk_action_group_set_sensitive(rotation_actions, FALSE);
	  else
		gtk_action_group_set_sensitive(rotation_actions, TRUE);
        		
		animation = gdk_pixbuf_loader_get_animation((loader));
	    gtk_anim_view_set_anim (aview,animation);	
		
		char* dir_path = g_path_get_dirname( file_path );
        image_list_open_dir(mw->img_list, dir_path, NULL );
        image_list_sort_by_name( mw->img_list, GTK_SORT_ASCENDING );
        image_list_sort_by_name( mw->img_list, GTK_SORT_DESCENDING );
        g_free( dir_path );
		
        char* base_name = g_path_get_basename( file_path );
        image_list_set_current( mw->img_list, base_name );

        char* disp_name = g_filename_display_name( base_name );
		update_title( disp_name, mw );

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

gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state )
{
    MainWin* mw = (MainWin*)widget;
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

void on_prev( GtkWidget* btn, MainWin* mw )
{
    const char* name;
    if( image_list_is_empty( mw->img_list ) )
        return;

    name = image_list_get_prev( mw->img_list );

    if( ! name && image_list_has_multiple_files( mw->img_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_get_last( mw->img_list );
    }

    if( name )
    {
        char* file_path = image_list_get_current_file_path( mw->img_list );
        main_win_open( mw, file_path, ZOOM_FIT );
        g_free( file_path );
    }
}

void on_next( GtkWidget* bnt, MainWin* mw )
{
    if( image_list_is_empty( mw->img_list ) )
        return;

    const char* name = image_list_get_next( mw->img_list );

    if( ! name && image_list_has_multiple_files( mw->img_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_get_first( mw->img_list );
    }

    if( name )
    {
        char* file_path = image_list_get_current_file_path( mw->img_list );
        main_win_open( mw, file_path, ZOOM_FIT );
        g_free( file_path );
    }
}

void next_for_slide_show( MainWin* mw )
{
    if( image_list_is_empty( mw->img_list ) )
        return;

    const char* name = image_list_get_next( mw->img_list );

    if( ! name && image_list_has_multiple_files( mw->img_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_get_first( mw->img_list );
    }

    if( name )
    {
        char* file_path = image_list_get_current_file_path( mw->img_list );
        main_win_open( mw, file_path, ZOOM_FIT );
        g_free( file_path );
    }
}

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

void on_full_screen( GtkWidget* btn, MainWin* mw )
{
    if( ! mw->full_screen )
	{
        gtk_window_fullscreen( (GtkWindow*)mw );
	    gtk_widget_hide(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
	}
	else
	{
        gtk_window_unfullscreen( (GtkWindow*)mw );
	    gtk_widget_show(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
	}
}

void
start_slideshow(GtkWidget* btn, MainWin* mw)
{	
	if (mw->slideshow == TRUE)
	{
        gtk_window_unfullscreen( (GtkWindow*)mw );
		g_source_remove (mw->ss_source_tag);
		gtk_widget_show(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
		mw->slideshow = FALSE;
	}
	else
	{
      gtk_window_fullscreen (mw);
      mw->ss_source_tag =    g_timeout_add_seconds (2,
                                                   (GSourceFunc)next_for_slide_show,
                                                   mw);
	
	  gtk_widget_hide(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
	  mw->slideshow = TRUE;
	}
}

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
rotate_ccw(MainWin *mw)
{
  	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
}

static void
rotate_cw(MainWin *mw)
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

void on_delete( GtkWidget* btn, MainWin* mw )
{
    char* file_path = image_list_get_current_file_path( mw->img_list );
    if( file_path )
    {
        int resp = GTK_RESPONSE_YES;
	if ( pref.ask_before_delete )
	{
            GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_QUESTION,
                    GTK_BUTTONS_YES_NO,
                    _("Are you sure you want to delete current file?\n\nWarning: Once deleted, the file cannot be recovered.") );
            resp = gtk_dialog_run( (GtkDialog*)dlg );
            gtk_widget_destroy( dlg );
        }

	if( resp == GTK_RESPONSE_YES )
        {
            const char* name = image_list_get_current( mw->img_list );

	    if( g_unlink( file_path ) != 0 )
		main_win_show_error( mw, g_strerror(errno) );
	    else
	    {
		const char* next_name = image_list_get_next( mw->img_list );
		if( ! next_name )
		    next_name = image_list_get_prev( mw->img_list );

		if( next_name )
		{
		    char* next_file_path = image_list_get_current_file_path( mw->img_list );
		    main_win_open( mw, next_file_path, ZOOM_FIT );
		    g_free( next_file_path );
		}

		image_list_remove ( mw->img_list, name );

		if ( ! next_name )
		{
		    main_win_close( mw );
		    image_list_close( mw->img_list );
		    gtk_window_set_title( (GtkWindow*) mw, _("Image Viewer"));
		}
	    }
        }
	g_free( file_path );
    }
}

void on_open( GtkWidget* btn, MainWin* mw )
{
    char* file = get_open_filename( (GtkWindow*)mw, image_list_get_dir( mw->img_list ) );
    if( file )
    {
        main_win_open( mw, file, ZOOM_NONE );
        g_free( file );
    }
}

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm )
{
    if (!mw->img_list)
		return;
	
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

void on_save_as( GtkWidget* btn, MainWin* mw )
{
   if( !mw->img_list )
       return;
	
    char *file, *type;

    if( ! aview )
        return;
	
	file = get_save_filename( GTK_WINDOW(mw), image_list_get_dir(mw->img_list), &type );
	
	if( file )
    {
	    char* dir;
        main_win_save( mw, file, type, TRUE );
        dir = g_path_get_dirname(file);
        const char* name = file + strlen(dir) + 1;
		
		if( strcmp( image_list_get_dir(mw->img_list), dir ) == 0 )
        {
            image_list_add_sorted( mw->img_list, name, TRUE );
        }
        else 
        {
            image_list_open_dir( mw->img_list, dir, NULL );
        }
		update_title(file,mw);
        g_free( dir );
        g_free( file );
        g_free( type );
	}
}

void on_save( GtkWidget* btn, MainWin* mw )
{
    char* file_name = g_build_filename( image_list_get_dir( mw->img_list ),
                                        image_list_get_current( mw->img_list ), NULL );
    GdkPixbufFormat* info;
    info = gdk_pixbuf_get_file_info( file_name, NULL, NULL );
    char* type = gdk_pixbuf_format_get_name( info );

    if(strcmp(type,"jpeg")==0)
    {
        if(!pref.rotate_exif_only) //|| ExifRotate(file_name, mw->rotation_angle) == FALSE)
        {
            // hialan notes:
            // ExifRotate retrun FALSE when
            //   1. Can not read file
            //   2. Exif do not have TAG_ORIENTATION tag
            //   3. Format unknown
            // And then we apply rotate_and_save_jpeg_lossless() ,
            // the result would not effected by EXIF Orientation...
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
    } else
        main_win_save( mw, file_name, type, pref.ask_before_save );
    g_free( file_name );
    g_free( type );
}

static void on_preference(MainWin* mw)
{
   edit_preferences(mw);
}

gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key)
{
    MainWin* mw = (MainWin*)widget;
    switch( key->keyval )
    {
        case GDK_Right:
        case GDK_KP_Right:
        case GDK_rightarrow:
            if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
                on_prev( NULL, mw );
            else
                on_next( NULL, mw );
            break;
        case GDK_Return:
        case GDK_space:
        case GDK_Next:
        case GDK_KP_Down:
        case GDK_Down:
        case GDK_downarrow:
            on_next( NULL, mw );
            break;
        case GDK_Left:
        case GDK_KP_Left:
        case GDK_leftarrow:
            if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
                on_next( NULL, mw );
            else
                on_prev( NULL, mw );
            break;
        case GDK_Prior:
        case GDK_BackSpace:
        case GDK_KP_Up:
        case GDK_Up:
        case GDK_uparrow:
            on_prev( NULL, mw );
            break;
        case GDK_KP_Add:
        case GDK_plus:
        case GDK_equal:
            zoom_in( mw );
            break;
        case GDK_KP_Subtract:
        case GDK_minus:
            zoom_out( mw );
            break;
        case GDK_s:
        case GDK_S:
            on_save( NULL, mw );
            break;
        case GDK_a:
        case GDK_A:
            on_save_as( NULL, mw );
            break;
        case GDK_l:
        case GDK_L:
            rotate_ccw( mw );
            break;
        case GDK_r:
        case GDK_R:
            rotate_cw( mw );
            break;
        case GDK_f:
        case GDK_F:
            fit(mw);
            break;
        case GDK_h:
        case GDK_H:
            flip_h( mw );
            break;
        case GDK_v:
        case GDK_V:
            flip_v( mw );
            break;
        case GDK_o:
        case GDK_O:
            on_open( NULL, mw );
            break;
        case GDK_Delete:
        case GDK_d:
        case GDK_D:
            on_delete( NULL, mw );
            break;
        case GDK_p:
        case GDK_P:
            on_preference( mw );
	    break;
        case GDK_Escape:
            if( mw->full_screen )
                on_full_screen( NULL, mw );
            else
                gtk_main_quit();
            break;
        case GDK_q:
	case GDK_Q:
            gtk_main_quit();
            break;
        case GDK_F11:
            on_full_screen( NULL, mw );
            break;

        default:
            GTK_WIDGET_CLASS(main_win_parent_class)->key_press_event( widget, key );
    }
    return FALSE;
}

//=========================================================
