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

#include "screenshot-dlg.h"

G_DEFINE_TYPE (ScreenshotDlgWin, win_screenshotdlg, G_TYPE_OBJECT);

GType
screenshotdlg_win_get_type (void)
{
  static GType type = 0;
  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (ScreenshotDlgWinClass),
      NULL,   /* base_init */
      NULL,   /* base_finalize */
      NULL,   /* class_init */
      NULL,   /* class_finalize */
      NULL,   /* class_data */
      sizeof (ScreenshotDlgWin),
      0,      /* n_preallocs */
      NULL    /* instance_init */
      };
      type = g_type_register_static (G_TYPE_OBJECT,
                                     "ScreenShotBarType",
                                     &info, 0);
    }
    return type;
}

static void
win_screenshotdlg_init (ScreenshotDlgWin *win)
{
}

static void screeshotdlg_dispose()
{}

static void
win_screenshotdlg_class_init (ScreenshotDlgWinClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class =  GTK_WIDGET_CLASS ( klass );

    gobject_class->dispose = screeshotdlg_dispose;
}


GtkWidget* screenshotdlg_new(MainWin* mw)
{
	ScreenshotDlgWin *win;
	
    win = g_object_new (SCREENSHOTDLG_WIN_TYPE, NULL );
 
	win->mw = mw;
	
	return (GtkWidget *) win;
}

static
void on_button_press(GtkWidget* widget, ScreenshotDlgWin* win_dlg)
{
	guint delay = gtk_spin_button_get_value_as_int(win_dlg->delay_button);
	
	if (gtk_toggle_button_get_active((GtkToggleButton*)win_dlg->radio_button1) == TRUE)
	{
		if (delay > 0)
		{
		  screenshot_delay(delay, win_dlg->mw);
		}
		else
		{
		   get_screenshot(win_dlg->mw);
		}
	}
	
	if (gtk_toggle_button_get_active((GtkToggleButton*)win_dlg->radio_button2) == TRUE)
	{
		if (delay > 0)
		{
		  screenshot_delay_with_cursor(delay, win_dlg->mw);
		}
		else
		{
		   get_screenshot_with_cursor(win_dlg->mw);
		}
	}
	
	if (gtk_toggle_button_get_active((GtkToggleButton*)win_dlg->radio_button3) == TRUE)
	{
		if (delay > 0)
		{
		  screenshot_delay_active_window(delay, win_dlg->mw);
		}
		else
		{
		   get_active_window_screenshot(win_dlg->mw);
		}
	}
	
	if (gtk_toggle_button_get_active((GtkToggleButton*)win_dlg->radio_button4) == TRUE)
	{		
	    ScreenshotWin* win = (ScreenshotWin*)screenshot_new(win_dlg->mw);
		screenshot_window(widget, win);
	}
	
    gtk_widget_destroy(GTK_WIDGET(win_dlg->screenshotdlg_window));
	g_object_unref(win_dlg);
}

void show_screenshot_window(GtkWidget* widget, ScreenshotDlgWin* win)
{		
    win->screenshotdlg_window = (GtkWindow*)gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable (win->screenshotdlg_window, FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(win->screenshotdlg_window), GTK_WINDOW(win->mw));
	gtk_window_set_position((GtkWindow*)win->screenshotdlg_window, GTK_WIN_POS_CENTER);
	
	GtkAdjustment *spinner_adj;
    spinner_adj = (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 100.0, 1.0, 5.0, 5.0);

	win->vbox = gtk_vbox_new (FALSE, 0);
	win->hbox = gtk_hbox_new(FALSE, 0);
	
	GtkHBox* hbox2 = (GtkHBox*)gtk_hbox_new (FALSE,0);
	
	win->label = GTK_LABEL(gtk_label_new("Delay"));
	
	win->radio_button1 = (GtkRadioButton*)gtk_radio_button_new_with_label(win->radio_list, "Take full screen screenshot");
	win->radio_button2 = (GtkRadioButton*) gtk_radio_button_new_with_label_from_widget(win->radio_button1, "Take full screen screenshot with cursor");
	win->radio_button3 = (GtkRadioButton*) gtk_radio_button_new_with_label_from_widget(win->radio_button1, "Take active window screenshot");
	win->radio_button4 = (GtkRadioButton*) gtk_radio_button_new_with_label_from_widget(win->radio_button1, "Take selection region screenshot");
	
	win->delay_button = GTK_SPIN_BUTTON(gtk_spin_button_new(spinner_adj, 1.0, 0));
	win->capture_button = GTK_BUTTON(gtk_button_new_with_label ("Capture"));

	win->align = GTK_ALIGNMENT(gtk_alignment_new( 1,0 ,0,0));
    gtk_container_add( (GtkContainer*)win->align, GTK_WIDGET(win->capture_button));

	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->radio_button1), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->radio_button2), TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->radio_button3), TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->radio_button4), TRUE, TRUE, 2);
	
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(hbox2), TRUE, FALSE, 2);
	
	gtk_box_pack_start(GTK_BOX(hbox2), GTK_WIDGET(win->label), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->delay_button), TRUE, TRUE, 2);
	
	gtk_box_pack_start(GTK_BOX(win->vbox), GTK_WIDGET(win->hbox), TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(win->hbox), GTK_WIDGET(win->align), TRUE, TRUE, 0);
	
	g_signal_connect( win->capture_button, "clicked", G_CALLBACK(on_button_press), win );
	
	gtk_container_add(GTK_CONTAINER(win->screenshotdlg_window), win->vbox);
	gtk_widget_show_all(GTK_WIDGET(win->screenshotdlg_window));

}
