/***************************************************************************
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
 *   2010 Kuleshov Alexander <kuleshovmail@gmail.com>                      *
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

#include <glib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <cairo/cairo.h>

#include "pref.h"
#include "image-list.h"
#include "ptk-menu.h"
#include "file-dlgs.h"
#include "jpeg-tran.h"
#include "crop.h"
#include "utils.h"
#include "wallpaper.h"
#include "exifdialog.h"

static GtkActionGroup *actions;
static GtkActionGroup *rotation_actions;
static GList* thumbnail_selected_list;
static GList* thumbnail_path_list;
static GList* thumbnail_loaded_list;

/* For drag & drop */
static GtkTargetEntry drop_targets[] =
{
    {"text/uri-list", 0, 0},
    {"text/plain", 0, 1}
};

static void main_win_init( MainWin*mw );
static void main_win_finalize( GObject* obj );
static void on_prev( GtkWidget* btn, MainWin* mw );
static void on_next( GtkWidget* btn, MainWin* mw );
static void zoom_in(GtkWidget* widget, MainWin* mw);
static void zoom_out(GtkWidget* widget, MainWin* mw);
static void fit(GtkWidget* widget, MainWin* mw);
static void normal_size(GtkWidget* widget, MainWin* mw);
static void next_for_slide_show( MainWin* mw );
static void rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle);
static void flip_pixbuf(MainWin *mw, gboolean horizontal);
static void rotate_cw(GtkWidget* widget, MainWin *mw);
static void rotate_ccw(GtkWidget* widget, MainWin *mw);
static void flip_v(GtkWidget* widget, MainWin *mw);
static void flip_h(GtkWidget* widget, MainWin *mw);
static void on_about( GtkWidget* menu, MainWin* mw );
static void show_popup_menu( MainWin* mw, GdkEventButton* evt );
static void hide_thumbnails(GtkWidget* widget, MainWin* mw);
static void on_zoom_fit( GtkToggleButton* btn, MainWin* mw );
static void on_zoom_fit_menu( GtkMenuItem* item, MainWin* mw );
static void on_full_screen( GtkWidget* btn, MainWin* mw );
static void start_slideshow(GtkWidget* btn, MainWin* mw);
static void on_orig_size( GtkToggleButton* btn, MainWin* mw );
static void on_orig_size_menu( GtkToggleButton* btn, MainWin* mw );
static void on_save_as( GtkWidget* btn, MainWin* mw );
static void on_save( GtkWidget* btn, MainWin* mw );
static void on_open( GtkWidget* widget, MainWin* mw );
static void on_delete( GtkWidget* btn, MainWin* mw );
static void update_title(const char *filename, MainWin *mw );
static void on_preference(GtkWidget* widget, MainWin* mw);
static void show_popup_menu( MainWin* mw, GdkEventButton* evt );
static void open_url( GtkAboutDialog *dlg, const gchar *url, gpointer data);
static gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw );
static void thumbnail_selected(GtkWidget* widget, MainWin* mw);
static void loading(JobParam* param);
static void on_rotate_auto_save( GtkWidget* btn, MainWin* mw );
static void set_wallpapaer(GtkWidget* widget, MainWin* mw);
static void crop_image (GtkWidget* widget, MainWin* mw, GdkEventMotion *event);
static void draw_rectangle(GtkWidget* widget, MainWin* mw);

static gboolean set_image(JobParam* param);
static gboolean load(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static void load_thumbnails(JobParam* param);
static gboolean job_func(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static gboolean job_func1(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static gboolean set_thumbnails(JobParam* param);
static void exif_information(GtkWidget* widget, MainWin* mw);

/* signal handlers */
static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key);
static void on_size_allocate( GtkWidget* widget, GtkAllocation    *allocation );
static gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state );

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
	
	if( G_LIKELY(mw->img_list) )
        image_list_free( mw->img_list );
	
    main_win_close(mw);
    gtk_main_quit();
}

