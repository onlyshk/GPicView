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
#include <gdk/gdkx.h>
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
#include "crop.h"
#include "utils.h"
#include "wallpaper.h"
#include "exifdialog.h"
#include "printing.h"
#include "about.h"
#include "screenshot-dlg.h"

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
static void on_next( GtkWidget* widget, MainWin* mw );
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
static gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw );
static void thumbnail_selected(GtkWidget* widget, MainWin* mw);
static void loading(JobParam* param);
static void on_rotate_auto_save( GtkWidget* btn, MainWin* mw );
static void set_wallpapaer(GtkWidget* widget, MainWin* mw);
static void crop_image (GtkWidget* widget, MainWin* mw, GdkEventMotion *event);
static void draw_rectangle(GtkWidget* widget, MainWin* mw);
static void printing_image( GtkWidget* widget, MainWin* mw);
static void select_nth_item(GtkIconView *iconview, gpointer user_data);
static void select_prev_item(GtkIconView *iconview, gpointer user_data) ;
static void take_screenshot(GtkWidget* widget, MainWin* mw);
static gboolean job_func2(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);

static gboolean set_image(JobParam* param);
static gboolean set_image_by_click(GtkWidget* widget, MainWin* mw);
static gboolean load(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static void load_thumbnails(JobParam* param);
static gboolean job_func(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static gboolean job_func1(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data);
static gboolean set_thumbnails(MainWin* mw);
static void exif_information(GtkWidget* widget, MainWin* mw);

/* signal handlers */
static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key);
static void on_size_allocate( GtkWidget* widget, GtkAllocation    *allocation );
static gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state );

// Begin of GObject-related stuff
G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW )

//
// Main window class initialize
//
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

//
// Main window class finalize
//
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
    return (GtkWidget*)g_object_new ( MAIN_WIN_TYPE, NULL );
}

