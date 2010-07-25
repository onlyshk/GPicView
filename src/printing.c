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

static void begin_print (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{	
   cairo_t *cr = gtk_print_context_get_cairo_context (context);
   GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(file_path_for_print,NULL);
   GtkPaperSize* page_size = gtk_paper_size_new(gtk_paper_size_get_default());
	
   guint  all_width  = 0;
   guint  all_height = 0; 
	
   all_width = gdk_pixbuf_get_width(pixbuf);     
   all_height = gdk_pixbuf_get_height(pixbuf);	   
	   
   int page_width = (gint)gtk_paper_size_get_width(page_size,GTK_UNIT_PIXEL);
   int page_height = (gint)gtk_paper_size_get_height(page_size,GTK_UNIT_PIXEL);
	
   if (all_width < page_width && all_height < page_height)
   {
	  gtk_print_operation_set_n_pages(oper,1);
	   
      gdk_cairo_set_source_pixbuf(cr, pixbuf , 0, 0);
	  list = g_list_prepend(list, pixbuf);
   }
   else
   {
	  GtkResponseType result;
	  GtkWidget *dialog;
      dialog = gtk_message_dialog_new(NULL,
                                      GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_QUESTION,
                                      GTK_BUTTONS_YES_NO,
                                      "The image does not fit on the A4, to break this image, or adjust the size?");
  
      result = gtk_dialog_run(GTK_DIALOG(dialog));
	  
	  if (result == GTK_RESPONSE_YES)
	  {
		  gtk_print_operation_set_n_pages(oper,1);
		  
		  if (all_width > page_width)
		      pixbuf =  gdk_pixbuf_scale_simple (pixbuf, page_width, all_height, GDK_INTERP_HYPER); 
		    
		  if (all_height > page_width)
		      pixbuf =  gdk_pixbuf_scale_simple (pixbuf, page_width, page_height, GDK_INTERP_HYPER); 
		  
          list = g_list_prepend(list, pixbuf);
		  
		  gtk_widget_destroy(dialog);
	  }
	  else
	  {
		  GdkPixbuf* sub_pixbuf = NULL;
		  
		  int i,j,k = 0;
          int temp  = 0;
		  
	      guint count_w = 0;
		  guint count_h = 0;
		  guint  width_balance   = 0;
          guint  height_balance  = 0;
		    
		  width_balance = gdk_pixbuf_get_width(pixbuf);
          height_balance = gdk_pixbuf_get_height(pixbuf);
		  
		  while (width_balance > page_width)
          {
	         width_balance = width_balance - page_width;
             count_w++;
          }
		  
		  if (width_balance != 0)
		      ++count_w;
		  
		  while (height_balance >= page_height)
          {
	         height_balance -= page_height;
             count_h++;
          }

	      if (height_balance != 0)
		      ++count_h;
		  		  
	      for (temp; temp < count_h; temp++)
	      {
			if (all_height < page_height)
			{
			  for (i; i < count_w; i++)
              {				
		         if (i == count_w - 1)
		         {
		           sub_pixbuf = gdk_pixbuf_new_subpixbuf(pixbuf, j, k,  width_balance, height_balance);
		           list = g_list_append (list, sub_pixbuf); 
		         }
		         else
		         {  
			       sub_pixbuf = gdk_pixbuf_new_subpixbuf(pixbuf, j, k,  page_width, height_balance);
	               list = g_list_append (list, sub_pixbuf); 
	               j += page_width;
    	         }    	
		       }
			}
			else
			  {
	          for (i; i < count_w; i++)
              {				
		         if (i == count_w - 1)
		         {
		           sub_pixbuf = gdk_pixbuf_new_subpixbuf(pixbuf, j, k,  width_balance, page_height);
		           list = g_list_append (list, sub_pixbuf); 
		         }
		         else
		         {  
			       sub_pixbuf = gdk_pixbuf_new_subpixbuf(pixbuf, j, k,  page_width, page_height);
	               list = g_list_append (list, sub_pixbuf); 
	               j += page_width;
    	         }    	
		       }
			  }
			   if ((all_height - k) < page_height)
		       {
		           k = height_balance;
		       }
		       else
	 	       {
		           k += height_balance;
		       }
		       
			i = 0;
		    j = 0;
	      }
		  
		  int n = g_list_length(list);
		  gtk_print_operation_set_n_pages(oper,n);
		  
		  gtk_widget_destroy(dialog);  
 	  }
   }
}

static void draw_page (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{
	cairo_t *cr = gtk_print_context_get_cairo_context (context);
	gdk_cairo_set_source_pixbuf(cr, g_list_nth_data(list, nr), 0 , 0);
    cairo_paint (cr);
	
    file_path_for_print = NULL;
}

static void end_print (GtkPrintOperation * oper, GtkPrintContext * context, 
            			  gint nr, gpointer user_data)
{
   list = NULL;
}

void   print_pixbuf(GtkWidget* widget, MainWin *mw)
{	
    GtkPrintOperation *op;
    GtkPrintOperationResult res;
    
    op = gtk_print_operation_new ();
		
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL);
    g_signal_connect (op, "begin-print", G_CALLBACK (begin_print),NULL);
	g_signal_connect (op, "draw-page", G_CALLBACK (draw_page), NULL);
	g_signal_connect (op, "end-print", G_CALLBACK (end_print) , NULL);
    res = gtk_print_operation_run (op, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, NULL, NULL);
	
	g_object_unref (op);
}


