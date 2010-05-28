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

#include "ui.h"

static GtkActionGroup *group = NULL;

static void 
parse_ui (GtkUIManager *uimanager)
{
  GError *err;
  if (!gtk_ui_manager_add_ui_from_string (uimanager, ui_info, -1, &err))
  {
	   g_warning ("Unable to create menus: %s", err->message);
	   g_free (err);
  }
}

static void
add_action_groups (GtkUIManager *uimanager)
{
   group = gtk_action_group_new ("image");
   gtk_action_group_add_actions (group,
	                             image_actions,
	                             G_N_ELEMENTS (image_actions),
	                             NULL);

   gtk_action_group_set_sensitive (group, FALSE);
	
   gtk_ui_manager_insert_action_group (uimanager,group, 0);
}