gchar *menu_ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='Go Back'/>"
        "    <menuitem action='Go Forward'/>"
               "<separator action='Sep1'/>"
        " <menu action ='Zoom'>"
        "    <menuitem action='Zoom out'/>"
        "    <menuitem action='Zoom in'/>"
        "    <menuitem action='ZoomFit'/>"
        "    <menuitem action='ZoomNormal'/>"
        " </menu>"
        "    <menuitem action='FullScreen'/>"
        "    <menuitem action='SlideShow'/>"
                "<separator action='Sep2' />"
        " <menu action ='Rotate and Flip' >"
        "    <menuitem action='ImageRotate1'/>"
        "    <menuitem action='ImageRotate2'/>"
        "    <menuitem action='ImageRotate3'/>"
        "    <menuitem action='ImageRotate4'/>"
        " </menu>"
                "<separator action='Sep3' />"
        " <menu action = 'Tools'>"
        "     <menuitem  action='Show exif data'/>"
        "     <menuitem  action='Crop Image'/>"
        "     <menuitem  action='Take screenshot'/>"
        " </menu>"
                "<separator action='Sep4' />"
        "     <menuitem  action='Open File'/>"
        "     <menuitem  action='Save File'/>"
        "     <menuitem  action='Save as File'/>"
        "     <menuitem  action='Print image' />"
        "     <menuitem  action='Delete File'/>"
                "<separator  action='Sep5' />"
        "     <menuitem   action='Preferences'/>"
        "     <menuitem   action='About'/>"
        "<separator action='Sep5' />"
        "     <menuitem   action='Quit' />"
        "  </popup>"
        "</ui>";

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
	{"Zoom",GTK_STOCK_ZOOM_IN,"Zoom out",NULL,"Zoom", NULL
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
	{"Tools",NULL,"Tools",
	NULL,"Tools", NULL
	},
	{"Show exif data",NULL,"Show exif data",
	"<control>e","Show exif data", G_CALLBACK(exif_information)
	},
	{"Crop Image",GTK_STOCK_CUT,"Crop Image",
	"<control>c","Crop Image", G_CALLBACK(crop_image)
	},
	{"Take screenshot",NULL,"Take screenshot",
	"<control>t","Take screenshot", G_CALLBACK(take_screenshot)
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
	{"About",GTK_STOCK_ABOUT,"About",
	 "<control>h", "About", G_CALLBACK(on_about)
	},
	{
	  "Quit",GTK_STOCK_QUIT,"Quit",
	   "<control>q", "Quit", G_CALLBACK(gtk_main_quit)
	}
};

static const GtkActionEntry entries1[] = {
	{"Rotate and Flip","object-rotate-left","Rotate and FLip",
	NULL,"Rotate image", NULL},
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

//
// Main window init
//
void main_win_init( MainWin*mw )
{	
	GError* error = NULL;
		
	mw->aview  =    GTK_ANIM_VIEW(gtk_anim_view_new());
	mw->generator_cancellable = g_cancellable_new();
    mw->thumbnail_cancellable = g_cancellable_new();
	mw->img_list = image_list_new();
	
	mw->align = gtk_alignment_new( 0.5, 0,0,0);
	mw->thumb_bar_hide = TRUE;
	
    gtk_window_set_title( (GtkWindow*)mw, _("Image Viewer"));
    gtk_window_set_icon_from_file( (GtkWindow*)mw, PACKAGE_DATA_DIR"/pixmaps/gpicview.png", NULL );
    gtk_window_set_default_size( (GtkWindow*)mw, 720, 480 );
	gtk_window_set_position((GtkWindow*)mw, GTK_WIN_POS_CENTER);
   
	mw->box = gtk_vbox_new(FALSE, 0);
	mw->toolbar_box = gtk_hbox_new(FALSE, 0);
	mw->img_box =   gtk_hpaned_new ();
    mw->thumb_box = gtk_vbox_new (FALSE,0);
	
    mw->model = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF);
	mw->view = GTK_ICON_VIEW(gtk_icon_view_new());
	 
    mw->thumbnail_scroll = gtk_scrolled_window_new (mw->adj, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mw->thumbnail_scroll),GTK_POLICY_AUTOMATIC, 
                                    GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (mw->thumbnail_scroll), GTK_SHADOW_IN);

	mw->scroll = gtk_image_scroll_win_new(GTK_IMAGE_VIEW(mw->aview));
	gtk_paned_add1(GTK_PANED(mw->img_box),mw->thumbnail_scroll);
	
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(mw->thumbnail_scroll), GTK_WIDGET(mw->view));
	gtk_paned_add2(GTK_PANED(mw->img_box),mw->scroll);	
		
	gtk_box_pack_start(GTK_BOX(mw->box), mw->img_box, TRUE, TRUE,0);
	
	actions = gtk_action_group_new ("Actions");
	rotation_actions = gtk_action_group_new("Rotate_Actions");
	
	mw->uimanager = gtk_ui_manager_new();
	
	gtk_ui_manager_insert_action_group (mw->uimanager, actions, 0);
	gtk_ui_manager_insert_action_group (mw->uimanager, rotation_actions,1);
	gtk_action_group_add_actions (actions, entries, n_entries, GTK_WINDOW(mw));
    gtk_action_group_add_actions (rotation_actions, entries1, n_entries1, GTK_WINDOW(mw));
	gtk_window_add_accel_group (GTK_WINDOW (mw), 
				                gtk_ui_manager_get_accel_group (mw->uimanager));
	
	if (!gtk_ui_manager_add_ui_from_string (mw->uimanager, ui_info, -1, &error))
	{
	  g_message ("building menus failed: %s", error->message);
	  g_error_free (error);
	}
	
	if (!gtk_ui_manager_add_ui_from_string (mw->uimanager, menu_ui_info, -1, &error))
	{
	  g_message ("building menus failed: %s", error->message);
	  g_error_free (error);
	}
    
	gtk_widget_set_size_request(mw->toolbar_box, 720,40);
	
	gtk_box_pack_end((GtkBox*)mw->toolbar_box, (GtkWidget*)gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"), TRUE, TRUE,0);
	gtk_container_add( (GtkContainer*)mw->align, mw->toolbar_box);
	
	gtk_toolbar_set_style(GTK_TOOLBAR(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar")), GTK_TOOLBAR_ICONS);
	
	g_signal_connect( mw->box, "button-press-event", G_CALLBACK(on_button_press), mw );
	gtk_box_pack_end( (GtkBox*)mw->box, mw->align, FALSE, TRUE, 0 );
	
    gtk_toolbar_set_style(GTK_TOOLBAR(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar")), GTK_TOOLBAR_ICONS);
			
	g_signal_connect( mw->box, "button-press-event", G_CALLBACK(on_button_press), mw );
	g_signal_connect( mw->view ,"selection-changed", G_CALLBACK(thumbnail_selected), mw);
	g_signal_connect(GTK_IMAGE_VIEW(mw->aview), "mouse-wheel-scroll", G_CALLBACK(on_scroll_wheel), mw);
   
	gtk_paned_set_position(GTK_PANED(mw->img_box),220);
	gtk_icon_view_set_text_column(GTK_ICON_VIEW(mw->view),0);
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(mw->view), 1);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(mw->view), GTK_SELECTION_SINGLE);
	
	gtk_widget_modify_bg (GTK_WIDGET(mw->aview), GTK_STATE_NORMAL, &pref.bg);	
   
	gtk_container_add(GTK_CONTAINER(mw), mw->box);
 
	gtk_widget_show_all( GTK_WIDGET(mw) );
}

void on_scroll_wheel(GtkImageView *view,GdkScrollDirection direction, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
	
   if (direction == GDK_SCROLL_UP)
   {
       on_prev(NULL, mw);
   }
	
   if (direction == GDK_SCROLL_DOWN)
   {
       on_next(NULL, mw);
   }
}

gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt )
{
    gtk_widget_destroy( widget );
    return TRUE;
}

//
// Update title of main window
//
static void update_title(const char *filename, MainWin *mw )
{
    static char fname[50];
    static int wid, hei;

    char buf[100];
    if(filename != NULL)
    {
      strncpy(fname, filename, 49);
      fname[49] = '\0';
    }
    snprintf(buf, 100, "%s", fname);
    gtk_window_set_title( (GtkWindow*)mw, buf );
    return;
}

//
// Loading normal image
//
void loading(JobParam* param)
{	
	GInputStream* input_stream = NULL;
	input_stream = G_INPUT_STREAM(g_file_read(param->file, param->generator_cancellable, NULL));
	param->mw->animation = load_animation_from_stream(G_INPUT_STREAM(input_stream), param->generator_cancellable);	
	
	g_input_stream_close(input_stream, param->generator_cancellable, NULL);
	g_object_unref (input_stream);
}

//
// Loading normal thumbnails
//
void load_thumbnails(JobParam* param)
{				
	GInputStream* input_stream = NULL;
	GtkTreeIter iter;
	GFile* file = NULL;
	
	thumbnail_loaded_list = NULL;
	param->mw->disp_list = NULL;
	thumbnail_path_list = NULL;
	int i = 0;
	int n = g_list_length((GList*)param->mw->img_list) - 1;
	
	for (i; i < n; i++)
	{		
	  GdkPixbuf* thumb_pixbuf;
	  GdkPixbuf* thumb_pixbuf1;

	  if (i == 0)
	      file =  g_file_new_for_path (image_list_get_first_file_path( param->mw->img_list ));
      
	  thumbnail_loaded_list = g_list_append(thumbnail_loaded_list,
											 image_list_get_current_file_path( param->mw->img_list ));
		
	  input_stream = G_INPUT_STREAM(g_file_read((GFile*)file, (GCancellable*)param->generator_cancellable, NULL));
	  
	  thumb_pixbuf = load_image_from_stream(G_INPUT_STREAM(input_stream), param->mw->thumbnail_cancellable);

	  thumb_pixbuf = scale_pix(thumb_pixbuf,128);
      
	  param->mw->disp_list = g_list_append(param->mw->disp_list, thumb_pixbuf);
	  thumbnail_path_list = g_list_append(thumbnail_path_list,g_file_get_basename (file));
	  
      if (!param->mw->img_list->current->next )
	      image_list_get_first(param->mw->img_list);
	  else
	      file = g_file_new_for_path(image_list_get_next_file_path(param->mw->img_list));

	  g_input_stream_close(input_stream, param->mw->thumbnail_cancellable, NULL);	
	}
	
	g_object_unref (input_stream);
}

gboolean main_win_open(MainWin* mw)
{        		
	//mw->disp_list                  = NULL;
	thumbnail_loaded_list          = NULL;	
	thumbnail_selected_list        = NULL;
	thumbnail_path_list            = NULL;

	g_cancellable_reset(mw->generator_cancellable);
	g_cancellable_reset(mw->thumbnail_cancellable);
	
	JobParam* param;
	param =  g_new (JobParam, 1);
	
    param->file                  = mw->loading_file;
	param->generator_cancellable = mw->generator_cancellable;
	param->mw                    = mw;
	
	g_io_scheduler_push_job (job_func1, param, NULL, G_PRIORITY_DEFAULT, mw->generator_cancellable);
	
	return TRUE;
}

gboolean main_win_open_without_thumbnails_loading(MainWin* mw)
{        			
	g_cancellable_reset(mw->generator_cancellable);
	
	JobParam* param;
	param =  g_new (JobParam, 1);
	
    param->file                  = mw->loading_file;
	param->generator_cancellable = mw->generator_cancellable;
	param->mw                    = mw;
	
	g_io_scheduler_push_job (job_func1, param, NULL , G_PRIORITY_DEFAULT, mw->generator_cancellable);

	return TRUE;
}

//
// Job func - load thumbnails
//
gboolean job_func(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data)
{
	JobParam* param = (JobParam*)user_data;
	
	load_thumbnails(param);
	g_io_scheduler_job_send_to_mainloop(job, (GSourceFunc)set_thumbnails, param->mw, NULL);
	
	return FALSE;
}

//
// Job func - load image
//
gboolean job_func1(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data)
{
	JobParam* param = (JobParam*)user_data;
	
	loading(param);

	g_io_scheduler_job_send_to_mainloop(job, (GSourceFunc)set_image, param, NULL);
	
	g_io_scheduler_push_job (job_func, param,  NULL, G_PRIORITY_DEFAULT,  param->mw->thumbnail_cancellable);

	return FALSE;
}

//
// Job func - load image list
//
gboolean job_func2(GIOSchedulerJob *job, GCancellable *cancellable, gpointer user_data)
{
	MainWin* mw = MAIN_WIN(user_data);
	
	list_load(mw);
	
	g_io_scheduler_job_send_to_mainloop(job, (GSourceFunc)load_list, mw, NULL);
	
	return FALSE;
}

//
// Set loading thumbnails in GtkIconView
//
gboolean set_thumbnails(MainWin* mw)
{  	
  GtkTreeIter* iter;
  GdkPixbuf* pixbuf;

  gtk_list_store_clear(mw->model);
  gtk_icon_view_set_model(mw->view,GTK_TREE_MODEL(mw->model));	
  
  int i = 0;
  int c = 0;
	
  int n = g_list_length(mw->disp_list);
  
  for (i; i < n; ++i)
  {
	char* paths = g_list_nth_data(thumbnail_path_list,i);

	if ((strcmp(paths,  g_file_get_basename (mw->loading_file)) == 0))
	{
		  c = i;
	}
	    
	pixbuf =  (GdkPixbuf*)g_list_nth_data(mw->disp_list, i);
	
	gtk_list_store_append(mw->model, (GtkTreeIter*)&iter);
	gtk_list_store_set(mw->model, (GtkTreeIter*)&iter, COL_DISPLAY_NAME, paths, 
					   COL_PIXBUF, (GdkPixbuf*)pixbuf, -1);
  }
         
  char tmp[32];
  g_sprintf(tmp, "%d", c);
  
  gtk_tree_model_get_iter_from_string ((GtkTreeModel*)mw->model, (GtkTreeIter*)&iter, tmp );
	
  GtkTreePath *path =  gtk_tree_model_get_path (GTK_TREE_MODEL(mw->model), (GtkTreeIter*)&iter);
  gtk_icon_view_select_path(GTK_ICON_VIEW(mw->view),path); 
  gtk_tree_path_free(path);
}

gboolean set_image(JobParam* param)
{
	gtk_anim_view_set_anim (param->mw->aview, param->mw->animation);
}

gboolean set_image_by_click(GtkWidget* widget, MainWin* mw)
{
   gtk_anim_view_set_anim (mw->aview, mw->animation);
}

void list_load(MainWin* mw)
{	
	char* file =  g_file_get_path(mw->loading_file);
	
  	mw->dir_path = g_path_get_dirname( file );
	image_list_open_dir(mw->img_list, mw->dir_path, mw->generator_cancellable, NULL );
	
	g_free(file);
	
	mw->loaded = TRUE;
}

static void
directory_monitor_changed (GFileMonitor      *monitor,
                           GFile             *file,
                           GFile             *other_file,
                           GFileMonitorEvent  event,
                           gpointer           data)
{
  MainWin* mw = MAIN_WIN(data);
	
  switch (event)
  {
	  G_FILE_MONITOR_EVENT_DELETED:
		break;
  }
}

void load_list(MainWin* mw)
{
	main_win_open (mw);	
}

//
// Open image dialog
//
void on_open( GtkWidget* widget, MainWin* mw )
{	
	char* file = NULL; 
	
	mw->loaded = FALSE;
	
	if ((file = get_open_filename( GTK_WINDOW(mw))) == NULL)  	
	     return;

	mw->loading_file = g_file_new_for_path(file);
    
	char* file_path =  g_file_get_path(mw->loading_file);
	char* dir       =  g_path_get_dirname( file_path );
	
	g_cancellable_cancel(mw->thumbnail_cancellable);
	g_cancellable_reset (mw->thumbnail_cancellable);
    
	if (mw->dir_path == NULL)
	{
		g_io_scheduler_push_job (job_func2, mw, NULL, G_PRIORITY_DEFAULT, mw->thumbnail_cancellable);
		
		update_title(file_path, mw);
    }
	else
	{
		if (strcmp (dir, mw->dir_path) == 0)
 		{ 		
			mw->loaded = TRUE;
			
			GtkTreeIter iter;
			int i = 0;
			int c = 0;
			int n = g_list_length (mw->disp_list);
			
			for (i; i< n; i++)
			{
			   char* paths = g_list_nth_data(thumbnail_path_list,i);

			   if ((strcmp(paths, g_file_get_basename (mw->loading_file))) == 0)
			   {
		          c = i;
				  			
				  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(g_list_nth_data(thumbnail_loaded_list,c), NULL);
			      gtk_image_view_set_pixbuf(mw->aview,  pixbuf, NULL);
				   
				  g_object_unref(pixbuf);
				  g_free(paths);
	           }
             }
         
             char tmp[32];
             g_sprintf(tmp, "%d", c);
	
		     gtk_tree_model_get_iter_from_string ((GtkTreeModel*)mw->model, (GtkTreeIter*)&iter, tmp );

             GtkTreePath *path =  gtk_tree_model_get_path (GTK_TREE_MODEL(mw->model), (GtkTreeIter*)&iter);
             gtk_icon_view_select_path(GTK_ICON_VIEW(mw->view),path); 
             gtk_tree_path_free(path);
		}
		else
		{
		  mw->loaded = FALSE;
			
		  image_list_close (mw->img_list);
		
		  g_list_free(mw->disp_list);
		  g_list_free(thumbnail_selected_list);
		  g_list_free(thumbnail_path_list);
		  g_list_free(thumbnail_loaded_list);
						
	      mw->disp_list = NULL;
	      thumbnail_selected_list= NULL;
          thumbnail_path_list= NULL;
          thumbnail_loaded_list = NULL;

	      g_io_scheduler_push_job (job_func2, mw, NULL, G_PRIORITY_DEFAULT, mw->thumbnail_cancellable);
		  update_title(file_path, mw);
		}
	}
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
	
  mw->loading_file = g_file_new_for_path (selecting_path);
	
  mw->animation = gdk_pixbuf_animation_new_from_file(selecting_path, NULL);
  set_image_by_click(NULL, mw);
 	
  g_free(base_name);
  g_free(path_to_string);
  g_free(disp_name);
	
  g_list_free(thumbnail_selected_list);
}

void printing_image(GtkWidget* widget, MainWin* mw)
{
   print_pixbuf(NULL, mw);
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
    MainWin* mw = MAIN_WIN(widget);
    mw->ss_source_tag = 2;
    if( state->new_window_state == GDK_WINDOW_STATE_FULLSCREEN )
    {
        mw->full_screen = TRUE;
		gtk_widget_modify_bg(GTK_WIDGET(mw->aview), GTK_STATE_NORMAL, &pref.bg_full );
	    gtk_widget_hide(gtk_paned_get_child1(GTK_PANED(mw->img_box)));
		gtk_widget_hide(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
    }
    else
    {
        mw->full_screen = FALSE;
		gtk_widget_modify_bg( GTK_WIDGET(mw->aview), GTK_STATE_NORMAL, &pref.bg );
		gtk_widget_show(gtk_paned_get_child1(GTK_PANED(mw->img_box)));
		gtk_widget_show(gtk_ui_manager_get_widget(mw->uimanager, "/ToolBar"));
		g_source_remove (mw->ss_source_tag);
		mw->ss_source_tag = 0;
    }

    return TRUE;
}

//
// Selecting next GtkIconView item
//
static void select_next_item(GtkIconView *iconview, gpointer user_data) 
{ 
	MainWin* mw = MAIN_WIN(user_data);
	
	if (mw->loaded == FALSE)
		return;
	
    GtkTreePath * path = gtk_icon_view_get_selected_items(mw->view)->data;
	char* path_to_string = gtk_tree_path_to_string(path);

    int image_count = g_list_length (mw->img_list->list);
	
	int n = atoi(path_to_string);

	if (n == image_count - 1)
	    path = gtk_tree_path_new_first();
	else
	    gtk_tree_path_next(path); 
	
    gtk_icon_view_select_path(GTK_ICON_VIEW(iconview),path); 
    
	gtk_tree_path_free (path);
	g_free(path_to_string);
} 

//
// Selecting previos GtkIconView item
//
static void select_prev_item(GtkIconView *iconview, gpointer user_data) 
{ 
	MainWin* mw = MAIN_WIN(user_data);
	
    GtkTreePath * path = gtk_icon_view_get_selected_items(mw->view)->data;
	char* path_to_string = gtk_tree_path_to_string(path);
	
    int image_count = g_list_length (mw->img_list->list);
	
	int n = atoi(path_to_string);

	if (n == 0)
	{
	   for (image_count - 1; image_count - 1 > 0; image_count--)
	   {
	     gtk_tree_path_next(path); 
	   }
	}
	else
	    gtk_tree_path_prev(path); 
	
    gtk_icon_view_select_path(GTK_ICON_VIEW(iconview),path); 
  
	gtk_tree_path_free (path);
	g_free(path_to_string);
} 

//
// Previos image
//
void on_prev( GtkWidget* widget, MainWin* mw )
{		
	if (mw->loaded == FALSE)
		return;
	
	if( image_list_is_empty( mw->img_list ) )
        return;
	
	const char* name = image_list_get_prev( mw->img_list );
    if( ! name && image_list_has_multiple_files( mw->img_list ) )
	{
	    name = image_list_get_last( mw->img_list );
	        
		mw->adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mw->thumbnail_scroll));
        gtk_adjustment_set_value (GTK_ADJUSTMENT(mw->adj), mw->adj->upper);
	}
	else
	{
        mw->adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mw->thumbnail_scroll));
        gtk_adjustment_set_value (GTK_ADJUSTMENT(mw->adj), mw->adj->value - 60);
	}
      
	if( name )
    {	
        char* file_path = image_list_get_current_file_path( mw->img_list );
		
		gtk_image_view_set_pixbuf(mw->aview,g_list_nth_data(mw->disp_list,0),NULL);
		
		update_title(file_path, mw);
				
		select_prev_item(GTK_ICON_VIEW(mw->view), mw);
		
		g_object_unref(mw->animation);
		g_free(file_path);
    }
}

