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

typedef struct
{
	gint       incl_border;     
	gint       incl_pointer;     

	gboolean  pointer_change_color; 
	GdkColor  pointer_color_val;    

	Window     window;           
	guint      delay;            

	guint     x;
	guint     y;
	guint     width;
	guint     height;

} ScreenshotValues;

GdkPixbuf* get_screenshot( gpointer user_dara );

GdkPixbuf* get_active_window_screenshot(gpointer user_data);

GdkPixbuf* get_screenshot_with_cursor(GtkWidget* widget, gpointer user_data);

GdkPixbuf *get_selected_area_screenshot(Display *display, ScreenshotValues *sel_values, gpointer user_data);

GdkPixbuf *get_selecting_area_screenshot(Display *display, ScreenshotValues *sel_values, gpointer user_data);

void screenshot_delay(int delay, gpointer user_data);

Display *get_xdisplay();

#endif
