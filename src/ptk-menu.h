/*
*  C Interface: ptk-menu
*
* Description: Some GUI utilities
*
*
* Author: Hong Jen Yee (PCMan) <pcman.tw (AT) gmail.com>, (C) 2006
*
* Copyright: See COPYING file that comes with this distribution
*
*/

/*
  I don't like GtkUIManager provided by gtk+, so I implement my own. ;-)
*/

#ifndef _PTK_MENU_H_
#define _PTK_UTILS_H_

#include <gtk/gtk.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

#define PTK_STOCK_MENU_ITEM( id, cb ) { id, NULL, G_CALLBACK(cb), 0, 0, NULL, NULL }
#define PTK_MENU_ITEM( label, cb, key, mod ) { label, NULL, G_CALLBACK(cb), key, mod, NULL, NULL }
#define PTK_CHECK_MENU_ITEM( label, cb, key, mod ) { label, (char*)1, G_CALLBACK(cb), key, mod, NULL, NULL }
#define PTK_RADIO_MENU_ITEM( label, cb, key, mod ) { label, (char*)2, G_CALLBACK(cb), key, mod, NULL, NULL }
#define PTK_IMG_MENU_ITEM( label, icon, cb, key, mod ) { label, icon, G_CALLBACK(cb), key, mod, NULL, NULL }
#define PTK_POPUP_MENU( label, sub ) { label, NULL, NULL, 0, 0, sub, NULL }
#define PTK_POPUP_IMG_MENU( label, icon, sub ) { label, icon, NULL, 0, 0, sub, NULL }
#define PTK_SEPARATOR_MENU_ITEM { NULL, (char *)(-1), NULL, 0, 0, NULL, 0}
#define PTK_MENU_END  {0}
#define PTK_IS_STOCK_ITEM( ent )  ( ent->label && (*(guint32*)ent->label) == *(guint32*)"gtk-" )
#define PTK_IS_CHECK_MENU_ITEM( ent )  ( ent->stock_icon == (char*)1 )
#define PTK_IS_RADIO_MENU_ITEM( ent )  ( ent->stock_icon == (char*)2 )

struct _PtkMenuItemEntry
{
  const char* label; /* or stock id */
  const char* stock_icon; /* or menu type  1: check, 2: radio */
  GCallback callback;
  guint key;
  guint mod;    /* GdkModifierType */
  struct _PtkMenuItemEntry* sub_menu;
  GtkWidget** ret;
};
typedef struct _PtkMenuItemEntry PtkMenuItemEntry;

GtkWidget* ptk_menu_new_from_data( PtkMenuItemEntry* entries,
                                   gpointer cb_data,
                                   GtkAccelGroup* accel_group );

void ptk_menu_add_items_from_data( GtkWidget* menu,
                                   PtkMenuItemEntry* entries,
                                   gpointer cb_data,
                                   GtkAccelGroup* accel_group );

G_END_DECLS

#endif

