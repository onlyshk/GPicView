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

#include "about.h"


void on_about( GtkWidget* menu, MainWin* mw )
{
    GtkWidget * about_dlg;
    const gchar *authors[] =
    {
        "洪任諭 Hong Jen Yee <pcman.tw@gmail.com>",
        "Martin Siggel <martinsiggel@googlemail.com>",
        "Hialan Liu <hialan.liu@gmail.com>",
        "Marty Jack <martyj19@comcast.net>",
        "Louis Casillas <oxaric@gmail.com>",
		"Kuleshov Alexander <kuleshovmail@gmail.com>",
        (" * Refer to source code of EOG image viewer and GThumb"),
        NULL
    };

    gchar *translators = ( "translator-credits" );

    gtk_about_dialog_set_url_hook( open_url, mw, NULL);

    about_dlg = gtk_about_dialog_new ();

    gtk_container_set_border_width ( ( GtkContainer*)about_dlg , 2 );
    gtk_about_dialog_set_version ( (GtkAboutDialog*)about_dlg, "0.1.0" );
    gtk_about_dialog_set_name ( (GtkAboutDialog*)about_dlg, ( "GPicView" ) );
    gtk_about_dialog_set_logo( (GtkAboutDialog*)about_dlg, gdk_pixbuf_new_from_file(  PACKAGE_DATA_DIR"/pixmaps/gpicview.png", NULL ) );
    gtk_about_dialog_set_copyright ( (GtkAboutDialog*)about_dlg, ( "Copyright (C) 2007 - 2010" ) );
    gtk_about_dialog_set_comments ( (GtkAboutDialog*)about_dlg, ( "Lightweight image viewer from LXDE project" ) );
    gtk_about_dialog_set_license ( (GtkAboutDialog*)about_dlg, "GPicView\n\nCopyright (C) 2007 Hong Jen Yee (PCMan)\n\nThis program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA." );
    gtk_about_dialog_set_website ( (GtkAboutDialog*)about_dlg, "http://wiki.lxde.org/en/GPicView" );
    gtk_about_dialog_set_authors ( (GtkAboutDialog*)about_dlg, authors );
    gtk_about_dialog_set_translator_credits ( (GtkAboutDialog*)about_dlg, translators );
    gtk_window_set_transient_for( (GtkWindow*) about_dlg, GTK_WINDOW( mw ) );

    gtk_dialog_run( ( GtkDialog*)about_dlg );
    gtk_widget_destroy( about_dlg );
}