GtkWidget* main_win_new()
{
    return (GObject*)g_object_new ( MAIN_WIN_TYPE, NULL );
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
           "<toolitem  action='Print image' />"
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
	{"Print image",GTK_STOCK_PRINT, "Print image",
	  "<control>p","Print image", G_CALLBACK(printing_image)
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
	GError* error = NULL;
		
	mw->aview  =    GTK_IMAGE_VIEW(gtk_anim_view_new());
	mw->generator_cancellable = g_cancellable_new();
    mw->thumbnail_cancellable = g_cancellable_new();
	mw->img_list = image_list_new();
    mw->loader =    gdk_pixbuf_loader_new();
	
	mw->align = gtk_alignment_new( 0.5, 0,0,0);
	mw->thumb_bar_hide = TRUE;
	
    gtk_window_set_title( (GtkWindow*)mw, _("Image Viewer"));
    gtk_window_set_icon_from_file( (GtkWindow*)mw, PACKAGE_DATA_DIR"/pixmaps/gpicview.png", NULL );
    gtk_window_set_default_size( (GtkWindow*)mw, 720, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);

	mw->box = gtk_vbox_new(FALSE, 0);
	mw->toolbar_box = gtk_hpaned_new ();
	mw->img_box =   gtk_hpaned_new ();
    mw->thumb_box = gtk_vbox_new (FALSE,0);
	
    mw->model = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	mw->view = gtk_icon_view_new();
	mw->thumbnail_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mw->thumbnail_scroll),GTK_POLICY_AUTOMATIC, 
                                    GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (mw->thumbnail_scroll), GTK_SHADOW_IN);

	mw->scroll = GTK_IMAGE_SCROLL_WIN (gtk_image_scroll_win_new (mw->aview));
	gtk_paned_add1(GTK_PANED(mw->img_box),mw->thumbnail_scroll);
	
	gtk_scrolled_window_add_with_viewport(mw->thumbnail_scroll,mw->view);
	gtk_paned_add2(GTK_PANED(mw->img_box),mw->scroll);	
		
	gtk_box_pack_start(GTK_BOX(mw->box), mw->img_box, TRUE, TRUE,0);
	
    gtk_box_pack_start(GTK_BOX(mw->img_box), mw->thumbnail_scroll, TRUE, TRUE,0);
	gtk_box_pack_start(GTK_BOX(mw->img_box),mw->scroll,TRUE,TRUE,0);
	
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
	gtk_widget_set_size_request(mw->toolbar_box, 720,40);
	
    gtk_paned_add1(mw->toolbar_box,gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
	gtk_container_add( (GtkContainer*)mw->align, mw->toolbar_box);
	gtk_box_pack_end( (GtkBox*)mw->box, mw->align, FALSE, TRUE, 2 );
	
	gtk_toolbar_set_style(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), GTK_TOOLBAR_ICONS);
	//end gtuimanager 
			
	g_signal_connect( mw->box, "button-press-event", G_CALLBACK(on_button_press), mw );
	g_signal_connect( mw->view ,"selection-changed", G_CALLBACK(thumbnail_selected), mw);
    
	gtk_paned_set_position(mw->img_box,170);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(mw->view),0);
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(mw->view), 1);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(mw->view), GTK_SELECTION_SINGLE);
	
	gtk_widget_modify_bg (mw->aview, GTK_STATE_NORMAL, &pref.bg);
	
	gtk_box_pack_start(mw->box, gtk_hseparator_new(), FALSE, TRUE,0);	

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

	  wid = gdk_pixbuf_get_width(  gtk_image_view_get_pixbuf (GTK_IMAGE_VIEW(mw->aview)) );
      hei = gdk_pixbuf_get_height( gtk_image_view_get_pixbuf ( GTK_IMAGE_VIEW(mw->aview)) );
    }
    snprintf(buf, 100, "%s", fname);
    gtk_window_set_title( (GtkWindow*)mw, buf );
    return;
}

void loading(JobParam* param)
{	
	GInputStream* input_stream = NULL;
	input_stream = g_file_read(param->file, param->generator_cancellable, NULL);
	param->animation = load_animation_from_stream(G_INPUT_STREAM(input_stream), param->generator_cancellable);	
	
	g_input_stream_close(input_stream, param->generator_cancellable, NULL);
	g_object_unref (input_stream);
}

