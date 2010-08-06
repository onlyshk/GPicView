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

/*
* Scale pix from pcmanfm/libfm/thumbnail.c
* Copyright 2010 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>s
*/

#ifndef UTILS_H
#define UTILS_H

#include <gtk/gtk.h>
#include <gtkimageview/gtkimageview.h>
#include <gtkimageview/gtkanimview.h>

void gtk_anim_view_set_static (GtkAnimView *aview, GdkPixbuf *pixbuf);

GdkPixbuf* scale_pix(GdkPixbuf* ori_pix, int size);

void gtk_view_set_static (GtkAnimView *aview, GdkPixbuf *pixbuf);

#endif
