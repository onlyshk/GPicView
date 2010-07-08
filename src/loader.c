/***************************************************************************
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
 *   2010 Kuleshov Alexander <kuleshovmail@gmail.com>                      *
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

#include "loader.h"

GdkPixbufAnimation* load_image_from_stream(GInputStream* input_stream, GCancellable* generator_cancellable)
{	
  	GError** error; 
	gboolean res;
	gssize n_read;
	guchar buffer[65535];
	
	GdkPixbufAnimation* animation;
	
	GdkPixbufLoader*    loader;
	loader = gdk_pixbuf_loader_new();
	
	res = TRUE;

	while (1)
	{
      n_read = g_input_stream_read (input_stream, buffer, sizeof (buffer), generator_cancellable, error);
	
	  if (n_read < 0)
	  {
		  res = FALSE;
		  error = NULL; 
	  	  break;
	  }
	
	  if (n_read == 0)
	  {
	  	  break;
	  }
		
	  if (!gdk_pixbuf_loader_write (loader, buffer, n_read, error))
	  {
		  res = FALSE;
		  error = NULL;
		  break;
	  }
	}
				
	animation = NULL;
	
	if (res) 
	{
		animation = gdk_pixbuf_loader_get_animation(loader);
		
		if (animation)
		{
			g_object_ref (animation);
		}
	}

	if (!gdk_pixbuf_loader_close (loader, error)) 
	{
		res = FALSE;
		error = NULL;
		g_object_unref (loader);
		return;
	}
	
	return animation; 
}
