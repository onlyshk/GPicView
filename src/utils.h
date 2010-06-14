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

#ifndef UTILS_H
#define UTILS_H

void
gtk_anim_view_set_static (GtkAnimView *aview, GdkPixbuf * pixbuf)
{
    GdkPixbufSimpleAnim *s_anim;

    s_anim = gdk_pixbuf_simple_anim_new (gdk_pixbuf_get_width(pixbuf),
                                         gdk_pixbuf_get_height(pixbuf),
                                         -1);
    gdk_pixbuf_simple_anim_add_frame(s_anim, pixbuf);

    /* Simple version of uni_anim_view_set_anim */
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

#endif
