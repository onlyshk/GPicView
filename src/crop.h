#ifndef __CROP_H_
#define __CROP_H_

#include <gtk/gtk.h>
#include "mainwin.h"

G_BEGIN_DECLS

typedef struct _Crop Crop;
typedef struct _CropClass CropClass;

struct _Crop {
    GObject parent;

    MainWin *win;
    GdkPixbuf *preview_pixbuf;

    gdouble zoom;
    gdouble width;
    gdouble height;

    GdkGC *gc;
    GtkWidget *image;
    GtkSpinButton *spin_x;
    GtkSpinButton *spin_y;
    GtkSpinButton *spin_width;
    GtkSpinButton *spin_height;

    gdouble sub_x;
    gdouble sub_y;
    gdouble sub_width;
    gdouble sub_height;

    gboolean drawing_rectangle;
    gboolean do_redraw;
    gdouble start_x;
    gdouble start_y;

    GdkRectangle area;
};

struct _CropClass {
    GObjectClass parent_class;
};

gboolean  crop_run ();

#endif /* __CROP_CROP_H_ */
