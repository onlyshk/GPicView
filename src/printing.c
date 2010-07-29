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

#include <math.h>

static gboolean is_on = FALSE;

static void begin_print (GtkPrintOperation *oper, GtkPrintContext *context, 
						 gpointer user_data)
{	
	MainWin* mw = MAIN_WIN(user_data);
	GdkPixbuf* pixbuf = gdk_pixbuf_animation_get_static_image(mw->animation);
    
  	cairo_t *cr = gtk_print_context_get_cairo_context (context);
		
	int image_width = gdk_pixbuf_get_width(pixbuf);
    int image_height = gdk_pixbuf_get_height(pixbuf);

	gint page_width =  (gint)gtk_print_context_get_width(context);
	gint page_height = (gint)gtk_print_context_get_height(context);
	
	int n_xpages = ceil((double)image_width / page_width);
	int n_ypages = ceil((double)image_height / page_height);
	int n_all_pages = n_xpages * n_ypages;
	
	is_on = FALSE;
	
	if (image_width < page_width && image_height < page_height)
	{
	   gtk_print_operation_set_n_pages(oper, 1);
	   gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	
	   is_on = FALSE;
	}
	else
	{
	   GtkResponseType result;
	   GtkWidget *dialog;
		
       dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
                                      "The image does not fit on the A4, to break this image, or adjust the size?");
  
       result = gtk_dialog_run(GTK_DIALOG(dialog));
	  
	   if (result == GTK_RESPONSE_YES)
	   {		  
		   gtk_print_operation_set_n_pages(oper,1);
			
		   if (image_width > page_width)
		       pixbuf =  gdk_pixbuf_scale_simple (pixbuf, page_width, image_height, GDK_INTERP_HYPER); 
		    
		   if (image_height > page_width)
		       pixbuf =  gdk_pixbuf_scale_simple (pixbuf, page_width, page_height, GDK_INTERP_HYPER); 
		  
	       gdk_cairo_set_source_pixbuf(cr, pixbuf, 0 , 0);
			
		   gtk_widget_destroy(dialog);
	    }
		else
		{
		   is_on = TRUE;
		
		   gtk_print_operation_set_n_pages(oper, n_all_pages);
	       
		   gtk_widget_destroy(dialog);
		}
	}
	
	g_object_unref(pixbuf);
}

static void draw_page (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{
  cairo_t *cr = gtk_print_context_get_cairo_context (context);
	
  MainWin* mw = MAIN_WIN(user_data);
	
  if (is_on == FALSE)
  {
     cairo_paint(cr);
  }
  else
  {
    int i = 0;
	int j = 0;
	int k = 0;
	  
    GdkPixbuf* pixbuf = gdk_pixbuf_animation_get_static_image(mw->animation);
	  
	gint page_width =  (gint)gtk_print_context_get_width(context);
	gint page_height = (gint)gtk_print_context_get_height(context);
			  
	int image_width = gdk_pixbuf_get_width(pixbuf);

	int count_w = 0;
	int count_h = 0;
	  
	while (image_width > page_width)
    {
	   image_width = image_width - page_width;
       count_w++;
    }
	     
	if (image_width != 0)
		count_w++;
	  
	for (i; i < nr + 1; i++)
	{
	    gdk_cairo_set_source_pixbuf(cr, pixbuf, j, k);
		j -= page_width + 40;
		
		if (i == count_w - 1)
		{
		  j = 0;
	      k -= page_height;		
		}	
	}  
	cairo_paint(cr);
  }
}

static void  printing_done(GtkPrintOperation  *operation, GtkPrintOperationResult result,
						   gpointer user_data)
{
    result = (GtkPrintOperationResult)user_data;
   	
	GtkWidget *dialog;
	GtkResponseType dialog_result;
	
    if (result == GTK_PRINT_OPERATION_RESULT_ERROR)
    {
		GError    *error;
		gtk_print_operation_get_error(operation, &error);
		
        dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                       error->message);
		
		dialog_result = gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
	else
	{
     
	   dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                      "Printing done!");
		
       dialog_result = gtk_dialog_run(GTK_DIALOG(dialog));
	   gtk_widget_destroy(dialog);
	}
}

void  print_pixbuf(GtkWidget* widget, MainWin *mw)
{	
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
	
    op = gtk_print_operation_new ();
	
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL);
    g_signal_connect (op, "begin-print", G_CALLBACK (begin_print), mw);
	g_signal_connect (op, "draw-page", G_CALLBACK (draw_page),  mw);
	g_signal_connect (op, "done", G_CALLBACK (printing_done),  &res);
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
	
	g_object_unref (op);
}
