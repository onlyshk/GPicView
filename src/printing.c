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
   GList* list =     NULL;
   GdkPixbuf* pixbuf = NULL;

   MainWin* mw = (MainWin*)user_data;
	
   int n = g_list_length (mw->img_list);
   int i = 0;
	
   for (i; i < n; ++i)
   {
	   pixbuf = gdk_pixbuf_new_from_file(image_list_get_current_file_path (mw->img_list),NULL);
	   
       pixbuf =  gdk_pixbuf_scale_simple (pixbuf, 
                                         197, 210, 
                                         GDK_INTERP_HYPER); 
	  
	   list = g_list_prepend (list, pixbuf);
	   	  
	   if (!mw->img_list->current->next )
	      image_list_get_first(mw->img_list);
	   else
	      image_list_get_next(mw->img_list);
   }
	
   cairo_t *cr = gtk_print_context_get_cairo_context (context);
   cairo_surface_t *image;
  
   i = 0;
	
  for (i; i < n; ++i)
         gdk_cairo_set_source_pixbuf(cr, g_list_nth_data(list,i), 0, 0);
	
   cairo_paint (cr);
   
   g_object_unref (pixbuf);   
   g_list_free(list);
}

static void end_print(GtkPrintOperation *operation,GtkPrintContext   *context,
                       gpointer           user_data)
{
   GtkWidget* dlg = gtk_message_dialog_new( NULL,
                                            GTK_DIALOG_MODAL,
                                            GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_OK,
                                            "%s", "Printing is fininsh" );
    gtk_dialog_run( (GtkDialog*)dlg );
    gtk_widget_destroy( dlg );
}

void   print_pixbuf(GtkWidget* widget, MainWin* mw)
{	
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
	
    op = gtk_print_operation_new ();

    gtk_print_operation_set_n_pages (op, g_list_length (mw->img_list->list));
    gtk_print_operation_set_unit (op, GTK_UNIT_MM);
    g_signal_connect (op, "draw_page", G_CALLBACK (draw_page), mw);
	g_signal_connect(op, "end-print", G_CALLBACK (end_print),mw);
	
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, mw, NULL);
	
	g_object_unref (op);
}
