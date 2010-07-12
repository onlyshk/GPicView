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

#include <string.h>


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
reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

static char*
itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* записываем знак */
        n = -n;          /* делаем n положительным числом */
    i = 0;
    do {       /* генерируем цифры в обратном порядке */
        s[i++] = n % 10 + '0';   /* берем следующую цифру */
    } while ((n /= 10) > 0);     /* удаляем */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
	
	return s;
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
	
	
	int Modified = FALSE;
    ReadMode_t ReadMode;
    ReadMode = READ_METADATA;
    CurrentFile = current_image;
    FilesMatched = 1; 

    ResetJpgfile();

    memset(&ImageInfo, 0, sizeof(ImageInfo));
    ImageInfo.FlashUsed = -1;
    ImageInfo.MeteringMode = -1;
    ImageInfo.Whitebalance = -1;
	
	    {
        struct stat st;
        if (stat(current_image, &st) >= 0){
            ImageInfo.FileDateTime = st.st_mtime;
            ImageInfo.FileSize = st.st_size;
        }else{
            printf("No such file");
        }
    }
	
	strncpy(ImageInfo.FileName, current_image, PATH_MAX);
		
    if (!ReadJpegFile(current_image, READ_METADATA)) return;
        #ifdef MATTHIAS
            if (AutoResize){
                // Automatic resize computation - to customize for each run...
                if (AutoResizeCmdStuff() == 0){
                    DiscardData();
                    return;
                }
            }
        #endif // MATTHIAS
	
	DiscardAllButExif();

    Modified = TRUE;
    ReadMode = READ_IMAGE;

    if (!ReadJpegFile(current_image, ReadMode)) return;
	
    ShowConciseImageInfo();
	ShowTags = TRUE;
    ShowImageInfo(TRUE);
	
	char  buf[35];
	char  buf1[35];
	char  buf2[35];
	char* size = itoa(ImageInfo.FileSize,buf);
	
	char* width = itoa(ImageInfo.Width, buf1);
	char* height = itoa(ImageInfo.Height, buf2);
	
	char Temp[20];
    FileTimeAsString(Temp);
	
	init_list(win->list);

	add_to_list(win->list, "File name:   ", ImageInfo.FileName);
	add_to_list(win->list, "File size (bytes)   :", size);
	add_to_list(win->list, "File date:   ", Temp);
	
	if (ImageInfo.CameraMake[0]){
	    add_to_list(win->list, "Camera make:   ", ImageInfo.CameraMake);
	    add_to_list(win->list, "Camer model:   ", ImageInfo.CameraModel);
    }

	add_to_list(win->list, "Date time:   ", ctime(&ImageInfo.FileDateTime));
	
	if (ImageInfo.IsColor == 0)
	{
        add_to_list(win->list, "Color/bw:   ", "Black and White");
    }
	else
	{
	    add_to_list(win->list, "Color/bw:   ", "Non Black and White");
	}
	
	add_to_list(win->list, "Image Width:   ", width);
	add_to_list(win->list, "Image Height:   ",height);
		
	gtk_container_add(win->exif_window, win->box);
	gtk_widget_show_all(win->exif_window);
}
