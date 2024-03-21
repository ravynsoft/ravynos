// common.cc -- handle common symbols for gold

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

#include <algorithm>

#include "workqueue.h"
#include "mapfile.h"
#include "layout.h"
#include "output.h"
#include "symtab.h"
#include "common.h"

namespace gold
{

// Allocate_commons_task methods.

// This task allocates the common symbols.  We arrange to run it
// before anything else which needs to access the symbol table.

Task_token*
Allocate_commons_task::is_runnable()
{
  return NULL;
}

// Release a blocker.

void
Allocate_commons_task::locks(Task_locker* tl)
{
  tl->add(this, this->blocker_);
}

// Allocate the common symbols.

void
Allocate_commons_task::run(Workqueue*)
{
  this->symtab_->allocate_commons(this->layout_, this->mapfile_);
}

// This class is used to sort the common symbol.  We normally put the
// larger common symbols first.  This can be changed by using
// --sort-commons, which tells the linker to sort by alignment.

template<int size>
class Sort_commons
{
 public:
  Sort_commons(const Symbol_table* symtab,
	       Symbol_table::Sort_commons_order sort_order)
    : symtab_(symtab), sort_order_(sort_order)
  { }

  bool operator()(const Symbol* a, const Symbol* b) const;

 private:
  // The symbol table.
  const Symbol_table* symtab_;
  // How to sort.
  Symbol_table::Sort_commons_order sort_order_;
};

template<int size>
bool
Sort_commons<size>::operator()(const Symbol* pa, const Symbol* pb) const
{
  if (pa == NULL)
    return false;
  if (pb == NULL)
    return true;

  const Symbol_table* symtab = this->symtab_;
  const Sized_symbol<size>* psa = symtab->get_sized_symbol<size>(pa);
  const Sized_symbol<size>* psb = symtab->get_sized_symbol<size>(pb);

  // The size.
  typename Sized_symbol<size>::Size_type sa = psa->symsize();
  typename Sized_symbol<size>::Size_type sb = psb->symsize();

  // The alignment.
  typename Sized_symbol<size>::Value_type aa = psa->value();
  typename Sized_symbol<size>::Value_type ab = psb->value();

  if (this->sort_order_ == Symbol_table::SORT_COMMONS_BY_ALIGNMENT_DESCENDING)
    {
      if (aa < ab)
	return false;
      else if (ab < aa)
	return true;
    }
  else if (this->sort_order_
	   == Symbol_table::SORT_COMMONS_BY_ALIGNMENT_ASCENDING)
    {
      if (aa < ab)
	return true;
      else if (ab < aa)
	return false;
    }
  else
    gold_assert(this->sort_order_
		== Symbol_table::SORT_COMMONS_BY_SIZE_DESCENDING);

  // Sort by descending size.
  if (sa < sb)
    return false;
  else if (sb < sa)
    return true;

  if (this->sort_order_ == Symbol_table::SORT_COMMONS_BY_SIZE_DESCENDING)
    {
      // When the symbols are the same size, we sort them by
      // alignment, largest alignment first.
      if (aa < ab)
	return false;
      else if (ab < aa)
	return true;
    }

  // Otherwise we stabilize the sort by sorting by name.
  return strcmp(psa->name(), psb->name()) < 0;
}

// Allocate the common symbols.

void
Symbol_table::allocate_commons(Layout* layout, Mapfile* mapfile)
{
  Sort_commons_order sort_order;
  if (!parameters->options().user_set_sort_common())
    sort_order = SORT_COMMONS_BY_SIZE_DESCENDING;
  else
    {
      const char* order = parameters->options().sort_common();
      if (*order == '\0' || strcmp(order, "descending") == 0)
	sort_order = SORT_COMMONS_BY_ALIGNMENT_DESCENDING;
      else if (strcmp(order, "ascending") == 0)
	sort_order = SORT_COMMONS_BY_ALIGNMENT_ASCENDING;
      else
	{
	  gold_error("invalid --sort-common argument: %s", order);
	  sort_order = SORT_COMMONS_BY_SIZE_DESCENDING;
	}
    }

  if (parameters->target().get_size() == 32)
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
      this->do_allocate_commons<32>(layout, mapfile, sort_order);
#else
      gold_unreachable();
#endif
    }
  else if (parameters->target().get_size() == 64)
    {
#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
      this->do_allocate_commons<64>(layout, mapfile, sort_order);
#else
      gold_unreachable();
#endif
    }
  else
    gold_unreachable();
}

// Allocated the common symbols, sized version.

