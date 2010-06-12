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
#include "glib-mem.h"

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
            char** exts = gdk_pixbuf_format_get_extensions( (GdkPixbufFormat*)format->data );
            char** ext;
            for( ext  = exts; *ext ; ++ext )
                supported_formats = g_slist_prepend( supported_formats, *ext );
            g_free( exts ); // g_strfreev is not needed since we stole its stirngs
        }
        // supported_formats = g_slist_reverse( supported_formats, *ext );
    }

    return il;
}

void image_list_free( ImageList* il )
{
    g_slice_free( ImageList, il );
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

gboolean image_list_open_dir( ImageList* il, const char* path, GError** error )
{
    const char* name = NULL;
    GDir* dir;
    struct stat stbuf;

    if( il->dir_path && 0 == strcmp( path, il->dir_path ) )
        return TRUE;

    image_list_close( il );

    if( stat( path, &stbuf ) == -1 )
        return FALSE;

    dir = g_dir_open( path, 0, error );
    if( ! dir )
        return FALSE;

    il->dir_path = g_strdup( path );
    il->mtime = stbuf.st_mtime;

    while( ( name = g_dir_read_name ( dir ) ) )
    {
//        char* file_path = g_build_filename( dir_path, name, NULL );
        if( image_list_is_file_supported( name ) )
            il->list = g_list_prepend( il->list, g_strdup(name) );
//        g_free( file_path );
    }
    g_dir_close( dir );
    il->list = g_list_reverse( il->list );
    il->current = il->list;
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
    il->current = il->list;
    return image_list_get_current( il );
}

const char* image_list_get_next( ImageList* il )
{
    if( il->current && il->current->next )
    {
        il->current = il->current->next;
        return image_list_get_current( il );
    }
    return NULL;
}

const char* image_list_get_prev( ImageList* il )
{
    if( il->current && il->current->prev )
    {
        il->current = il->current->prev;
        return image_list_get_current( il );
    }
    return NULL;
}

const char* image_list_get_last( ImageList* il )
{
    il->current = g_list_last( il->list );
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
    const char* ext = strrchr( name, '.' );
    if( ! ext )
        return FALSE;
    ++ext;

    return !!g_slist_find_custom ( supported_formats, ext,  (GCompareFunc)g_ascii_strcasecmp);
}

char* image_list_get_current_file_path( ImageList* il )
{
    const char* name;
    if( il->dir_path && (name = image_list_get_current( il )) )
        return g_build_filename( il->dir_path, name, NULL );
    return NULL;
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

void image_list_sort_by_name(  ImageList* il, GtkSortType type )
{
    il->list = g_list_sort_with_data( il->list, (GCompareDataFunc)comp_by_name, (gpointer)type );
}


void image_list_remove( ImageList* il, const char* name )
{
    il->list = g_list_remove( il->list, name );
}
