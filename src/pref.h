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
#ifndef _PREF_H_
#define _PREF_H_

#include <gtk/gtk.h>

#include "mainwin.h"

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtkimageview/gtkimageview.h>
#include <gtkimageview/gtkanimview.h>
#include <gtkimageview/gtkimagescrollwin.h>

#define PREF_WIN_TYPE            (pref_win_get_type())
#define PREF_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PREF_WIN_TYPE, Win))
#define PREF_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  PREF_WIN_TYPE, WinClass))
#define IS_PREF_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PREF_WIN_TYPE))
#define IS_PREF_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  PREF_WIN_TYPE))
#define PREF_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  PREF_WIN_TYPE, WinClass))


typedef struct _Pref        Pref;
typedef struct _PrefClass   PrefClass;

struct _Pref
{
	GObject parent;
	MainWin*   mw;
	GtkWindow *pref_window;
    GtkWidget *auto_save_btn;
    GtkWidget *ask_before_save_btn;
    GtkWidget *set_default_btn;
    GtkWidget *rotate_exif_only_btn;
    GtkWidget *ask_before_del_btn;
    GtkWidget *bg_btn;
    GtkWidget *bg_full_btn;
    GtkWidget *vbox;
    GtkLabel  *bg_label;
    GtkLabel  *bg_full_label;
    GtkWidget *hbox1;
    GtkLabel  *label2;
    GtkLabel  *label3;
    GtkButton *close_btn;
	
    gboolean auto_save_rotated; 
    gboolean ask_before_save;
    gboolean rotate_exif_only;
    gboolean ask_before_delete;
    gboolean open_maximized;
    GdkColor bg;
    GdkColor bg_full;

    int jpg_quality;
    int png_compression;
};

typedef struct _PrefClass {
    GObjectClass parent_class;
};

GType pref_win_get_type(void);

extern Pref pref; 

void load_preferences();

void save_preferences(); 

void edit_preferences(GtkWidget* widget, Pref *win );

GtkWidget* pref_win_new(MainWin* mw);

#endif
