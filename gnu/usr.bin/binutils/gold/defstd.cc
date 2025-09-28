// defstd.cc -- define standard symbols for gold.

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of gold.

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
// MA 02110-1301, USA.

#include "gold.h"

#include "symtab.h"
#include "layout.h"
#include "defstd.h"

// This is a simple file which defines the standard symbols like
// "_end".

namespace
{

using namespace gold;

const Define_symbol_in_section in_section[] =
{
  {
    "__preinit_array_start",	// name
    ".preinit_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    false,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__preinit_array_end",	// name
    ".preinit_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    true,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__init_array_start",	// name
    ".init_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    false,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__init_array_end",		// name
    ".init_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    true,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__fini_array_start",	// name
    ".fini_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    false,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__fini_array_end",		// name
    ".fini_array",		// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    true,			// offset_is_from_end
    true			// only_if_ref
  },
  {
    "__stack",			// name
    ".stack",			// output_section
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    false,			// offset_is_from_end
    true			// only_if_ref
  },
};

const int in_section_count = sizeof in_section / sizeof in_section[0];

const Define_symbol_in_segment in_segment[] =
{
  {
    "__executable_start",	// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF(0),		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_START,	// offset_from_base
    true			// only_if_ref
  },
  {
    "__ehdr_start",		// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF(0),		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_HIDDEN,		// visibility
    0,				// nonvis
    Symbol::SEGMENT_START,	// offset_from_base
    true			// only_if_ref
  },
  {
    "etext",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_X,		// segment_flags_set
    elfcpp::PF_W,		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_END,	// offset_from_base
    true			// only_if_ref
  },
  {
    "_etext",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_X,		// segment_flags_set
    elfcpp::PF_W,		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_END,	// offset_from_base
    true			// only_if_ref
  },
  {
    "__etext",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_X,		// segment_flags_set
    elfcpp::PF_W,		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_END,	// offset_from_base
    true			// only_if_ref
  },
  {
    "_edata",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_W,		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_BSS,	// offset_from_base
    false			// only_if_ref
  },
  {
    "edata",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_W,		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_BSS,	// offset_from_base
    true			// only_if_ref
  },
  {
    "__bss_start",		// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_W,		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_BSS,	// offset_from_base
    false			// only_if_ref
  },
  {
    "_end",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_W,		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_END,	// offset_from_base
    false			// only_if_ref
  },
  {
    "end",			// name
    elfcpp::PT_LOAD,		// segment_type
    elfcpp::PF_W,		// segment_flags_set
    elfcpp::PF(0),		// segment_flags_clear
    0,				// value
    0,				// size
    elfcpp::STT_NOTYPE,		// type
    elfcpp::STB_GLOBAL,		// binding
    elfcpp::STV_DEFAULT,	// visibility
    0,				// nonvis
    Symbol::SEGMENT_END,	// offset_from_base
    true			// only_if_ref
  }
};

const int in_segment_count = sizeof in_segment / sizeof in_segment[0];

} // End anonymous namespace.

namespace gold
{

void
define_standard_symbols(Symbol_table* symtab, const Layout* layout)
{
  bool saw_sections_clause = layout->script_options()->saw_sections_clause();
  symtab->define_symbols(layout, in_section_count, in_section,
			 saw_sections_clause);
  symtab->define_symbols(layout, in_segment_count, in_segment,
			 saw_sections_clause);
}

} // End namespace gold.
