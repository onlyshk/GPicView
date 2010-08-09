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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "mainwin.h"

#include<X11/Xlib.h>

#define SCREENSHOT_WIN_TYPE            (screenshot_win_get_type())
#define SCREENSHOT_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCREENSHOT_WIN_TYPE, ScreenshotWin))
#define SCREENSHOT_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCREENSHOT_WIN_TYPE, ScreenshotWinClass))
#define IS_SCREENSHOT_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCREENSHOT_WIN_TYPE))
#define IS_SCREENSHOT_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCREENSHOT_WIN_TYPE))
#define SCREENSHOT_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCREENSHOT_WIN_TYPE, ScreenshotWinClass))

typedef struct _ScreenshotWin  ScreenshotWin;
typedef struct _ScreenshotWinClass ScreenshotWinClass;

struct _ScreenshotWin
{
    GObject parent;
	MainWin*   mw;
	GtkWindow* screenshot_window;
	GtkImageView* view;
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
	GdkPixbuf* pixbuf;
	GdkRectangle area;
};

struct _ScreenshotWinClass {
    GObjectClass parent_class;
};

GType screenshot_win_get_type (void);

GtkWidget *screenshot_new (MainWin *mw);

GdkPixbuf* get_screenshot( gpointer user_dara );

GdkPixbuf* get_active_window_screenshot(gpointer user_data);

GdkPixbuf* get_screenshot_with_cursor(gpointer user_data);

void screenshot_delay(int delay, gpointer user_data);

void screenshot_window(GtkWidget* widget, ScreenshotWin* win);

void screenshot_delay_with_cursor(int delay, gpointer user_data);

void screenshot_delay_active_window(int delay, gpointer user_data);

#endif
