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

#include "exifdialog.h"

G_DEFINE_TYPE (ExifWin, win_exif, G_TYPE_OBJECT);

/*************/
// INIT CLASS*/
/*************/
GType
exif_win_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (ExifWinClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      NULL,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (ExifWin),
      0,      /* n_preallocs */
      NULL    /* instance_init */
      };
      type = g_type_register_static (G_TYPE_OBJECT,
                                     "CropBarType",
                                     &info, 0);
    }
    return type;
}

static void
exif_dispose (GObject *gobject)
{}

static void
win_exif_class_init (ExifWinClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->dispose = exif_dispose;
}

GtkWidget* exif_win_new(struct MainWin* mw)
{
	ExifWin *win;
	
    win = (GObject*)g_object_new (EXIF_WIN_TYPE, NULL );
 
	win->mw = mw;
	
	return (GObject *) win;
}

static void
win_exif_init (ExifWin *win)
{
	win->exif_window = NULL;
}

void show_exif(ExifWin * win)
{	
	GError* error = NULL;
	
	win->exif_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable (win->exif_window, FALSE);
    gtk_window_set_position(win->exif_window,GTK_WIN_POS_CENTER);
    gtk_window_set_title(win->exif_window, "Exif information");
	
	gtk_widget_show_all(win);;
}
