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
 
#include "printing.h"

static void draw_page (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{
   MainWin* mw = (MainWin*)user_data;
   GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(image_list_get_current_file_path (mw->img_list),NULL);
	
   pixbuf =  gdk_pixbuf_scale_simple (pixbuf, 
                                      197, 210, 
                                      GDK_INTERP_HYPER); 
 
	
   cairo_t *cr = gtk_print_context_get_cairo_context (context);
   cairo_surface_t *image;
  
   gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
   cairo_paint (cr);
   cairo_surface_destroy (image);
   g_object_unref (pixbuf);   
}

void   print_pixbuf(GtkWidget* widget, MainWin* mw)
{	
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
    GtkPrintSettings *settings;

    op = gtk_print_operation_new ();

    gtk_print_operation_set_n_pages (op, g_list_length (mw->img_list->list));
    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "draw_page", G_CALLBACK (draw_page), mw);
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, mw, NULL);
}
