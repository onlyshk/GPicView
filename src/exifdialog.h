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

#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H

#include "mainwin.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtkimageview/gtkimageview.h>
#include <gtkimageview/gtkanimview.h>
#include <gtkimageview/gtkimagescrollwin.h>

#define EXIF_WIN_TYPE              (exif_win_get_type ())
#define EXIF_WIN(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), EXIF_WIN_TYPE, ExifWin))
#define EXIF_WIN_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), EXIF_WIN_TYPE,  ExifWinClass))
#define IS_EXIF_WIN(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EXIF_WIN_TYPE))
#define IS_EXIF_WIN_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), EXIF_WIN_TYPE))
#define EXIF_WIN_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), EXIF_WIN_TYPE, ExifWinClass))

enum
{
  LIST_ITEM = 0,
  N_COLUMNS
};

typedef struct _ExifWin        ExifWin;
typedef struct _ExifWinClass   ExifWinClass;

struct _ExifWinClass
{
    GtkWindowClass parent_class;
};

struct _ExifWin
{
    GObject parent;
	MainWin*   mw;
	GtkWindow* exif_window;
	GtkVBox   *box;
	GtkHBox   *hbox;
	GtkWidget* align;
	GtkScrolledWindow* scroll;
	GtkLabel  *exif_label;
	GtkButton *exif_button;
	GtkWidget *list;
};

GType exif_win_get_type(void);

GObject  *exif_new (MainWin *mw);

void show_exif_window(GtkWidget* widget, ExifWin * win);

GtkWidget* exif_win_new(MainWin* mw);

void ProcessExifDir(unsigned char * DirStart, unsigned char * OffsetBase, 
        unsigned ExifLength, int NestingLevel);


#endif
