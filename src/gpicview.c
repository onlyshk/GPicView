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

#include "mainwin.h"

#define PIXMAP_DIR        PACKAGE_DATA_DIR "/gpicview/pixmaps/"

static char** files = NULL;
static gboolean should_display_version = FALSE;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &should_display_version,
                 N_("Print version information and exit"), NULL },
    { NULL }
};

int main(int argc, char** argv)
{
    GError* err = NULL;
    MainWin *win;
	
	gtk_init (&argc, &argv);
	
    /* gettext support */
#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    /* TODO: create GUI here */
	win = (MainWin*)main_win_new();
    gtk_widget_show(win);
    gtk_main();

	return 0;
}
