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

/* argbdata_to_pixdata, get_cursor_pixbuf - functions got from gscreendump
 * Copyright (c) Linux community repr. by <osmoma@online.no>
 */

#include "screenshot.h"
#include "file-dlgs.h"

#include <gdk/gdkx.h>
#include<X11/cursorfont.h>
#include <X11/extensions/Xdamage.h>

G_DEFINE_TYPE (ScreenshotWin, win_screenshot, G_TYPE_OBJECT);

static
GdkPixbuf *get_gdk_cursor_pixbuf(int cursor_type)
{
    GdkCursor *cursor;
    GdkPixbuf *cursor_pixbuf;

    cursor = gdk_cursor_new_for_display(gdk_display_get_default (), cursor_type);
    cursor_pixbuf = gdk_cursor_get_image(cursor);
    gdk_cursor_unref(cursor);

    return cursor_pixbuf;
}

static void argbdata_to_pixdata(long *argb_data, int len, guchar **pixdata)
{
        guchar *p;
        int i;

        *pixdata = g_new(guchar, len * 4);
        p = *pixdata;

        i = 0;
        while (i < len)
        {
                guint argb;
                guint rgba;

                argb = argb_data[i];
                rgba = (argb << 8) | (argb >> 24);

                *p = rgba >> 24;
                ++p;
                *p = (rgba >> 16) & 0xff;
                ++p;
                *p = (rgba >> 8) & 0xff;
                ++p;
                *p = rgba & 0xff;
                ++p;

                i++;
        }
}

void free_pixel_data(guchar *pixels,  gpointer data)
{
        g_free(pixels);
}

GdkPixbuf *get_cursor_pixbuf(Display *display, GdkWindow *gdk_win, gint *cursor_x, gint *cursor_y, gint *offset_x , gint *offset_y/*x-y hot spot */)
{
        GdkWindow *gdk_root = gdk_get_default_root_window();

        GdkPixbuf *cursor_pixbuf = NULL;

        int event, error;
        if (XFixesQueryExtension(display, &event, &error))
        {
                XFixesCursorImage *cur = XFixesGetCursorImage(display);

                *cursor_x = cur->x;
                *cursor_y = cur->x;
                *offset_x = cur->xhot;
                *offset_y = cur->yhot;

                Window child_w; 
                XTranslateCoordinates(display, GDK_WINDOW_XWINDOW(gdk_root), GDK_WINDOW_XWINDOW(gdk_win), cur->x, cur->y, cursor_x, cursor_y,  &child_w);

                guchar *pixdata;
                argbdata_to_pixdata((long*)cur->pixels, cur->width*cur->height, &pixdata);

                cursor_pixbuf = gdk_pixbuf_new_from_data((const guchar *)pixdata, GDK_COLORSPACE_RGB, TRUE, 8, cur->width, cur->height, cur->width * 4,  free_pixel_data, NULL);

                XFree(cur);
        }
        else
        {
                cursor_pixbuf = get_gdk_cursor_pixbuf(GDK_LEFT_PTR);

                #define OFFSET_X 6
                #define OFFSET_Y 4

                *offset_x = OFFSET_X;
                *offset_y = OFFSET_Y;

                gdk_window_get_pointer(gdk_win, cursor_x, cursor_y, NULL);
        }
        return cursor_pixbuf;
}

GdkPixbuf* get_screenshot_with_cursor(gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
	
   GdkPixbuf *cursor_pixbuf;
   GdkPixbuf *screenshot;
   GdkWindow *root_window;
   Display *d = XOpenDisplay (NULL);
  
   int chid_x = 0;
   int chid_y = 0;
   int offset_x = 0;
   int offset_y = 0;
   
   Window window = RootWindow(d,0);
   GdkWindow* gdk_win = gdk_window_foreign_new(window);
	
   cursor_pixbuf = get_cursor_pixbuf(d, gdk_win, &chid_x, &chid_y, &offset_x, &offset_y); 
	
   guint cursor_width = gdk_pixbuf_get_width(cursor_pixbuf);
   guint cursor_height = gdk_pixbuf_get_height(cursor_pixbuf);

   gint x_orig, y_orig;
   gint width, height;
		
   root_window = gdk_get_default_root_window ();
   
   gdk_drawable_get_size (root_window, &width, &height);      
   gdk_window_get_origin (root_window, &x_orig, &y_orig);

   screenshot = gdk_pixbuf_get_from_drawable (screenshot, root_window, NULL,
                                              x_orig, y_orig, 0, 0, width, height);	
 
   gdk_pixbuf_composite(cursor_pixbuf, screenshot, 0, 0, gdk_pixbuf_get_width(screenshot),  gdk_pixbuf_get_height(screenshot),
                                                            chid_x - offset_x, chid_y - offset_y, 1.0, 1.0, GDK_INTERP_NEAREST, 0xFF);
   
   gtk_image_view_set_pixbuf(mw->aview, screenshot, TRUE);
   
   return cursor_pixbuf;
}

