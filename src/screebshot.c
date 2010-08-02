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

/* argbdata_to_pixdata, get_cursor_pixbuf, get_screenshot_with_cursor
 * draw_selection, get_selection -  functions got from gscreendump
 * Copyright (c) Linux community repr. by <osmoma@online.no>
 */

#include "screenshot.h"
#include "file-dlgs.h"

#include <gdk/gdkx.h>
#include<X11/cursorfont.h>
#include <X11/extensions/Xdamage.h>

static gboolean is_shot = FALSE;

static void draw_selection(Display *display, XSegment *rect, GC gc, long sel_color)
{
    int    screen_num;
    Screen *screen;
    Window root_win;
	
    screen_num = DefaultScreen(display);
    screen = XScreenOfDisplay(display, screen_num);
    root_win = RootWindow(display, XScreenNumberOfScreen(screen));

    int x = (rect->x1 < rect->x2) ? rect->x1 : rect->x2;
    int y = (rect->y1 < rect->y2) ? rect->y1 : rect->y2;

    int width  = abs(rect->x2 - rect->x1);
    int height = abs(rect->y2 - rect->y1);

    XSetForeground(display, gc, XWhitePixel(display, screen_num));
    XDrawRectangle(display, root_win, gc, x, y, width, height);

    if (width-2 > 0 && height-2 > 0)
    {
        XSetForeground(display, gc, sel_color);
        XFillRectangle(display, root_win, gc, x+1, y+1, width-2, height-2);
    }
}

static
GdkPixbuf *capture_area(Display *display, ScreenshotValues *sel_values)
{
    GdkPixbuf *image;

    GdkWindow *root = gdk_get_default_root_window();

    image = gdk_pixbuf_get_from_drawable(NULL, root, NULL, sel_values->x, sel_values->y, 0, 0, sel_values->width, sel_values->height);

    return image;
}

static
gboolean get_selection(Display *display, ScreenshotValues *sel_values)
{
    int    screen_num;
    Screen *screen;
    Window root_win;

    Cursor a_cursor;
    GC  gc_line;
    XSegment  rect;

    screen_num = DefaultScreen(display);
    screen = XScreenOfDisplay(display, screen_num);
    root_win = RootWindow(display, XScreenNumberOfScreen(screen));

    a_cursor = XCreateFontCursor(display, XC_cross);

    long sel_color = 0xFFFFFF;

    XGCValues gc_val;
    gc_val.function           = GXxor;
    gc_val.plane_mask         = AllPlanes;
    gc_val.foreground         = WhitePixel(display, screen_num);
    gc_val.background         = BlackPixel(display, screen_num);
    gc_val.line_width         = 2;
    gc_val.line_style         = LineSolid;
    gc_val.cap_style          = CapButt;
    gc_val.join_style         = JoinMiter;
    gc_val.fill_style         = FillOpaqueStippled;
    gc_val.fill_rule          = WindingRule;
    gc_val.graphics_exposures = False;
    gc_val.clip_x_origin      = 0;
    gc_val.clip_y_origin      = 0;
    gc_val.clip_mask          = None;
    gc_val.subwindow_mode     = IncludeInferiors;

    gc_line = XCreateGC(display, root_win, GCFunction | GCPlaneMask |  GCForeground | GCBackground | GCLineWidth | GCLineStyle |
                        GCCapStyle  | GCJoinStyle  |  GCFillStyle  |  GCFillRule  |  GCGraphicsExposures |
                        GCClipXOrigin |  GCClipYOrigin  |  GCClipMask  | GCSubwindowMode, &gc_val);

    if ((XGrabPointer(display, root_win, False, ButtonMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, 
         GrabModeAsync, root_win, a_cursor, CurrentTime) != GrabSuccess))
    {
        fprintf(stderr, "Error in select_area_with_mouse: Cannot grab mouse pointer.\n");
    }

    if ((XGrabKeyboard(display, root_win, False, GrabModeAsync, GrabModeAsync, CurrentTime) != GrabSuccess))
    {
        fprintf(stderr, "Error in select_area_with_mouse: Cannot grab keyborad.\n");
    }

    Window tmp_win;
    int tmp_int;
    XQueryPointer(display, root_win, &tmp_win, &sel_values->window, &tmp_int, &tmp_int, &tmp_int, &tmp_int, (unsigned int*)&tmp_int);

    rect.x1 = rect.y1 = rect.x2 = rect.y2 = 0;

    XEvent event;
    int done = 0;
    int drawed = 0;

    while (1)
    {
    while (!done && XPending(display)) 
    {
      XNextEvent(display, &event);

      switch (event.type) 
      {
        case ButtonPress: 
        {
            XButtonPressedEvent *ev = (XButtonPressedEvent*)&event;

            sel_values->window = ev->subwindow; 
            if (sel_values->window == None) 
                sel_values->window = root_win;

            rect.x1 = rect.x2 = ev->x;
            rect.y1 = rect.y2 = ev->y;
        }
        break;

        case ButtonRelease: 
            done = 1;
            break;

        case MotionNotify:
        {
            XPointerMovedEvent *ev = (XPointerMovedEvent*)&event;
            if (drawed)
            {
                draw_selection(display, &rect, gc_line, sel_color);
            }

            rect.x2 = ev->x;
            rect.y2 = ev->y;
            draw_selection(display, &rect, gc_line, sel_color);
            XSync (display, False);
            drawed = 1;
        }
        break;

        case KeyPress:
        {
            XKeyEvent *kev = (XKeyEvent*)&event.xkey;
            KeySym key_symbol;

            key_symbol = XKeycodeToKeysym(display, kev->keycode, 0);

            if (key_symbol >= XK_A && key_symbol <= XK_Z)
            {
                /* int ascii_key; */
                /* ascii_key = key_symbol - XK_A + 'A';  */
                done = 1;
            }
            else if (key_symbol >= XK_a && key_symbol <= XK_z)
            {
                done = 1;
            }
            else if (key_symbol == XK_space)
            {
                printf("Spacebar\n");
            }

            else if (key_symbol >= XK_0 && key_symbol <= XK_9)
            {
                done = 1;
            }
            else if (key_symbol == XK_Escape)
            {
                done = 3;
            }

            if (done != 0)
            {
                rect.x2 = kev->x;
                rect.y2 = kev->y;
                sel_values->window = kev->window;
            }

         }
         break;   

        default:
            break;

       }

     } 
    if (done != 0) break;
   } 

    if (drawed)
    {
        draw_selection(display, &rect, gc_line, sel_color);
    }

    XUngrabPointer(display, CurrentTime);
    XUngrabKeyboard(display, CurrentTime);
    XFreeCursor(display, a_cursor);
    XFreeGC(display, gc_line);
    XSync(display, False);

    if (done == 3)
    {
        return FALSE;
    }

    if (sel_values->window == None)
        sel_values->window = root_win;

    sel_values->x = (rect.x1 < rect.x2) ? rect.x1 : rect.x2;
    sel_values->y = (rect.y1 < rect.y2) ? rect.y1 : rect.y2;
    sel_values->width  = abs(rect.x2 - rect.x1);
    sel_values->height = abs(rect.y2 - rect.y1);

    return TRUE;
}

