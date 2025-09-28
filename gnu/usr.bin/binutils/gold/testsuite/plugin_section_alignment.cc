// plugin_section_alignment.cc -- plugins to test ordering with {size,alignment}
//
// Copyright (C) 2016-2023 Free Software Foundation, Inc.
// Written by Than McIntosh <thanm@google.com>.
//
// This file is part of gold.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <algorithm>

#include "plugin-api.h"

static ld_plugin_get_input_section_count get_input_section_count = NULL;
static ld_plugin_get_input_section_alignment get_input_section_alignment = NULL;
static ld_plugin_get_input_section_size get_input_section_size = NULL;
static ld_plugin_get_input_section_name get_input_section_name = NULL;
static ld_plugin_update_section_order update_section_order = NULL;
static ld_plugin_allow_section_ordering allow_section_ordering = NULL;

extern "C" {
  enum ld_plugin_status onload(struct ld_plugin_tv *tv);
  enum ld_plugin_status claim_file_hook(const struct ld_plugin_input_file *file,
                                        int *claimed);
  enum ld_plugin_status all_symbols_read_hook(void);
}

typedef enum { FL_BSS, FL_DATA, FL_RODATA, FL_UNKNOWN } sec_flavor;

inline static int is_prefix_of(const char *prefix, const char *str)
{
  return strncmp(prefix, str, strlen(prefix)) == 0;
}

inline static sec_flavor flavor_from_name(const char *secname)
{
  if (is_prefix_of(".data.v", secname)) {
    return FL_DATA;
  } else if (is_prefix_of(".bss.v", secname)) {
    return FL_BSS;
  } else if (is_prefix_of(".rodata.v", secname)) {
    return FL_RODATA;
  } else {
    return FL_UNKNOWN;
  }
}

struct SectionInfo {
  ld_plugin_section plugin_section;
  std::string name;
  uint64_t size;
  sec_flavor flavor;
  unsigned align;

  static bool SectionInfoLt(const SectionInfo &i1,
                            const SectionInfo &i2)
  {
    if (i1.flavor != i2.flavor) {
      return ((unsigned) i1.flavor) < ((unsigned) i2.flavor);
    }
    switch (i1.flavor) {
      case FL_DATA:
        // Sort data items by increasing alignment then increasing size
        if (i1.align != i2.align) {
          return ((unsigned) i1.align) < ((unsigned) i2.align);
        }
        if (i1.size != i2.size) {
          return ((unsigned) i1.size) < ((unsigned) i2.size);
        }
        break;
      case FL_BSS:
        // Sort bss items by decreasing alignment then decreasing size
        if (i1.align != i2.align) {
          return ((unsigned) i2.align) < ((unsigned) i1.align);
        }
        if (i1.size != i2.size) {
          return ((unsigned) i2.size) < ((unsigned) i1.size);
        }
        break;
      case FL_RODATA:
        // Sort rodata items by decreasing alignment then increasing size
        if (i1.align != i2.align) {
          return ((unsigned) i2.align) < ((unsigned) i1.align);
        }
        if (i1.size != i2.size) {
          return ((unsigned) i1.size) < ((unsigned) i2.size);
        }
      default:
        break;
    }

    // Break ties by name
    return i1.name.compare(i2.name) < 0;
  }

};
typedef std::vector<SectionInfo> section_info_vector;
section_info_vector raw_sections;

// Set to 1 for debugging output
unsigned trace = 0;

