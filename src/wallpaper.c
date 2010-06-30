/***************************************************************************
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
 *   2010 Kuleshov Alexander <kuleshovmail@gmail.com>                      *
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mainwin.h"
#include "wallpaper.h"

void set_as_wallpapaer(GtkWidget* widget, MainWin* mw)
{
	FILE *fp;
	char path[65535];
	int max = 65535;
	int length;
	char* de = NULL;
	char* res;
	
	// Gnome
	fp = popen("ps -C nautilus", "r");
	
	if (fp == NULL)
		return;
	
	while (fgets(path, 65535, fp) != NULL)
           ;
			
	if ((strlen (path)) == 33)
		de = "Gnome";
	
	//LXDE
	fp = popen("ps -C pcmanfm", "r");
	
	if (fp == NULL)
		return;
	
	while (fgets(path, 65535, fp) != NULL)
           ;
			
	if ((strlen (path)) == 33)
		de = "LXDE";
	
	//XFCE
	fp = popen("ps -C pcmanfm", "r");
	
	if (fp == NULL)
		return;
	
	while (fgets(path, 65535, fp) != NULL)
           ;
			
	if ((strlen (path)) == 33)
		de = "XFCE";
	
	//pcmanfm --set-wallpaper /home/user_name/path_to_image
	if (strcmp(de,"LXDE") == 0)
	{
	   printf("LXDE");
	   
	   char* result = NULL;		
	   const char* command;
		
	   result = image_list_get_current_file_path(mw->img_list);
		
	   char temp1[max];	
	   strncpy (temp1, "pcmanfm --set-wallpaper ", max);
	   command =  strncat (temp1, result, max);
	   system(command);
	   
	   return;
	}	
	
	//xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitor0/image-path -s <image-file>
	if (strcmp(de , "XFCE"),0)
	{
	   char* result = NULL;		
	   
	   const char* command1;
	   const char* command2;
	   const char* command3;
		
	   const char* dir_path;
	   char* base_name = NULL;
		
	   result = image_list_get_current_file_path(mw->img_list);
	   dir_path = g_path_get_dirname(result);
	 
	   base_name = g_path_get_basename( result );
	   
	   char temp1[max];	
	   
	   strncpy (temp1, "xfconf-query -c xfce4-desktop -p  ", max);
		
	   command1 =  strncat (temp1, dir_path,  max);
	   command2 =  strncat (temp1, " -s ",    max);
	   command3 =  strncat (temp1, base_name, max);

	   system(command3);
		
	   return;
	}	
	if (strcmp(de,"Gnome") == 0)
	{
       char* result = NULL;		
	   
	   const char* command1;
	   const char* command2;
	   const char* command3;
	   const char* command4;
		
	   const char* dir_path;
	   char* base_name = NULL;
		
	   result = image_list_get_current_file_path(mw->img_list);
	   dir_path = g_path_get_dirname(result);
	 
	   base_name = g_path_get_basename( result );
		
	   char temp1[max];	
	
	   strncpy (temp1, "gconftool-2 -t string -s ", max);
		
	   command1 =  strncat (temp1, "/desktop/gnome/background/picture_filename ",  max);
       command2 =  strncat (temp1,  result, max);
	   
	   system(command2);
		
	   return;
	}	
	
	g_free(fp);
}