//
// Next image
//
void on_next( GtkWidget* widget, MainWin* mw )
{
	if (mw->loaded == FALSE)
		return;
	
    if( image_list_is_empty( mw->img_list ) )
        return;
		
    const char* name = image_list_get_next( mw->img_list );
    if( ! name && image_list_has_multiple_files( mw->img_list ) )
	{
	    name = image_list_get_first( mw->img_list );
		
		mw->adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mw->thumbnail_scroll));
        gtk_adjustment_set_value (GTK_ADJUSTMENT(mw->adj), mw->adj->lower);	
	}
	else
	{
        mw->adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(mw->thumbnail_scroll));
        gtk_adjustment_set_value (GTK_ADJUSTMENT(mw->adj), mw->adj->value + 60);	
	}

    if( name )
	{		
        char* file_path = image_list_get_current_file_path( mw->img_list );
						    
		gtk_image_view_set_pixbuf(mw->aview,g_list_nth_data(mw->disp_list,0),NULL);
		
		update_title(file_path, mw);
		
		select_next_item(GTK_ICON_VIEW(mw->view), mw);
		
		g_object_unref(mw->animation);
		g_free(file_path);
    }
}

void on_rotate_auto_save( GtkWidget* btn, MainWin* mw )
{
    if(pref.auto_save_rotated)
        on_save(btn,mw);
}

