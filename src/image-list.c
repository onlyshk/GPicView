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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "image-list.h"

#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

static GSList* supported_formats = NULL;

static gboolean image_list_is_file_supported( const char* name );

ImageList* image_list_new()
{
    ImageList* il = g_slice_new0( ImageList );

    if( ! supported_formats )
    {
        GSList* formats = gdk_pixbuf_get_formats();
        GSList* format;
        for( format = formats; format; format = format->next )
        {
            char** exts = gdk_pixbuf_format_get_mime_types( (GdkPixbufFormat*)format->data );
            char** ext;
            for( ext  = exts; *ext ; ++ext )
                supported_formats = g_slist_prepend( supported_formats, *ext );
            g_free( exts );
        }
    }
	//printf(g_list_nth_data(supported_formats, 6));
	g_static_mutex_init(&il->mutex);

    return il;
}

void image_list_free( ImageList* il )
{
    g_slice_free( ImageList, il );
	g_static_mutex_free(&il->mutex);
}

const char* image_list_get_dir( ImageList* il )
{
    return il->dir_path;
}

const char* image_list_get_current( ImageList* il )
{
    return il->current ? (char*)il->current->data : NULL;
}

gboolean image_list_is_empty( ImageList* il )
{
   return (il->list == NULL);
}

gboolean image_list_has_multiple_files( ImageList* il )
{
    return (il->list && il->list->next);
}

static int comp_by_name( char* name1, char* name2, GtkSortType type )
{
    // According to the glib API doc, UTF-8 should be considered here,
    // So the simple strcmp couldn't be used here. What a pity!

    char* utf8;

    utf8 = g_filename_display_name(name1);
    name1 = g_utf8_casefold( utf8, -1 );
    g_free( utf8 );

    utf8 = g_filename_display_name(name2);
    name2 = g_utf8_casefold( utf8, -1 );
    g_free( utf8 );
    int ret = g_utf8_collate( name1, name2 );
    g_free( name1 );
    g_free( name2 );
    return type == GTK_SORT_ASCENDING ? -ret : ret;
}

gboolean image_list_open_dir( ImageList* il, const char* path, 
							 GCancellable* generator_cancellable, GError** error )
{
    const char* name = NULL;
	
    if( il->dir_path && 0 == strcmp( path, il->dir_path ) )
        return TRUE;

    image_list_close( il );
	
	GFile* file = g_file_new_for_path (path);
    GFileEnumerator* enumerator;
    GFileInfo *info;
	
	const char* file_path;
	const char* mime;
	
	il->dir_path = g_strdup( path );
	
    enumerator = g_file_enumerate_children(file,"standard::name,standard::content-type",
	   		                               G_FILE_QUERY_INFO_NONE,generator_cancellable, NULL);
	
	if (enumerator == NULL)
	{
	   g_object_unref (file);
	   g_object_unref (enumerator);
	   g_object_unref (info);
		
	   return FALSE;
	}
	
	while ((info = g_file_enumerator_next_file (enumerator, generator_cancellable, error)) != NULL)
	{
        mime = g_file_info_get_content_type(info);
		 
		if (image_list_is_file_supported(mime))
		{
		   g_static_mutex_lock (&il->mutex);
		   file_path = g_file_info_get_name (info);
		   il->list = g_list_insert_sorted( il->list, g_strdup(file_path), (GCompareFunc)comp_by_name);
		   g_static_mutex_unlock (&il->mutex);
		}

		g_object_unref(info);
	}
	
	il->current = il->list;
	
	g_object_unref (file);
	g_object_unref (enumerator);
	
	return TRUE;
}

gboolean image_list_set_current(  ImageList* il,const char* name )
{
    if( ! il->list || !name )
        return FALSE;

    GList* cur = g_list_find_custom( il->list, name, (GCompareFunc)strcmp );
    if( ! cur )
        return FALSE;
    il->current = cur;
    return TRUE;
}

const char* image_list_get_first( ImageList* il )
{
	g_static_mutex_lock (&il->mutex);
    il->current = il->list;
	g_static_mutex_unlock (&il->mutex);
	
    return image_list_get_current( il );
}

const char* image_list_get_next( ImageList* il )
{
	g_static_mutex_lock (&il->mutex);
    if( il->current && il->current->next )
    {
        il->current = il->current->next;
		g_static_mutex_unlock (&il->mutex);
        return image_list_get_current( il );
    }
	
	g_static_mutex_unlock (&il->mutex);
    return NULL;
}

const char* image_list_get_prev( ImageList* il )
{
	g_static_mutex_lock (&il->mutex);
    if( il->current && il->current->prev )
    {
        il->current = il->current->prev;
	    g_static_mutex_unlock (&il->mutex);
		return image_list_get_current( il );
    }

	g_static_mutex_unlock (&il->mutex);
    return NULL;
}

const char* image_list_get_last( ImageList* il )
{
	g_static_mutex_lock(&il->mutex);
    il->current = g_list_last( il->list );
	g_static_mutex_unlock (&il->mutex);
	
    return image_list_get_current( il );
}

void image_list_close( ImageList* il )
{
    g_list_foreach( il->list, (GFunc)g_free, NULL );
    g_list_free( il->list );
    il->list = NULL;
    il->mtime = 0;

    g_free( il->dir_path );
    il->dir_path = NULL;
}

static gboolean image_list_is_file_supported( const char* name )
{
    const char* ext = name;

    if( ! ext )
        return FALSE;
    ++ext;

    return !!g_slist_find_custom ( supported_formats, name,  (GCompareFunc)g_ascii_strcasecmp);
}

char* image_list_get_current_file_path( ImageList* il )
{
    const char* name;
    if( il->dir_path && (name = image_list_get_current( il )) )
        return g_build_filename( il->dir_path, name, NULL );
    return NULL;
}

char* image_list_get_first_file_path( ImageList* il)
{
    const char* name;
	if( il->dir_path && (name = image_list_get_first( il )) )
        return g_build_filename( il->dir_path, name, NULL );
    return NULL;	
}

void image_list_sort_by_name(  ImageList* il, GtkSortType type )
{
    il->list = g_list_sort_with_data( il->list, (GCompareDataFunc)comp_by_name, (gpointer)type );
}


void image_list_remove( ImageList* il, const char* name )
{
    il->list = g_list_remove( il->list, name );
}

void image_list_add_sorted( ImageList* il, const char* name, gboolean set_current )
{
    il->list = g_list_insert_sorted_with_data( il->list, g_strdup(name), (GCompareDataFunc)comp_by_name, (gpointer)GTK_SORT_ASCENDING );
}
