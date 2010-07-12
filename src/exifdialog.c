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

GtkWidget* exif_win_new( MainWin* mw)
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

static void
init_list(GtkWidget *list)
{

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkListStore *store;

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("List Items",
          renderer, "text", LIST_ITEM, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);

  gtk_tree_view_set_model(GTK_TREE_VIEW(list), 
      GTK_TREE_MODEL(store));

  g_object_unref(store);
}

static void
add_to_list(GtkWidget *list, const char *str, char *str2)
{
  GtkListStore *store;
  GtkTreeIter iter;
	
  char* result = NULL;
  int max = 65535;
  char temp1[max];
	
  strncpy (temp1, str, max);
  result = strncat (temp1, str2, max);

  store = GTK_LIST_STORE(gtk_tree_view_get_model
      (GTK_TREE_VIEW(list)));

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, LIST_ITEM, result, -1);
}

void show_exif_window(GtkWidget* widget, ExifWin * win)
{	
	GError* error = NULL;
	
	const char* current_image = image_list_get_current_file_path(win->mw->img_list);
		
	win->exif_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable (win->exif_window, FALSE);
    gtk_window_set_position(win->exif_window,GTK_WIN_POS_CENTER);
    gtk_window_set_title(win->exif_window, "Exif information");
	
	win->box = gtk_vbox_new (FALSE,0);
	
	win->exif_label = gtk_label_new("Exif data");
	gtk_label_set_justify(GTK_LABEL(win->exif_label), GTK_JUSTIFY_CENTER);
    gtk_box_pack_start(GTK_BOX(win->box), win->exif_label, FALSE, FALSE, 5);
	
	win->list = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(win->list), FALSE);
	gtk_box_pack_start(GTK_BOX(win->box), win->list, TRUE, TRUE, 5);
	
	ProcessFile(current_image);
	
	init_list(win->list);
    add_to_list(win->list, "FileName:   ", ImageInfo.FileName);

	gtk_container_add(win->exif_window, win->box);
	gtk_widget_show_all(win->exif_window);
}
