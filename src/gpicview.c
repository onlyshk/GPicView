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
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <string.h>

#include "pref.h"
#include "mainwin.h"

static char** files = NULL;
static gboolean should_display_version = FALSE;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &should_display_version,
                 N_("Print version information and exit"), NULL },
    { NULL }
};

#define PIXMAP_DIR        PACKAGE_DATA_DIR "/gpicview/pixmaps/"

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context = NULL;
    MainWin* win;
	
	gtk_init (&argc, &argv);
	
#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    context = g_option_context_new ("- simple image viewer");
    g_option_context_add_main_entries (context, opt_entries, GETTEXT_PACKAGE);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if ( !g_option_context_parse (context, &argc, &argv, &error) )
    {
        g_print( "option parsing failed: %s\n", error->message);
        return 1;
    }

    if( should_display_version )
    {
        printf( "gpicview %s\n", VERSION );
        return 0;
    }

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), PIXMAP_DIR);

    /* Allocate and show the window.
     * We must show the window now in case the file open needs to put up an error dialog. */
    win = (MainWin*)main_win_new();
    gtk_widget_show( GTK_WIDGET(win) );
	
    if ( pref.open_maximized )
        gtk_window_maximize( (GtkWindow*)win );

	if( files )
    {
        if( G_UNLIKELY( *files[0] != '/' && strstr( files[0], "://" )) )    // This is an URI
        {
			g_io_scheduler_cancel_all_jobs();
			
            char* path = g_filename_from_uri( files[0], NULL, NULL );			
            main_win_open((MainWin*)win);
			
            g_free( path );
        }
        else 
		   g_io_scheduler_cancel_all_jobs();
		   
		   GFile* file = g_file_new_for_path(files[0]);
		   win->loading_file = file;
												   
           main_win_open( (MainWin*)win );
    }
	
	gtk_main();
    return 0;
}