//
// Slide show
//
void next_for_slide_show( MainWin* mw )
{        
    if( image_list_is_empty( mw->img_list ) )
        return;

    const char* name = image_list_get_next( mw->img_list );

	if( ! name && image_list_has_multiple_files( mw->img_list ) )
        name = image_list_get_first( mw->img_list );
	
	if (name)
	{
	  char* file_path = image_list_get_current_file_path( mw->img_list );
      mw->loading_file = g_file_new_for_path(file_path);

	  main_win_open_without_thumbnails_loading(mw);
		
	  g_free(file_path);
	}
}

void
start_slideshow(GtkWidget* btn, MainWin* mw)
{	
	  gtk_window_fullscreen (GTK_WINDOW(mw));
      mw->ss_source_tag =    g_timeout_add_seconds (2,
                                                   (GSourceFunc)next_for_slide_show,mw); 
	  gtk_widget_hide(mw->toolbar_box);
}

gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw )
{
    if( ! GTK_WIDGET_HAS_FOCUS( widget ) )
        gtk_widget_grab_focus( widget );

    if( evt->type == GDK_BUTTON_PRESS)
    {
        if( evt->button == 3 ) 
        {
            gtk_menu_popup( (GtkMenu*)gtk_ui_manager_get_widget(mw->uimanager, "/PopupMenu"), NULL, NULL, NULL, NULL, evt->button, evt->time );        }
    }
    else if( evt->type == GDK_2BUTTON_PRESS && evt->button == 1 )  
         on_full_screen( NULL, mw );
    return FALSE;
}