template<int size>
void
Symbol_table::do_allocate_commons(Layout* layout, Mapfile* mapfile,
				  Sort_commons_order sort_order)
{
  if (!this->commons_.empty())
    this->do_allocate_commons_list<size>(layout, COMMONS_NORMAL,
					 &this->commons_, mapfile,
					 sort_order);
  if (!this->tls_commons_.empty())
    this->do_allocate_commons_list<size>(layout, COMMONS_TLS,
					 &this->tls_commons_, mapfile,
					 sort_order);
  if (!this->small_commons_.empty())
    this->do_allocate_commons_list<size>(layout, COMMONS_SMALL,
					 &this->small_commons_, mapfile,
					 sort_order);
  if (!this->large_commons_.empty())
    this->do_allocate_commons_list<size>(layout, COMMONS_LARGE,
					 &this->large_commons_, mapfile,
					 sort_order);
}

// Allocate the common symbols in a list.  IS_TLS indicates whether
// these are TLS common symbols.

template<int size>
void
Symbol_table::do_allocate_commons_list(
    Layout* layout,
    Commons_section_type commons_section_type,
    Commons_type* commons,
    Mapfile* mapfile,
    Sort_commons_order sort_order)
{
  // We've kept a list of all the common symbols.  But the symbol may
  // have been resolved to a defined symbol by now.  And it may be a
  // forwarder.  First remove all non-common symbols.
  bool any = false;
  uint64_t addralign = 0;
  for (Commons_type::iterator p = commons->begin();
       p != commons->end();
       ++p)
    {
      Symbol* sym = *p;
      if (sym->is_forwarder())
	{
	  sym = this->resolve_forwards(sym);
	  *p = sym;
	}
      if (!sym->is_common())
	*p = NULL;
      else
	{
	  any = true;
	  Sized_symbol<size>* ssym = this->get_sized_symbol<size>(sym);
	  if (ssym->value() > addralign)
	    addralign = ssym->value();
	}
    }
  if (!any)
    return;

  // Sort the common symbols.
  std::sort(commons->begin(), commons->end(),
	    Sort_commons<size>(this, sort_order));

  // Place them in a newly allocated BSS section.
  elfcpp::Elf_Xword flags = elfcpp::SHF_WRITE | elfcpp::SHF_ALLOC;
  const char* name;
  const char* ds_name;
  switch (commons_section_type)
    {
    case COMMONS_NORMAL:
      name = ".bss";
      ds_name = "** common";
      break;
    case COMMONS_TLS:
      flags |= elfcpp::SHF_TLS;
      name = ".tbss";
      ds_name = "** tls common";
      break;
    case COMMONS_SMALL:
      flags |= parameters->target().small_common_section_flags();
      name = ".sbss";
      ds_name = "** small common";
      break;
    case COMMONS_LARGE:
      flags |= parameters->target().large_common_section_flags();
      name = ".lbss";
      ds_name = "** large common";
      break;
    default:
      gold_unreachable();
    }

  Output_data_space* poc;
  Output_section* os;

  if (!parameters->incremental_update())
    {
      poc = new Output_data_space(addralign, ds_name);
      os = layout->add_output_section_data(name, elfcpp::SHT_NOBITS, flags,
					   poc, ORDER_INVALID, false);
    }
  else
    {
      // When doing an incremental update, we need to allocate each common
      // directly from the output section's free list.
      poc = NULL;
      os = layout->find_output_section(name);
    }

  if (os != NULL)
    {
      if (commons_section_type == COMMONS_SMALL)
	os->set_is_small_section();
      else if (commons_section_type == COMMONS_LARGE)
	os->set_is_large_section();
    }

  // Allocate them all.

  off_t off = 0;
  for (Commons_type::iterator p = commons->begin();
       p != commons->end();
       ++p)
    {
      Symbol* sym = *p;
      if (sym == NULL)
	break;

      // Because we followed forwarding symbols above, but we didn't
      // do it reliably before adding symbols to the list, it is
      // possible for us to have the same symbol on the list twice.
      // This can happen in the horrible case where a program defines
      // a common symbol with the same name as a versioned libc
      // symbol.  That will show up here as a symbol which has already
      // been allocated and is therefore no longer a common symbol.
      if (!sym->is_common())
	continue;

      Sized_symbol<size>* ssym = this->get_sized_symbol<size>(sym);

      // Record the symbol in the map file now, before we change its
      // value.  Pass the size in separately so that we don't have to
      // templatize the map code, which is not performance sensitive.
      if (mapfile != NULL)
	mapfile->report_allocate_common(sym, ssym->symsize());

      if (poc != NULL)
	{
	  off = align_address(off, ssym->value());
	  ssym->allocate_common(poc, off);
	  off += ssym->symsize();
	}
      else
	{
	  // For an incremental update, allocate from the free list.
	  off = os->allocate(ssym->symsize(), ssym->value());
	  if (off == -1)
	    gold_fallback(_("out of patch space in section %s; "
			    "relink with --incremental-full"),
			  os->name());
	  ssym->allocate_common(os, off);
	}
    }

  if (poc != NULL)
    poc->set_current_data_size(off);

  commons->clear();
}

} // End namespace gold.
