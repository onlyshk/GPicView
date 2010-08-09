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

#ifndef SCREENSHOT_DLG_H
#define SCREENSHOT_DLG_H

#include "screenshot.h"
#include "mainwin.h"

#define SCREENSHOTDLG_WIN_TYPE            (screenshotdlg_win_get_type())
#define SCREENSHOTDLG_WIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SCREENSHOTDLG_WIN_TYPE, ScreenshotDlgWin))
#define SCREENSHOTDLG_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SCREENSHOTDLG_WIN_TYPE, ScreenshotDlgWinClass))
#define IS_SCREENSHOTDLG_WIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SCREENSHOTDLG_WIN_TYPE))
#define IS_SCREENSHOTDLG_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SCREENSHOTDLG_WIN_TYPE))
#define SCREENSHOTDLG_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SCREENSHOTDLG_WIN_TYPE, ScreenshotDlgWinClass))

typedef struct _ScreenshotDlgWin  ScreenshotDlgWin;
typedef struct _ScreenshotDlgWinClass ScreenshotDlgWinClass;

struct _ScreenshotDlgWin
{
    GObject parent;
	MainWin*   mw;
	GtkWindow* screenshotdlg_window;
	GtkButton* capture_button;
	GtkLabel*  label;
	GtkRadioButton* radio_button1;
	GtkRadioButton* radio_button2;
	GtkRadioButton* radio_button3;
	GtkRadioButton* radio_button4;
	GtkSpinButton*  delay_button;
	
	GtkAlignment*     align;
	GtkAlignment*     align1;
	GtkWidget*        vbox;
	GtkWidget*        hbox;
	GSList*           radio_list;
	
	guint delay;
};

struct _ScreenshotDlgWinClass {
    GObjectClass parent_class;
};

GType screenshotdlg_win_get_type (void);

GtkWidget *screenshotdlg_new (MainWin *mw);

void show_screenshot_window(GtkWidget* widget, ScreenshotDlgWin* win);

#endif