extern "C" {

// Plugin entry point.
enum ld_plugin_status onload(struct ld_plugin_tv *tv)
{
  struct ld_plugin_tv *entry;
  for (entry = tv; entry->tv_tag != LDPT_NULL; ++entry)
  {
    switch (entry->tv_tag)
    {
      case LDPT_OPTION:
        if (!strcmp(entry->tv_u.tv_string,"-trace")) {
          fprintf(stderr, "enabling tracing\n");
          trace += 1;
        } else {
          fprintf(stderr, "unknown plugin option %s", entry->tv_u.tv_string);
        }
        break;
      case LDPT_REGISTER_CLAIM_FILE_HOOK:
        assert((*entry->tv_u.tv_register_claim_file)(claim_file_hook) ==
               LDPS_OK);
        break;
      case LDPT_REGISTER_ALL_SYMBOLS_READ_HOOK:
        assert((*entry->tv_u.tv_register_all_symbols_read)(
            all_symbols_read_hook) == LDPS_OK);
        break;
      case LDPT_GET_INPUT_SECTION_COUNT:
        get_input_section_count = *entry->tv_u.tv_get_input_section_count;
        break;
      case LDPT_GET_INPUT_SECTION_NAME:
        get_input_section_name = *entry->tv_u.tv_get_input_section_name;
        break;
      case LDPT_GET_INPUT_SECTION_ALIGNMENT:
        get_input_section_alignment =
            *entry->tv_u.tv_get_input_section_alignment;
        break;
      case LDPT_GET_INPUT_SECTION_SIZE:
        get_input_section_size = *entry->tv_u.tv_get_input_section_size;
        break;
      case LDPT_UPDATE_SECTION_ORDER:
        update_section_order = *entry->tv_u.tv_update_section_order;
        break;
      case LDPT_ALLOW_SECTION_ORDERING:
        allow_section_ordering = *entry->tv_u.tv_allow_section_ordering;
        break;
      default:
        break;
    }
  }

  if (get_input_section_count == NULL || get_input_section_alignment == NULL ||
      get_input_section_size == NULL || get_input_section_name == NULL ||
      update_section_order == NULL || allow_section_ordering == NULL) {
    fprintf(stderr, "Some interfaces are missing\n");
    return LDPS_ERR;
  }

  return LDPS_OK;
}

// This function is called by the linker for every new object it encounters.
enum ld_plugin_status claim_file_hook(const struct ld_plugin_input_file *file,
                                      int *claimed)
{
  static bool is_ordering_specified = false;
  struct ld_plugin_section section;
  unsigned count = 0;
  unsigned shndx;

  *claimed = 0;
  if (!is_ordering_specified) {
    // Inform the linker to prepare for section reordering.
    (*allow_section_ordering)();
    is_ordering_specified = true;
  }

  (*get_input_section_count)(file->handle, &count);

  for (shndx = 0; shndx < count; ++shndx) {
    char *name = NULL;

    section.handle = file->handle;
    section.shndx = shndx;
    (*get_input_section_name)(section, &name);
    if (is_prefix_of(".data.v", name) ||
        is_prefix_of(".bss.v", name) ||
        is_prefix_of(".rodata.v", name)) {
      SectionInfo si;
      si.plugin_section.handle = file->handle;
      si.plugin_section.shndx = shndx;
      (*get_input_section_size)(section, &si.size);
      (*get_input_section_alignment)(section, &si.align);
      si.name = name;
      si.flavor = flavor_from_name(name);
      raw_sections.push_back(si);
    }
  }

  return LDPS_OK;
}

// This function is called by the linker after all the symbols have been read.
// At this stage, it is fine to tell the linker the desired function order.

enum ld_plugin_status all_symbols_read_hook(void)
{
  // We expect to see a total of twelve sections -- if this is not the case
  // then something went wrong somewhere along the way.
  if (raw_sections.size() != 12) {
    fprintf(stderr, "Expected 12 sections, found %u sections",
            (unsigned) raw_sections.size());
    return LDPS_ERR;
  }

  std::sort(raw_sections.begin(), raw_sections.end(),
            SectionInfo::SectionInfoLt);

  struct ld_plugin_section section_list[12];
  for (unsigned ii = 0; ii < 12; ++ii) {
    section_list[ii] = raw_sections[ii].plugin_section;
  }

  if (trace) {
    fprintf(stderr, "Specified section order is:\n");
    for (section_info_vector::iterator it = raw_sections.begin();
         it != raw_sections.end();
         ++it) {
      const SectionInfo &si = (*it);
      fprintf(stderr, "Idx=%u Name=%s Align=%u Size=%u\n",
              si.plugin_section.shndx, si.name.c_str(), si.align,
              (unsigned) si.size);
    }
  }

  update_section_order(section_list, 12);

  return LDPS_OK;
}

} // end extern "C"
