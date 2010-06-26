#include "crop.h"

static void spin_x_cb       (GtkSpinButton *spinbutton, Crop *crop);
static void spin_width_cb   (GtkSpinButton *spinbutton, Crop *crop);
static void spin_y_cb       (GtkSpinButton *spinbutton, Crop *crop);
static void spin_height_cb  (GtkSpinButton *spinbutton, Crop *crop);

static gboolean drawable_expose_cb (GtkWidget *widget, GdkEventExpose *event, Crop *crop);
static gboolean drawable_button_press_cb (GtkWidget *widget, GdkEventButton *event, Crop *crop);
static gboolean drawable_button_release_cb (GtkWidget *widget, GdkEventButton *event, Crop *crop);
static gboolean drawable_motion_cb (GtkWidget *widget, GdkEventMotion *event, Crop *crop);

static void
vnr_crop_clear_rectangle(Crop *crop)
{
    if(crop->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(crop->image->window), crop->gc, FALSE,
                            crop->sub_x, crop->sub_y,
                            crop->sub_width, crop->sub_height);
}

static void
vnr_crop_draw_rectangle(Crop *crop)
{
    if(crop->do_redraw)
        gdk_draw_rectangle (GDK_DRAWABLE(crop->image->window), crop->gc, FALSE,
                            crop->sub_x, crop->sub_y,
                            crop->sub_width, crop->sub_height);
}

static void
vnr_crop_update_spin_button_values (Crop *crop)
{
    gtk_spin_button_set_value (crop->spin_height, crop->sub_height / crop->zoom);
    gtk_spin_button_set_value (crop->spin_width, crop->sub_width / crop->zoom);

    gtk_spin_button_set_value (crop->spin_x, crop->sub_x / crop->zoom);
    gtk_spin_button_set_value (crop->spin_y, crop->sub_y / crop->zoom);
}