void load_thumbnails(JobParam* param)
{				
	GInputStream* input_stream = NULL;
	GtkTreeIter iter;
	GFile* file = NULL;
	GdkPixbuf* thumb_pixbuf;
	
	int i = 0;
	int n = g_list_length(param->mw->img_list) - 1;
	
	char* buffer = NULL;
	
	for (i; i < n; ++i)
	{
	  file =  g_file_new_for_path (image_list_get_current_file_path( param->mw->img_list ));
	 
	  thumbnail_loaded_list = g_list_prepend(thumbnail_loaded_list,
											 image_list_get_current_file_path( param->mw->img_list ));
		
	  input_stream = g_file_read(file, param->generator_cancellable, NULL);
	  
	  thumb_pixbuf = load_image_from_stream(G_INPUT_STREAM(input_stream), param->mw->thumbnail_cancellable);
	  thumb_pixbuf = scale_pix(thumb_pixbuf,128);
      
	  param->mw->disp_list = g_list_prepend (param->mw->disp_list, thumb_pixbuf);
	  
	  buffer    = g_file_get_basename (file);
	  thumbnail_path_list     = g_list_prepend(thumbnail_path_list,buffer);
	  
	  if (!param->mw->img_list->current->next )
	      image_list_get_first(param->mw->img_list);
	  else
	      image_list_get_next(param->mw->img_list);
	}
	
	g_input_stream_close(input_stream, param->mw->thumbnail_cancellable, NULL);
	g_object_unref (input_stream);
}

gboolean main_win_open(MainWin* mw)
{        		
	mw->disp_list                  = NULL;
	thumbnail_loaded_list          = NULL;	
	thumbnail_selected_list        = NULL;
	
	g_cancellable_reset(mw->generator_cancellable);
	g_cancellable_reset(mw->thumbnail_cancellable);
	
	JobParam* param;
	param =  g_new (JobParam, 1);
	
    param->file                  = mw->loading_file;
	param->generator_cancellable = mw->generator_cancellable;
	param->animation             = mw->animation;
	param->mw                    = mw;
	
	g_io_scheduler_push_job (job_func1, param, NULL, G_PRIORITY_DEFAULT, mw->generator_cancellable);
	g_io_scheduler_push_job (job_func, param, NULL, G_PRIORITY_DEFAULT,  mw->thumbnail_cancellable);

	return TRUE;
}

gboolean main_win_open_without_thumbnails_loading(MainWin* mw)
{        			
	g_cancellable_reset(mw->generator_cancellable);
	
	JobParam* param;
	param =  g_new (JobParam, 1);
	
    param->file                  = mw->loading_file;
	param->generator_cancellable = mw->generator_cancellable;
	param->animation             = mw->animation;
	param->mw                    = mw;
	
	g_io_scheduler_push_job (job_func1, param, NULL, G_PRIORITY_DEFAULT, mw->generator_cancellable);

	return TRUE;
}

gboolean job_func(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data)
{
	JobParam* param = (JobParam*)user_data;
	
	load_thumbnails(param);
	g_io_scheduler_job_send_to_mainloop(job, set_thumbnails, param, NULL);
	
	return FALSE;
}

gboolean job_func1(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data)
{
	JobParam* param = (JobParam*)user_data;
	
	loading(param);
	g_io_scheduler_job_send_to_mainloop(job, set_image, param, NULL);

	return FALSE;
}

gboolean set_thumbnails(JobParam* param)
{  	
  GtkTreeIter* iter;
  GdkPixbuf* pixbuf;
	
  gtk_list_store_clear(param->mw->model);
  gtk_icon_view_set_model(param->mw->view,GTK_TREE_MODEL(param->mw->model));	
	
  int i = 0;
  int n = g_list_length(param->mw->disp_list);
  
  for (i; i < n; ++i)
  {
	char* paths = g_list_nth_data(thumbnail_path_list,i);
	  
	pixbuf =  (GdkPixbuf*)g_list_nth_data(param->mw->disp_list, i);
	
	gtk_list_store_append(param->mw->model, &iter);
	gtk_list_store_set(param->mw->model, &iter, COL_DISPLAY_NAME, paths, 
					   COL_PIXBUF, (GdkPixbuf*)pixbuf, -1);
	 
	if (!param->mw->img_list->current->next )
	     image_list_get_first(param->mw->img_list);
	else
	     image_list_get_next(param->mw->img_list);
  }
}

gboolean set_image(JobParam* param)
{
    param->mw->animation = g_object_ref(param->animation);

	gtk_anim_view_set_anim (param->mw->aview, param->mw->animation);
}

