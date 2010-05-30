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

#ifndef UI_H
#define UI_H

static const GtkActionEntry entries[] = {
	{
	  "Go Back",
	  GTK_STOCK_GO_BACK,
	  "Go Back",
	  "<control>b",
	  "Go Back",
	   NULL
	},
	{
	 "Go Forward",
	 GTK_STOCK_GO_FORWARD,
	 "Go Forward",
	 "<control>g",
	 "Go Forward",
	  NULL
	},
	{
	 "Zoom out",
	 GTK_STOCK_ZOOM_OUT,
	 "Zoom out",
	 "<control>u",
	 "Zoom out",
	  NULL
	},
     "Zoom in",
	 GTK_STOCK_ZOOM_IN,
	 "Zoom in",
	 "<control>u",
	 "Zoom in",
	  NULL
};

static guint n_entries = G_N_ELEMENTS (entries);

#endif 