static void
spin_x_cb (GtkSpinButton *spinbutton, Crop *crop)
{
    if(crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle (crop);

    gboolean old_do_redraw = crop->do_redraw;
    crop->do_redraw = FALSE;

    crop->do_redraw = old_do_redraw;

    crop->sub_x = gtk_spin_button_get_value (spinbutton) * crop->zoom;

    vnr_crop_draw_rectangle (crop);
}

static void
spin_width_cb (GtkSpinButton *spinbutton, Crop *crop)
{
    if(crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle (crop);

    crop->sub_width = gtk_spin_button_get_value (spinbutton) * crop->zoom;

    if(crop->sub_width <1)
        crop->sub_width = 1;

    vnr_crop_draw_rectangle (crop);
}

static void
spin_y_cb (GtkSpinButton *spinbutton, Crop *crop)
{
    if(crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle (crop);

    gboolean old_do_redraw = crop->do_redraw;
    crop->do_redraw = FALSE;

    crop->do_redraw = old_do_redraw;

    crop->sub_y = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    vnr_crop_draw_rectangle (crop);
}

static void
spin_height_cb (GtkSpinButton *spinbutton, Crop  *crop)
{
    if(crop->drawing_rectangle)
        return;

    vnr_crop_clear_rectangle (crop);

    crop->sub_height = gtk_spin_button_get_value(spinbutton) * crop->zoom;

    if(crop->sub_height <1)
        crop->sub_height = 1;

    vnr_crop_draw_rectangle (crop);
}

static GtkWidget*
crop_build_dialog (Crop *crop)
{
	GtkWindow* window;
    GdkPixbuf *original;
    GdkPixbuf *preview;
    GError *error = NULL;
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_resizable(window,FALSE);
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	gtk_window_set_title(window, "Crop");
	
	gtk_widget_show_all(window);
	
	return window;
}

static gboolean
drawable_expose_cb (GtkWidget *widget, GdkEventExpose *event, Crop *crop)
{
    gdk_draw_pixbuf (GDK_DRAWABLE(widget->window), NULL, crop->preview_pixbuf,
                     0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);

    crop->gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
    gdk_gc_set_function (crop->gc, GDK_INVERT);
    gdk_gc_set_line_attributes (crop->gc,
                                2,
                                GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

    if(crop->sub_width == -1)
    {
        crop->sub_x = 0;
        crop->sub_y = 0;
        crop->sub_width = crop->width;
        crop->sub_height = crop->height;
    }
    vnr_crop_clear_rectangle (crop);

    return FALSE;
}

static gboolean
drawable_button_press_cb (GtkWidget *widget, GdkEventButton *event, Crop *crop)
{
    if(event->button == 1)
    {
        crop->drawing_rectangle = TRUE;
        crop->start_x =  event->x;
        crop->start_y =  event->y;
    }

    return FALSE;
}

static gboolean
drawable_button_release_cb (GtkWidget *widget, GdkEventButton *event, Crop *crop)
{
    if(event->button == 1)
    {
        crop->drawing_rectangle = FALSE;

        gtk_spin_button_set_range(crop->spin_width, 1,
                                  (crop->width - crop->sub_x) / crop->zoom);
        gtk_spin_button_set_range(crop->spin_height, 1,
                                  (crop->height - crop->sub_y) / crop->zoom);

        vnr_crop_update_spin_button_values (crop);
    }
    return FALSE;
}

static gboolean
drawable_motion_cb (GtkWidget *widget, GdkEventMotion *event, Crop *crop)
{
    if(!crop->drawing_rectangle)
        return FALSE;

    gdouble x, y;
    x = event->x;
    y = event->y;

    x = CLAMP(x, 0, crop->width);
    y = CLAMP(y, 0, crop->height);

    vnr_crop_clear_rectangle (crop);

    if(x > crop->start_x)
    {
        crop->sub_x = crop->start_x;
        crop->sub_width = x - crop->start_x;
    }
    else if(x == crop->start_x)
    {
        crop->sub_x = x;
        crop->sub_width = 1;
    }
    else
    {
        crop->sub_x = x;
        crop->sub_width = crop->start_x - x;
    }

    if(y > crop->start_y)
    {
        crop->sub_y = crop->start_y;
        crop->sub_height = y - crop->start_y;
    }
    else if(y == crop->start_y)
    {
        crop->sub_y = y;
        crop->sub_height = 1;
    }
    else
    {
        crop->sub_y = y;
        crop->sub_height = crop->start_y - y;
    }

    crop->drawing_rectangle = FALSE;
    crop->do_redraw= FALSE;

    vnr_crop_update_spin_button_values (crop);

    crop->drawing_rectangle = TRUE;
    crop->do_redraw= TRUE;

    vnr_crop_draw_rectangle (crop);

    return FALSE;
}

static void
vnr_crop_dispose (GObject *gobject)
{
}

static void
vnr_crop_class_init (CropClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
}

static void
vnr_crop_init (Crop *crop)
{
    crop->drawing_rectangle = FALSE;
    crop->do_redraw = TRUE;

    crop->sub_x = -1;
    crop->sub_y = -1;
    crop->sub_height = -1;
    crop->sub_width = -1;
    crop->height = -1;
    crop->width = -1;

    crop->gc = NULL;
    crop->image = NULL;
    crop->spin_x = NULL;
    crop->spin_y = NULL;
    crop->spin_width = NULL;
    crop->spin_height = NULL;
    crop->preview_pixbuf = NULL;
}

gboolean
crop_run ()
{
	Crop* crop;
    GtkWidget *dialog;
    gint crop_dialog_response;

    dialog = crop_build_dialog(crop);

    if(dialog == NULL)
        return FALSE;

    crop_dialog_response = gtk_dialog_run (GTK_DIALOG (dialog));

    crop->area.x = gtk_spin_button_get_value_as_int (crop->spin_x);
    crop->area.y = gtk_spin_button_get_value_as_int (crop->spin_y);
    crop->area.width = gtk_spin_button_get_value_as_int (crop->spin_width);
    crop->area.height = gtk_spin_button_get_value_as_int (crop->spin_height);

    gtk_widget_destroy (dialog);
}