void on_open( GtkWidget* widget, MainWin* mw )
{	
	char* file;
	
	if ((file = get_open_filename( mw, NULL)) == NULL)  	
	     return;
	
	mw->loading_file = g_file_new_for_path(file);
    
	char* file_path =  g_file_get_path(mw->loading_file);
	char* dir_path = g_path_get_dirname( file_path);
    image_list_open_dir(mw->img_list, dir_path, NULL );
    image_list_sort_by_name( mw->img_list, GTK_SORT_DESCENDING );
    image_list_sort_by_name( mw->img_list, GTK_SORT_DESCENDING );
        
    char* base_name = g_path_get_basename( file_path );
    image_list_set_current( mw->img_list, base_name );
    char* disp_name = g_filename_display_name( base_name );
	
	update_title(file_path, mw);
	
	g_free( file );
	g_free( dir_path );
    g_free( base_name );
    g_free( disp_name );	
    
	main_win_open (mw);
}

void thumbnail_selected( GtkWidget* widget, MainWin* mw)
{
  thumbnail_selected_list = gtk_icon_view_get_selected_items(mw->view); 

  if (thumbnail_loaded_list == NULL)
	   return;
	
  if (thumbnail_selected_list == NULL)
 	   return;
 
  gchar* path_to_string = gtk_tree_path_to_string(thumbnail_selected_list->data); 
  
  int n = atoi(path_to_string); 
  const char* selecting_path = g_list_nth_data(thumbnail_loaded_list,n);

  if (strcmp(selecting_path , g_file_get_path(mw->loading_file)) == 0)	
	  return;

  char* base_name = g_path_get_basename( selecting_path );
  image_list_set_current( mw->img_list, base_name );
  char* disp_name = g_filename_display_name( base_name );
	
  mw->loading_file = g_file_new_for_path(selecting_path);
  main_win_open_without_thumbnails_loading(mw);
	
  g_free(base_name);
  g_free(selecting_path);
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
		gtk_widget_modify_bg( mw->aview, GTK_STATE_NORMAL, &pref.bg_full );
		gtk_widget_hide(gtk_paned_get_child1(mw->img_box));
		gtk_widget_hide(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
    }
    else
    {
        mw->full_screen = FALSE;
		gtk_widget_modify_bg( mw->aview, GTK_STATE_NORMAL, &pref.bg );
		gtk_widget_show(gtk_paned_get_child1(mw->img_box));
		gtk_widget_show(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
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
        name = image_list_get_last( mw->img_list );
      
	if( name )
    {
		g_cancellable_cancel(mw->generator_cancellable);
		
        const char* file_path = image_list_get_current_file_path( mw->img_list );
		mw->loading_file = g_file_new_for_path(file_path);
		
		update_title(file_path, mw);
		
        main_win_open_without_thumbnails_loading(mw);
		g_free( file_path ); 
    }
}

void on_next( GtkWidget* bnt, MainWin* mw )
{
    if( image_list_is_empty( mw->img_list ) )
        return;
	
    const char* name = image_list_get_next( mw->img_list );
    if( ! name && image_list_has_multiple_files( mw->img_list ) )
          name = image_list_get_first( mw->img_list );

    if( name )
    {
		g_cancellable_cancel(mw->generator_cancellable);
		
        const char* file_path = image_list_get_current_file_path( mw->img_list );
		mw->loading_file = g_file_new_for_path(file_path);
		
		update_title(file_path, mw);
		
        main_win_open_without_thumbnails_loading(mw);
		g_free( file_path ); 
    }
}

void on_rotate_auto_save( GtkWidget* btn, MainWin* mw )
{
    if(pref.auto_save_rotated)
        on_save(btn,mw);
}

void next_for_slide_show( MainWin* mw )
{
    if( image_list_is_empty( mw->img_list ) )
        return;

    const char* name = image_list_get_next( mw->img_list );
	
	if( ! name && image_list_has_multiple_files( mw->img_list ) )
        name = image_list_get_first( mw->img_list );

    if( name )
    {
        char* file_path = image_list_get_current_file_path( mw->img_list );
				
		GFile* file = g_file_new_for_path(file_path);
		mw->loading_file = file;
		
        main_win_open_without_thumbnails_loading(mw);;
        g_free( file_path );
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

gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw )
{
    if( ! GTK_WIDGET_HAS_FOCUS( widget ) )
        gtk_widget_grab_focus( widget );

    if( evt->type == GDK_BUTTON_PRESS)
    {
        if( evt->button == 3 ) 
        {
            show_popup_menu( mw, evt );
        }
    }
    else if( evt->type == GDK_2BUTTON_PRESS && evt->button == 1 )  
         on_full_screen( NULL, mw );
    return FALSE;
}

/* zoom **********************/
void zoom_out(GtkWidget* widget, MainWin* mw)
{ 
  gtk_image_view_zoom_out(mw->aview);
}

void zoom_in(GtkWidget* widget, MainWin* mw)
{
  gtk_image_view_zoom_in(mw->aview);
}

void fit(GtkWidget* widget, MainWin* mw)
{	
  gtk_image_view_set_fitting(mw->aview, TRUE);
}

void normal_size(GtkWidget* widget, MainWin* mw)
{
  gtk_image_view_set_zoom((mw->aview), 1);
  mw->scale = 1.0;
  update_title (NULL,mw);
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

void hide_thumbnails(GtkWidget* widget, MainWin* mw)
{
   if (mw->thumb_bar_hide)
   {
	   gtk_paned_set_position(mw->img_box,0);
	   mw->thumb_bar_hide = FALSE;
   }
   else
   {
 		gtk_paned_set_position(mw->img_box,170);
	    mw->thumb_bar_hide = TRUE;
   }
}

static void
rotate_pixbuf(MainWin *mw, GdkPixbufRotation angle)
{
	if (!mw->animation)
		return;
	
    GdkPixbuf *result = NULL;
	gdk_flush();
	result = gdk_pixbuf_rotate_simple(GTK_IMAGE_VIEW(mw->aview)->pixbuf,angle);
	
	if(result == NULL)
        return;
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(mw->aview), result);
	
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
	if (!mw->animation)
		return;
    
	GdkPixbuf *result = NULL;
    gdk_flush();
	
	result = gdk_pixbuf_flip(GTK_IMAGE_VIEW(mw->aview)->pixbuf,horizontal);
	
	if(result == NULL)
        return;
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(mw->aview), result);
	
	g_object_unref(result);
	
	mw->modifications ^= (mw->modifications&4)?1+horizontal:2-horizontal;
}

void
rotate_ccw(GtkWidget* widget, MainWin *mw)
{
  	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
}

void
rotate_cw(GtkWidget* widget, MainWin *mw)
{
	rotate_pixbuf(mw ,GDK_PIXBUF_ROTATE_CLOCKWISE);
}

void
flip_v(GtkWidget* widget, MainWin *mw)
{
  flip_pixbuf(mw,FALSE);
}

void
flip_h(GtkWidget* widget ,MainWin * mw)
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
		    ;

		if( next_name )
		{
		    char* next_file_path = image_list_get_current_file_path( mw->img_list );
		    //main_win_open( mw, ZOOM_FIT );
			update_title(next_file_path, mw);
		    g_free( next_file_path );
		}

		image_list_remove ( mw->img_list, name );

		if ( ! next_name )
		{
		    image_list_close( mw->img_list );
		    gtk_window_set_title( (GtkWindow*) mw, _("Image Viewer"));
		}
	    }
        }
	g_free( file_path );
    }
}

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm )
{
    if (!mw->img_list)
		return;
	
    gboolean result1,gdk_save_supported;
    GSList *gdk_formats;
    GSList *gdk_formats_i;
		
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
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(mw->aview)),
								  file_path, type, &err, "quality", tmp, NULL );
    }
    else if( strcmp( type, "png" ) == 0 )
    {
        char tmp[32];
        g_sprintf(tmp, "%d", pref.png_compression);
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(mw->aview)),
								  file_path, type, &err, "compression", tmp, NULL );
    }
    else
        result1 = gdk_pixbuf_save( gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(mw->aview)),
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
   if(image_list_is_empty(mw->img_list))
       return;
	
    char *file, *type;

    if( ! mw->aview )
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
	if(image_list_is_empty(mw->img_list))
       return;
	
    char* file_name = g_build_filename( image_list_get_dir( mw->img_list ),
                                        image_list_get_current( mw->img_list ), NULL );
    GdkPixbufFormat* info;
    info = gdk_pixbuf_get_file_info( file_name, NULL, NULL );
    char* type = gdk_pixbuf_format_get_name( info );

    if(strcmp(type,"jpeg")==0)
    {
        if(!pref.rotate_exif_only)
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
    } else
        main_win_save( mw, file_name, type, pref.ask_before_save );
    g_free( file_name );
    g_free( type );
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
            zoom_in(NULL, mw );
            break;
        case GDK_KP_Subtract:
        case GDK_minus:
            zoom_out(NULL, mw );
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
            rotate_ccw(NULL, mw );
            break;
        case GDK_r:
        case GDK_R:
            rotate_cw(NULL, mw );
            break;
        case GDK_f:
        case GDK_F:
            fit(NULL,mw);
            break;
        case GDK_h:
        case GDK_H:
            flip_h(NULL, mw );
            break;
        case GDK_v:
        case GDK_V:
            flip_v(NULL, mw );
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
            on_preference(NULL,mw);
	        break;
		case GDK_t:
		case GDK_T:
		     hide_thumbnails(NULL,mw);
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

