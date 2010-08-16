/***************************************************************************
 *   Copyright (C) 2007 by PCMan (Hong Jen Yee)   *
 *   pcman.tw@gmail.com   *
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
#ifndef MAINWIN_H
#define MAINWIN_H

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtkimageview/gtkimageview.h>
#include <gtkimageview/gtkanimview.h>
#include <gtkimageview/gtkimagescrollwin.h>

#include "image-list.h"
#include "loader.h"
#include "jhead.h"
#include "utils.h"

#define LOAD_BUFFER_SIZE 65536 

/**
    @author PCMan (Hong Jen Yee) <pcman.tw@gmail.com>
*/

#define MAIN_WIN_TYPE        (main_win_get_type ())
#define MAIN_WIN(obj)        (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAIN_WIN_TYPE, MainWin))
#define MAIN_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MAIN_WIN_TYPE, MainWinClass))
#define IS_MAIN_WIN(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAIN_WIN_TYPE))
#define IS_MAIN_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MAIN_WIN_TYPE))
#define MAIN_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MAIN_WIN_TYPE, MainWinClass))

typedef enum
{
     ZOOM_NONE = 0,
     ZOOM_FIT,
     ZOOM_ORIG,
     ZOOM_SCALE
} ZoomMode;

enum 
{ 
  COL_DISPLAY_NAME , 
  COL_PIXBUF, 
  NUM_COLS  
};

typedef struct _MainWinClass
{
    GtkWindowClass parent_class;
} MainWinClass;

typedef struct _MainWin
{
    GtkWindow parent;
	GtkWidget* scroll;
	GtkWidget* box;
	GtkWidget* align;
	GtkWidget* toolbar_box;
    GtkUIManager *uimanager;
	GtkAccelGroup *accels;
	GtkWidget*  img_box;
	gint max_width;
    gint max_height;
	gint current_image_height;
    gint current_image_width;
	guint8 modifications;
	gboolean full_screen;
	gboolean slideshow;
	guint ss_source_tag;
	gint ss_timeout;
	double scale;
	ImageList* img_list;
	guint      rotation_angle;
	char* dir_path;
	GtkAdjustment* adj;
	GdkPixbuf* pixbuf;
	GdkPixbuf* normal_pix;
	char*      thumbnail_title;
	GFileMonitor* monitor;
	GtkAnimView* aview;
    GdkPixbufAnimation* animation;
	guint loaded_img;

	//Thumbnail
	GtkIconView*  view;
	ImageList* thmb_list;
    GtkWidget* thumb_box;
	GtkWidget* thumbnail_scroll;
	GtkListStore* model;
	GtkTreeIter* iter;
	GtkVPaned *box1;
	GList     *displaing_thumbnail_list;
	gint      prev_pos;
	gboolean  thumb_bar_hide;
	
	//Drawing area
	GtkWidget* image;
	gdouble sub_x;
    gdouble sub_y;
    gdouble sub_width;
    gdouble sub_height;

    gboolean drawing_rectangle;
    gboolean do_redraw;
    gdouble start_x;
    gdouble start_y;
	
	gboolean loaded;
	GdkGC *gc;
	
	GFile* loading_file;
	GCancellable* generator_cancellable;
	GCancellable* thumbnail_cancellable;

} MainWin;

typedef struct _JobParam
{
  GFile* file;
  GCancellable* generator_cancellable;
  MainWin* mw;
  GIOSchedulerJob* job;
}JobParam;

GtkWidget* main_win_new();

gboolean main_win_open( MainWin* mw );

gboolean main_win_open_without_thumbnails_loading(MainWin* mw);

void main_win_close( MainWin* mw );

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm );

void main_win_show_error( MainWin* mw, const char* message );

void main_win_fit_size( MainWin* mw, int width, int height, gboolean can_strech, GdkInterpType type );

void main_win_fit_window_size( MainWin* mw, gboolean can_strech, GdkInterpType type );

void main_win_center_image( MainWin* mw );

gboolean main_win_open_thumbnails_loading(MainWin* mw);

gboolean main_win_scale_image(  MainWin* mw, double new_scale, GdkInterpType type );

void build_thumbnails(GtkWidget* widget, MainWin *mw);

void gtk_view_set_static (GtkAnimView *aview, GdkPixbuf *pixbuf);

void on_scroll_wheel(GtkImageView *view,GdkScrollDirection direction, gpointer user_data);

void list_load(MainWin* mw);

void load_list(MainWin* mw);

int selecting(MainWin* mw);

GType main_win_get_type();

#endif
