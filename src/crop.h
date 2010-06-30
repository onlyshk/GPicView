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

#ifndef CROP_H
#define CROP_H


#include "mainwin.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtkimageview/gtkimageview.h>
#include <gtkimageview/gtkanimview.h>
#include <gtkimageview/gtkimagescrollwin.h>

#define CHECK_SIZE  8
#define CHECK_LIGHT 0x00cccccc
#define CHECK_DARK  0x00808080

#define CROP_WIN_TYPE            (crop_win_get_type())
#define CROP_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CROP_WIN_TYPE, Win))
#define CROP_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CROP_WIN_TYPE, WinClass))
#define IS_CROP_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CROP_WIN_TYPE))
#define IS_CROP_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CROP_WIN_TYPE))
#define CROP_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CROP_WIN_TYPE, WinClass))

typedef struct _Win        Win;
typedef struct _WinClass   WinClass;

struct _Win
{
    GObject parent;
	MainWin*   mw;
	GtkWindow* crop_window;
	GtkVBox   *box;
    GtkHBox   *hbox;
    GtkWidget* prev_image;
    GtkWidget* scroll;
    gdouble zoom;
    gdouble height;
    gdouble width;
    GdkGC *gc;
    GtkWidget *image;
    GtkSpinButton *spin_x;
    GtkSpinButton *spin_y;
    GtkSpinButton *spin_width;
    GtkSpinButton *spin_height;
    gdouble sub_x;
    gdouble sub_y;
    gdouble sub_width;
    gdouble sub_height;
    gboolean drawing_rectangle;
    gboolean do_redraw;
    gdouble start_x;
    gdouble start_y;
	GtkButton* crop_button;
	GdkPixbuf *original;
	GdkPixbuf *preview;
	
	GdkPixbuf *preview_pixbuf;
	GdkRectangle area;
};

typedef struct _WinClass {
    GObjectClass parent_class;
};

GType crop_win_get_type(void);

GObject  *crop_new (MainWin *mw);

void show_window(GtkWidget* widget, Win *win);

void adapt_image(GdkPixbuf* ori_pix, int size);

void fit_to_size_double (gdouble * width, gdouble * height, gint max_width, gint max_height);

gboolean drawable_expose_cb (GtkWidget *widget, GdkEventExpose *event, Win* win);

gboolean drawable_button_press_cb (GtkWidget *widget, GdkEventButton *event, Win* win);

gboolean drawable_motion_cb (GtkWidget *widget, GdkEventMotion *event, Win *win);

gboolean drawable_button_release_cb (GtkWidget *widget, GdkEventButton *event, Win* win);

void tools_fit_to_size (gint * width, gint * height, gint max_width, gint max_height);

void crop_click(GtkWidget* widget, Win* win);

GtkWidget* win_new(MainWin* mw);

void
gtk_pixbuf_scale_blend (GdkPixbuf * src, GdkPixbuf * dst, int dst_x, int dst_y, int dst_width, int dst_height,
                        gdouble offset_x, gdouble offset_y, gdouble zoom, GdkInterpType interp, int check_x, int check_y);

#endif
