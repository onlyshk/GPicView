/*
 *      pref.h
 *
 *      Copyright (C) 2007 PCMan <pcman.tw@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include "pref.h"
#include "mainwin.h"

#define CFG_DIR    "GPicView"
#define CFG_FILE    CFG_DIR"/gpicview.conf"

G_DEFINE_TYPE (Pref, win_pref, G_TYPE_OBJECT);

Pref pref = {0};

static void
win_pref_init (Pref *win)
{
}

static void
pref_dispose (GObject *gobject)
{}

static void
win_pref_class_init (PrefClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->dispose = pref_dispose;
}

GType
pref_win_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (PrefClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      NULL,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (Pref),
      0,      /* n_preallocs */
      NULL    /* instance_init */
      };
      type = g_type_register_static (G_TYPE_OBJECT,
                                     "CropBarType",
                                     &info, 0);
    }
    return type;
}

GtkWidget* pref_win_new(MainWin* mw)
{
	Pref *win;
	
    win = (GObject*)g_object_new (PREF_WIN_TYPE, NULL );
 
	win->mw = mw;
	
	return (GObject *) win;
}

static gboolean kf_get_bool(GKeyFile* kf, const char* grp, const char* name, gboolean* ret )
{
    GError* err = NULL;
    gboolean val = g_key_file_get_boolean(kf, grp, name, &err);
    if( G_UNLIKELY(err) )
    {
        g_error_free(err);
        return FALSE;
    }
    if(G_LIKELY(ret))
        *ret = val;
    return TRUE;
}

static int kf_get_int(GKeyFile* kf, const char* grp, const char* name, int* ret )
{
    GError* err = NULL;
    int val = g_key_file_get_integer(kf, grp, name, &err);
    if( G_UNLIKELY(err) )
    {
        g_error_free(err);
        return FALSE;
    }
    if(G_LIKELY(ret))
        *ret = val;
    return TRUE;
}

void load_preferences()
{
    GKeyFile* kf;
    char* path;
    char* color;
	
    pref.ask_before_save = TRUE;
    pref.ask_before_delete = TRUE;
    pref.rotate_exif_only = TRUE;

    pref.bg.red = pref.bg.green = pref.bg.blue = 65535;
    pref.bg_full.red = pref.bg_full.green = pref.bg_full.blue = 0;

    pref.jpg_quality = 90;
    pref.png_compression = 9;

    kf = g_key_file_new();
    path = g_build_filename( g_get_user_config_dir(),  CFG_FILE, NULL );
	
    if( g_key_file_load_from_file( kf, path, 0, NULL ) )
    {
        kf_get_bool( kf, "General", "auto_save_rotated", &pref.auto_save_rotated );
        kf_get_bool( kf, "General", "ask_before_save", &pref.ask_before_save );
        kf_get_bool( kf, "General", "ask_before_delete", &pref.ask_before_delete );
        kf_get_bool( kf, "General", "rotate_exif_only", &pref.rotate_exif_only );
        kf_get_bool( kf, "General", "open_maximized", &pref.open_maximized );

        kf_get_int( kf, "General", "jpg_quality", &pref.jpg_quality);
        kf_get_int( kf, "General", "png_compression", &pref.png_compression );

        color = g_key_file_get_string(kf, "General", "bg", NULL);
        if( color )
        {
            gdk_color_parse(color, &pref.bg);
            g_free(color);
        }

        color = g_key_file_get_string(kf, "General", "bg_full", NULL);
        if( color )
        {
            gdk_color_parse(color, &pref.bg_full);
            g_free(color);
        }
    }

    g_free( path );
    g_key_file_free( kf );
}

void save_preferences()
{
    FILE* f;
    char* dir = g_build_filename( g_get_user_config_dir(), CFG_DIR, NULL );
    char* path = g_build_filename( g_get_user_config_dir(),  CFG_FILE, NULL );
    if( ! g_file_test( dir, G_FILE_TEST_IS_DIR ) )
    {
        g_mkdir( g_get_user_config_dir(), 0766 );
        g_mkdir( dir, 0766 );
    }
    g_free( dir );

    if(  (f = fopen( path, "w" )) )
    {
        fputs( "[General]\n", f );
        fprintf( f, "auto_save_rotated=%d\n", pref.auto_save_rotated );
        fprintf( f, "ask_before_save=%d\n", pref.ask_before_save );
        fprintf( f, "ask_before_delete=%d\n", pref.ask_before_delete );
        fprintf( f, "rotate_exif_only=%d\n", pref.rotate_exif_only );
        fprintf( f, "open_maximized=%d\n", pref.open_maximized );
        fprintf( f, "bg=#%02x%02x%02x\n", pref.bg.red/256, pref.bg.green/256, pref.bg.blue/256 );
        fprintf( f, "bg_full=#%02x%02x%02x\n", pref.bg_full.red/256, pref.bg_full.green/256, pref.bg_full.blue/256 );

        fprintf( f, "jpg_quality=%d\n", pref.jpg_quality );
        fprintf( f, "png_compression=%d\n", pref.png_compression );
        fclose( f );
    }
    g_free( path );
}