GdkPixbuf* get_screenshot(gpointer user_data)
{
   GError* err;
   GdkPixbuf *screenshot;
   GdkWindow *root_window;
	
   MainWin* mw = MAIN_WIN(user_data);
    
   gint x_orig, y_orig;
   gint width, height;
		
   root_window = gdk_get_default_root_window ();
   
   gdk_drawable_get_size (root_window, &width, &height);      
   gdk_window_get_origin (root_window, &x_orig, &y_orig);

   screenshot = gdk_pixbuf_get_from_drawable (screenshot, root_window, NULL,
                                              x_orig, y_orig, 0, 0, width, height);
	
   gtk_image_view_set_pixbuf(mw->aview, screenshot, TRUE);	
  
   return screenshot;
}

GdkPixbuf* get_active_window_screenshot(gpointer user_data)
{
   GError* err;
   GdkPixbuf *screenshot;
   GdkWindow *root_window;
   GdkScreen *screen = gdk_screen_get_default();
	
   MainWin* mw = MAIN_WIN(user_data);
    
   gint x_orig, y_orig;
   gint width, height;
	
   root_window =  gdk_screen_get_active_window(screen);
   
   gdk_drawable_get_size (root_window, &width, &height);      

   screenshot = gdk_pixbuf_get_from_drawable (NULL, root_window, NULL,
                                              0, 0, 0, 0, width, height);
	
   gtk_image_view_set_pixbuf(mw->aview, screenshot, TRUE);	
	   
   g_object_unref( root_window);
  
   return screenshot;
}

static void
clear_rectangle(GtkWidget* widget, ScreenshotWin* win)
{	
    if(win->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(win->image->window), win->gc, FALSE,
                            win->sub_x, win->sub_y,
                            win->sub_width, win->sub_height);
}

static void
draw_rectangle(GtkWidget* widget, ScreenshotWin* win)
{		
    if(win->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(win->image->window), win->gc, FALSE,
                            win->sub_x, win->sub_y,
                            win->sub_width, win->sub_height);
}

static gboolean
drawable_motion_cb (GtkWidget *widget, GdkEventMotion *event, ScreenshotWin *win)
{	
    if(!win->drawing_rectangle)
        return FALSE;
	
    gdouble x, y;
    x = event->x;
    y = event->y;

    x = CLAMP(x, 0, 100000);
    y = CLAMP(y, 0, 100000);
	
	clear_rectangle (NULL, win);

    if(x > win->start_x)
    {
        win->sub_x = win->start_x;
        win->sub_width = x - win->start_x;
    }
    else if(x == win->start_x)
    {
        win->sub_x = x;
        win->sub_width = 1;
    }
    else
    {
        win->sub_x = x;
        win->sub_width = win->start_x - x;
    }

    if(y > win->start_y)
    {
        win->sub_y = win->start_y;
        win->sub_height = y - win->start_y;
    }
    else if(y == win->start_y)
    {
        win->sub_y = y;
        win->sub_height = 1;
    }
    else
    {
        win->sub_y = y;
        win->sub_height = win->start_y - y;
    }

    win->drawing_rectangle = FALSE;
    win->do_redraw= FALSE;

    win->drawing_rectangle = TRUE;
    win->do_redraw= TRUE;

    draw_rectangle (NULL, win);

    return FALSE;
    
}

