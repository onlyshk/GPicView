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
#include "utils.h"

void
gtk_anim_view_set_static (GtkAnimView *aview, GdkPixbuf *pixbuf)
{
    GdkPixbufSimpleAnim *s_anim;

    s_anim = gdk_pixbuf_simple_anim_new (gdk_pixbuf_get_width(pixbuf),
                                         gdk_pixbuf_get_height(pixbuf),
                                         -1);
    gdk_pixbuf_simple_anim_add_frame(s_anim, pixbuf);

    if (aview->anim)
        g_object_unref (aview->anim);

    aview->anim = (GdkPixbufAnimation*)s_anim;

    g_object_ref (aview->anim);
    if (aview->iter)
        g_object_unref (aview->iter);

    gtk_image_view_set_pixbuf (GTK_IMAGE_VIEW (aview), pixbuf, TRUE);
    gtk_anim_view_set_is_playing (aview, FALSE);
    aview->delay = -1;
    aview->iter = NULL;

    g_object_unref(pixbuf);
}

void
gtk_view_set_static (GtkAnimView *aview, GdkPixbuf *pixbuf)
{
    GdkPixbufSimpleAnim *s_anim;

    s_anim = gdk_pixbuf_simple_anim_new (gdk_pixbuf_get_width(pixbuf),
                                         gdk_pixbuf_get_height(pixbuf),
                                         -1);
    gdk_pixbuf_simple_anim_add_frame(s_anim, pixbuf);

    if (aview->anim)
        g_object_unref (aview->anim);

    aview->anim = (GdkPixbufAnimation*)s_anim;

    g_object_ref (aview->anim);
    if (aview->iter)
        g_object_unref (aview->iter);

    gtk_image_view_set_pixbuf (GTK_IMAGE_VIEW (aview), pixbuf, TRUE);
    gtk_anim_view_set_is_playing (aview, FALSE);
    aview->delay = -1;
    aview->iter = NULL;

    g_object_unref(pixbuf);
}

GdkPixbuf* scale_pix(GdkPixbuf* ori_pix, int size)
{
      GdkPixbuf* scaled_pix;
      /* keep aspect ratio and scale to thumbnail size: 128 or 256 */
      int width = gdk_pixbuf_get_width(ori_pix);
      int height = gdk_pixbuf_get_height(ori_pix);
      int new_width;
      int new_height;
  
      if(width > height)
      {
          gdouble aspect = (gdouble)height / width;
          new_width = size;
          new_height = size * aspect;
      }
      else if(width < height)
      {
          gdouble aspect = (gdouble)width / height;
          new_height = size;
          new_width = size * aspect;
      }
      else
      {
          new_width = new_height = size;
      }
  
      if((new_width == width && new_height == height) ||
         (size > width && size > height )) /* don't scale up */
      {
          /* if size is not changed or original size is smaller, use original size. */
          scaled_pix = (GdkPixbuf*)g_object_ref(ori_pix);
      }
      else
          scaled_pix = gdk_pixbuf_scale_simple(ori_pix, new_width, new_height, GDK_INTERP_BILINEAR);
  
      return scaled_pix;
}
  

void open_url( GtkAboutDialog *dlg, const gchar *url, gpointer data)
{
    char* programs[] = { "xdg-open", "gnome-open", "exo-open" };
    int i;
    for(  i = 0; i < G_N_ELEMENTS(programs); ++i)
    {
        gchar* open_cmd = NULL;
        if( (open_cmd = g_find_program_in_path( programs[i] )) )
        {
             char* argv [3];
             argv [0] = programs[i];
             argv [1] = (gchar *) url;
             argv [2] = NULL;
             g_spawn_async (NULL, argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, NULL);
             g_free( open_cmd );
             break;
        }
    }
}
