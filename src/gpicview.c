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
#include <glib.h>
#include <glib/gi18n.h>

#include "mainwin.h"

#define G_THREADS_ENABLED
#define PIXMAP_DIR        PACKAGE_DATA_DIR "/gpicview/pixmaps/"

char** files = NULL;
static gboolean should_display_version = FALSE;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &should_display_version,
                 N_("Print version information and exit"), NULL },
    { NULL }
};

void*
argument_thread(void *args)
{
  Data *data = (Data*)args;
  main_win_open (data->win,data->argv);
}

int main(int argc, char** argv)
{
    GError*   err;
	GThread*  thread;
	GOptionContext *context;
	
	Data data;
    MainWin *win;
	
/* gettext support */
#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

	// init thread support
    if(!g_thread_supported())
		g_thread_init(NULL);
        gdk_threads_init();
		
	// init GTK+
	gtk_init (&argc, &argv);
	
	context = g_option_context_new ("- simple image viewer");
    g_option_context_add_main_entries (context, opt_entries, GETTEXT_PACKAGE);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if ( !g_option_context_parse (context, &argc, &argv, &err) )
    {
        g_print( "option parsing failed: %s\n", err->message);
        return 1;
    }
	
	if( should_display_version )
    {
        printf( "gpicview %s\n", VERSION );
        return 0;
    }

    /* TODO: create GUI here */
	win = (MainWin*)main_win_new();
    gtk_widget_show(GTK_WIDGET(win));
		
	data.win = win;
    
	if(files)
    {
        if( G_UNLIKELY( *files[0] != '/' && strstr( files[0], "://" )) )    // This is an URI
        {
		    data.argv = files[0];
            char* path = g_filename_from_uri( files[0], NULL, NULL );
            thread = g_thread_create((GThreadFunc)argument_thread,&data,FALSE, &err);
            g_free( path );
		}  
       else
		    data.argv = files[0];
            thread = g_thread_create((GThreadFunc)argument_thread,&data,FALSE, &err);
    }

    /* enter the GTK main loop */
    gtk_main();
	return 0;
}