void set_wallpapaer(GtkWidget* widget, MainWin* mw)
{
  set_as_wallpapaer(widget, mw);
}

void show_popup_menu( MainWin* mw, GdkEventButton* evt )
{
    static PtkMenuItemEntry menu_def[] =
    {
        PTK_IMG_MENU_ITEM( N_( "Previous" ), GTK_STOCK_GO_BACK, on_prev, GDK_leftarrow, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Next" ), GTK_STOCK_GO_FORWARD, on_next, GDK_rightarrow, 0 ),
        PTK_SEPARATOR_MENU_ITEM,
        PTK_IMG_MENU_ITEM( N_( "Zoom Out" ), GTK_STOCK_ZOOM_OUT, zoom_out, GDK_minus, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Zoom In" ), GTK_STOCK_ZOOM_IN, zoom_in, GDK_plus, 0 ),
	    PTK_IMG_MENU_ITEM( N_( "Fit Image To Window Size" ), GTK_STOCK_ZOOM_FIT, fit, GDK_F, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Original Size" ), GTK_STOCK_ZOOM_100, normal_size, GDK_G, 0 ),
		PTK_IMG_MENU_ITEM( N_( "Full Screen" ), GTK_STOCK_FULLSCREEN, on_full_screen, GDK_F11, 0 ),
		PTK_IMG_MENU_ITEM( N_( "Slide show" ), GTK_STOCK_DND_MULTIPLE, start_slideshow, GDK_F12, 0 ),
		PTK_IMG_MENU_ITEM( N_( "Set as wallpaper"), GTK_STOCK_INDEX, set_as_wallpapaer, GDK_W, 0),
        PTK_SEPARATOR_MENU_ITEM,
		PTK_IMG_MENU_ITEM( N_( "Crop image"),GTK_STOCK_CUT, crop_image, GDK_C,0),
		PTK_IMG_MENU_ITEM( N_( "Exif Data"),NULL, exif_information, GDK_E,0),
		PTK_SEPARATOR_MENU_ITEM,
		PTK_IMG_MENU_ITEM( N_( "Rotate Counterclockwise" ), "object-rotate-left", rotate_ccw, GDK_L, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Rotate Clockwise" ), "object-rotate-right", rotate_cw, GDK_R, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Flip Horizontal" ), "object-flip-horizontal", flip_h, GDK_H, 0 ),
        PTK_IMG_MENU_ITEM( N_( "Flip Vertical" ), "object-flip-vertical", flip_v, GDK_V, 0 ),
		PTK_SEPARATOR_MENU_ITEM,
		PTK_IMG_MENU_ITEM( N_("Open File"), GTK_STOCK_OPEN, G_CALLBACK(on_open), GDK_O, 0 ),
        PTK_IMG_MENU_ITEM( N_("Save File"), GTK_STOCK_SAVE, G_CALLBACK(on_save), GDK_S, 0 ),
        PTK_IMG_MENU_ITEM( N_("Save As"), GTK_STOCK_SAVE_AS, G_CALLBACK(on_save_as), GDK_A, 0 ),
		PTK_IMG_MENU_ITEM( N_("Delete File"), GTK_STOCK_DELETE, G_CALLBACK(on_delete), GDK_Delete, 0 ),
        PTK_SEPARATOR_MENU_ITEM,
		PTK_IMG_MENU_ITEM( N_("Preferences"), GTK_STOCK_PREFERENCES, G_CALLBACK(on_preference), GDK_P, 0 ),
        PTK_STOCK_MENU_ITEM( GTK_STOCK_ABOUT, on_about ),
        PTK_SEPARATOR_MENU_ITEM,
        PTK_IMG_MENU_ITEM( N_("Quit"), GTK_STOCK_QUIT, G_CALLBACK(gtk_main_quit), GDK_Q, 0 ),
        PTK_MENU_END
		
    };
	
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    GtkMenuShell* popup = (GtkMenuShell*)ptk_menu_new_from_data( menu_def, mw, accel_group );

    gtk_widget_show_all( (GtkWidget*)popup );
    g_signal_connect( popup, "selection-done", G_CALLBACK(gtk_widget_destroy), NULL );
    gtk_menu_popup( (GtkMenu*)popup, NULL, NULL, NULL, NULL, evt->button, evt->time );
}

void crop_image (GtkWidget* widget, MainWin* mw, GdkEventMotion *event)
{
	Win *win;
	win = (Win*)win_new (mw);
	show_window(NULL, win);
}

void on_preference(GtkWidget* widget, MainWin* mw)
{
    Pref *win;
	win = (Pref*)pref_win_new(mw);
	edit_preferences(NULL, win);
}

void on_about( GtkWidget* menu, MainWin* mw )
{
    GtkWidget * about_dlg;
    const gchar *authors[] =
    {
        "洪任諭 Hong Jen Yee <pcman.tw@gmail.com>",
        "Martin Siggel <martinsiggel@googlemail.com>",
        "Hialan Liu <hialan.liu@gmail.com>",
        "Marty Jack <martyj19@comcast.net>",
        "Louis Casillas <oxaric@gmail.com>",
		"Kuleshov Alexander <kuleshovmail@gmail.com>",
        _(" * Refer to source code of EOG image viewer and GThumb"),
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar *translators = _( "translator-credits" );

    gtk_about_dialog_set_url_hook( open_url, mw, NULL);

    about_dlg = gtk_about_dialog_new ();

    gtk_container_set_border_width ( ( GtkContainer*)about_dlg , 2 );
    gtk_about_dialog_set_version ( (GtkAboutDialog*)about_dlg, VERSION );
    gtk_about_dialog_set_name ( (GtkAboutDialog*)about_dlg, _( "GPicView" ) );
    gtk_about_dialog_set_logo( (GtkAboutDialog*)about_dlg, gdk_pixbuf_new_from_file(  PACKAGE_DATA_DIR"/pixmaps/gpicview.png", NULL ) );
    gtk_about_dialog_set_copyright ( (GtkAboutDialog*)about_dlg, _( "Copyright (C) 2007 - 2009" ) );
    gtk_about_dialog_set_comments ( (GtkAboutDialog*)about_dlg, _( "Lightweight image viewer from LXDE project" ) );
    gtk_about_dialog_set_license ( (GtkAboutDialog*)about_dlg, "GPicView\n\nCopyright (C) 2007 Hong Jen Yee (PCMan)\n\nThis program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA." );
    gtk_about_dialog_set_website ( (GtkAboutDialog*)about_dlg, "http://wiki.lxde.org/en/GPicView" );
    gtk_about_dialog_set_authors ( (GtkAboutDialog*)about_dlg, authors );
    gtk_about_dialog_set_translator_credits ( (GtkAboutDialog*)about_dlg, translators );
    gtk_window_set_transient_for( (GtkWindow*) about_dlg, GTK_WINDOW( mw ) );

    gtk_dialog_run( ( GtkDialog*)about_dlg );
    gtk_widget_destroy( about_dlg );
}

static void open_url( GtkAboutDialog *dlg, const gchar *url, gpointer data)
{
    /* FIXME: is there any better way to do this? */
    char* programs[] = { "xdg-open", "gnome-open", "exo-open" };
    int i;
    for(  i = 0; i < G_N_ELEMENTS(programs); ++i)
    {
        gchar* open_cmd = NULL;
        if( (open_cmd = g_find_program_in_path( programs[i] )) )
        {
             char* argv [3];
             argv [0] = programs[i];
             argv [1] = (gchar *) url;
             argv [2] = NULL;
             g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
             g_free( open_cmd );
             break;
        }
    }
}

void printing_image(MainWin* mw)
{
  print_pixbuf(mw);
}

void exif_information(GtkWidget* widget, MainWin* mw)
{
  	ExifWin *win;
	win = (ExifWin*)exif_win_new (mw);
	show_exif_window(widget,win);
}
