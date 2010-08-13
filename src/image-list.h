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

#ifndef IMAGELIST_H
#define IMAGELIST_H

#include <glib.h>
#include <gtk/gtk.h>

/* for stat and time_t */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/**
    @author PCMan (Hong Jen Yee) <pcman.tw@gmail.com>
*/
typedef struct _ImageList
{
    gchar* dir_path;
    GList* list;
    GList* current;
	GStaticMutex mutex;
    time_t mtime;
} ImageList;

ImageList* image_list_new();

const char* image_list_get_dir( ImageList* il );

gboolean image_list_open_dir( ImageList* il, const char* path,GCancellable* generator_cancellable, GError** error);

char* image_list_set_current(  ImageList* il,const char* name );

const char* image_list_get_current( ImageList* il );

const char* image_list_get_first( ImageList* il );

const char* image_list_get_next( ImageList* il );

const char* image_list_get_prev( ImageList* il );

const char* image_list_get_last( ImageList* il );

void image_list_free( ImageList* il );

void image_list_close( ImageList* il );

gboolean image_list_is_empty( ImageList* il );

gboolean image_list_has_multiple_files( ImageList* il );

char* image_list_get_current_file_path( ImageList* il );

void image_list_sort_by_name( ImageList* il, GtkSortType type );

void image_list_remove( ImageList* il, const char* name );

void image_list_add_sorted( ImageList* il, const char* name, gboolean set_current );

char* image_list_get_first_file_path( ImageList* il);

const char* image_list_get_first_current_path( ImageList* il );

const char* image_list_get_next_file_path( ImageList* il );

guint image_list_get_num_of_current_path( ImageList* il );

#endif
