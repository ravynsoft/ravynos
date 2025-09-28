/* plugin_section_reorder.c -- Simple plugin to reorder function sections

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Sriraman Tallam <tmsriram@google.com>.

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "plugin-api.h"

static ld_plugin_get_input_section_count get_input_section_count = NULL;
static ld_plugin_get_input_section_type get_input_section_type = NULL;
static ld_plugin_get_input_section_name get_input_section_name = NULL;
static ld_plugin_get_input_section_contents get_input_section_contents = NULL;
static ld_plugin_update_section_order update_section_order = NULL;
static ld_plugin_allow_section_ordering allow_section_ordering = NULL;
static ld_plugin_allow_unique_segment_for_sections 
    allow_unique_segment_for_sections = NULL;
static ld_plugin_unique_segment_for_sections unique_segment_for_sections = NULL;

enum ld_plugin_status onload(struct ld_plugin_tv *tv);
enum ld_plugin_status claim_file_hook(const struct ld_plugin_input_file *file,
                                      int *claimed);
enum ld_plugin_status all_symbols_read_hook(void);

/* Plugin entry point.  */
enum ld_plugin_status
onload(struct ld_plugin_tv *tv)
{
  struct ld_plugin_tv *entry;
  for (entry = tv; entry->tv_tag != LDPT_NULL; ++entry)
    {
      switch (entry->tv_tag)
        {
        case LDPT_REGISTER_CLAIM_FILE_HOOK:
          assert((*entry->tv_u.tv_register_claim_file) (claim_file_hook)
		 == LDPS_OK);
          break;
	case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
          assert((*entry->tv_u.tv_register_all_symbols_read)
		     (all_symbols_read_hook)
		 == LDPS_OK);
          break;
        case LDPT_GET_INPUT_SECTION_COUNT:
          get_input_section_count = *entry->tv_u.tv_get_input_section_count;
          break;
        case LDPT_GET_INPUT_SECTION_TYPE:
          get_input_section_type = *entry->tv_u.tv_get_input_section_type;
          break;
        case LDPT_GET_INPUT_SECTION_NAME:
          get_input_section_name = *entry->tv_u.tv_get_input_section_name;
          break;
        case LDPT_GET_INPUT_SECTION_CONTENTS:
          get_input_section_contents
	      = *entry->tv_u.tv_get_input_section_contents;
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
        default:
          break;
        }
    }

  if (get_input_section_count == NULL
      || get_input_section_type == NULL
      || get_input_section_name == NULL
      || get_input_section_contents == NULL
      || update_section_order == NULL
      || allow_section_ordering == NULL
      || allow_unique_segment_for_sections == NULL
      || unique_segment_for_sections == NULL)
    {
      fprintf(stderr, "Some interfaces are missing\n");
      return LDPS_ERR;
    }

  return LDPS_OK;
}

inline static int is_prefix_of(const char *prefix, const char *str)
{
  return strncmp(prefix, str, strlen (prefix)) == 0;
}

struct ld_plugin_section section_list[3];
int num_entries = 0;

/* This function is called by the linker for every new object it encounters.  */
enum ld_plugin_status
claim_file_hook(const struct ld_plugin_input_file *file, int *claimed)
{
  static int is_ordering_specified = 0;
  struct ld_plugin_section section;
  unsigned int count = 0;
  unsigned int shndx;

  *claimed = 0;
  if (is_ordering_specified == 0)
    {
      /* Inform the linker to prepare for section reordering.  */
      (*allow_section_ordering)();
      /* Inform the linker to prepare to map some sections to unique
	 segments.  */
      (*allow_unique_segment_for_sections)(); 
      is_ordering_specified = 1;
    }

  (*get_input_section_count)(file->handle, &count);

  for (shndx = 0; shndx < count; ++shndx)
    {
      char *name = NULL;
      int position = 3;

      section.handle = file->handle;
      section.shndx = shndx;
      (*get_input_section_name)(section, &name);

      /* Order is foo() followed by bar() followed by baz()  */
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
	  section_list[position].handle = file->handle;
	  section_list[position].shndx = shndx;
	  num_entries++;
	}
    }
  return LDPS_OK;
}

/* This function is called by the linker after all the symbols have been read.
   At this stage, it is fine to tell the linker the desired function order.  */

enum ld_plugin_status
all_symbols_read_hook(void)
{
  if (num_entries == 3)
    { 
      update_section_order(section_list, num_entries);
      unique_segment_for_sections (".text.plugin_created_unique", 0, 0x1000,
				   section_list, num_entries);
    }

  return LDPS_OK;
}