static
GdkPixbuf *get_gdk_cursor_pixbuf(int cursor_type)
{
    GdkCursor *cursor;
    GdkPixbuf *cursor_pixbuf;

    /* Possible pointer types are:
       http://library.gnome.org/devel/gdk/stable/gdk-Cursors.html#GdkCursorType
     */
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

GdkPixbuf *capture_window(Display *display, Window window, gboolean bring_to_front, ScreenshotValues *screenshot_values, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
   GdkPixbuf* pixbuf = get_screenshot_with_cursor(NULL, mw);
		
   return pixbuf;
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

GdkPixbuf* get_screenshot_with_cursor(GtkWidget* widget, gpointer user_data)
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

Display *get_xdisplay()
{
        GdkDisplay *gdk_display = gdk_display_get_default();
        return GDK_DISPLAY_XDISPLAY(gdk_display);
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

GdkPixbuf *get_selecting_area_screenshot(Display *display, ScreenshotValues *sel_values, gpointer user_data)
{
    GdkPixbuf *image = NULL;
	MainWin* mw = MAIN_WIN(user_data);
	
    if (!get_selection(display, sel_values)) 
		return NULL;
	
	if (sel_values->width < 4 )
    {
		sel_values->incl_pointer = TRUE;
		sel_values->incl_border = TRUE;
		sel_values->delay = 30; 

		image = capture_window(display, sel_values->window, FALSE, sel_values, mw);
		return image;
    }

    image = capture_area(display, sel_values);
	gtk_image_view_set_pixbuf(mw->aview, image, TRUE);	

    return image;
}


void screenshot_delay(int delay, gpointer user_data)
{
   MainWin* mw = MAIN_WIN(user_data);
   gboolean b = TRUE;

   int source_tag = g_timeout_add_seconds (delay,(GSourceFunc)get_screenshot, mw);
   g_timeout_add_seconds(1, g_source_remove,source_tag);
	
   return;
}
