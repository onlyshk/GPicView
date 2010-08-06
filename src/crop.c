/***************************************************************************
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
 *   2010 Kuleshov Alexander <kuleshovmail@gmail.com>                      *
 *                                                                         *
 *   Based on code by Siyan Panayotov <xsisqox@gmail.com>                  *
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

#include "crop.h"

G_DEFINE_TYPE (Win, win_crop, G_TYPE_OBJECT);

void show_window(GtkWidget* widget, Win *win)
{
	const char* current_image;
    	   
	gdouble width, height;
	
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
	
	if (image_list_get_current(win->mw->img_list) == NULL)
		return;
		
    win->crop_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (win->crop_window, 450, 370);
	gtk_window_set_resizable (win->crop_window, FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(win->crop_window), GTK_WINDOW(win->mw));
    gtk_window_set_position(win->crop_window,GTK_WIN_POS_CENTER);
    gtk_window_set_title(win->crop_window, "Crop Image");
	
	win->box = gtk_vbox_new (FALSE,0);
	win->hbox = gtk_hbox_new (FALSE, 0);
	
	win->image = gtk_drawing_area_new();
	gtk_widget_set_events (win->image, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_BUTTON_MOTION_MASK);
	
	win->crop_button = gtk_button_new();
	gtk_button_set_label(win->crop_button, "Crop Image");
	win->cancel_button = gtk_button_new();
	gtk_button_set_label(win->cancel_button, "Cancel");
	
	win->original = gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(win->mw->aview));
		
    width = gdk_pixbuf_get_width(  gtk_image_view_get_pixbuf (win->mw->aview) );
    height = gdk_pixbuf_get_height( gtk_image_view_get_pixbuf ( win->mw->aview) );
	
   	fit_to_size_double(&height, &width, 450,450);	
	win->width = width;
	win->height = height;
	win->zoom = (width/gdk_pixbuf_get_width(  gtk_image_view_get_pixbuf (win->mw->aview))
				 + height/gdk_pixbuf_get_height(  gtk_image_view_get_pixbuf (win->mw->aview))) / 2;
		
	win->preview =        gdk_pixbuf_new (gdk_pixbuf_get_colorspace (win->original),
                          gdk_pixbuf_get_has_alpha (win->original),
                          gdk_pixbuf_get_bits_per_sample (win->original),
                          width, height);
		
	gtk_pixbuf_scale_blend(win->original, win->preview, 0, 0, width, height, 0, 0,
                        win->zoom, GDK_INTERP_BILINEAR, 0, 0);
		
	win->preview_pixbuf = win->preview;
		
	
	gtk_widget_set_size_request(win->image, width, height);
	gtk_box_pack_start(win->box, win->image, TRUE, TRUE, 1);
	
	GtkHBox* vbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(win->box, vbox, TRUE, TRUE, 0);
	gtk_box_pack_end(vbox, win->cancel_button, FALSE, FALSE, 0);
	gtk_box_pack_end(vbox, win->crop_button, FALSE, FALSE, 10);
	
	g_signal_connect (win->image, "motion-notify-event", G_CALLBACK (drawable_motion_cb), win);
	g_signal_connect (win->image, "expose-event", G_CALLBACK (drawable_expose_cb), win);
	g_signal_connect (win->image, "button-press-event", G_CALLBACK (drawable_button_press_cb), win);
	g_signal_connect (win->image, "button-release-event", G_CALLBACK (drawable_button_release_cb), win);
	
	g_signal_connect (win->crop_button, "clicked", G_CALLBACK(crop_click), win);
	g_signal_connect (win->cancel_button, "clicked", G_CALLBACK(cancel_click), win);
	
	gtk_container_add (win->crop_window, win->box);
	gtk_widget_show_all(win->crop_window);
}



void
cancel_click(GtkWidget* widget, Win* win)
{
   gtk_widget_destroy(win->crop_window);
}

void
crop_click(GtkWidget* widget, Win* win)
{
  GdkPixbuf *cropped;
  GdkPixbuf *original;
	
  win->area.x = win->sub_x;
  win->area.y = win->sub_y;
  win->area.width = win->sub_width;
  win->area.height = win->sub_height;
 
  original = gtk_image_view_get_pixbuf(GTK_IMAGE_VIEW(win->mw->aview));

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

  gtk_widget_destroy((GtkWidget*)win->crop_window);
	
  g_object_unref(win);
}

void
gtk_pixbuf_scale_blend (GdkPixbuf * src,
                        GdkPixbuf * dst,
                        int dst_x,
                        int dst_y,
                        int dst_width,
                        int dst_height,
                        gdouble offset_x,
                        gdouble offset_y,
                        gdouble zoom,
                        GdkInterpType interp, int check_x, int check_y)
{
    if (gdk_pixbuf_get_has_alpha (src))
        gdk_pixbuf_composite_color (src, dst,
                                    dst_x, dst_y, dst_width, dst_height,
                                    offset_x, offset_y,
                                    zoom, zoom,
                                    interp,
                                    255,
                                    check_x, check_y,
                                    CHECK_SIZE, CHECK_LIGHT, CHECK_DARK);
    else
        gdk_pixbuf_scale (src, dst,
                          dst_x, dst_y, dst_width, dst_height,
                          offset_x, offset_y, zoom, zoom, interp);
}

void
fit_to_size_double (gdouble * width, gdouble * height, gint max_width, gint max_height)
{
    gdouble ratio, max_ratio;

    if (*width < max_width && *height < max_height)
        return;

    if (*width == 0 || max_height == 0)
        return;

    ratio = 1. * (*height) / (*width);
    max_ratio = 1. * max_height / max_width;

    if (max_ratio > ratio)
    {
        *width = max_width;
        *height = ratio * (*width);
    }
    else if (ratio > max_ratio)
    {
        *height = max_height;
        *width = (*height) / ratio;
    }
    else
    {
        *width = max_width;
        *height = max_height;
    }

    return;
}

static void
clear_rectangle(GtkWidget* widget, Win* win)
{	
    if(win->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(win->image->window), win->gc, FALSE,
                            win->sub_x, win->sub_y,
                            win->sub_width, win->sub_height);
}

static void
draw_rectangle(GtkWidget* widget, Win* win)
{		
    if(win->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(win->image->window), win->gc, FALSE,
                            win->sub_x, win->sub_y,
                            win->sub_width, win->sub_height);
}

gboolean
drawable_button_release_cb (GtkWidget *widget, GdkEventButton *event, Win* win)
{	
    if(event->button == 1)
    {
        win->drawing_rectangle = FALSE;
    }
    return FALSE;
}


gboolean
drawable_expose_cb (GtkWidget *widget, GdkEventExpose *event, Win* win)
{	
    gdk_draw_pixbuf (GDK_DRAWABLE(widget->window), NULL, win->preview_pixbuf,
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

gboolean
drawable_button_press_cb (GtkWidget *widget, GdkEventButton *event, Win* win)
{
    if(event->button == 1)
    {
        win->drawing_rectangle = TRUE;
        win->start_x =  event->x;
        win->start_y =  event->y;
    }
    return FALSE;
}

gboolean
drawable_motion_cb (GtkWidget *widget, GdkEventMotion *event, Win *win)
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

/*************/
// INIT CLASS*/
/*************/
GType
crop_win_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (WinClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      NULL,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (Win),
      0,      /* n_preallocs */
      NULL    /* instance_init */
      };
      type = g_type_register_static (G_TYPE_OBJECT,
                                     "CropBarType",
                                     &info, 0);
    }
    return type;
}

static void
crop_dispose (GObject *gobject)
{
   
}

static void
win_crop_class_init (WinClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class =  GTK_WIDGET_CLASS ( klass );

    gobject_class->dispose = crop_dispose;
}

GtkWidget* win_new(MainWin* mw)
{
	Win *win;
	
    win = (GObject*)g_object_new (CROP_WIN_TYPE, NULL );
 
	win->mw = mw;
	
	return (GObject *) win;
}

static void
win_crop_init (Win *win)
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

// drawable_motion_cb - 1 click
// drawable_button_release_cb - 2 click
