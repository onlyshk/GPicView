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

#include <glib/gi18n.h>
#include <string.h>

#include "file-dlgs.h"
#include "pref.h"

static void on_update_preview( GtkFileChooser *chooser, GtkImage* img )
{
    char* file = gtk_file_chooser_get_preview_filename( chooser );
    GdkPixbuf* pix = NULL;
    if( file )
    {
        pix = gdk_pixbuf_new_from_file_at_scale( file, 128, 128, TRUE, NULL );
        g_free( file );
    }
    if( pix )
    {
        gtk_image_set_from_pixbuf( img, pix );
        g_object_unref( pix );
    }
}

char* get_open_filename( GtkWindow* parent, const char* cwd )
{
    char* file = NULL;
    GtkFileChooser* dlg = (GtkFileChooser*)gtk_file_chooser_dialog_new( NULL, parent,
            GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL,
            GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL );
    GtkWidget* img;
    GtkFileFilter *filter;

    if( cwd )
        gtk_file_chooser_set_current_folder( dlg, cwd );

    img = gtk_image_new();
    gtk_widget_set_size_request( img, 128, 128 );
    gtk_file_chooser_set_preview_widget( dlg, img );
    g_signal_connect( dlg, "update-preview", G_CALLBACK(on_update_preview), img );

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter, _("All Supported Images") );
    gtk_file_filter_add_pixbuf_formats( filter );
    gtk_file_chooser_add_filter( dlg, filter );

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter, _("All Files") );
    gtk_file_filter_add_pattern( filter, "*" );
    gtk_file_chooser_add_filter( dlg, filter );

    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
        file = gtk_file_chooser_get_filename( dlg );
    gtk_widget_destroy( (GtkWidget*)dlg );
    return file;
}

static void on_int_val_changed(GtkRange* range, gpointer user_data)
{
    int* val = (int*)user_data;
    *val = (int)gtk_range_get_value(range);
}

static void on_file_save_filter_changed(GObject* obj, GParamSpec* pspec, gpointer user_data)
{
    GtkFileChooser* dlg = (GtkFileChooser*)obj;
    GtkFileFilter* filter = gtk_file_chooser_get_filter( dlg );
    const char* type = (const char*)g_object_get_data(G_OBJECT(filter), "type");
    GtkWidget* extra = gtk_file_chooser_get_extra_widget(dlg);

    if(extra)
    {
        gtk_file_chooser_set_extra_widget(dlg, NULL);
        extra = NULL;
    }

    if( type )
    {
        GtkWidget* label, *label2;
        GtkWidget* scale;
        if( strcmp( type, "jpeg" ) == 0 )
        {
            extra = gtk_hbox_new(FALSE, 12);
            label = gtk_label_new(_("JPEG Quality:"));
            label2 = gtk_label_new(_("Lower quality values yield smaller file sizes,\nbut the image quality will be poorer."));
            scale = gtk_hscale_new_with_range( 0, 100, 5 );
            gtk_range_set_value(GTK_RANGE(scale), pref.jpg_quality);
            gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
            g_signal_connect(G_OBJECT(scale), "value-changed", G_CALLBACK(on_int_val_changed), &pref.jpg_quality);
            gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
            gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
            gtk_box_pack_start(GTK_BOX(extra), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(extra), scale, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(extra), label2, FALSE, TRUE, 0);
        }
        else if( strcmp( type, "png" ) == 0 )
        {
            extra = gtk_hbox_new(FALSE, 12);
            label = gtk_label_new(_("PNG Compression Level:"));
            label2 = gtk_label_new(_("Higher compression levels yield smaller file sizes,\nbut takes more time to do the compression."));
            scale = gtk_hscale_new_with_range( 0, 9, 1 );
            gtk_range_set_value(GTK_RANGE(scale), pref.png_compression);
            gtk_range_set_update_policy(GTK_RANGE(scale), GTK_UPDATE_DISCONTINUOUS);
            g_signal_connect(G_OBJECT(scale), "value-changed", G_CALLBACK(on_int_val_changed), &pref.png_compression);
            gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
            gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
            gtk_box_pack_start(GTK_BOX(extra), label, FALSE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(extra), scale, TRUE, TRUE, 0);
            gtk_box_pack_start(GTK_BOX(extra), label2, FALSE, TRUE, 0);
        }
        /* FIXME: provide "depth settings for *.ico files" */

        if( extra )
        {
            gtk_widget_show_all(extra);
            gtk_file_chooser_set_extra_widget(dlg, extra);
        }
    }
}

char* get_save_filename( GtkWindow* parent, const char* cwd, char** type )
{
    char* file = NULL;
    GtkFileChooser* dlg = (GtkFileChooser*)gtk_file_chooser_dialog_new( NULL, parent,
            GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL,
            GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL );
    GSList* modules, *module;
    GtkFileFilter *filter;

    gtk_file_chooser_set_current_folder( dlg, cwd );

    GtkWidget* img = gtk_image_new();
    gtk_widget_set_size_request( img, 128, 128 );
    gtk_file_chooser_set_preview_widget( dlg, img );
    g_signal_connect( dlg, "update-preview", G_CALLBACK(on_update_preview), img );
    g_signal_connect( dlg, "notify::filter", G_CALLBACK(on_file_save_filter_changed), NULL );

    /*
    /// TODO: determine file type from file name
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name( filter, _("Determined by File Name") );
    gtk_file_filter_add_pixbuf_formats( filter );
    gtk_file_chooser_add_filter( dlg, filter );
    */

    modules = gdk_pixbuf_get_formats();
    for( module = modules; module; module = module->next )
    {
        char *name, *desc, *tmp;
        char **exts, **mimes, **mime;

        GdkPixbufFormat* format = (GdkPixbufFormat*)module->data;
        if( ! gdk_pixbuf_format_is_writable( format ) )
            continue;

        filter = gtk_file_filter_new();

        name = gdk_pixbuf_format_get_name( format );
        desc = gdk_pixbuf_format_get_description( format );
        exts = gdk_pixbuf_format_get_extensions( format );
        mimes = gdk_pixbuf_format_get_mime_types( format );
        tmp = g_strdup_printf( "%s (*.%s)", desc, exts[0], NULL );

        g_object_set_data_full(G_OBJECT(filter), "type", name, (GDestroyNotify)g_free);
        g_strfreev(exts);
        g_free( desc );
        gtk_file_filter_set_name( filter, tmp );
        g_free( tmp );

        for( mime  = mimes; *mime ; ++mime )
            gtk_file_filter_add_mime_type( filter, *mime );
        g_strfreev( mimes );
        gtk_file_chooser_add_filter( dlg, filter );
    }
    g_slist_free( modules );

    int initial_jpg_quality = pref.jpg_quality;
    int initial_png_compression = pref.png_compression;

    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        filter = gtk_file_chooser_get_filter( dlg );
        file = gtk_file_chooser_get_filename( dlg );
        *type = g_object_steal_data(G_OBJECT(filter), "type");

        if( !*type )   // auto detection
        {
            /// TODO: auto file type
        }
        else
        {
            /* TODO: append appropriate extension if needed. */
        }
    }
    gtk_widget_destroy( (GtkWidget*)dlg );

    if ((initial_jpg_quality != pref.jpg_quality) || (initial_png_compression != pref.png_compression))
        save_preferences();

    return file;
}