static void on_set_default( GtkButton* btn, gpointer user_data )
{
    GtkWindow* parent=(GtkWindow*)user_data;
    GtkWidget* dlg=gtk_message_dialog_new_with_markup( parent, 0,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
            _("GPicView will become the default viewer for all supported image files on your system.\n"
                "(This will be done through \'xdg-mime\' program)\n\n"
                "<b>Are you sure you really want to do this?</b>") );
    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        const char cmd[]="xdg-mime default gpicview.desktop image/bmp image/gif image/jpeg image/jpg image/png image/tiff image/x-bmp image/x-pcx image/x-tga image/x-portable-pixmap image/x-portable-bitmap image/x-targa image/x-portable-greymap application/pcx image/svg+xml image/svg-xml";
        g_spawn_command_line_sync( cmd, NULL, NULL, NULL, NULL );
    }
    gtk_widget_destroy( dlg );
}

static void on_set_bg( GtkColorButton* btn, gpointer user_data )
{
    MainWin* parent =(MainWin*)user_data;
    gtk_color_button_get_color(btn, &pref.bg);
    if( !parent->full_screen )
    {
        gtk_widget_modify_bg( parent->aview, GTK_STATE_NORMAL, &pref.bg);
        gtk_widget_queue_draw(parent);
    }
}

static void on_set_bg_full( GtkColorButton* btn, gpointer user_data )
{
    MainWin* parent =(MainWin*)user_data;
    gtk_color_button_get_color(btn, &pref.bg_full);
    if( !parent->full_screen )
    {
        gtk_widget_modify_bg( parent->aview, GTK_STATE_NORMAL, &pref.bg_full);
        gtk_widget_queue_draw(parent);
    }
}

static void on_delete_event(GtkWidget* widget, GdkEventAny* evt)
{ 
	gtk_widget_hide(widget);	
}


void edit_preferences(GtkWidget* widget, Pref *win )
{	
	win->pref_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable (win->pref_window, FALSE);
	gtk_window_set_position(win->pref_window,GTK_WIN_POS_CENTER);
	gtk_window_set_title(win->pref_window, "Preferences");
	    
	win->vbox = gtk_vbox_new (FALSE,0);
	gtk_box_set_spacing(win->vbox,6);
	
	win->ask_before_save_btn = gtk_check_button_new();
	gtk_button_set_label(win->ask_before_save_btn,"Ask before saving images");
	gtk_box_pack_start(win->vbox,win->ask_before_save_btn,FALSE,TRUE,1);
		
	win->ask_before_del_btn = gtk_check_button_new();
	gtk_button_set_label(win->ask_before_del_btn,"Automatically save rotated images");
	gtk_box_pack_start(win->vbox,win->ask_before_del_btn,FALSE,TRUE,2);
	
	win->auto_save_btn = gtk_check_button_new();
	gtk_button_set_label(win->auto_save_btn,"Ask before deleting images");
	gtk_box_pack_start(win->vbox, win->auto_save_btn,FALSE,TRUE,4);
	
	win->rotate_exif_only_btn = gtk_check_button_new();
	gtk_button_set_label(win->rotate_exif_only_btn,"Rotate JPEG file by changing EXIF orientation value (only if EXIF orientation tag exists)");
	gtk_box_pack_start(win->vbox,win->rotate_exif_only_btn,FALSE,TRUE,3);
	
	win->hbox1 = gtk_hbox_new (FALSE,12);
    win->hbox  = gtk_hbox_new (FALSE, 0);
	
	win->label2 = gtk_label_new("Normal:");
	gtk_box_pack_start(GTK_BOX(win->hbox1), win->label2, FALSE, FALSE,40);
	
	win->bg_btn = gtk_color_button_new();
	gtk_color_button_set_color(win->bg_btn, &pref.bg_full);
	gtk_box_pack_start(GTK_BOX(win->hbox1), win->bg_btn, FALSE, FALSE,20);
	
	win->label3 = gtk_label_new("Fullscreen:");
	gtk_box_pack_start(GTK_BOX(win->hbox1), win->label3, FALSE, FALSE,40);
	
	win->bg_full_btn = gtk_color_button_new();
	gtk_color_button_set_color(win->bg_btn, &pref.bg);
	gtk_box_pack_start(GTK_BOX(win->hbox1), win->bg_full_btn, FALSE, FALSE,0);
	
	gtk_box_pack_start(GTK_BOX(win->vbox), win->hbox1, FALSE, FALSE,0);

	
	win->set_default_btn = gtk_button_new();
	gtk_button_set_label(win->set_default_btn,"Make GPicView the default viewer for images");
	gtk_box_pack_start(GTK_BOX(win->hbox), win->set_default_btn, FALSE, FALSE, 180);
	
	gtk_box_pack_start(GTK_BOX(win->vbox), win->hbox, FALSE, FALSE,0);
	
	gtk_container_add(win->pref_window, win->vbox);
	
	g_signal_connect( win->bg_btn, "color-set", G_CALLBACK(on_set_bg), win );
	g_signal_connect( win->bg_full_btn, "color-set", G_CALLBACK(on_set_bg_full), win );
	g_signal_connect (win->pref_window, "delete-event", G_CALLBACK(on_delete_event) , win);
	
	gtk_widget_show_all(win->pref_window);
}