static gboolean
drawable_expose_cb (GtkWidget *widget, GdkEventExpose *event, ScreenshotWin* win)
{	
    gdk_draw_pixbuf (GDK_DRAWABLE(widget->window), NULL, win->pixbuf,
                     0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
    
    win->gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
	
    gdk_gc_set_function (win->gc, GDK_INVERT);
    gdk_gc_set_line_attributes (win->gc,
                                2,
                                GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	
    if(win->sub_width == -1)
    {
        win->sub_x = 0;
        win->sub_y = 0;
        win->sub_width =  win->width;
        win->sub_height = win->height;
    }
	
	clear_rectangle (NULL, win);
	
    return FALSE;
}

static gboolean
drawable_button_press_cb (GtkWidget *widget, GdkEventButton *event, ScreenshotWin* win)
{
    if(event->button == 1)
    {
        win->drawing_rectangle = TRUE;
        win->start_x =  event->x;
        win->start_y =  event->y;
    }
    return FALSE;
}

static gboolean
drawable_button_release_cb (GtkWidget *widget, GdkEventButton *event, ScreenshotWin* win)
{	
    if(event->button == 1)
    {
        win->drawing_rectangle = FALSE;
		
	    GdkPixbuf *cropped;
        GdkPixbuf *original;
	
        win->area.x = win->sub_x;
        win->area.y = win->sub_y;
        win->area.width = win->sub_width;
        win->area.height = win->sub_height;
		
		original = win->pixbuf;
		
		cropped = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (original),
                                  gdk_pixbuf_get_has_alpha (original),
                                  gdk_pixbuf_get_bits_per_sample (original),
                                  win->area.width, win->area.height);
	
        gdk_pixbuf_copy_area((const GdkPixbuf*)original, win->area.x, win->area.y,
                              win->area.width, win->area.height, cropped, 0, 0);
	
        gtk_view_set_static(GTK_ANIM_VIEW(win->mw->aview), cropped);
		
		g_object_unref(cropped);

        win->mw->modifications |= 8;
	
        win->mw->current_image_width = win->area.width;
        win->mw->current_image_height = win->area.height;
		
		gtk_widget_destroy(win->screenshot_window);
    }
    return FALSE;
}

void screenshot_window(GtkWidget* widget, ScreenshotWin* win)
{	
   win->image = NULL;
   win->drawing_rectangle = FALSE;
   win->do_redraw = TRUE;
   win->sub_x = 10;
   win->sub_y = -1;
   win->sub_height = -1;
   win->sub_width = -1;
   win->height = -1;
   win->width = -1;
   win->gc = NULL;
	
   win->view = gtk_image_view_new();
   win->screenshot_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   win->box = gtk_vbox_new (FALSE,0);
   
   win->pixbuf = get_screenshot(win->mw);
   gtk_image_view_set_pixbuf(win->view, win->pixbuf, TRUE);
	
   win->image = gtk_drawing_area_new();
   gtk_widget_set_events (win->image, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_MOTION_MASK);
  
   g_signal_connect (win->image, "motion-notify-event", G_CALLBACK (drawable_motion_cb), win);
   g_signal_connect (win->image, "expose-event", G_CALLBACK (drawable_expose_cb), win);
   g_signal_connect (win->image, "button-press-event", G_CALLBACK (drawable_button_press_cb), win);
   g_signal_connect (win->image, "button-release-event", G_CALLBACK (drawable_button_release_cb), win);
  
   gtk_container_add (GTK_CONTAINER (win->screenshot_window), win->image);
   
   gtk_window_fullscreen(win->screenshot_window);
   gtk_widget_show_all(win->screenshot_window);
}

void screenshot_delay(int delay, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
   int source_tag = g_timeout_add_seconds (delay,(GSourceFunc)get_screenshot, mw);
   g_timeout_add_seconds(delay + 1, g_source_remove,source_tag);
	
   return;
}

void screenshot_delay_with_cursor(int delay, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
   int source_tag = g_timeout_add_seconds (delay,(GSourceFunc)get_screenshot_with_cursor, mw);
   g_timeout_add_seconds(delay + 1, g_source_remove,source_tag);
	
   return;
}

void screenshot_delay_active_window(int delay, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
   int source_tag = g_timeout_add_seconds (delay,(GSourceFunc)get_active_window_screenshot, mw);
   g_timeout_add_seconds(delay + 1, g_source_remove,source_tag);
	
   return;
}

GType
screenshot_win_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (ScreenshotWinClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      NULL,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (ScreenshotWin),
      0,      /* n_preallocs */
      NULL    /* instance_init */
      };
      type = g_type_register_static (G_TYPE_OBJECT,
                                     "ScreenshotBarType",
                                     &info, 0);
    }
    return type;
}

static void
win_screenshot_init (ScreenshotWin *win)
{
    win->drawing_rectangle = FALSE;
    win->do_redraw = TRUE;

    win->sub_x = -1;
    win->sub_y = -1;
    win->sub_height = -1;
    win->sub_width = -1;
    win->height = -1;
    win->width = -1;

    win->gc = NULL;
    win->image = NULL;
    win->spin_x = NULL;
    win->spin_y = NULL;
    win->spin_width = NULL;
    win->spin_height = NULL;
}

static void screeshot_dispose()
{}

static void
win_screenshot_class_init (ScreenshotWinClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class =  GTK_WIDGET_CLASS ( klass );

    gobject_class->dispose = screeshot_dispose;
}


GtkWidget* screenshot_new(MainWin* mw)
{
	ScreenshotWin *win;
	
    win = (GObject*)g_object_new (SCREENSHOT_WIN_TYPE, NULL );
 
	win->mw = mw;
	
	return (GObject *) win;
}

