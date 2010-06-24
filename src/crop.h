#ifndef __CROP_H_
#define __CROP_H_

#include <gtk/gtk.h>
#include "mainwin.h"

G_BEGIN_DECLS

typedef struct _Crop Crop;
typedef struct _CropClass CropClass;

#define CROP_TYPE_CROP             (crop_get_type ())
#define CROP_CROP(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CROP_TYPE_CROP, CropCrop))
#define CROP_CROP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  CROP_TYPE_CROP, CropCropClass))
#define CROP_IS_CROP(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CROP_TYPE_CROP))
#define CROP_IS_CROP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  CROP_TYPE_CROP))
#define CROP_CROP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  CROP_TYPE_CROP, CropCropClass))

struct _Crop {
    GObject parent;

    struct MainWin *win;

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

GType     crop_get_type (void) G_GNUC_CONST;

GObject  *crop_new      (MainWin* win);
gboolean  crop_run      (MainWin* crop);

#endif /* __CROP_CROP_H_ */
