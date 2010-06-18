/*
*  C Implementation: ptkutils
*
* Description:
*
*
* Author: Hong Jen Yee (PCMan) <pcman.tw (AT) gmail.com>, (C) 2006
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "ptk-menu.h"
#include <glib.h>
#include <glib/gi18n.h>

GtkWidget* ptk_menu_new_from_data( PtkMenuItemEntry* entries,
                                   gpointer cb_data,
                                   GtkAccelGroup* accel_group )
{
  GtkWidget* menu;
  menu = gtk_menu_new();
  ptk_menu_add_items_from_data( menu, entries, cb_data, accel_group );
  return menu;
}

void ptk_menu_add_items_from_data( GtkWidget* menu,
                                   PtkMenuItemEntry* entries,
                                   gpointer cb_data,
                                   GtkAccelGroup* accel_group )
{
  PtkMenuItemEntry* ent;
  GtkWidget* menu_item = NULL;
  GtkWidget* sub_menu;
  GtkWidget* image;
  GSList* radio_group = NULL;
  const char* signal;

  for( ent = entries; ; ++ent )
  {
    if( G_LIKELY( ent->label ) )
    {
      /* Stock item */
      signal = "activate";
      if( G_UNLIKELY( PTK_IS_STOCK_ITEM(ent) ) )  {
        menu_item = gtk_image_menu_item_new_from_stock( ent->label, accel_group );
      }
      else if( G_LIKELY(ent->stock_icon) )  {
        if( G_LIKELY( ent->stock_icon > (char *)2 ) )  {
          menu_item = gtk_image_menu_item_new_with_mnemonic(_(ent->label));
          if( g_str_has_prefix(ent->stock_icon, "gtk-") )
            image = gtk_image_new_from_stock( ent->stock_icon, GTK_ICON_SIZE_MENU );
          else
            image = gtk_image_new_from_icon_name( ent->stock_icon, GTK_ICON_SIZE_MENU );
          gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(menu_item), image );
        }
        else if( G_UNLIKELY( PTK_IS_CHECK_MENU_ITEM(ent) ) )  {
          menu_item = gtk_check_menu_item_new_with_mnemonic(_(ent->label));
          signal = "toggled";
        }
        else if( G_UNLIKELY( PTK_IS_RADIO_MENU_ITEM(ent) ) )  {
          menu_item = gtk_radio_menu_item_new_with_mnemonic( radio_group, _(ent->label) );
          if( G_LIKELY( PTK_IS_RADIO_MENU_ITEM( (ent + 1) ) ) )
            radio_group = gtk_radio_menu_item_get_group( GTK_RADIO_MENU_ITEM(menu_item) );
          else
            radio_group = NULL;
          signal = "toggled";
        }
      }
      else  {
        menu_item = gtk_menu_item_new_with_mnemonic(_(ent->label));
      }

      if( G_LIKELY(accel_group) && ent->key ) {
        gtk_widget_add_accelerator (menu_item, "activate", accel_group,
                                    ent->key, ent->mod, GTK_ACCEL_VISIBLE);
      }

      if( G_LIKELY(ent->callback) )  { /* Callback */
        g_signal_connect( menu_item, signal, ent->callback, cb_data);
      }

      if( G_UNLIKELY( ent->sub_menu ) )  { /* Sub menu */
        sub_menu = ptk_menu_new_from_data( ent->sub_menu, cb_data, accel_group );
        gtk_menu_item_set_submenu( GTK_MENU_ITEM(menu_item), sub_menu );
      }
    }
    else
    {
      if( ! ent->stock_icon ) /* End of menu */
        break;
      menu_item = gtk_separator_menu_item_new();
    }
    gtk_menu_shell_append ( GTK_MENU_SHELL(menu), menu_item );
    if( G_UNLIKELY(ent->ret) ) {/* Return */
      *ent->ret = menu_item;
      ent->ret = NULL;
    }
  }
}
