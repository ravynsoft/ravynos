// dwarf_reader.cc -- parse dwarf2/3 debug information

// Copyright (C) 2007-2023 Free Software Foundation, Inc.
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
#include <utility>
#include <vector>

#include "debug.h"
#include "elfcpp_swap.h"
#include "dwarf.h"
#include "object.h"
#include "reloc.h"
#include "dwarf_reader.h"
#include "int_encoding.h"
#include "compressed_output.h"

namespace gold {

// Class Sized_elf_reloc_mapper

// Initialize the relocation tracker for section RELOC_SHNDX.

template<int size, bool big_endian>
bool
Sized_elf_reloc_mapper<size, big_endian>::do_initialize(
    unsigned int reloc_shndx, unsigned int reloc_type)
{
  this->reloc_type_ = reloc_type;
  return this->track_relocs_.initialize(this->object_, reloc_shndx,
					reloc_type);
}

// Looks in the symtab to see what section a symbol is in.

template<int size, bool big_endian>
unsigned int
Sized_elf_reloc_mapper<size, big_endian>::symbol_section(
    unsigned int symndx, Address* value, bool* is_ordinary)
{
  const int symsize = elfcpp::Elf_sizes<size>::sym_size;
  gold_assert(static_cast<off_t>((symndx + 1) * symsize) <= this->symtab_size_);
  elfcpp::Sym<size, big_endian> elfsym(this->symtab_ + symndx * symsize);
  *value = elfsym.get_st_value();
  return this->object_->adjust_sym_shndx(symndx, elfsym.get_st_shndx(),
					 is_ordinary);
}

// Return the section index and offset within the section of
// the target of the relocation for RELOC_OFFSET.

template<int size, bool big_endian>
unsigned int
Sized_elf_reloc_mapper<size, big_endian>::do_get_reloc_target(
    off_t reloc_offset, off_t* target_offset)
{
  this->track_relocs_.advance(reloc_offset);
  if (reloc_offset != this->track_relocs_.next_offset())
    return 0;
  unsigned int symndx = this->track_relocs_.next_symndx();
  typename elfcpp::Elf_types<size>::Elf_Addr value;
  bool is_ordinary;
  unsigned int target_shndx = this->symbol_section(symndx, &value,
						   &is_ordinary);
  if (!is_ordinary)
    return 0;
  if (this->reloc_type_ == elfcpp::SHT_RELA)
    value += this->track_relocs_.next_addend();
  *target_offset = value;
  return target_shndx;
}

static inline Elf_reloc_mapper*
make_elf_reloc_mapper(Relobj* object, const unsigned char* symtab,
		      off_t symtab_size)
{
  if (object->elfsize() == 32)
    {
      if (object->is_big_endian())
        {
#ifdef HAVE_TARGET_32_BIG
	  return new Sized_elf_reloc_mapper<32, true>(object, symtab,
						      symtab_size);
#else
	  gold_unreachable();
#endif
        }
      else
        {
#ifdef HAVE_TARGET_32_LITTLE
	  return new Sized_elf_reloc_mapper<32, false>(object, symtab,
						       symtab_size);
#else
	  gold_unreachable();
#endif
        }
    }
  else if (object->elfsize() == 64)
    {
      if (object->is_big_endian())
        {
#ifdef HAVE_TARGET_64_BIG
	  return new Sized_elf_reloc_mapper<64, true>(object, symtab,
						      symtab_size);
#else
	  gold_unreachable();
#endif
        }
      else
        {
#ifdef HAVE_TARGET_64_LITTLE
	  return new Sized_elf_reloc_mapper<64, false>(object, symtab,
						       symtab_size);
#else
	  gold_unreachable();
#endif
        }
    }
  else
    gold_unreachable();
}

// class Dwarf_abbrev_table

void
Dwarf_abbrev_table::clear_abbrev_codes()
{
  for (unsigned int code = 0; code < this->low_abbrev_code_max_; ++code)
    {
      if (this->low_abbrev_codes_[code] != NULL)
	{
	  delete this->low_abbrev_codes_[code];
	  this->low_abbrev_codes_[code] = NULL;
	}
    }
  for (Abbrev_code_table::iterator it = this->high_abbrev_codes_.begin();
       it != this->high_abbrev_codes_.end();
       ++it)
    {
      if (it->second != NULL)
	delete it->second;
    }
  this->high_abbrev_codes_.clear();
}

// Read the abbrev table from an object file.

bool
Dwarf_abbrev_table::do_read_abbrevs(
    Relobj* object,
    unsigned int abbrev_shndx,
    off_t abbrev_offset)
{
  this->clear_abbrev_codes();

  // If we don't have relocations, abbrev_shndx will be 0, and
  // we'll have to hunt for the .debug_abbrev section.
  if (abbrev_shndx == 0 && this->abbrev_shndx_ > 0)
    abbrev_shndx = this->abbrev_shndx_;
  else if (abbrev_shndx == 0)
    {
      for (unsigned int i = 1; i < object->shnum(); ++i)
	{
	  std::string name = object->section_name(i);
	  if (name == ".debug_abbrev" || name == ".zdebug_abbrev")
	    {
	      abbrev_shndx = i;
	      // Correct the offset.  For incremental update links, we have a
	      // relocated offset that is relative to the output section, but
	      // here we need an offset relative to the input section.
	      abbrev_offset -= object->output_section_offset(i);
	      break;
	    }
	}
      if (abbrev_shndx == 0)
	return false;
    }

  // Get the section contents and decompress if necessary.
  if (abbrev_shndx != this->abbrev_shndx_)
    {
      if (this->owns_buffer_ && this->buffer_ != NULL)
        {
	  delete[] this->buffer_;
	  this->owns_buffer_ = false;
        }

      section_size_type buffer_size;
      this->buffer_ =
	  object->decompressed_section_contents(abbrev_shndx,
						&buffer_size,
						&this->owns_buffer_);
      this->buffer_end_ = this->buffer_ + buffer_size;
      this->abbrev_shndx_ = abbrev_shndx;
    }

  this->buffer_pos_ = this->buffer_ + abbrev_offset;
  return true;
}

// Lookup the abbrev code entry for CODE.  This function is called
// only when the abbrev code is not in the direct lookup table.
// It may be in the hash table, it may not have been read yet,
// or it may not exist in the abbrev table.

const Dwarf_abbrev_table::Abbrev_code*
Dwarf_abbrev_table::do_get_abbrev(unsigned int code)
{
  // See if the abbrev code is already in the hash table.
  Abbrev_code_table::const_iterator it = this->high_abbrev_codes_.find(code);
  if (it != this->high_abbrev_codes_.end())
    return it->second;

  // Read and store abbrev code definitions until we find the
  // one we're looking for.
  for (;;)
    {
      // Read the abbrev code.  A zero here indicates the end of the
      // abbrev table.
      size_t len;
      if (this->buffer_pos_ >= this->buffer_end_)
	return NULL;
      uint64_t nextcode = read_unsigned_LEB_128(this->buffer_pos_, &len);
      if (nextcode == 0)
	{
	  this->buffer_pos_ = this->buffer_end_;
	  return NULL;
	}
      this->buffer_pos_ += len;

      // Read the tag.
      if (this->buffer_pos_ >= this->buffer_end_)
	return NULL;
      uint64_t tag = read_unsigned_LEB_128(this->buffer_pos_, &len);
      this->buffer_pos_ += len;

      // Read the has_children flag.
      if (this->buffer_pos_ >= this->buffer_end_)
	return NULL;
      bool has_children = *this->buffer_pos_ == elfcpp::DW_CHILDREN_yes;
      this->buffer_pos_ += 1;

      // Read the list of (attribute, form) pairs.
      Abbrev_code* entry = new Abbrev_code(tag, has_children);
      for (;;)
	{
	  // Read the attribute.
	  if (this->buffer_pos_ >= this->buffer_end_)
	    return NULL;
	  uint64_t attr = read_unsigned_LEB_128(this->buffer_pos_, &len);
	  this->buffer_pos_ += len;

	  // Read the form.
	  if (this->buffer_pos_ >= this->buffer_end_)
	    return NULL;
	  uint64_t form = read_unsigned_LEB_128(this->buffer_pos_, &len);
	  this->buffer_pos_ += len;

	  // For DW_FORM_implicit_const, read the constant.
	  int64_t implicit_const = 0;
	  if (form == elfcpp::DW_FORM_implicit_const)
	    {
	      implicit_const = read_signed_LEB_128(this->buffer_pos_, &len);
	      this->buffer_pos_ += len;
	    }

	  // A (0,0) pair terminates the list.
	  if (attr == 0 && form == 0)
	    break;

	  if (attr == elfcpp::DW_AT_sibling)
	    entry->has_sibling_attribute = true;

	  entry->add_attribute(attr, form, implicit_const);
	}

      this->store_abbrev(nextcode, entry);
      if (nextcode == code)
	return entry;
    }

  return NULL;
}

// class Dwarf_ranges_table

// Read the ranges table from an object file.

bool
Dwarf_ranges_table::read_ranges_table(
    Relobj* object,
    const unsigned char* symtab,
    off_t symtab_size,
    unsigned int ranges_shndx,
    unsigned int version)
{
  const std::string section_name(version < 5
				 ? ".debug_ranges"
				 : ".debug_rnglists");
  const std::string compressed_section_name(version < 5
					    ? ".zdebug_ranges"
					    : ".zdebug_rnglists");

  // If we've already read this abbrev table, return immediately.
  if (this->ranges_shndx_ > 0
      && this->ranges_shndx_ == ranges_shndx)
    return true;

  // If we don't have relocations, ranges_shndx will be 0, and
  // we'll have to hunt for the .debug_ranges section.
  if (ranges_shndx == 0 && this->ranges_shndx_ > 0)
    ranges_shndx = this->ranges_shndx_;
  else if (ranges_shndx == 0)
    {
      for (unsigned int i = 1; i < object->shnum(); ++i)
	{
	  std::string name = object->section_name(i);
	  if (name == section_name || name == compressed_section_name)
	    {
	      ranges_shndx = i;
	      this->output_section_offset_ = object->output_section_offset(i);
	      break;
	    }
	}
      if (ranges_shndx == 0)
	return false;
    }

  // Get the section contents and decompress if necessary.
  if (ranges_shndx != this->ranges_shndx_)
    {
      if (this->owns_ranges_buffer_ && this->ranges_buffer_ != NULL)
        {
	  delete[] this->ranges_buffer_;
	  this->owns_ranges_buffer_ = false;
        }

      section_size_type buffer_size;
      this->ranges_buffer_ =
	  object->decompressed_section_contents(ranges_shndx,
						&buffer_size,
						&this->owns_ranges_buffer_);
      this->ranges_buffer_end_ = this->ranges_buffer_ + buffer_size;
      this->ranges_shndx_ = ranges_shndx;
    }

  if (this->ranges_reloc_mapper_ != NULL)
    {
      delete this->ranges_reloc_mapper_;
      this->ranges_reloc_mapper_ = NULL;
    }

  // For incremental objects, we have no relocations.
  if (object->is_incremental())
    return true;

  // Find the relocation section for ".debug_ranges".
  unsigned int reloc_shndx = 0;
  unsigned int reloc_type = 0;
  for (unsigned int i = 0; i < object->shnum(); ++i)
    {
      reloc_type = object->section_type(i);
      if ((reloc_type == elfcpp::SHT_REL
	   || reloc_type == elfcpp::SHT_RELA)
	  && object->section_info(i) == ranges_shndx)
	{
	  reloc_shndx = i;
	  break;
	}
    }

  this->ranges_reloc_mapper_ = make_elf_reloc_mapper(object, symtab,
						     symtab_size);
  this->ranges_reloc_mapper_->initialize(reloc_shndx, reloc_type);
  this->reloc_type_ = reloc_type;

  return true;
}

// Read a range list from section RANGES_SHNDX at offset RANGES_OFFSET.

Dwarf_range_list*
Dwarf_ranges_table::read_range_list(
    Relobj* object,
    const unsigned char* symtab,
    off_t symtab_size,
    unsigned int addr_size,
    unsigned int ranges_shndx,
    off_t offset)
{
  Dwarf_range_list* ranges;

  if (!this->read_ranges_table(object, symtab, symtab_size, ranges_shndx, 4))
    return NULL;

  // Correct the offset.  For incremental update links, we have a
  // relocated offset that is relative to the output section, but
  // here we need an offset relative to the input section.
  offset -= this->output_section_offset_;

  // Read the range list at OFFSET.
  ranges = new Dwarf_range_list();
  off_t base = 0;
  for (;
       this->ranges_buffer_ + offset < this->ranges_buffer_end_;
       offset += 2 * addr_size)
    {
      off_t start;
      off_t end;

      // Read the raw contents of the section.
      if (addr_size == 4)
	{
	  start = this->dwinfo_->read_from_pointer<32>(this->ranges_buffer_
						       + offset);
	  end = this->dwinfo_->read_from_pointer<32>(this->ranges_buffer_
						     + offset + 4);
	}
      else
	{
	  start = this->dwinfo_->read_from_pointer<64>(this->ranges_buffer_
						       + offset);
	  end = this->dwinfo_->read_from_pointer<64>(this->ranges_buffer_
						     + offset + 8);
	}

      // Check for relocations and adjust the values.
      unsigned int shndx1 = 0;
      unsigned int shndx2 = 0;
      if (this->ranges_reloc_mapper_ != NULL)
        {
	  shndx1 = this->lookup_reloc(offset, &start);
	  shndx2 = this->lookup_reloc(offset + addr_size, &end);
        }

      // End of list is marked by a pair of zeroes.
      if (shndx1 == 0 && start == 0 && end == 0)
        break;

      // A "base address selection entry" is identified by
      // 0xffffffff for the first value of the pair.  The second
      // value is used as a base for subsequent range list entries.
      if (shndx1 == 0 && start == -1)
	base = end;
      else if (shndx1 == shndx2)
	{
	  if (shndx1 == 0 || object->is_section_included(shndx1))
	    ranges->add(shndx1, base + start, base + end);
	}
      else
	gold_warning(_("%s: DWARF info may be corrupt; offsets in a "
		       "range list entry are in different sections"),
		     object->name().c_str());
    }

  return ranges;
}

// Read a DWARF 5 range list from section RANGES_SHNDX at offset RANGES_OFFSET.

Dwarf_range_list*
Dwarf_ranges_table::read_range_list_v5(
    Relobj* object,
    const unsigned char* symtab,
    off_t symtab_size,
    unsigned int addr_size,
    unsigned int ranges_shndx,
    off_t offset)
{
  Dwarf_range_list* ranges;

  if (!this->read_ranges_table(object, symtab, symtab_size, ranges_shndx, 5))
    return NULL;

  ranges = new Dwarf_range_list();
  off_t base = 0;
  unsigned int shndx0 = 0;

  // Correct the offset.  For incremental update links, we have a
  // relocated offset that is relative to the output section, but
  // here we need an offset relative to the input section.
  offset -= this->output_section_offset_;

  // Read the range list at OFFSET.
  const unsigned char* prle = this->ranges_buffer_ + offset;
  while (prle < this->ranges_buffer_end_)
    {
      off_t start;
      off_t end;
      unsigned int shndx1 = 0;
      unsigned int shndx2 = 0;
      size_t len;

      // Read the entry type.
      unsigned int rle_type = *prle++;
      offset += 1;

      if (rle_type == elfcpp::DW_RLE_end_of_list)
	break;

      switch (rle_type)
	{
	  case elfcpp::DW_RLE_base_address:
	    if (addr_size == 4)
	      base = this->dwinfo_->read_from_pointer<32>(prle);
	    else
	      base = this->dwinfo_->read_from_pointer<64>(prle);
	    if (this->ranges_reloc_mapper_ != NULL)
		shndx0 = this->lookup_reloc(offset, &base);
	    prle += addr_size;
	    offset += addr_size;
	    break;

	  case elfcpp::DW_RLE_offset_pair:
	    start = read_unsigned_LEB_128(prle, &len);
	    prle += len;
	    offset += len;
	    end = read_unsigned_LEB_128(prle, &len);
	    prle += len;
	    offset += len;
	    if (shndx0 == 0 || object->is_section_included(shndx0))
	      ranges->add(shndx0, base + start, base + end);
	    break;

	  case elfcpp::DW_RLE_start_end:
	    if (addr_size == 4)
	      {
		start = this->dwinfo_->read_from_pointer<32>(prle);
		end = this->dwinfo_->read_from_pointer<32>(prle + 4);
	      }
	    else
	      {
		start = this->dwinfo_->read_from_pointer<64>(prle);
		end = this->dwinfo_->read_from_pointer<64>(prle + 8);
	      }
	    if (this->ranges_reloc_mapper_ != NULL)
	      {
		shndx1 = this->lookup_reloc(offset, &start);
		shndx2 = this->lookup_reloc(offset + addr_size, &end);
		if (shndx1 != shndx2)
		  gold_warning(_("%s: DWARF info may be corrupt; offsets in a "
				 "range list entry are in different sections"),
			       object->name().c_str());
	      }
	    prle += addr_size * 2;
	    offset += addr_size * 2;
	    if (shndx1 == 0 || object->is_section_included(shndx1))
	      ranges->add(shndx1, start, end);
	    break;

	  case elfcpp::DW_RLE_start_length:
	    if (addr_size == 4)
	      start = this->dwinfo_->read_from_pointer<32>(prle);
	    else
	      start = this->dwinfo_->read_from_pointer<64>(prle);
	    if (this->ranges_reloc_mapper_ != NULL)
	      shndx1 = this->lookup_reloc(offset, &start);
	    prle += addr_size;
	    offset += addr_size;
	    end = start + read_unsigned_LEB_128(prle, &len);
	    prle += len;
	    offset += len;
	    if (shndx1 == 0 || object->is_section_included(shndx1))
	      ranges->add(shndx1, start, end);
	    break;

	  default:
	    gold_warning(_("%s: DWARF range list contains "
			   "unsupported entry type (%d)"),
			 object->name().c_str(), rle_type);
	    break;
	}
    }

  return ranges;
}

// Look for a relocation at offset OFF in the range table,
// and return the section index and offset of the target.

unsigned int
Dwarf_ranges_table::lookup_reloc(off_t off, off_t* target_off)
{
  off_t value;
  unsigned int shndx =
      this->ranges_reloc_mapper_->get_reloc_target(off, &value);
  if (shndx == 0)
    return 0;
  if (this->reloc_type_ == elfcpp::SHT_REL)
    *target_off += value;
  else
    *target_off = value;
  return shndx;
}

// class Dwarf_pubnames_table

// Read the pubnames section from the object file.

bool
Dwarf_pubnames_table::read_section(Relobj* object, const unsigned char* symtab,
                                   off_t symtab_size)
{
  section_size_type buffer_size;
  unsigned int shndx = 0;
  const char* name = this->is_pubtypes_ ? "pubtypes" : "pubnames";
  const char* gnu_name = (this->is_pubtypes_
			  ? "gnu_pubtypes"
			  : "gnu_pubnames");

  for (unsigned int i = 1; i < object->shnum(); ++i)
    {
      std::string section_name = object->section_name(i);
      const char* section_name_suffix = section_name.c_str();
      if (is_prefix_of(".debug_", section_name_suffix))
	section_name_suffix += 7;
      else if (is_prefix_of(".zdebug_", section_name_suffix))
	section_name_suffix += 8;
      else
	continue;
      if (strcmp(section_name_suffix, name) == 0)
        {
          shndx = i;
          break;
        }
      else if (strcmp(section_name_suffix, gnu_name) == 0)
        {
          shndx = i;
          this->is_gnu_style_ = true;
          break;
        }
    }
  if (shndx == 0)
    return false;

  this->buffer_ = object->decompressed_section_contents(shndx,
							&buffer_size,
							&this->owns_buffer_);
  if (this->buffer_ == NULL)
    return false;
  this->buffer_end_ = this->buffer_ + buffer_size;

  // For incremental objects, we have no relocations.
  if (object->is_incremental())
    return true;

  // Find the relocation section
  unsigned int reloc_shndx = 0;
  unsigned int reloc_type = 0;
  for (unsigned int i = 0; i < object->shnum(); ++i)
    {
      reloc_type = object->section_type(i);
      if ((reloc_type == elfcpp::SHT_REL
	   || reloc_type == elfcpp::SHT_RELA)
	  && object->section_info(i) == shndx)
	{
	  reloc_shndx = i;
	  break;
	}
    }

  this->reloc_mapper_ = make_elf_reloc_mapper(object, symtab, symtab_size);
  this->reloc_mapper_->initialize(reloc_shndx, reloc_type);
  this->reloc_type_ = reloc_type;

  return true;
}

// Read the header for the set at OFFSET.

bool
Dwarf_pubnames_table::read_header(off_t offset)
{
  // Make sure we have actually read the section.
  gold_assert(this->buffer_ != NULL);

  if (offset < 0 || offset + 14 >= this->buffer_end_ - this->buffer_)
    return false;

  const unsigned char* pinfo = this->buffer_ + offset;

  // Read the unit_length field.
  uint64_t unit_length = this->dwinfo_->read_from_pointer<32>(pinfo);
  pinfo += 4;
  if (unit_length == 0xffffffff)
    {
      unit_length = this->dwinfo_->read_from_pointer<64>(pinfo);
      this->unit_length_ = unit_length + 12;
      pinfo += 8;
      this->offset_size_ = 8;
    }
  else
    {
      this->unit_length_ = unit_length + 4;
      this->offset_size_ = 4;
    }
  this->end_of_table_ = pinfo + unit_length;

  // If unit_length is too big, maybe we should reject the whole table,
  // but in cases we know about, it seems OK to assume that the table
  // is valid through the actual end of the section.
  if (this->end_of_table_ > this->buffer_end_)
    this->end_of_table_ = this->buffer_end_;

  // Check the version.
  unsigned int version = this->dwinfo_->read_from_pointer<16>(pinfo);
  pinfo += 2;
  if (version != 2)
    return false;

  this->reloc_mapper_->get_reloc_target(pinfo - this->buffer_,
                                        &this->cu_offset_);

  // Skip the debug_info_offset and debug_info_size fields.
  pinfo += 2 * this->offset_size_;

  if (pinfo >= this->buffer_end_)
    return false;

  this->pinfo_ = pinfo;
  return true;
}

// Read the next name from the set.

const char*
Dwarf_pubnames_table::next_name(uint8_t* flag_byte)
{
  const unsigned char* pinfo = this->pinfo_;

  // Check for end of list.  The table should be terminated by an
  // entry containing nothing but a DIE offset of 0.
  if (pinfo + this->offset_size_ >= this->end_of_table_)
    return NULL;

  // Skip the offset within the CU.  If this is zero, but we're not
  // at the end of the table, then we have a real pubnames entry
  // whose DIE offset is 0 (likely to be a GCC bug).  Since we
  // don't actually use the DIE offset in building .gdb_index,
  // it's harmless.
  pinfo += this->offset_size_;

  if (this->is_gnu_style_)
    *flag_byte = *pinfo++;
  else
    *flag_byte = 0;

  // Return a pointer to the string at the current location,
  // and advance the pointer to the next entry.
  const char* ret = reinterpret_cast<const char*>(pinfo);
  while (pinfo < this->buffer_end_ && *pinfo != '\0')
    ++pinfo;
  if (pinfo < this->buffer_end_)
    ++pinfo;

  this->pinfo_ = pinfo;
  return ret;
}

// class Dwarf_die

Dwarf_die::Dwarf_die(
    Dwarf_info_reader* dwinfo,
    off_t die_offset,
    Dwarf_die* parent)
  : dwinfo_(dwinfo), parent_(parent), die_offset_(die_offset),
    child_offset_(0), sibling_offset_(0), abbrev_code_(NULL), attributes_(),
    attributes_read_(false), name_(NULL), name_off_(-1), linkage_name_(NULL),
    linkage_name_off_(-1), string_shndx_(0), specification_(0),
    abstract_origin_(0)
{
  size_t len;
  const unsigned char* pdie = dwinfo->buffer_at_offset(die_offset);
  if (pdie == NULL)
    return;
  unsigned int code = read_unsigned_LEB_128(pdie, &len);
  if (code == 0)
    {
      if (parent != NULL)
	parent->set_sibling_offset(die_offset + len);
      return;
    }
  this->attr_offset_ = len;

  // Lookup the abbrev code in the abbrev table.
  this->abbrev_code_ = dwinfo->get_abbrev(code);
}

// Read all the attributes of the DIE.

bool
Dwarf_die::read_attributes()
{
  if (this->attributes_read_)
    return true;

  gold_assert(this->abbrev_code_ != NULL);

  const unsigned char* pdie =
      this->dwinfo_->buffer_at_offset(this->die_offset_);
  if (pdie == NULL)
    return false;
  const unsigned char* pattr = pdie + this->attr_offset_;

  unsigned int nattr = this->abbrev_code_->attributes.size();
  this->attributes_.reserve(nattr);
  for (unsigned int i = 0; i < nattr; ++i)
    {
      size_t len;
      unsigned int attr = this->abbrev_code_->attributes[i].attr;
      unsigned int form = this->abbrev_code_->attributes[i].form;
      if (form == elfcpp::DW_FORM_indirect)
        {
          form = read_unsigned_LEB_128(pattr, &len);
          pattr += len;
        }
      off_t attr_off = this->die_offset_ + (pattr - pdie);
      bool ref_form = false;
      Attribute_value attr_value;
      attr_value.attr = attr;
      attr_value.form = form;
      attr_value.aux.shndx = 0;
      switch(form)
	{
	  case elfcpp::DW_FORM_flag_present:
	    attr_value.val.intval = 1;
	    break;
	  case elfcpp::DW_FORM_implicit_const:
	    attr_value.val.intval =
		this->abbrev_code_->attributes[i].implicit_const;
	    break;
	  case elfcpp::DW_FORM_strp:
	  case elfcpp::DW_FORM_strp_sup:
	  case elfcpp::DW_FORM_line_strp:
	    {
	      off_t str_off;
	      if (this->dwinfo_->offset_size() == 4)
		str_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      else
		str_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &str_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = str_off;
	      break;
	    }
	  case elfcpp::DW_FORM_strx:
	  case elfcpp::DW_FORM_GNU_str_index:
	    attr_value.val.uintval = read_unsigned_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_strx1:
	    attr_value.val.uintval = *pattr++;
	    break;
	  case elfcpp::DW_FORM_strx2:
	    attr_value.val.uintval =
		this->dwinfo_->read_from_pointer<16>(&pattr);
	    break;
	  case elfcpp::DW_FORM_strx3:
	    attr_value.val.uintval =
		this->dwinfo_->read_3bytes_from_pointer(&pattr);
	    break;
	  case elfcpp::DW_FORM_strx4:
	    attr_value.val.uintval =
		this->dwinfo_->read_from_pointer<32>(&pattr);
	    break;
	  case elfcpp::DW_FORM_sec_offset:
	    {
	      off_t sec_off;
	      if (this->dwinfo_->offset_size() == 4)
		sec_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      else
		sec_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = sec_off;
	      ref_form = true;
	      break;
	    }
	  case elfcpp::DW_FORM_addr:
	    {
	      off_t sec_off;
	      if (this->dwinfo_->address_size() == 4)
		sec_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      else
		sec_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = sec_off;
	      break;
	    }
	  case elfcpp::DW_FORM_ref_addr:
	    {
	      off_t sec_off;
	      if (this->dwinfo_->ref_addr_size() == 4)
		sec_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      else
		sec_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = sec_off;
	      ref_form = true;
	      break;
	    }
	  case elfcpp::DW_FORM_block1:
	    attr_value.aux.blocklen = *pattr++;
	    attr_value.val.blockval = pattr;
	    pattr += attr_value.aux.blocklen;
	    break;
	  case elfcpp::DW_FORM_block2:
	    attr_value.aux.blocklen =
		this->dwinfo_->read_from_pointer<16>(&pattr);
	    attr_value.val.blockval = pattr;
	    pattr += attr_value.aux.blocklen;
	    break;
	  case elfcpp::DW_FORM_block4:
	    attr_value.aux.blocklen =
		this->dwinfo_->read_from_pointer<32>(&pattr);
	    attr_value.val.blockval = pattr;
	    pattr += attr_value.aux.blocklen;
	    break;
	  case elfcpp::DW_FORM_block:
	  case elfcpp::DW_FORM_exprloc:
	    attr_value.aux.blocklen = read_unsigned_LEB_128(pattr, &len);
	    attr_value.val.blockval = pattr + len;
	    pattr += len + attr_value.aux.blocklen;
	    break;
	  case elfcpp::DW_FORM_data1:
	  case elfcpp::DW_FORM_flag:
	    attr_value.val.intval = *pattr++;
	    break;
	  case elfcpp::DW_FORM_ref1:
	    attr_value.val.refval = *pattr++;
	    ref_form = true;
	    break;
	  case elfcpp::DW_FORM_data2:
	    attr_value.val.intval =
		this->dwinfo_->read_from_pointer<16>(&pattr);
	    break;
	  case elfcpp::DW_FORM_ref2:
	    attr_value.val.refval =
		this->dwinfo_->read_from_pointer<16>(&pattr);
	    ref_form = true;
	    break;
	  case elfcpp::DW_FORM_data4:
	    {
	      off_t sec_off;
	      sec_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.intval = sec_off;
	      break;
	    }
	  case elfcpp::DW_FORM_ref4:
	  case elfcpp::DW_FORM_ref_sup4:
	    {
	      off_t sec_off;
	      sec_off = this->dwinfo_->read_from_pointer<32>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = sec_off;
	      ref_form = true;
	      break;
	    }
	  case elfcpp::DW_FORM_data8:
	    {
	      off_t sec_off;
	      sec_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.intval = sec_off;
	      break;
	    }
	  case elfcpp::DW_FORM_data16:
	    {
	      // For now, treat this as a 16-byte block.
	      attr_value.val.blockval = pattr;
	      attr_value.aux.blocklen = 16;
	      pattr += 16;
	      break;
	    }
	  case elfcpp::DW_FORM_ref_sig8:
	    attr_value.val.uintval =
		this->dwinfo_->read_from_pointer<64>(&pattr);
	    break;
	  case elfcpp::DW_FORM_ref8:
	  case elfcpp::DW_FORM_ref_sup8:
	    {
	      off_t sec_off;
	      sec_off = this->dwinfo_->read_from_pointer<64>(&pattr);
	      unsigned int shndx =
		  this->dwinfo_->lookup_reloc(attr_off, &sec_off);
	      attr_value.aux.shndx = shndx;
	      attr_value.val.refval = sec_off;
	      ref_form = true;
	      break;
	    }
	  case elfcpp::DW_FORM_ref_udata:
	    attr_value.val.refval = read_unsigned_LEB_128(pattr, &len);
	    ref_form = true;
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_udata:
	    attr_value.val.uintval = read_unsigned_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_addrx:
	  case elfcpp::DW_FORM_GNU_addr_index:
	    attr_value.val.uintval = read_unsigned_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_addrx1:
	    attr_value.val.uintval = *pattr++;
	    break;
	  case elfcpp::DW_FORM_addrx2:
	    attr_value.val.uintval =
		this->dwinfo_->read_from_pointer<16>(&pattr);
	    break;
	  case elfcpp::DW_FORM_addrx3:
	    attr_value.val.uintval =
		this->dwinfo_->read_3bytes_from_pointer(&pattr);
	    break;
	  case elfcpp::DW_FORM_addrx4:
	    attr_value.val.uintval =
		this->dwinfo_->read_from_pointer<32>(&pattr);
	    break;
	  case elfcpp::DW_FORM_sdata:
	    attr_value.val.intval = read_signed_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_string:
	    attr_value.val.stringval = reinterpret_cast<const char*>(pattr);
	    len = strlen(attr_value.val.stringval);
	    pattr += len + 1;
	    break;
	  case elfcpp::DW_FORM_loclistx:
	  case elfcpp::DW_FORM_rnglistx:
	    attr_value.val.uintval = read_unsigned_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  default:
	    return false;
	}

      // Cache the most frequently-requested attributes.
      switch (attr)
	{
	  case elfcpp::DW_AT_name:
	    if (form == elfcpp::DW_FORM_string)
	      this->name_ = attr_value.val.stringval;
	    else if (form == elfcpp::DW_FORM_strp)
	      {
		// All indirect strings should refer to the same
		// string section, so we just save the last one seen.
		this->string_shndx_ = attr_value.aux.shndx;
		this->name_off_ = attr_value.val.refval;
	      }
	    break;
	  case elfcpp::DW_AT_linkage_name:
	  case elfcpp::DW_AT_MIPS_linkage_name:
	    if (form == elfcpp::DW_FORM_string)
	      this->linkage_name_ = attr_value.val.stringval;
	    else if (form == elfcpp::DW_FORM_strp)
	      {
		// All indirect strings should refer to the same
		// string section, so we just save the last one seen.
		this->string_shndx_ = attr_value.aux.shndx;
		this->linkage_name_off_ = attr_value.val.refval;
	      }
	    break;
	  case elfcpp::DW_AT_specification:
	    if (ref_form)
	      this->specification_ = attr_value.val.refval;
	    break;
	  case elfcpp::DW_AT_abstract_origin:
	    if (ref_form)
	      this->abstract_origin_ = attr_value.val.refval;
	    break;
	  case elfcpp::DW_AT_sibling:
	    if (ref_form && attr_value.aux.shndx == 0)
	      this->sibling_offset_ = attr_value.val.refval;
	  default:
	    break;
	}

      this->attributes_.push_back(attr_value);
    }

  // Now that we know where the next DIE begins, record the offset
  // to avoid later recalculation.
  if (this->has_children())
    this->child_offset_ = this->die_offset_ + (pattr - pdie);
  else
    this->sibling_offset_ = this->die_offset_ + (pattr - pdie);

  this->attributes_read_ = true;
  return true;
}

// Skip all the attributes of the DIE and return the offset of the next DIE.

off_t
Dwarf_die::skip_attributes()
{
  gold_assert(this->abbrev_code_ != NULL);

  const unsigned char* pdie =
      this->dwinfo_->buffer_at_offset(this->die_offset_);
  if (pdie == NULL)
    return 0;
  const unsigned char* pattr = pdie + this->attr_offset_;

  for (unsigned int i = 0; i < this->abbrev_code_->attributes.size(); ++i)
    {
      size_t len;
      unsigned int form = this->abbrev_code_->attributes[i].form;
      if (form == elfcpp::DW_FORM_indirect)
        {
          form = read_unsigned_LEB_128(pattr, &len);
          pattr += len;
        }
      switch(form)
	{
	  case elfcpp::DW_FORM_flag_present:
	  case elfcpp::DW_FORM_implicit_const:
	    break;
	  case elfcpp::DW_FORM_strp:
	  case elfcpp::DW_FORM_sec_offset:
	  case elfcpp::DW_FORM_strp_sup:
	  case elfcpp::DW_FORM_line_strp:
	    pattr += this->dwinfo_->offset_size();
	    break;
	  case elfcpp::DW_FORM_addr:
	    pattr += this->dwinfo_->address_size();
	    break;
	  case elfcpp::DW_FORM_ref_addr:
	    pattr += this->dwinfo_->ref_addr_size();
	    break;
	  case elfcpp::DW_FORM_block1:
	    pattr += 1 + *pattr;
	    break;
	  case elfcpp::DW_FORM_block2:
	    {
	      uint16_t block_size;
	      block_size = this->dwinfo_->read_from_pointer<16>(&pattr);
	      pattr += block_size;
	      break;
	    }
	  case elfcpp::DW_FORM_block4:
	    {
	      uint32_t block_size;
	      block_size = this->dwinfo_->read_from_pointer<32>(&pattr);
	      pattr += block_size;
	      break;
	    }
	  case elfcpp::DW_FORM_block:
	  case elfcpp::DW_FORM_exprloc:
	    {
	      uint64_t block_size;
	      block_size = read_unsigned_LEB_128(pattr, &len);
	      pattr += len + block_size;
	      break;
	    }
	  case elfcpp::DW_FORM_data1:
	  case elfcpp::DW_FORM_ref1:
	  case elfcpp::DW_FORM_flag:
	  case elfcpp::DW_FORM_strx1:
	  case elfcpp::DW_FORM_addrx1:
	    pattr += 1;
	    break;
	  case elfcpp::DW_FORM_data2:
	  case elfcpp::DW_FORM_ref2:
	  case elfcpp::DW_FORM_strx2:
	  case elfcpp::DW_FORM_addrx2:
	    pattr += 2;
	    break;
	  case elfcpp::DW_FORM_strx3:
	  case elfcpp::DW_FORM_addrx3:
	    pattr += 3;
	    break;
	  case elfcpp::DW_FORM_data4:
	  case elfcpp::DW_FORM_ref4:
	  case elfcpp::DW_FORM_ref_sup4:
	  case elfcpp::DW_FORM_strx4:
	  case elfcpp::DW_FORM_addrx4:
	    pattr += 4;
	    break;
	  case elfcpp::DW_FORM_data8:
	  case elfcpp::DW_FORM_ref8:
	  case elfcpp::DW_FORM_ref_sig8:
	  case elfcpp::DW_FORM_ref_sup8:
	    pattr += 8;
	    break;
	  case elfcpp::DW_FORM_data16:
	    pattr += 16;
	    break;
	  case elfcpp::DW_FORM_ref_udata:
	  case elfcpp::DW_FORM_udata:
	  case elfcpp::DW_FORM_addrx:
	  case elfcpp::DW_FORM_strx:
	  case elfcpp::DW_FORM_loclistx:
	  case elfcpp::DW_FORM_rnglistx:
	  case elfcpp::DW_FORM_GNU_addr_index:
	  case elfcpp::DW_FORM_GNU_str_index:
	    read_unsigned_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_sdata:
	    read_signed_LEB_128(pattr, &len);
	    pattr += len;
	    break;
	  case elfcpp::DW_FORM_string:
	    len = strlen(reinterpret_cast<const char*>(pattr));
	    pattr += len + 1;
	    break;
	  default:
	    return 0;
	}
    }

  return this->die_offset_ + (pattr - pdie);
}

// Get the name of the DIE and cache it.

void
Dwarf_die::set_name()
{
  if (this->name_ != NULL || !this->read_attributes())
    return;
  if (this->name_off_ != -1)
    this->name_ = this->dwinfo_->get_string(this->name_off_,
					    this->string_shndx_);
}

// Get the linkage name of the DIE and cache it.

void
Dwarf_die::set_linkage_name()
{
  if (this->linkage_name_ != NULL || !this->read_attributes())
    return;
  if (this->linkage_name_off_ != -1)
    this->linkage_name_ = this->dwinfo_->get_string(this->linkage_name_off_,
						    this->string_shndx_);
}

// Return the value of attribute ATTR.

const Dwarf_die::Attribute_value*
Dwarf_die::attribute(unsigned int attr)
{
  if (!this->read_attributes())
    return NULL;
  for (unsigned int i = 0; i < this->attributes_.size(); ++i)
    {
      if (this->attributes_[i].attr == attr)
        return &this->attributes_[i];
    }
  return NULL;
}

const char*
Dwarf_die::string_attribute(unsigned int attr)
{
  const Attribute_value* attr_val = this->attribute(attr);
  if (attr_val == NULL)
    return NULL;
  switch (attr_val->form)
    {
      case elfcpp::DW_FORM_string:
        return attr_val->val.stringval;
      case elfcpp::DW_FORM_strp:
	return this->dwinfo_->get_string(attr_val->val.refval,
					 attr_val->aux.shndx);
      default:
        return NULL;
    }
}

int64_t
Dwarf_die::int_attribute(unsigned int attr)
{
  const Attribute_value* attr_val = this->attribute(attr);
  if (attr_val == NULL)
    return 0;
  switch (attr_val->form)
    {
      case elfcpp::DW_FORM_flag_present:
      case elfcpp::DW_FORM_data1:
      case elfcpp::DW_FORM_flag:
      case elfcpp::DW_FORM_data2:
      case elfcpp::DW_FORM_data4:
      case elfcpp::DW_FORM_data8:
      case elfcpp::DW_FORM_sdata:
        return attr_val->val.intval;
      default:
        return 0;
    }
}

uint64_t
Dwarf_die::uint_attribute(unsigned int attr)
{
  const Attribute_value* attr_val = this->attribute(attr);
  if (attr_val == NULL)
    return 0;
  switch (attr_val->form)
    {
      case elfcpp::DW_FORM_flag_present:
      case elfcpp::DW_FORM_data1:
      case elfcpp::DW_FORM_flag:
      case elfcpp::DW_FORM_data4:
      case elfcpp::DW_FORM_data8:
      case elfcpp::DW_FORM_ref_sig8:
      case elfcpp::DW_FORM_udata:
        return attr_val->val.uintval;
      default:
        return 0;
    }
}

off_t
Dwarf_die::ref_attribute(unsigned int attr, unsigned int* shndx)
{
  const Attribute_value* attr_val = this->attribute(attr);
  if (attr_val == NULL)
    return -1;
  switch (attr_val->form)
    {
      case elfcpp::DW_FORM_sec_offset:
      case elfcpp::DW_FORM_addr:
      case elfcpp::DW_FORM_ref_addr:
      case elfcpp::DW_FORM_ref1:
      case elfcpp::DW_FORM_ref2:
      case elfcpp::DW_FORM_ref4:
      case elfcpp::DW_FORM_ref8:
      case elfcpp::DW_FORM_ref_udata:
        *shndx = attr_val->aux.shndx;
        return attr_val->val.refval;
      case elfcpp::DW_FORM_ref_sig8:
        *shndx = attr_val->aux.shndx;
        return attr_val->val.uintval;
      case elfcpp::DW_FORM_data4:
      case elfcpp::DW_FORM_data8:
        *shndx = attr_val->aux.shndx;
        return attr_val->val.intval;
      default:
        return -1;
    }
}

off_t
Dwarf_die::address_attribute(unsigned int attr, unsigned int* shndx)
{
  const Attribute_value* attr_val = this->attribute(attr);
  if (attr_val == NULL || attr_val->form != elfcpp::DW_FORM_addr)
    return -1;

  *shndx = attr_val->aux.shndx;
  return attr_val->val.refval;
}

// Return the offset of this DIE's first child.

off_t
Dwarf_die::child_offset()
{
  gold_assert(this->abbrev_code_ != NULL);
  if (!this->has_children())
    return 0;
  if (this->child_offset_ == 0)
    this->child_offset_ = this->skip_attributes();
  return this->child_offset_;
}

// Return the offset of this DIE's next sibling.

off_t
Dwarf_die::sibling_offset()
{
  gold_assert(this->abbrev_code_ != NULL);

  if (this->sibling_offset_ != 0)
    return this->sibling_offset_;

  if (!this->has_children())
    {
      this->sibling_offset_ = this->skip_attributes();
      return this->sibling_offset_;
    }

  if (this->has_sibling_attribute())
    {
      if (!this->read_attributes())
	return 0;
      if (this->sibling_offset_ != 0)
	return this->sibling_offset_;
    }

  // Skip over the children.
  off_t child_offset = this->child_offset();
  while (child_offset > 0)
    {
      Dwarf_die die(this->dwinfo_, child_offset, this);
      // The Dwarf_die ctor will set this DIE's sibling offset
      // when it reads a zero abbrev code.
      if (die.tag() == 0)
	break;
      child_offset = die.sibling_offset();
    }

  // This should be set by now.  If not, there was a problem reading
  // the DWARF info, and we return 0.
  return this->sibling_offset_;
}

// class Dwarf_info_reader

// Begin parsing the debug info.  This calls visit_compilation_unit()
// or visit_type_unit() for each compilation or type unit found in the
// section, and visit_die() for each top-level DIE.

void
Dwarf_info_reader::parse()
{
  if (this->object_->is_big_endian())
    {
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
      this->do_parse<true>();
#else
      gold_unreachable();
#endif
    }
  else
    {
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
      this->do_parse<false>();
#else
      gold_unreachable();
#endif
    }
}

template<bool big_endian>
void
Dwarf_info_reader::do_parse()
{
  // Get the section contents and decompress if necessary.
  section_size_type buffer_size;
  bool buffer_is_new;
  this->buffer_ = this->object_->decompressed_section_contents(this->shndx_,
							       &buffer_size,
							       &buffer_is_new);
  if (this->buffer_ == NULL || buffer_size == 0)
    return;
  this->buffer_end_ = this->buffer_ + buffer_size;

  // The offset of this input section in the output section.
  off_t section_offset = this->object_->output_section_offset(this->shndx_);

  // Start tracking relocations for this section.
  this->reloc_mapper_ = make_elf_reloc_mapper(this->object_, this->symtab_,
					      this->symtab_size_);
  this->reloc_mapper_->initialize(this->reloc_shndx_, this->reloc_type_);

  // Loop over compilation units (or type units).
  unsigned int abbrev_shndx = this->abbrev_shndx_;
  off_t abbrev_offset = 0;
  const unsigned char* pinfo = this->buffer_;
  while (pinfo < this->buffer_end_)
    {
      // Read the compilation (or type) unit header.
      const unsigned char* cu_start = pinfo;
      this->cu_offset_ = cu_start - this->buffer_;
      this->cu_length_ = this->buffer_end_ - cu_start;

      // Read unit_length (4 or 12 bytes).
      if (!this->check_buffer(pinfo + 4))
	break;
      uint32_t unit_length =
          elfcpp::Swap_unaligned<32, big_endian>::readval(pinfo);
      pinfo += 4;
      if (unit_length == 0xffffffff)
	{
	  if (!this->check_buffer(pinfo + 8))
	    break;
	  unit_length = elfcpp::Swap_unaligned<64, big_endian>::readval(pinfo);
	  pinfo += 8;
	  this->offset_size_ = 8;
	}
      else
	this->offset_size_ = 4;
      if (!this->check_buffer(pinfo + unit_length))
	break;
      const unsigned char* cu_end = pinfo + unit_length;
      this->cu_length_ = cu_end - cu_start;
      if (!this->check_buffer(pinfo + 2 + this->offset_size_ + 1))
	break;

      // Read version (2 bytes).
      this->cu_version_ =
	  elfcpp::Swap_unaligned<16, big_endian>::readval(pinfo);
      pinfo += 2;

      // DWARF 5: Read the unit type (1 byte) and address size (1 byte).
      if (this->cu_version_ >= 5)
	{
	  this->unit_type_ = *pinfo++;
	  this->address_size_ = *pinfo++;
	}

      // Read debug_abbrev_offset (4 or 8 bytes).
      if (this->offset_size_ == 4)
	abbrev_offset = elfcpp::Swap_unaligned<32, big_endian>::readval(pinfo);
      else
	abbrev_offset = elfcpp::Swap_unaligned<64, big_endian>::readval(pinfo);
      if (this->reloc_shndx_ > 0)
	{
	  off_t reloc_offset = pinfo - this->buffer_;
	  off_t value;
	  abbrev_shndx =
	      this->reloc_mapper_->get_reloc_target(reloc_offset, &value);
	  if (abbrev_shndx == 0)
	    return;
	  if (this->reloc_type_ == elfcpp::SHT_REL)
	    abbrev_offset += value;
	  else
	    abbrev_offset = value;
	}
      pinfo += this->offset_size_;

      // DWARF 2-4: Read address_size (1 byte).
      if (this->cu_version_ < 5)
	this->address_size_ = *pinfo++;

      // For type units, read the two extra fields.
      uint64_t signature = 0;
      off_t type_offset = 0;
      if (this->is_type_unit())
        {
	  if (!this->check_buffer(pinfo + 8 + this->offset_size_))
	    break;

	  // Read type_signature (8 bytes).
	  signature = elfcpp::Swap_unaligned<64, big_endian>::readval(pinfo);
	  pinfo += 8;

	  // Read type_offset (4 or 8 bytes).
	  if (this->offset_size_ == 4)
	    type_offset =
		elfcpp::Swap_unaligned<32, big_endian>::readval(pinfo);
	  else
	    type_offset =
		elfcpp::Swap_unaligned<64, big_endian>::readval(pinfo);
	  pinfo += this->offset_size_;
	}

      // Read the .debug_abbrev table.
      this->abbrev_table_.read_abbrevs(this->object_, abbrev_shndx,
				       abbrev_offset);

      // Visit the root DIE.
      Dwarf_die root_die(this,
			 pinfo - (this->buffer_ + this->cu_offset_),
			 NULL);
      if (root_die.tag() != 0)
	{
	  // Visit the CU or TU.
	  if (this->is_type_unit())
	    this->visit_type_unit(section_offset + this->cu_offset_,
				  cu_end - cu_start, type_offset, signature,
				  &root_die);
	  else
	    this->visit_compilation_unit(section_offset + this->cu_offset_,
					 cu_end - cu_start, &root_die);
	}

      // Advance to the next CU.
      pinfo = cu_end;
    }

  if (buffer_is_new)
    {
      delete[] this->buffer_;
      this->buffer_ = NULL;
    }
}

// Read the DWARF string table.

bool
Dwarf_info_reader::do_read_string_table(unsigned int string_shndx)
{
  Relobj* object = this->object_;

  // If we don't have relocations, string_shndx will be 0, and
  // we'll have to hunt for the .debug_str section.
  if (string_shndx == 0)
    {
      for (unsigned int i = 1; i < this->object_->shnum(); ++i)
	{
	  std::string name = object->section_name(i);
	  if (name == ".debug_str" || name == ".zdebug_str")
	    {
	      string_shndx = i;
	      this->string_output_section_offset_ =
		  object->output_section_offset(i);
	      break;
	    }
	}
      if (string_shndx == 0)
	return false;
    }

  if (this->owns_string_buffer_ && this->string_buffer_ != NULL)
    {
      delete[] this->string_buffer_;
      this->owns_string_buffer_ = false;
    }

  // Get the secton contents and decompress if necessary.
  section_size_type buffer_size;
  const unsigned char* buffer =
      object->decompressed_section_contents(string_shndx,
					    &buffer_size,
					    &this->owns_string_buffer_);
  this->string_buffer_ = reinterpret_cast<const char*>(buffer);
  this->string_buffer_end_ = this->string_buffer_ + buffer_size;
  this->string_shndx_ = string_shndx;
  return true;
}

// Read a possibly unaligned integer of SIZE.
template <int valsize>
inline typename elfcpp::Valtype_base<valsize>::Valtype
Dwarf_info_reader::read_from_pointer(const unsigned char* source)
{
  typename elfcpp::Valtype_base<valsize>::Valtype return_value;
  if (this->object_->is_big_endian())
    return_value = elfcpp::Swap_unaligned<valsize, true>::readval(source);
  else
    return_value = elfcpp::Swap_unaligned<valsize, false>::readval(source);
  return return_value;
}

// Read a possibly unaligned integer of SIZE.  Update SOURCE after read.
template <int valsize>
inline typename elfcpp::Valtype_base<valsize>::Valtype
Dwarf_info_reader::read_from_pointer(const unsigned char** source)
{
  typename elfcpp::Valtype_base<valsize>::Valtype return_value;
  if (this->object_->is_big_endian())
    return_value = elfcpp::Swap_unaligned<valsize, true>::readval(*source);
  else
    return_value = elfcpp::Swap_unaligned<valsize, false>::readval(*source);
  *source += valsize / 8;
  return return_value;
}

// Read a 3-byte integer.  Update SOURCE after read.
inline typename elfcpp::Valtype_base<32>::Valtype
Dwarf_info_reader::read_3bytes_from_pointer(const unsigned char** source)
{
  typename elfcpp::Valtype_base<32>::Valtype return_value;
  if (this->object_->is_big_endian())
    return_value = ((*source)[0] << 16) | ((*source)[1] << 8) | (*source)[2];
  else
    return_value = ((*source)[2] << 16) | ((*source)[1] << 8) | (*source)[0];
  *source += 3;
  return return_value;
}

// Look for a relocation at offset ATTR_OFF in the dwarf info,
// and return the section index and offset of the target.

unsigned int
Dwarf_info_reader::lookup_reloc(off_t attr_off, off_t* target_off)
{
  off_t value;
  attr_off += this->cu_offset_;
  unsigned int shndx = this->reloc_mapper_->get_reloc_target(attr_off, &value);
  if (shndx == 0)
    return 0;
  if (this->reloc_type_ == elfcpp::SHT_REL)
    *target_off += value;
  else
    *target_off = value;
  return shndx;
}

// Return a string from the DWARF string table.

const char*
Dwarf_info_reader::get_string(off_t str_off, unsigned int string_shndx)
{
  if (!this->read_string_table(string_shndx))
    return NULL;

  // Correct the offset.  For incremental update links, we have a
  // relocated offset that is relative to the output section, but
  // here we need an offset relative to the input section.
  str_off -= this->string_output_section_offset_;

  const char* p = this->string_buffer_ + str_off;

  if (p < this->string_buffer_ || p >= this->string_buffer_end_)
    return NULL;

  return p;
}

// The following are default, do-nothing, implementations of the
// hook methods normally provided by a derived class.  We provide
// default implementations rather than no implementation so that
// a derived class needs to implement only the hooks that it needs
// to use.

// Process a compilation unit and parse its child DIE.

void
Dwarf_info_reader::visit_compilation_unit(off_t, off_t, Dwarf_die*)
{
}

// Process a type unit and parse its child DIE.

void
Dwarf_info_reader::visit_type_unit(off_t, off_t, off_t, uint64_t, Dwarf_die*)
{
}

// Print a warning about a corrupt debug section.

void
Dwarf_info_reader::warn_corrupt_debug_section() const
{
  gold_warning(_("%s: corrupt debug info in %s"),
	       this->object_->name().c_str(),
	       this->object_->section_name(this->shndx_).c_str());
}

// class Sized_dwarf_line_info

struct LineStateMachine
{
  int file_num;
  uint64_t address;
  int line_num;
  int column_num;
  unsigned int shndx;    // the section address refers to
  bool is_stmt;          // stmt means statement.
  bool basic_block;
  bool end_sequence;
};

static void
ResetLineStateMachine(struct LineStateMachine* lsm, bool default_is_stmt)
{
  lsm->file_num = 1;
  lsm->address = 0;
  lsm->line_num = 1;
  lsm->column_num = 0;
  lsm->shndx = -1U;
  lsm->is_stmt = default_is_stmt;
  lsm->basic_block = false;
  lsm->end_sequence = false;
}

template<int size, bool big_endian>
Sized_dwarf_line_info<size, big_endian>::Sized_dwarf_line_info(
    Object* object,
    unsigned int read_shndx)
  : data_valid_(false), buffer_(NULL), buffer_start_(NULL),
    str_buffer_(NULL), str_buffer_start_(NULL),
    reloc_mapper_(NULL), symtab_buffer_(NULL), directories_(), files_(),
    current_header_index_(-1), reloc_map_(), line_number_map_()
{
  unsigned int debug_line_shndx = 0;
  unsigned int debug_line_str_shndx = 0;

  for (unsigned int i = 1; i < object->shnum(); ++i)
    {
      section_size_type buffer_size;
      bool is_new = false;

      // FIXME: do this more efficiently: section_name() isn't super-fast
      std::string name = object->section_name(i);
      if (name == ".debug_line" || name == ".zdebug_line")
	{
	  this->buffer_ =
	      object->decompressed_section_contents(i, &buffer_size, &is_new);
	  if (is_new)
	    this->buffer_start_ = this->buffer_;
	  this->buffer_end_ = this->buffer_ + buffer_size;
	  debug_line_shndx = i;
	}
      else if (name == ".debug_line_str" || name == ".zdebug_line_str")
	{
	  this->str_buffer_ =
	      object->decompressed_section_contents(i, &buffer_size, &is_new);
	  if (is_new)
	    this->str_buffer_start_ = this->str_buffer_;
	  this->str_buffer_end_ = this->str_buffer_ + buffer_size;
	  debug_line_str_shndx = i;
	}
      if (debug_line_shndx > 0 && debug_line_str_shndx > 0)
        break;
    }
  if (this->buffer_ == NULL)
    return;

  // Find the relocation section for ".debug_line".
  // We expect these for relobjs (.o's) but not dynobjs (.so's).
  unsigned int reloc_shndx = 0;
  for (unsigned int i = 0; i < object->shnum(); ++i)
    {
      unsigned int reloc_sh_type = object->section_type(i);
      if ((reloc_sh_type == elfcpp::SHT_REL
	   || reloc_sh_type == elfcpp::SHT_RELA)
	  && object->section_info(i) == debug_line_shndx)
	{
	  reloc_shndx = i;
	  this->track_relocs_type_ = reloc_sh_type;
	  break;
	}
    }

  // Finally, we need the symtab section to interpret the relocs.
  if (reloc_shndx != 0)
    {
      unsigned int symtab_shndx;
      for (symtab_shndx = 0; symtab_shndx < object->shnum(); ++symtab_shndx)
        if (object->section_type(symtab_shndx) == elfcpp::SHT_SYMTAB)
          {
	    this->symtab_buffer_ = object->section_contents(
		symtab_shndx, &this->symtab_buffer_size_, false);
            break;
          }
      if (this->symtab_buffer_ == NULL)
        return;
    }

  this->reloc_mapper_ =
      new Sized_elf_reloc_mapper<size, big_endian>(object,
						   this->symtab_buffer_,
						   this->symtab_buffer_size_);
  if (!this->reloc_mapper_->initialize(reloc_shndx, this->track_relocs_type_))
    return;

  // Now that we have successfully read all the data, parse the debug
  // info.
  this->data_valid_ = true;
  this->read_line_mappings(read_shndx);
}

// Read the DWARF header.

template<int size, bool big_endian>
const unsigned char*
Sized_dwarf_line_info<size, big_endian>::read_header_prolog(
    const unsigned char* lineptr)
{
  uint32_t initial_length = elfcpp::Swap_unaligned<32, big_endian>::readval(lineptr);
  lineptr += 4;

  // In DWARF, if the initial length is all 1 bits, then the offset
  // size is 8 and we need to read the next 8 bytes for the real length.
  if (initial_length == 0xffffffff)
    {
      this->header_.offset_size = 8;
      initial_length = elfcpp::Swap_unaligned<64, big_endian>::readval(lineptr);
      lineptr += 8;
    }
  else
    this->header_.offset_size = 4;

  this->header_.total_length = initial_length;

  this->end_of_unit_ = lineptr + initial_length;
  gold_assert(this->end_of_unit_ <= buffer_end_);

  this->header_.version =
      elfcpp::Swap_unaligned<16, big_endian>::readval(lineptr);
  lineptr += 2;

  // We can only read versions 2-5 of the DWARF line number table.
  // For other versions, just skip the entire line number table.
  if (this->header_.version < 2 || this->header_.version > 5)
    return this->end_of_unit_;

  // DWARF 5 only: address size and segment selector.
  if (this->header_.version >= 5)
    {
      this->header_.address_size = *lineptr;
      // We ignore the segment selector.
      lineptr += 2;
    }

  if (this->header_.offset_size == 4)
    this->header_.prologue_length =
	elfcpp::Swap_unaligned<32, big_endian>::readval(lineptr);
  else
    this->header_.prologue_length =
	elfcpp::Swap_unaligned<64, big_endian>::readval(lineptr);
  lineptr += this->header_.offset_size;

  this->end_of_header_length_ = lineptr;

  this->header_.min_insn_length = *lineptr;
  lineptr += 1;

  if (this->header_.version < 4)
    this->header_.max_ops_per_insn = 1;
  else
    {
      // DWARF 4 added the maximum_operations_per_instruction field.
      this->header_.max_ops_per_insn = *lineptr;
      lineptr += 1;
      // TODO: Add support for values other than 1.
      gold_assert(this->header_.max_ops_per_insn == 1);
    }

  this->header_.default_is_stmt = *lineptr;
  lineptr += 1;

  this->header_.line_base = *reinterpret_cast<const signed char*>(lineptr);
  lineptr += 1;

  this->header_.line_range = *lineptr;
  lineptr += 1;

  this->header_.opcode_base = *lineptr;
  lineptr += 1;

  this->header_.std_opcode_lengths.resize(this->header_.opcode_base + 1);
  this->header_.std_opcode_lengths[0] = 0;
  for (int i = 1; i < this->header_.opcode_base; i++)
    {
      this->header_.std_opcode_lengths[i] = *lineptr;
      lineptr += 1;
    }

  return lineptr;
}

// The header for a debug_line section is mildly complicated, because
// the line info is very tightly encoded.
// This routine is for DWARF versions 2, 3, and 4.

template<int size, bool big_endian>
const unsigned char*
Sized_dwarf_line_info<size, big_endian>::read_header_tables_v2(
    const unsigned char* lineptr)
{
  ++this->current_header_index_;

  // Create a new directories_ entry and a new files_ entry for our new
  // header.  We initialize each with a single empty element, because
  // dwarf indexes directory and filenames starting at 1.
  gold_assert(static_cast<int>(this->directories_.size())
	      == this->current_header_index_);
  gold_assert(static_cast<int>(this->files_.size())
	      == this->current_header_index_);
  this->directories_.push_back(std::vector<std::string>(1));
  this->files_.push_back(std::vector<std::pair<int, std::string> >(1));

  // It is legal for the directory entry table to be empty.
  if (*lineptr)
    {
      int dirindex = 1;
      while (*lineptr)
        {
	  const char* dirname = reinterpret_cast<const char*>(lineptr);
          gold_assert(dirindex
		      == static_cast<int>(this->directories_.back().size()));
          this->directories_.back().push_back(dirname);
          lineptr += this->directories_.back().back().size() + 1;
          dirindex++;
        }
    }
  lineptr++;

  // It is also legal for the file entry table to be empty.
  if (*lineptr)
    {
      int fileindex = 1;
      size_t len;
      while (*lineptr)
        {
          const char* filename = reinterpret_cast<const char*>(lineptr);
          lineptr += strlen(filename) + 1;

          uint64_t dirindex = read_unsigned_LEB_128(lineptr, &len);
          lineptr += len;

          if (dirindex >= this->directories_.back().size())
            dirindex = 0;
	  int dirindexi = static_cast<int>(dirindex);

          read_unsigned_LEB_128(lineptr, &len);   // mod_time
          lineptr += len;

          read_unsigned_LEB_128(lineptr, &len);   // filelength
          lineptr += len;

          gold_assert(fileindex
		      == static_cast<int>(this->files_.back().size()));
          this->files_.back().push_back(std::make_pair(dirindexi, filename));
          fileindex++;
        }
    }
  lineptr++;

  return lineptr;
}

// This routine is for DWARF version 5.

template<int size, bool big_endian>
const unsigned char*
Sized_dwarf_line_info<size, big_endian>::read_header_tables_v5(
    const unsigned char* lineptr)
{
  size_t len;

  ++this->current_header_index_;

  gold_assert(static_cast<int>(this->directories_.size())
	      == this->current_header_index_);
  gold_assert(static_cast<int>(this->files_.size())
	      == this->current_header_index_);

  // Read the directory list.
  unsigned int format_count = *lineptr;
  lineptr += 1;

  unsigned int *types = new unsigned int[format_count];
  unsigned int *forms = new unsigned int[format_count];

  for (unsigned int i = 0; i < format_count; i++)
    {
      types[i] = read_unsigned_LEB_128(lineptr, &len);
      lineptr += len;
      forms[i] = read_unsigned_LEB_128(lineptr, &len);
      lineptr += len;
    }

  uint64_t entry_count = read_unsigned_LEB_128(lineptr, &len);
  lineptr += len;
  this->directories_.push_back(std::vector<std::string>(0));
  std::vector<std::string>& dir_list = this->directories_.back();

  for (unsigned int j = 0; j < entry_count; j++)
    {
      std::string dirname;

      for (unsigned int i = 0; i < format_count; i++)
       {
	 if (types[i] == elfcpp::DW_LNCT_path)
	   {
	     if (forms[i] == elfcpp::DW_FORM_string)
	       {
		 dirname = reinterpret_cast<const char*>(lineptr);
		 lineptr += dirname.size() + 1;
	       }
	     else if (forms[i] == elfcpp::DW_FORM_line_strp)
	       {
		 uint64_t offset;
		 if (this->header_.offset_size == 4)
		   offset =
		       elfcpp::Swap_unaligned<32, big_endian>::readval(lineptr);
		 else
		   offset =
		       elfcpp::Swap_unaligned<64, big_endian>::readval(lineptr);
		 typename Reloc_map::const_iterator it
		     = this->reloc_map_.find(lineptr - this->buffer_);
		 if (it != reloc_map_.end())
		   {
		     if (this->track_relocs_type_ == elfcpp::SHT_RELA)
		       offset = 0;
		     offset += it->second.second;
		   }
		 lineptr += this->header_.offset_size;
		 dirname = reinterpret_cast<const char*>(this->str_buffer_
							 + offset);
	       }
	     else
	       return lineptr;
	   }
	 else
	   return lineptr;
       }
      dir_list.push_back(dirname);
    }

  delete[] types;
  delete[] forms;

  // Read the filenames list.
  format_count = *lineptr;
  lineptr += 1;

  types = new unsigned int[format_count];
  forms = new unsigned int[format_count];

  for (unsigned int i = 0; i < format_count; i++)
    {
      types[i] = read_unsigned_LEB_128(lineptr, &len);
      lineptr += len;
      forms[i] = read_unsigned_LEB_128(lineptr, &len);
      lineptr += len;
    }

  entry_count = read_unsigned_LEB_128(lineptr, &len);
  lineptr += len;
  this->files_.push_back(
      std::vector<std::pair<int, std::string> >(0));
  std::vector<std::pair<int, std::string> >& file_list = this->files_.back();

  for (unsigned int j = 0; j < entry_count; j++)
    {
      const char* path = NULL;
      int dirindex = 0;

      for (unsigned int i = 0; i < format_count; i++)
       {
	 if (types[i] == elfcpp::DW_LNCT_path)
	   {
	     if (forms[i] == elfcpp::DW_FORM_string)
	       {
		 path = reinterpret_cast<const char*>(lineptr);
		 lineptr += strlen(path) + 1;
	       }
	     else if (forms[i] == elfcpp::DW_FORM_line_strp)
	       {
		 uint64_t offset;
		 if (this->header_.offset_size == 4)
		   offset = elfcpp::Swap_unaligned<32, big_endian>::readval(lineptr);
		 else
		   offset = elfcpp::Swap_unaligned<64, big_endian>::readval(lineptr);
		 typename Reloc_map::const_iterator it
		     = this->reloc_map_.find(lineptr - this->buffer_);
		 if (it != reloc_map_.end())
		   {
		     if (this->track_relocs_type_ == elfcpp::SHT_RELA)
		       offset = 0;
		     offset += it->second.second;
		   }
		 lineptr += this->header_.offset_size;
		 path = reinterpret_cast<const char*>(this->str_buffer_
						      + offset);
	       }
	     else
	       return lineptr;
	   }
	 else if (types[i] == elfcpp::DW_LNCT_directory_index)
	   {
	     if (forms[i] == elfcpp::DW_FORM_udata)
	       {
		 dirindex = read_unsigned_LEB_128(lineptr, &len);
		 lineptr += len;
	       }
	     else
	       return lineptr;
	   }
	 else
	   return lineptr;
       }
      gold_debug(DEBUG_LOCATION, "File %3d: %s",
		 static_cast<int>(file_list.size()), path);
      file_list.push_back(std::make_pair(dirindex, path));
    }

  delete[] types;
  delete[] forms;

  return lineptr;
}

// Process a single opcode in the .debug.line structure.

template<int size, bool big_endian>
bool
Sized_dwarf_line_info<size, big_endian>::process_one_opcode(
    const unsigned char* start, struct LineStateMachine* lsm, size_t* len)
{
  size_t oplen = 0;
  size_t templen;
  unsigned char opcode = *start;
  oplen++;
  start++;

  // If the opcode is great than the opcode_base, it is a special
  // opcode. Most line programs consist mainly of special opcodes.
  if (opcode >= this->header_.opcode_base)
    {
      opcode -= this->header_.opcode_base;
      const int advance_address = ((opcode / this->header_.line_range)
                                   * this->header_.min_insn_length);
      lsm->address += advance_address;

      const int advance_line = ((opcode % this->header_.line_range)
                                + this->header_.line_base);
      lsm->line_num += advance_line;
      lsm->basic_block = true;
      *len = oplen;
      return true;
    }

  // Otherwise, we have the regular opcodes
  switch (opcode)
    {
    case elfcpp::DW_LNS_copy:
      lsm->basic_block = false;
      *len = oplen;
      return true;

    case elfcpp::DW_LNS_advance_pc:
      {
        const uint64_t advance_address
            = read_unsigned_LEB_128(start, &templen);
        oplen += templen;
        lsm->address += this->header_.min_insn_length * advance_address;
      }
      break;

    case elfcpp::DW_LNS_advance_line:
      {
        const int64_t advance_line = read_signed_LEB_128(start, &templen);
        oplen += templen;
        lsm->line_num += advance_line;
      }
      break;

    case elfcpp::DW_LNS_set_file:
      {
        const uint64_t fileno = read_unsigned_LEB_128(start, &templen);
        oplen += templen;
        lsm->file_num = fileno;
      }
      break;

    case elfcpp::DW_LNS_set_column:
      {
        const uint64_t colno = read_unsigned_LEB_128(start, &templen);
        oplen += templen;
        lsm->column_num = colno;
      }
      break;

    case elfcpp::DW_LNS_negate_stmt:
      lsm->is_stmt = !lsm->is_stmt;
      break;

    case elfcpp::DW_LNS_set_basic_block:
      lsm->basic_block = true;
      break;

    case elfcpp::DW_LNS_fixed_advance_pc:
      {
        int advance_address;
        advance_address = elfcpp::Swap_unaligned<16, big_endian>::readval(start);
        oplen += 2;
        lsm->address += advance_address;
      }
      break;

    case elfcpp::DW_LNS_const_add_pc:
      {
        const int advance_address = (this->header_.min_insn_length
                                     * ((255 - this->header_.opcode_base)
                                        / this->header_.line_range));
        lsm->address += advance_address;
      }
      break;

    case elfcpp::DW_LNS_extended_op:
      {
        const uint64_t extended_op_len
            = read_unsigned_LEB_128(start, &templen);
        start += templen;
        oplen += templen + extended_op_len;

        const unsigned char extended_op = *start;
        start++;

        switch (extended_op)
          {
          case elfcpp::DW_LNE_end_sequence:
            // This means that the current byte is the one immediately
            // after a set of instructions.  Record the current line
            // for up to one less than the current address.
            lsm->line_num = -1;
            lsm->end_sequence = true;
            *len = oplen;
            return true;

          case elfcpp::DW_LNE_set_address:
            {
              lsm->address =
		elfcpp::Swap_unaligned<size, big_endian>::readval(start);
              typename Reloc_map::const_iterator it
                  = this->reloc_map_.find(start - this->buffer_);
              if (it != reloc_map_.end())
                {
		  // If this is a SHT_RELA section, then ignore the
		  // section contents.  This assumes that this is a
		  // straight reloc which just uses the reloc addend.
		  // The reloc addend has already been included in the
		  // symbol value.
		  if (this->track_relocs_type_ == elfcpp::SHT_RELA)
		    lsm->address = 0;
		  // Add in the symbol value.
		  lsm->address += it->second.second;
                  lsm->shndx = it->second.first;
                }
              else
                {
                  // If we're a normal .o file, with relocs, every
                  // set_address should have an associated relocation.
		  if (this->input_is_relobj())
                    this->data_valid_ = false;
                }
              break;
            }
          case elfcpp::DW_LNE_define_file:
            {
              const char* filename  = reinterpret_cast<const char*>(start);
              templen = strlen(filename) + 1;
              start += templen;

              uint64_t dirindex = read_unsigned_LEB_128(start, &templen);

              if (dirindex >= this->directories_.back().size())
                dirindex = 0;
	      int dirindexi = static_cast<int>(dirindex);

              // This opcode takes two additional ULEB128 parameters
              // (mod_time and filelength), but we don't use those
              // values.  Because OPLEN already tells us how far to
              // skip to the next opcode, we don't need to read
              // them at all.

              this->files_.back().push_back(std::make_pair(dirindexi,
							   filename));
            }
            break;
          }
      }
      break;

    default:
      {
        // Ignore unknown opcode  silently
        for (int i = 0; i < this->header_.std_opcode_lengths[opcode]; i++)
          {
            size_t templen;
            read_unsigned_LEB_128(start, &templen);
            start += templen;
            oplen += templen;
          }
      }
      break;
  }
  *len = oplen;
  return false;
}

// Read the debug information at LINEPTR and store it in the line
// number map.

template<int size, bool big_endian>
unsigned const char*
Sized_dwarf_line_info<size, big_endian>::read_lines(unsigned const char* lineptr,
                                                    unsigned const char* endptr,
                                                    unsigned int shndx)
{
  struct LineStateMachine lsm;

  while (lineptr < endptr)
    {
      ResetLineStateMachine(&lsm, this->header_.default_is_stmt);
      while (!lsm.end_sequence)
        {
          size_t oplength;

	  if (lineptr >= endptr)
	    break;

          bool add_line = this->process_one_opcode(lineptr, &lsm, &oplength);
          lineptr += oplength;

          if (add_line
              && (shndx == -1U || lsm.shndx == -1U || shndx == lsm.shndx))
            {
              Offset_to_lineno_entry entry
                  = { static_cast<off_t>(lsm.address),
		      this->current_header_index_,
		      static_cast<unsigned int>(lsm.file_num),
		      true, lsm.line_num };
	      std::vector<Offset_to_lineno_entry>&
		map(this->line_number_map_[lsm.shndx]);
	      // If we see two consecutive entries with the same
	      // offset and a real line number, then mark the first
	      // one as non-canonical.
	      if (!map.empty()
		  && (map.back().offset == static_cast<off_t>(lsm.address))
		  && lsm.line_num != -1
		  && map.back().line_num != -1)
		map.back().last_line_for_offset = false;
	      map.push_back(entry);
            }
        }
    }

  return endptr;
}

// Read the relocations into a Reloc_map.

template<int size, bool big_endian>
void
Sized_dwarf_line_info<size, big_endian>::read_relocs()
{
  if (this->symtab_buffer_ == NULL)
    return;

  off_t value;
  off_t reloc_offset;
  while ((reloc_offset = this->reloc_mapper_->next_offset()) != -1)
    {
      const unsigned int shndx =
          this->reloc_mapper_->get_reloc_target(reloc_offset, &value);

      // There is no reason to record non-ordinary section indexes, or
      // SHN_UNDEF, because they will never match the real section.
      if (shndx != 0)
	this->reloc_map_[reloc_offset] = std::make_pair(shndx, value);

      this->reloc_mapper_->advance(reloc_offset + 1);
    }
}

// Read the line number info.

template<int size, bool big_endian>
void
Sized_dwarf_line_info<size, big_endian>::read_line_mappings(unsigned int shndx)
{
  gold_assert(this->data_valid_ == true);

  this->read_relocs();
  while (this->buffer_ < this->buffer_end_)
    {
      const unsigned char* lineptr = this->buffer_;
      lineptr = this->read_header_prolog(lineptr);
      if (this->header_.version >= 2 && this->header_.version <= 4)
	{
	  lineptr = this->read_header_tables_v2(lineptr);
	  lineptr = this->read_lines(lineptr, this->end_of_unit_, shndx);
	}
      else if (this->header_.version == 5)
	{
	  lineptr = this->read_header_tables_v5(lineptr);
	  lineptr = this->read_lines(lineptr, this->end_of_unit_, shndx);
	}
      this->buffer_ = this->end_of_unit_;
    }

  // Sort the lines numbers, so addr2line can use binary search.
  for (typename Lineno_map::iterator it = line_number_map_.begin();
       it != line_number_map_.end();
       ++it)
    // Each vector needs to be sorted by offset.
    std::sort(it->second.begin(), it->second.end());
}

// Some processing depends on whether the input is a .o file or not.
// For instance, .o files have relocs, and have .debug_lines
// information on a per section basis.  .so files, on the other hand,
// lack relocs, and offsets are unique, so we can ignore the section
// information.

template<int size, bool big_endian>
bool
Sized_dwarf_line_info<size, big_endian>::input_is_relobj()
{
  // Only .o files have relocs and the symtab buffer that goes with them.
  return this->symtab_buffer_ != NULL;
}

// Given an Offset_to_lineno_entry vector, and an offset, figure out
// if the offset points into a function according to the vector (see
// comments below for the algorithm).  If it does, return an iterator
// into the vector that points to the line-number that contains that
// offset.  If not, it returns vector::end().

static std::vector<Offset_to_lineno_entry>::const_iterator
offset_to_iterator(const std::vector<Offset_to_lineno_entry>* offsets,
                   off_t offset)
{
  const Offset_to_lineno_entry lookup_key = { offset, 0, 0, true, 0 };

  // lower_bound() returns the smallest offset which is >= lookup_key.
  // If no offset in offsets is >= lookup_key, returns end().
  std::vector<Offset_to_lineno_entry>::const_iterator it
      = std::lower_bound(offsets->begin(), offsets->end(), lookup_key);

  // This code is easiest to understand with a concrete example.
  // Here's a possible offsets array:
  // {{offset = 3211, header_num = 0, file_num = 1, last, line_num = 16},  // 0
  //  {offset = 3224, header_num = 0, file_num = 1, last, line_num = 20},  // 1
  //  {offset = 3226, header_num = 0, file_num = 1, last, line_num = 22},  // 2
  //  {offset = 3231, header_num = 0, file_num = 1, last, line_num = 25},  // 3
  //  {offset = 3232, header_num = 0, file_num = 1, last, line_num = -1},  // 4
  //  {offset = 3232, header_num = 0, file_num = 1, last, line_num = 65},  // 5
  //  {offset = 3235, header_num = 0, file_num = 1, last, line_num = 66},  // 6
  //  {offset = 3236, header_num = 0, file_num = 1, last, line_num = -1},  // 7
  //  {offset = 5764, header_num = 0, file_num = 1, last, line_num = 48},  // 8
  //  {offset = 5764, header_num = 0, file_num = 1,!last, line_num = 47},  // 9
  //  {offset = 5765, header_num = 0, file_num = 1, last, line_num = 49},  // 10
  //  {offset = 5767, header_num = 0, file_num = 1, last, line_num = 50},  // 11
  //  {offset = 5768, header_num = 0, file_num = 1, last, line_num = 51},  // 12
  //  {offset = 5773, header_num = 0, file_num = 1, last, line_num = -1},  // 13
  //  {offset = 5787, header_num = 1, file_num = 1, last, line_num = 19},  // 14
  //  {offset = 5790, header_num = 1, file_num = 1, last, line_num = 20},  // 15
  //  {offset = 5793, header_num = 1, file_num = 1, last, line_num = 67},  // 16
  //  {offset = 5793, header_num = 1, file_num = 1, last, line_num = -1},  // 17
  //  {offset = 5793, header_num = 1, file_num = 1,!last, line_num = 66},  // 18
  //  {offset = 5795, header_num = 1, file_num = 1, last, line_num = 68},  // 19
  //  {offset = 5798, header_num = 1, file_num = 1, last, line_num = -1},  // 20
  // The entries with line_num == -1 mark the end of a function: the
  // associated offset is one past the last instruction in the
  // function.  This can correspond to the beginning of the next
  // function (as is true for offset 3232); alternately, there can be
  // a gap between the end of one function and the start of the next
  // (as is true for some others, most obviously from 3236->5764).
  //
  // Case 1: lookup_key has offset == 10.  lower_bound returns
  //         offsets[0].  Since it's not an exact match and we're
  //         at the beginning of offsets, we return end() (invalid).
  // Case 2: lookup_key has offset 10000.  lower_bound returns
  //         offset[21] (end()).  We return end() (invalid).
  // Case 3: lookup_key has offset == 3211.  lower_bound matches
  //         offsets[0] exactly, and that's the entry we return.
  // Case 4: lookup_key has offset == 3232.  lower_bound returns
  //         offsets[4].  That's an exact match, but indicates
  //         end-of-function.  We check if offsets[5] is also an
  //         exact match but not end-of-function.  It is, so we
  //         return offsets[5].
  // Case 5: lookup_key has offset == 3214.  lower_bound returns
  //         offsets[1].  Since it's not an exact match, we back
  //         up to the offset that's < lookup_key, offsets[0].
  //         We note offsets[0] is a valid entry (not end-of-function),
  //         so that's the entry we return.
  // Case 6: lookup_key has offset == 4000.  lower_bound returns
  //         offsets[8].  Since it's not an exact match, we back
  //         up to offsets[7].  Since offsets[7] indicates
  //         end-of-function, we know lookup_key is between
  //         functions, so we return end() (not a valid offset).
  // Case 7: lookup_key has offset == 5794.  lower_bound returns
  //         offsets[19].  Since it's not an exact match, we back
  //         up to offsets[16].  Note we back up to the *first*
  //         entry with offset 5793, not just offsets[19-1].
  //         We note offsets[16] is a valid entry, so we return it.
  //         If offsets[16] had had line_num == -1, we would have
  //         checked offsets[17].  The reason for this is that
  //         16 and 17 can be in an arbitrary order, since we sort
  //         only by offset and last_line_for_offset.  (Note it
  //         doesn't help to use line_number as a tertiary sort key,
  //         since sometimes we want the -1 to be first and sometimes
  //         we want it to be last.)

  // This deals with cases (1) and (2).
  if ((it == offsets->begin() && offset < it->offset)
      || it == offsets->end())
    return offsets->end();

  // This deals with cases (3) and (4).
  if (offset == it->offset)
    {
      while (it != offsets->end()
             && it->offset == offset
             && it->line_num == -1)
        ++it;
      if (it == offsets->end() || it->offset != offset)
        return offsets->end();
      else
        return it;
    }

  // This handles the first part of case (7) -- we back up to the
  // *first* entry that has the offset that's behind us.
  gold_assert(it != offsets->begin());
  std::vector<Offset_to_lineno_entry>::const_iterator range_end = it;
  --it;
  const off_t range_value = it->offset;
  while (it != offsets->begin() && (it-1)->offset == range_value)
    --it;

  // This handles cases (5), (6), and (7): if any entry in the
  // equal_range [it, range_end) has a line_num != -1, it's a valid
  // match.  If not, we're not in a function.  The line number we saw
  // last for an offset will be sorted first, so it'll get returned if
  // it's present.
  for (; it != range_end; ++it)
    if (it->line_num != -1)
      return it;
  return offsets->end();
}

// Returns the canonical filename:lineno for the address passed in.
// If other_lines is not NULL, appends the non-canonical lines
// assigned to the same address.

template<int size, bool big_endian>
std::string
Sized_dwarf_line_info<size, big_endian>::do_addr2line(
    unsigned int shndx,
    off_t offset,
    std::vector<std::string>* other_lines)
{
  gold_debug(DEBUG_LOCATION, "do_addr2line: shndx %u offset %08x",
	     shndx, static_cast<int>(offset));

  if (this->data_valid_ == false)
    return "";

  const std::vector<Offset_to_lineno_entry>* offsets;
  // If we do not have reloc information, then our input is a .so or
  // some similar data structure where all the information is held in
  // the offset.  In that case, we ignore the input shndx.
  if (this->input_is_relobj())
    offsets = &this->line_number_map_[shndx];
  else
    offsets = &this->line_number_map_[-1U];
  if (offsets->empty())
    return "";

  typename std::vector<Offset_to_lineno_entry>::const_iterator it
      = offset_to_iterator(offsets, offset);
  if (it == offsets->end())
    return "";

  std::string result = this->format_file_lineno(*it);
  gold_debug(DEBUG_LOCATION, "do_addr2line: canonical result: %s",
	     result.c_str());
  if (other_lines != NULL)
    {
      unsigned int last_file_num = it->file_num;
      int last_line_num = it->line_num;
      // Return up to 4 more locations from the beginning of the function
      // for fuzzy matching.
      for (++it; it != offsets->end(); ++it)
	{
	  if (it->offset == offset && it->line_num == -1)
	    continue;  // The end of a previous function.
	  if (it->line_num == -1)
	    break;  // The end of the current function.
	  if (it->file_num != last_file_num || it->line_num != last_line_num)
	    {
	      other_lines->push_back(this->format_file_lineno(*it));
	      gold_debug(DEBUG_LOCATION, "do_addr2line: other: %s",
			 other_lines->back().c_str());
	      last_file_num = it->file_num;
	      last_line_num = it->line_num;
	    }
	  if (it->offset > offset && other_lines->size() >= 4)
	    break;
	}
    }

  return result;
}

// Convert the file_num + line_num into a string.

template<int size, bool big_endian>
std::string
Sized_dwarf_line_info<size, big_endian>::format_file_lineno(
    const Offset_to_lineno_entry& loc) const
{
  std::string ret;

  gold_assert(loc.header_num < static_cast<int>(this->files_.size()));
  gold_assert(loc.file_num
	      < static_cast<unsigned int>(this->files_[loc.header_num].size()));
  const std::pair<int, std::string>& filename_pair
      = this->files_[loc.header_num][loc.file_num];
  const std::string& filename = filename_pair.second;

  gold_assert(loc.header_num < static_cast<int>(this->directories_.size()));
  gold_assert(filename_pair.first
              < static_cast<int>(this->directories_[loc.header_num].size()));
  const std::string& dirname
      = this->directories_[loc.header_num][filename_pair.first];

  if (!dirname.empty())
    {
      ret += dirname;
      ret += "/";
    }
  ret += filename;
  if (ret.empty())
    ret = "(unknown)";

  char buffer[64];   // enough to hold a line number
  snprintf(buffer, sizeof(buffer), "%d", loc.line_num);
  ret += ":";
  ret += buffer;

  return ret;
}

// Dwarf_line_info routines.

static unsigned int next_generation_count = 0;

struct Addr2line_cache_entry
{
  Object* object;
  unsigned int shndx;
  Dwarf_line_info* dwarf_line_info;
  unsigned int generation_count;
  unsigned int access_count;

