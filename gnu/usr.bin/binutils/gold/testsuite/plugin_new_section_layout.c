/* plugin_new_section_layout.c -- Simple plugin to reorder function sections in
   plugin-generated objects

   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   Written by Stephen Crane <sjc@immunant.com>.

   This file is part of gold.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* This plugin tests the new_input API of the linker plugin interface that
 * allows plugins to modify section layout and assign sections to segments for
 * sections in plugin-generated object files. It assumes that another plugin is
 * also in use which will add new files. In practice a plugin is likely to
 * generate new input files itself in all_symbols_read and want to
 * reorder/assign sections for these files in the new_input_hook callback. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "plugin-api.h"
#include "elf/common.h"

static ld_plugin_get_input_section_count get_input_section_count = NULL;
static ld_plugin_get_input_section_type get_input_section_type = NULL;
static ld_plugin_get_input_section_name get_input_section_name = NULL;
static ld_plugin_update_section_order update_section_order = NULL;
static ld_plugin_allow_section_ordering allow_section_ordering = NULL;
static ld_plugin_allow_unique_segment_for_sections 
    allow_unique_segment_for_sections = NULL;
static ld_plugin_unique_segment_for_sections unique_segment_for_sections = NULL;

enum ld_plugin_status onload(struct ld_plugin_tv *tv);
enum ld_plugin_status new_input_hook(const struct ld_plugin_input_file *file);

/* Plugin entry point.  */
enum ld_plugin_status
onload(struct ld_plugin_tv *tv)
{
  struct ld_plugin_tv *entry;
  for (entry = tv; entry->tv_tag != LDPT_NULL; ++entry)
    {
      switch (entry->tv_tag)
        {
        case LDPT_GET_INPUT_SECTION_COUNT:
          get_input_section_count = *entry->tv_u.tv_get_input_section_count;
          break;
        case LDPT_GET_INPUT_SECTION_TYPE:
          get_input_section_type = *entry->tv_u.tv_get_input_section_type;
          break;
        case LDPT_GET_INPUT_SECTION_NAME:
          get_input_section_name = *entry->tv_u.tv_get_input_section_name;
          break;
	case LDPT_UPDATE_SECTION_ORDER:
	  update_section_order = *entry->tv_u.tv_update_section_order;
	  break;
	case LDPT_ALLOW_SECTION_ORDERING:
	  allow_section_ordering = *entry->tv_u.tv_allow_section_ordering;
	  break;
	case LDPT_ALLOW_UNIQUE_SEGMENT_FOR_SECTIONS:
	  allow_unique_segment_for_sections
	      = *entry->tv_u.tv_allow_unique_segment_for_sections;
	  break;
	case LDPT_UNIQUE_SEGMENT_FOR_SECTIONS:
	  unique_segment_for_sections
	      = *entry->tv_u.tv_unique_segment_for_sections;
	  break;
        case LDPT_REGISTER_NEW_INPUT_HOOK:
          assert((*entry->tv_u.tv_register_new_input) (new_input_hook)
		 == LDPS_OK);
          break;
        default:
          break;
        }
    }

  if (get_input_section_count == NULL
      || get_input_section_type == NULL
      || get_input_section_name == NULL
      || update_section_order == NULL
      || allow_section_ordering == NULL
      || allow_unique_segment_for_sections == NULL
      || unique_segment_for_sections == NULL)
    {
      fprintf(stderr, "Some interfaces are missing\n");
      return LDPS_ERR;
    }

  /* Inform the linker to prepare for section reordering.  */
  (*allow_section_ordering)();
  /* Inform the linker to prepare to map some sections to unique
     segments.  */
  (*allow_unique_segment_for_sections)(); 

  return LDPS_OK;
}

inline static int is_prefix_of(const char *prefix, const char *str)
{
  return strncmp(prefix, str, strlen (prefix)) == 0;
}

/* This function is called by the linker when new files are added by a plugin.
   We can now tell the linker the desired function order since we have a file
   handle for the newly added file.  */

enum ld_plugin_status
new_input_hook(const struct ld_plugin_input_file *file)
{
  struct ld_plugin_section section_list[3];
  int num_entries = 0;
  unsigned int count;

  if (get_input_section_count(file->handle, &count) != LDPS_OK)
    return LDPS_ERR;

  unsigned int i;
  for (i = 0; i < count; ++i)
  {
    struct ld_plugin_section section;
    unsigned int type = 0;
    char *name = NULL;
    int position = 3;

    section.handle = file->handle;
    section.shndx = i;

    if (get_input_section_type(section, &type) != LDPS_OK)
      return LDPS_ERR;
    if (type != SHT_PROGBITS)
      continue;

    if (get_input_section_name(section, &name))
      return LDPS_ERR;

    /* As in plugin_section_order.c, order is foo() followed by bar()
       followed by baz() */
    if (is_prefix_of(".text.", name))
    {
      if (strstr(name, "_Z3foov") != NULL)
        position = 0;
      else if (strstr(name, "_Z3barv") != NULL)
        position = 1;
      else if (strstr(name, "_Z3bazv") != NULL)
        position = 2;
      else
        position = 3;
    }
    if (position < 3)
    {
      section_list[position] = section;
      num_entries++;
    }
  }

  if (num_entries != 3)
    return LDPS_ERR;

  update_section_order(section_list, num_entries);
  unique_segment_for_sections (".text.plugin_created_unique", 0, 0x1000,
                               section_list, num_entries);

  return LDPS_OK;
}
