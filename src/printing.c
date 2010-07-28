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

static GList* list;
static gboolean is_on = FALSE;

static void begin_print (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{	
  	cairo_t *cr = gtk_print_context_get_cairo_context (context);
	GtkPrintSettings* settings = gtk_print_operation_get_print_settings(oper);	
	
	GtkPageSetup* page_setup = gtk_page_setup_new();
	GtkPaperSize* page_size = gtk_paper_size_new(gtk_page_setup_get_paper_size(page_setup)); 
	    
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(file_path_for_print,NULL);
	
	int image_width = gdk_pixbuf_get_width(pixbuf);
    int image_height = gdk_pixbuf_get_height(pixbuf);
	
	gint page_width = (gint)gtk_paper_size_get_width(page_size, GTK_UNIT_PIXEL);
	gint page_height = (gint)gtk_paper_size_get_height(page_size, GTK_UNIT_PIXEL);
	
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
		   guint count_h = 0;
		   guint count_w = 0;
			
		   while (image_width > page_width)
           {
	          image_width = image_width - page_width;
              count_w++;
           }
		  
		   if (image_width != 0)
		       count_w++;
		   
		   while (image_height > page_height)
           {
	          image_height = image_height - page_height;
              count_h++;
           }
			
	       if (image_height != 0)
		       count_h++;
			
		   gtk_print_operation_set_n_pages(oper, count_w * count_h);
	       
		   gtk_widget_destroy(dialog);
		}
	}
	
	g_object_unref(pixbuf);
	g_object_unref(page_size);
}

static void draw_page (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{
  cairo_t *cr = gtk_print_context_get_cairo_context (context);
  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(file_path_for_print,NULL);
	
  if (is_on == FALSE)
  {
     cairo_paint(cr);
  }
  else
  {
    int i = 0;
	int j = 0;
	int k = 0;
	  
	GtkPageSetup* page_setup = gtk_page_setup_new();
	GtkPaperSize* page_size = gtk_paper_size_new(gtk_page_setup_get_paper_size(page_setup)); 
	    	
	int image_width = gdk_pixbuf_get_width(pixbuf);
    int image_height = gdk_pixbuf_get_height(pixbuf);
	
	gint page_width = (gint)gtk_paper_size_get_width(page_size, GTK_UNIT_PIXEL);
	gint page_height = (gint)gtk_paper_size_get_height(page_size, GTK_UNIT_PIXEL);
	  
	guint count_w = 0;
			
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
		j -= page_width;
		
		if (i == count_w - 1)
		{
		  j = 0;
	      k -= page_height;		
		}	
	}  
	  
	cairo_paint(cr);
	  
	g_object_unref (pixbuf);
	g_object_unref (page_size);
	g_object_unref (page_setup);
  }
}

void  print_pixbuf(GtkWidget* widget, MainWin *mw)
{	
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
   
    op = gtk_print_operation_new ();
	
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL);
    g_signal_connect (op, "begin-print", G_CALLBACK (begin_print),NULL);
	g_signal_connect (op, "draw-page", G_CALLBACK (draw_page),  NULL);
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
	
	g_object_unref (op);
}