void zoom_out(GtkWidget* widget, MainWin* mw)
{ 
  gtk_image_view_zoom_out(GTK_IMAGE_VIEW(mw->aview));
}

void zoom_in(GtkWidget* widget, MainWin* mw)
{
  gtk_image_view_zoom_in(GTK_IMAGE_VIEW(mw->aview));
}

void fit(GtkWidget* widget, MainWin* mw)
{	
  gtk_image_view_set_fitting(GTK_IMAGE_VIEW(mw->aview), TRUE);
}

void normal_size(GtkWidget* widget, MainWin* mw)
{
  gtk_image_view_set_zoom(GTK_IMAGE_VIEW(mw->aview), 1);
  mw->scale = 1.0;
  update_title (NULL,mw);
}

void on_full_screen( GtkWidget* btn, MainWin* mw )
{
    if( ! mw->full_screen )
	{
        gtk_window_fullscreen( (GtkWindow*)mw );
	    gtk_widget_hide(mw->toolbar_box);
	}
	else
	{
        gtk_window_unfullscreen( (GtkWindow*)mw );
	    gtk_widget_show(mw->toolbar_box);
	}
}

void hide_thumbnails(GtkWidget* widget, MainWin* mw)
{
   if (mw->thumb_bar_hide)
   {
	   gtk_paned_set_position(GTK_PANED(mw->img_box),0);
	   mw->thumb_bar_hide = FALSE;
   }
   else
   {
 		gtk_paned_set_position(GTK_PANED(mw->img_box),170);
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
	
	gtk_anim_view_set_static(GTK_ANIM_VIEW(mw->aview), (GdkPixbuf*)result);
	
	g_object_unref(result);
	
	mw->current_image_width = gdk_pixbuf_get_width ((GdkPixbuf*)result);
    mw->current_image_height = gdk_pixbuf_get_height ((GdkPixbuf*)result);
	
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

	    //if( g_unlink( file_path ) != 0 )
	    //main_win_show_error( mw, g_strerror(errno) );
	    //else
	    // {
		const char* next_name = image_list_get_next( mw->img_list );
		
		if( ! next_name )
		     next_name = image_list_get_prev( mw->img_list );

		if( next_name )
		{
			char* next_file_path = image_list_get_current_file_path( mw->img_list );
			
			image_list_remove ( mw->img_list, name );

            GList* elements = gtk_icon_view_get_selected_items (mw->view);
			
			GtkTreeIter iter;
			GtkTreePath* path = g_list_nth_data(elements,0);
			gtk_tree_model_get_iter((GtkTreeModel*)mw->model, &iter, path);
            gtk_list_store_remove(mw->model, &iter);
			
			g_list_free(elements);
			gtk_tree_path_free(path);
		}
	
		if ( ! next_name )
		{
		    image_list_close( mw->img_list );
		    gtk_window_set_title( (GtkWindow*) mw, _("Image Viewer"));
		}
	}
     // }
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
            image_list_open_dir( mw->img_list, dir, mw->thumbnail_cancellable, NULL );
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

void crop_image (GtkWidget* widget, MainWin* mw, GdkEventMotion *event)
{
	Win *win;
	win = (Win*)win_new (mw);
	show_window(NULL, win);
}

void on_preference(GtkWidget* widget, MainWin* mw)
{
	edit_preferences( (GtkWindow*)mw );
}

void exif_information(GtkWidget* widget, MainWin* mw)
{
  	ExifWin *win;
	win = (ExifWin*)exif_win_new (mw);
	show_exif_window(widget,win);
}

static void take_screenshot(GtkWidget* widget, MainWin* mw)
{
    ScreenshotDlgWin* win;
    win = (ScreenshotDlgWin*)screenshotdlg_new(mw);
    show_screenshot_window(NULL, win);
}
