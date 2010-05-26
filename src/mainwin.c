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

#include "mainwin.h"

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

/* For drag & drop */
static GtkTargetEntry drop_targets[] =
{
    {"text/uri-list", 0, 0},
    {"text/plain", 0, 1}
};

/* main window initialize and finalize*/
static void main_win_init(MainWin *mw);
static void main_win_finalize(GObject* obj);

// Begin of GObject-related stuff
G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW)

void main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;

    obj_class = ( GObjectClass * ) klass;
    
	//    obj_class->set_property = _set_property;
    //   obj_class->get_property = _get_property;
    
	obj_class->finalize = main_win_finalize;

    widget_class = GTK_WIDGET_CLASS ( klass );
    
	//widget_class->delete_event = on_delete_event;
    // widget_class->size_allocate = on_size_allocate;
    // widget_class->key_press_event = on_key_press_event;
    // widget_class->window_state_event = on_win_state_event;
}

void main_win_finalize( GObject* obj )
{
    gtk_main_quit();
}

GtkWidget* main_win_new()
{
    return (GtkWidget*)g_object_new (MAIN_WIN_TYPE, NULL);
}

// End of GObject-related stuff
void main_win_init( MainWin*mw )
{
    gtk_window_set_title( (GtkWindow*)mw, "Image Viewer");
    gtk_window_set_default_size( (GtkWindow*)mw, 640, 480 );
}