  Addr2line_cache_entry(Object* o, unsigned int s, Dwarf_line_info* d)
      : object(o), shndx(s), dwarf_line_info(d),
        generation_count(next_generation_count), access_count(0)
  {
    if (next_generation_count < (1U << 31))
      ++next_generation_count;
  }
};
// We expect this cache to be small, so don't bother with a hashtable
// or priority queue or anything: just use a simple vector.
static std::vector<Addr2line_cache_entry> addr2line_cache;

std::string
Dwarf_line_info::one_addr2line(Object* object,
                               unsigned int shndx, off_t offset,
                               size_t cache_size,
                               std::vector<std::string>* other_lines)
{
  Dwarf_line_info* lineinfo = NULL;
  std::vector<Addr2line_cache_entry>::iterator it;

  // First, check the cache.  If we hit, update the counts.
  for (it = addr2line_cache.begin(); it != addr2line_cache.end(); ++it)
    {
      if (it->object == object && it->shndx == shndx)
        {
          lineinfo = it->dwarf_line_info;
          it->generation_count = next_generation_count;
          // We cap generation_count at 2^31 -1 to avoid overflow.
          if (next_generation_count < (1U << 31))
            ++next_generation_count;
          // We cap access_count at 31 so 2^access_count doesn't overflow
          if (it->access_count < 31)
            ++it->access_count;
          break;
        }
    }

  // If we don't hit the cache, create a new object and insert into the
  // cache.
  if (lineinfo == NULL)
  {
    switch (parameters->size_and_endianness())
      {
#ifdef HAVE_TARGET_32_LITTLE
        case Parameters::TARGET_32_LITTLE:
          lineinfo = new Sized_dwarf_line_info<32, false>(object, shndx); break;
#endif
#ifdef HAVE_TARGET_32_BIG
        case Parameters::TARGET_32_BIG:
          lineinfo = new Sized_dwarf_line_info<32, true>(object, shndx); break;
#endif
#ifdef HAVE_TARGET_64_LITTLE
        case Parameters::TARGET_64_LITTLE:
          lineinfo = new Sized_dwarf_line_info<64, false>(object, shndx); break;
#endif
#ifdef HAVE_TARGET_64_BIG
        case Parameters::TARGET_64_BIG:
          lineinfo = new Sized_dwarf_line_info<64, true>(object, shndx); break;
#endif
        default:
          gold_unreachable();
      }
    addr2line_cache.push_back(Addr2line_cache_entry(object, shndx, lineinfo));
  }

  // Now that we have our object, figure out the answer
  std::string retval = lineinfo->addr2line(shndx, offset, other_lines);

  // Finally, if our cache has grown too big, delete old objects.  We
  // assume the common (probably only) case is deleting only one object.
  // We use a pretty simple scheme to evict: function of LRU and MFU.
  while (addr2line_cache.size() > cache_size)
    {
      unsigned int lowest_score = ~0U;
      std::vector<Addr2line_cache_entry>::iterator lowest
          = addr2line_cache.end();
      for (it = addr2line_cache.begin(); it != addr2line_cache.end(); ++it)
        {
          const unsigned int score = (it->generation_count
                                      + (1U << it->access_count));
          if (score < lowest_score)
            {
              lowest_score = score;
              lowest = it;
            }
        }
      if (lowest != addr2line_cache.end())
        {
          delete lowest->dwarf_line_info;
          addr2line_cache.erase(lowest);
        }
    }

  return retval;
}

void
Dwarf_line_info::clear_addr2line_cache()
{
  for (std::vector<Addr2line_cache_entry>::iterator it = addr2line_cache.begin();
       it != addr2line_cache.end();
       ++it)
    delete it->dwarf_line_info;
  addr2line_cache.clear();
}

#ifdef HAVE_TARGET_32_LITTLE
template
class Sized_dwarf_line_info<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Sized_dwarf_line_info<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Sized_dwarf_line_info<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Sized_dwarf_line_info<64, true>;
#endif

} // End namespace gold.
