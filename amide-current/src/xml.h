/* xml.c - convience functions for working with xml files 
 *
 * Part of amide - Amide's a Medical Image Dataset Examiner
 * Copyright (C) 2001-2002 Andy Loening
 *
 * Author: Andy Loening <loening@ucla.edu>
 */

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA.
*/

#ifndef __XML_H__
#define __XML_H__

/* header files that are always associated with this header file */
#include <amide.h>
#include <libxml/tree.h>
#include <libxml/parser.h>


/* functions */

xmlNodePtr xml_get_node(xmlNodePtr nodes, const gchar * descriptor);
gchar * xml_get_string(xmlNodePtr nodes, const gchar * descriptor);
amide_time_t xml_get_time(xmlNodePtr nodes, const gchar * descriptor);
amide_time_t * xml_get_times(xmlNodePtr nodes, const gchar * descriptor, guint num_times);
amide_data_t xml_get_data(xmlNodePtr nodes, const gchar * descriptor);
amide_real_t xml_get_real(xmlNodePtr node, const gchar * descriptor);
gboolean xml_get_boolean(xmlNodePtr nodes, const gchar * descriptor);
gint xml_get_int(xmlNodePtr nodes, const gchar * descriptor);
void xml_save_string(xmlNodePtr node, const gchar * descriptor, const gchar * string);
void xml_save_time(xmlNodePtr node, const gchar * descriptor, const amide_time_t num);
void xml_save_times(xmlNodePtr node, const gchar * descriptor, const amide_time_t * numbers, const int num);
void xml_save_data(xmlNodePtr node, const gchar * descriptor, const amide_data_t num);
void xml_save_real(xmlNodePtr node, const gchar * descriptor, const amide_real_t num);
void xml_save_boolean(xmlNodePtr node, const gchar * descriptor, const gboolean value);
void xml_save_int(xmlNodePtr node, const gchar * descriptor, const int num);

#endif /* __XML_H__ */

