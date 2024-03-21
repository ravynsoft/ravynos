// output.h -- manage the output file for gold   -*- C++ -*-

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

#ifndef GOLD_OUTPUT_H
#define GOLD_OUTPUT_H

#include <algorithm>
#include <list>
#include <vector>

#include "elfcpp.h"
#include "mapfile.h"
#include "layout.h"
#include "reloc-types.h"

namespace gold
{

class General_options;
class Object;
class Symbol;
class Output_merge_base;
class Output_section;
class Relocatable_relocs;
class Target;
template<int size, bool big_endian>
class Sized_target;
template<int size, bool big_endian>
class Sized_relobj;
template<int size, bool big_endian>
class Sized_relobj_file;

// This class represents the output file.

class Output_file
{
 public:
  Output_file(const char* name);

  // Indicate that this is a temporary file which should not be
  // output.
  void
  set_is_temporary()
  { this->is_temporary_ = true; }

  // Try to open an existing file. Returns false if the file doesn't
  // exist, has a size of 0 or can't be mmaped.  This method is
  // thread-unsafe.  If BASE_NAME is not NULL, use the contents of
  // that file as the base for incremental linking.
  bool
  open_base_file(const char* base_name, bool writable);

  // Open the output file.  FILE_SIZE is the final size of the file.
  // If the file already exists, it is deleted/truncated.  This method
  // is thread-unsafe.
  void
  open(off_t file_size);

  // Resize the output file.  This method is thread-unsafe.
  void
  resize(off_t file_size);

  // Close the output file (flushing all buffered data) and make sure
  // there are no errors.  This method is thread-unsafe.
  void
  close();

  // Return the size of this file.
  off_t
  filesize()
  { return this->file_size_; }

  // Return the name of this file.
  const char*
  filename()
  { return this->name_; }

  // We currently always use mmap which makes the view handling quite
  // simple.  In the future we may support other approaches.

  // Write data to the output file.
  void
  write(off_t offset, const void* data, size_t len)
  { memcpy(this->base_ + offset, data, len); }

  // Get a buffer to use to write to the file, given the offset into
  // the file and the size.
  unsigned char*
  get_output_view(off_t start, size_t size)
  {
    gold_assert(start >= 0
		&& start + static_cast<off_t>(size) <= this->file_size_);
    return this->base_ + start;
  }

  // VIEW must have been returned by get_output_view.  Write the
  // buffer to the file, passing in the offset and the size.
  void
  write_output_view(off_t, size_t, unsigned char*)
  { }

  // Get a read/write buffer.  This is used when we want to write part
  // of the file, read it in, and write it again.
  unsigned char*
  get_input_output_view(off_t start, size_t size)
  { return this->get_output_view(start, size); }

  // Write a read/write buffer back to the file.
  void
  write_input_output_view(off_t, size_t, unsigned char*)
  { }

  // Get a read buffer.  This is used when we just want to read part
  // of the file back it in.
  const unsigned char*
  get_input_view(off_t start, size_t size)
  { return this->get_output_view(start, size); }

  // Release a read bfufer.
  void
  free_input_view(off_t, size_t, const unsigned char*)
  { }

 private:
  // Map the file into memory or, if that fails, allocate anonymous
  // memory.
  void
  map();

  // Allocate anonymous memory for the file.
  bool
  map_anonymous();

  // Map the file into memory.
  bool
  map_no_anonymous(bool);

  // Unmap the file from memory (and flush to disk buffers).
  void
  unmap();

  // File name.
  const char* name_;
  // File descriptor.
  int o_;
  // File size.
  off_t file_size_;
  // Base of file mapped into memory.
  unsigned char* base_;
  // True iff base_ points to a memory buffer rather than an output file.
  bool map_is_anonymous_;
  // True if base_ was allocated using new rather than mmap.
  bool map_is_allocated_;
  // True if this is a temporary file which should not be output.
  bool is_temporary_;
};

// An abtract class for data which has to go into the output file.

class Output_data
{
 public:
  explicit Output_data()
    : address_(0), data_size_(0), offset_(-1),
      is_address_valid_(false), is_data_size_valid_(false),
      is_offset_valid_(false), is_data_size_fixed_(false),
      has_dynamic_reloc_(false)
  { }

  virtual
  ~Output_data();

  // Return the address.  For allocated sections, this is only valid
  // after Layout::finalize is finished.
  uint64_t
  address() const
  {
    gold_assert(this->is_address_valid_);
    return this->address_;
  }

  // Return the size of the data.  For allocated sections, this must
  // be valid after Layout::finalize calls set_address, but need not
  // be valid before then.
  off_t
  data_size() const
  {
    gold_assert(this->is_data_size_valid_);
    return this->data_size_;
  }

  // Get the current data size.
  off_t
  current_data_size() const
  { return this->current_data_size_for_child(); }

  // Return true if data size is fixed.
  bool
  is_data_size_fixed() const
  { return this->is_data_size_fixed_; }

  // Return the file offset.  This is only valid after
  // Layout::finalize is finished.  For some non-allocated sections,
  // it may not be valid until near the end of the link.
  off_t
  offset() const
  {
    gold_assert(this->is_offset_valid_);
    return this->offset_;
  }

  // Reset the address, file offset and data size.  This essentially
  // disables the sanity testing about duplicate and unknown settings.
  void
  reset_address_and_file_offset()
  {
    this->is_address_valid_ = false;
    this->is_offset_valid_ = false;
    if (!this->is_data_size_fixed_)
      this->is_data_size_valid_ = false;
    this->do_reset_address_and_file_offset();
  }

  // As above, but just for data size.
  void
  reset_data_size()
  {
    if (!this->is_data_size_fixed_)
      this->is_data_size_valid_ = false;
  }

  // Return true if address and file offset already have reset values. In
  // other words, calling reset_address_and_file_offset will not change them.
  bool
  address_and_file_offset_have_reset_values() const
  { return this->do_address_and_file_offset_have_reset_values(); }

  // Return the required alignment.
  uint64_t
  addralign() const
  { return this->do_addralign(); }

  // Return whether this has a load address.
  bool
  has_load_address() const
  { return this->do_has_load_address(); }

  // Return the load address.
  uint64_t
  load_address() const
  { return this->do_load_address(); }

  // Return whether this is an Output_section.
  bool
  is_section() const
  { return this->do_is_section(); }

  // Return whether this is an Output_section of the specified type.
  bool
  is_section_type(elfcpp::Elf_Word stt) const
  { return this->do_is_section_type(stt); }

  // Return whether this is an Output_section with the specified flag
  // set.
  bool
  is_section_flag_set(elfcpp::Elf_Xword shf) const
  { return this->do_is_section_flag_set(shf); }

  // Return the output section that this goes in, if there is one.
  Output_section*
  output_section()
  { return this->do_output_section(); }

  const Output_section*
  output_section() const
  { return this->do_output_section(); }

  // Return the output section index, if there is an output section.
  unsigned int
  out_shndx() const
  { return this->do_out_shndx(); }

  // Set the output section index, if this is an output section.
  void
  set_out_shndx(unsigned int shndx)
  { this->do_set_out_shndx(shndx); }

  // Set the address and file offset of this data, and finalize the
  // size of the data.  This is called during Layout::finalize for
  // allocated sections.
  void
  set_address_and_file_offset(uint64_t addr, off_t off)
  {
    this->set_address(addr);
    this->set_file_offset(off);
    this->finalize_data_size();
  }

  // Set the address.
  void
  set_address(uint64_t addr)
  {
    gold_assert(!this->is_address_valid_);
    this->address_ = addr;
    this->is_address_valid_ = true;
  }

  // Set the file offset.
  void
  set_file_offset(off_t off)
  {
    gold_assert(!this->is_offset_valid_);
    this->offset_ = off;
    this->is_offset_valid_ = true;
  }

  // Update the data size without finalizing it.
  void
  pre_finalize_data_size()
  {
    if (!this->is_data_size_valid_)
      {
	// Tell the child class to update the data size.
	this->update_data_size();
      }
  }

  // Finalize the data size.
  void
  finalize_data_size()
  {
    if (!this->is_data_size_valid_)
      {
	// Tell the child class to set the data size.
	this->set_final_data_size();
	gold_assert(this->is_data_size_valid_);
      }
  }

  // Set the TLS offset.  Called only for SHT_TLS sections.
  void
  set_tls_offset(uint64_t tls_base)
  { this->do_set_tls_offset(tls_base); }

  // Return the TLS offset, relative to the base of the TLS segment.
  // Valid only for SHT_TLS sections.
  uint64_t
  tls_offset() const
  { return this->do_tls_offset(); }

  // Write the data to the output file.  This is called after
  // Layout::finalize is complete.
  void
  write(Output_file* file)
  { this->do_write(file); }

  // This is called by Layout::finalize to note that the sizes of
  // allocated sections must now be fixed.
  static void
  layout_complete()
  { Output_data::allocated_sizes_are_fixed = true; }

  // Used to check that layout has been done.
  static bool
  is_layout_complete()
  { return Output_data::allocated_sizes_are_fixed; }

  // Note that a dynamic reloc has been applied to this data.
  void
  add_dynamic_reloc()
  { this->has_dynamic_reloc_ = true; }

  // Return whether a dynamic reloc has been applied.
  bool
  has_dynamic_reloc() const
  { return this->has_dynamic_reloc_; }

  // Whether the address is valid.
  bool
  is_address_valid() const
  { return this->is_address_valid_; }

  // Whether the file offset is valid.
  bool
  is_offset_valid() const
  { return this->is_offset_valid_; }

  // Whether the data size is valid.
  bool
  is_data_size_valid() const
  { return this->is_data_size_valid_; }

  // Print information to the map file.
  void
  print_to_mapfile(Mapfile* mapfile) const
  { return this->do_print_to_mapfile(mapfile); }

 protected:
  // Functions that child classes may or in some cases must implement.

  // Write the data to the output file.
  virtual void
  do_write(Output_file*) = 0;

  // Return the required alignment.
  virtual uint64_t
  do_addralign() const = 0;

  // Return whether this has a load address.
  virtual bool
  do_has_load_address() const
  { return false; }

  // Return the load address.
  virtual uint64_t
  do_load_address() const
  { gold_unreachable(); }

  // Return whether this is an Output_section.
  virtual bool
  do_is_section() const
  { return false; }

  // Return whether this is an Output_section of the specified type.
  // This only needs to be implement by Output_section.
  virtual bool
  do_is_section_type(elfcpp::Elf_Word) const
  { return false; }

  // Return whether this is an Output_section with the specific flag
  // set.  This only needs to be implemented by Output_section.
  virtual bool
  do_is_section_flag_set(elfcpp::Elf_Xword) const
  { return false; }

  // Return the output section, if there is one.
  virtual Output_section*
  do_output_section()
  { return NULL; }

  virtual const Output_section*
  do_output_section() const
  { return NULL; }

  // Return the output section index, if there is an output section.
  virtual unsigned int
  do_out_shndx() const
  { gold_unreachable(); }

  // Set the output section index, if this is an output section.
  virtual void
  do_set_out_shndx(unsigned int)
  { gold_unreachable(); }

  // This is a hook for derived classes to set the preliminary data size.
  // This is called by pre_finalize_data_size, normally called during
  // Layout::finalize, before the section address is set, and is used
  // during an incremental update, when we need to know the size of a
  // section before allocating space in the output file.  For classes
  // where the current data size is up to date, this default version of
  // the method can be inherited.
  virtual void
  update_data_size()
  { }

  // This is a hook for derived classes to set the data size.  This is
  // called by finalize_data_size, normally called during
  // Layout::finalize, when the section address is set.
  virtual void
  set_final_data_size()
  { gold_unreachable(); }

  // A hook for resetting the address and file offset.
  virtual void
  do_reset_address_and_file_offset()
  { }

  // Return true if address and file offset already have reset values. In
  // other words, calling reset_address_and_file_offset will not change them.
  // A child class overriding do_reset_address_and_file_offset may need to
  // also override this.
  virtual bool
  do_address_and_file_offset_have_reset_values() const
  { return !this->is_address_valid_ && !this->is_offset_valid_; }

  // Set the TLS offset.  Called only for SHT_TLS sections.
  virtual void
  do_set_tls_offset(uint64_t)
  { gold_unreachable(); }

  // Return the TLS offset, relative to the base of the TLS segment.
  // Valid only for SHT_TLS sections.
  virtual uint64_t
  do_tls_offset() const
  { gold_unreachable(); }

  // Print to the map file.  This only needs to be implemented by
  // classes which may appear in a PT_LOAD segment.
  virtual void
  do_print_to_mapfile(Mapfile*) const
  { gold_unreachable(); }

  // Functions that child classes may call.

  // Reset the address.  The Output_section class needs this when an
  // SHF_ALLOC input section is added to an output section which was
  // formerly not SHF_ALLOC.
  void
  mark_address_invalid()
  { this->is_address_valid_ = false; }

  // Set the size of the data.
  void
  set_data_size(off_t data_size)
  {
    gold_assert(!this->is_data_size_valid_
		&& !this->is_data_size_fixed_);
    this->data_size_ = data_size;
    this->is_data_size_valid_ = true;
  }

  // Fix the data size.  Once it is fixed, it cannot be changed
  // and the data size remains always valid.
  void
  fix_data_size()
  {
    gold_assert(this->is_data_size_valid_);
    this->is_data_size_fixed_ = true;
  }

  // Get the current data size--this is for the convenience of
  // sections which build up their size over time.
  off_t
  current_data_size_for_child() const
  { return this->data_size_; }

  // Set the current data size--this is for the convenience of
  // sections which build up their size over time.
  void
  set_current_data_size_for_child(off_t data_size)
  {
    gold_assert(!this->is_data_size_valid_);
    this->data_size_ = data_size;
  }

  // Return default alignment for the target size.
  static uint64_t
  default_alignment();

  // Return default alignment for a specified size--32 or 64.
  static uint64_t
  default_alignment_for_size(int size);

 private:
  Output_data(const Output_data&);
  Output_data& operator=(const Output_data&);

  // This is used for verification, to make sure that we don't try to
  // change any sizes of allocated sections after we set the section
  // addresses.
  static bool allocated_sizes_are_fixed;

  // Memory address in output file.
  uint64_t address_;
  // Size of data in output file.
  off_t data_size_;
  // File offset of contents in output file.
  off_t offset_;
  // Whether address_ is valid.
  bool is_address_valid_ : 1;
  // Whether data_size_ is valid.
  bool is_data_size_valid_ : 1;
  // Whether offset_ is valid.
  bool is_offset_valid_ : 1;
  // Whether data size is fixed.
  bool is_data_size_fixed_ : 1;
  // Whether any dynamic relocs have been applied to this section.
  bool has_dynamic_reloc_ : 1;
};

// Output the section headers.

class Output_section_headers : public Output_data
{
 public:
  Output_section_headers(const Layout*,
			 const Layout::Segment_list*,
			 const Layout::Section_list*,
			 const Layout::Section_list*,
			 const Stringpool*,
			 const Output_section*);

 protected:
  // Write the data to the file.
  void
  do_write(Output_file*);

  // Return the required alignment.
  uint64_t
  do_addralign() const
  { return Output_data::default_alignment(); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** section headers")); }

  // Update the data size.
  void
  update_data_size()
  { this->set_data_size(this->do_size()); }

  // Set final data size.
  void
  set_final_data_size()
  { this->set_data_size(this->do_size()); }

 private:
  // Write the data to the file with the right size and endianness.
  template<int size, bool big_endian>
  void
  do_sized_write(Output_file*);

  // Compute data size.
  off_t
  do_size() const;

  const Layout* layout_;
  const Layout::Segment_list* segment_list_;
  const Layout::Section_list* section_list_;
  const Layout::Section_list* unattached_section_list_;
  const Stringpool* secnamepool_;
  const Output_section* shstrtab_section_;
};

// Output the segment headers.

class Output_segment_headers : public Output_data
{
 public:
  Output_segment_headers(const Layout::Segment_list& segment_list);

 protected:
  // Write the data to the file.
  void
  do_write(Output_file*);

  // Return the required alignment.
  uint64_t
  do_addralign() const
  { return Output_data::default_alignment(); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** segment headers")); }

  // Set final data size.
  void
  set_final_data_size()
  { this->set_data_size(this->do_size()); }

 private:
  // Write the data to the file with the right size and endianness.
  template<int size, bool big_endian>
  void
  do_sized_write(Output_file*);

  // Compute the current size.
  off_t
  do_size() const;

  const Layout::Segment_list& segment_list_;
};

// Output the ELF file header.

class Output_file_header : public Output_data
{
 public:
  Output_file_header(Target*,
		     const Symbol_table*,
		     const Output_segment_headers*);

  // Add information about the section headers.  We lay out the ELF
  // file header before we create the section headers.
  void set_section_info(const Output_section_headers*,
			const Output_section* shstrtab);

 protected:
  // Write the data to the file.
  void
  do_write(Output_file*);

  // Return the required alignment.
  uint64_t
  do_addralign() const
  { return Output_data::default_alignment(); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** file header")); }

  // Set final data size.
  void
  set_final_data_size(void)
  { this->set_data_size(this->do_size()); }

 private:
  // Write the data to the file with the right size and endianness.
  template<int size, bool big_endian>
  void
  do_sized_write(Output_file*);

  // Return the value to use for the entry address.
  template<int size>
  typename elfcpp::Elf_types<size>::Elf_Addr
  entry();

  // Compute the current data size.
  off_t
  do_size() const;

  Target* target_;
  const Symbol_table* symtab_;
  const Output_segment_headers* segment_header_;
  const Output_section_headers* section_header_;
  const Output_section* shstrtab_;
};

// Output sections are mainly comprised of input sections.  However,
// there are cases where we have data to write out which is not in an
// input section.  Output_section_data is used in such cases.  This is
// an abstract base class.

class Output_section_data : public Output_data
{
 public:
  Output_section_data(off_t data_size, uint64_t addralign,
		      bool is_data_size_fixed)
    : Output_data(), output_section_(NULL), addralign_(addralign)
  {
    this->set_data_size(data_size);
    if (is_data_size_fixed)
      this->fix_data_size();
  }

  Output_section_data(uint64_t addralign)
    : Output_data(), output_section_(NULL), addralign_(addralign)
  { }

  // Return the output section.
  Output_section*
  output_section()
  { return this->output_section_; }

  const Output_section*
  output_section() const
  { return this->output_section_; }

  // Record the output section.
  void
  set_output_section(Output_section* os);

  // Add an input section, for SHF_MERGE sections.  This returns true
  // if the section was handled.
  bool
  add_input_section(Relobj* object, unsigned int shndx)
  { return this->do_add_input_section(object, shndx); }

  // Given an input OBJECT, an input section index SHNDX within that
  // object, and an OFFSET relative to the start of that input
  // section, return whether or not the corresponding offset within
  // the output section is known.  If this function returns true, it
  // sets *POUTPUT to the output offset.  The value -1 indicates that
  // this input offset is being discarded.
  bool
  output_offset(const Relobj* object, unsigned int shndx,
		section_offset_type offset,
		section_offset_type* poutput) const
  { return this->do_output_offset(object, shndx, offset, poutput); }

  // Write the contents to a buffer.  This is used for sections which
  // require postprocessing, such as compression.
  void
  write_to_buffer(unsigned char* buffer)
  { this->do_write_to_buffer(buffer); }

  // Print merge stats to stderr.  This should only be called for
  // SHF_MERGE sections.
  void
  print_merge_stats(const char* section_name)
  { this->do_print_merge_stats(section_name); }

 protected:
  // The child class must implement do_write.

  // The child class may implement specific adjustments to the output
  // section.
  virtual void
  do_adjust_output_section(Output_section*)
  { }

  // May be implemented by child class.  Return true if the section
  // was handled.
  virtual bool
  do_add_input_section(Relobj*, unsigned int)
  { gold_unreachable(); }

  // The child class may implement output_offset.
  virtual bool
  do_output_offset(const Relobj*, unsigned int, section_offset_type,
		   section_offset_type*) const
  { return false; }

  // The child class may implement write_to_buffer.  Most child
  // classes can not appear in a compressed section, and they do not
  // implement this.
  virtual void
  do_write_to_buffer(unsigned char*)
  { gold_unreachable(); }

  // Print merge statistics.
  virtual void
  do_print_merge_stats(const char*)
  { gold_unreachable(); }

  // Return the required alignment.
  uint64_t
  do_addralign() const
  { return this->addralign_; }

  // Return the output section.
  Output_section*
  do_output_section()
  { return this->output_section_; }

  const Output_section*
  do_output_section() const
  { return this->output_section_; }

  // Return the section index of the output section.
  unsigned int
  do_out_shndx() const;

  // Set the alignment.
  void
  set_addralign(uint64_t addralign);

 private:
  // The output section for this section.
  Output_section* output_section_;
  // The required alignment.
  uint64_t addralign_;
};

// Some Output_section_data classes build up their data step by step,
// rather than all at once.  This class provides an interface for
// them.

class Output_section_data_build : public Output_section_data
{
 public:
  Output_section_data_build(uint64_t addralign)
    : Output_section_data(addralign)
  { }

  Output_section_data_build(off_t data_size, uint64_t addralign)
    : Output_section_data(data_size, addralign, false)
  { }

  // Set the current data size.
  void
  set_current_data_size(off_t data_size)
  { this->set_current_data_size_for_child(data_size); }

 protected:
  // Set the final data size.
  virtual void
  set_final_data_size()
  { this->set_data_size(this->current_data_size_for_child()); }
};

// A simple case of Output_data in which we have constant data to
// output.

class Output_data_const : public Output_section_data
{
 public:
  Output_data_const(const std::string& data, uint64_t addralign)
    : Output_section_data(data.size(), addralign, true), data_(data)
  { }

  Output_data_const(const char* p, off_t len, uint64_t addralign)
    : Output_section_data(len, addralign, true), data_(p, len)
  { }

  Output_data_const(const unsigned char* p, off_t len, uint64_t addralign)
    : Output_section_data(len, addralign, true),
      data_(reinterpret_cast<const char*>(p), len)
  { }

 protected:
  // Write the data to the output file.
  void
  do_write(Output_file*);

  // Write the data to a buffer.
  void
  do_write_to_buffer(unsigned char* buffer)
  { memcpy(buffer, this->data_.data(), this->data_.size()); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** fill")); }

 private:
  std::string data_;
};

// Another version of Output_data with constant data, in which the
// buffer is allocated by the caller.

class Output_data_const_buffer : public Output_section_data
{
 public:
  Output_data_const_buffer(const unsigned char* p, off_t len,
			   uint64_t addralign, const char* map_name)
    : Output_section_data(len, addralign, true),
      p_(p), map_name_(map_name)
  { }

 protected:
  // Write the data the output file.
  void
  do_write(Output_file*);

  // Write the data to a buffer.
  void
  do_write_to_buffer(unsigned char* buffer)
  { memcpy(buffer, this->p_, this->data_size()); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(this->map_name_)); }

 private:
  // The data to output.
  const unsigned char* p_;
  // Name to use in a map file.  Maps are a rarely used feature, but
  // the space usage is minor as aren't very many of these objects.
  const char* map_name_;
};

// A place holder for a fixed amount of data written out via some
// other mechanism.

class Output_data_fixed_space : public Output_section_data
{
 public:
  Output_data_fixed_space(off_t data_size, uint64_t addralign,
			  const char* map_name)
    : Output_section_data(data_size, addralign, true),
      map_name_(map_name)
  { }

 protected:
  // Write out the data--the actual data must be written out
  // elsewhere.
  void
  do_write(Output_file*)
  { }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(this->map_name_)); }

 private:
  // Name to use in a map file.  Maps are a rarely used feature, but
  // the space usage is minor as aren't very many of these objects.
  const char* map_name_;
};

// A place holder for variable sized data written out via some other
// mechanism.

class Output_data_space : public Output_section_data_build
{
 public:
  explicit Output_data_space(uint64_t addralign, const char* map_name)
    : Output_section_data_build(addralign),
      map_name_(map_name)
  { }

  explicit Output_data_space(off_t data_size, uint64_t addralign,
			     const char* map_name)
    : Output_section_data_build(data_size, addralign),
      map_name_(map_name)
  { }

  // Set the alignment.
  void
  set_space_alignment(uint64_t align)
  { this->set_addralign(align); }

 protected:
  // Write out the data--the actual data must be written out
  // elsewhere.
  void
  do_write(Output_file*)
  { }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _(this->map_name_)); }

 private:
  // Name to use in a map file.  Maps are a rarely used feature, but
  // the space usage is minor as aren't very many of these objects.
  const char* map_name_;
};

// Fill fixed space with zeroes.  This is just like
// Output_data_fixed_space, except that the map name is known.

class Output_data_zero_fill : public Output_section_data
{
 public:
  Output_data_zero_fill(off_t data_size, uint64_t addralign)
    : Output_section_data(data_size, addralign, true)
  { }

 protected:
  // There is no data to write out.
  void
  do_write(Output_file*)
  { }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, "** zero fill"); }
};

// A string table which goes into an output section.

class Output_data_strtab : public Output_section_data
{
 public:
  Output_data_strtab(Stringpool* strtab)
    : Output_section_data(1), strtab_(strtab)
  { }

 protected:
  // This is called to update the section size prior to assigning
  // the address and file offset.
  void
  update_data_size()
  { this->set_final_data_size(); }

  // This is called to set the address and file offset.  Here we make
  // sure that the Stringpool is finalized.
  void
  set_final_data_size();

  // Write out the data.
  void
  do_write(Output_file*);

  // Write the data to a buffer.
  void
  do_write_to_buffer(unsigned char* buffer)
  { this->strtab_->write_to_buffer(buffer, this->data_size()); }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** string table")); }

 private:
  Stringpool* strtab_;
};

// This POD class is used to represent a single reloc in the output
// file.  This could be a private class within Output_data_reloc, but
// the templatization is complex enough that I broke it out into a
// separate class.  The class is templatized on either elfcpp::SHT_REL
// or elfcpp::SHT_RELA, and also on whether this is a dynamic
// relocation or an ordinary relocation.

// A relocation can be against a global symbol, a local symbol, a
// local section symbol, an output section, or the undefined symbol at
// index 0.  We represent the latter by using a NULL global symbol.

template<int sh_type, bool dynamic, int size, bool big_endian>
class Output_reloc;

template<bool dynamic, int size, bool big_endian>
class Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Addend;

  static const Address invalid_address = static_cast<Address>(0) - 1;

  // An uninitialized entry.  We need this because we want to put
  // instances of this class into an STL container.
  Output_reloc()
    : local_sym_index_(INVALID_CODE)
  { }

  // We have a bunch of different constructors.  They come in pairs
  // depending on how the address of the relocation is specified.  It
  // can either be an offset in an Output_data or an offset in an
  // input section.

  // A reloc against a global symbol.

  Output_reloc(Symbol* gsym, unsigned int type, Output_data* od,
	       Address address, bool is_relative, bool is_symbolless,
	       bool use_plt_offset);

  Output_reloc(Symbol* gsym, unsigned int type,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, bool is_relative,
	       bool is_symbolless, bool use_plt_offset);

  // A reloc against a local symbol or local section symbol.

  Output_reloc(Sized_relobj<size, big_endian>* relobj,
	       unsigned int local_sym_index, unsigned int type,
	       Output_data* od, Address address, bool is_relative,
	       bool is_symbolless, bool is_section_symbol,
	       bool use_plt_offset);

  Output_reloc(Sized_relobj<size, big_endian>* relobj,
	       unsigned int local_sym_index, unsigned int type,
	       unsigned int shndx, Address address, bool is_relative,
	       bool is_symbolless, bool is_section_symbol,
	       bool use_plt_offset);

  // A reloc against the STT_SECTION symbol of an output section.

  Output_reloc(Output_section* os, unsigned int type, Output_data* od,
	       Address address, bool is_relative);

  Output_reloc(Output_section* os, unsigned int type,
	       Sized_relobj<size, big_endian>* relobj, unsigned int shndx,
	       Address address, bool is_relative);

  // An absolute or relative relocation with no symbol.

  Output_reloc(unsigned int type, Output_data* od, Address address,
	       bool is_relative);

  Output_reloc(unsigned int type, Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, bool is_relative);

  // A target specific relocation.  The target will be called to get
  // the symbol index, passing ARG.  The type and offset will be set
  // as for other relocation types.

  Output_reloc(unsigned int type, void* arg, Output_data* od,
	       Address address);

  Output_reloc(unsigned int type, void* arg,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address);

  // Return the reloc type.
  unsigned int
  type() const
  { return this->type_; }

  // Return whether this is a RELATIVE relocation.
  bool
  is_relative() const
  { return this->is_relative_; }

  // Return whether this is a relocation which should not use
  // a symbol, but which obtains its addend from a symbol.
  bool
  is_symbolless() const
  { return this->is_symbolless_; }

  // Return whether this is against a local section symbol.
  bool
  is_local_section_symbol() const
  {
    return (this->local_sym_index_ != GSYM_CODE
	    && this->local_sym_index_ != SECTION_CODE
	    && this->local_sym_index_ != INVALID_CODE
	    && this->local_sym_index_ != TARGET_CODE
	    && this->is_section_symbol_);
  }

  // Return whether this is a target specific relocation.
  bool
  is_target_specific() const
  { return this->local_sym_index_ == TARGET_CODE; }

  // Return the argument to pass to the target for a target specific
  // relocation.
  void*
  target_arg() const
  {
    gold_assert(this->local_sym_index_ == TARGET_CODE);
    return this->u1_.arg;
  }

  // For a local section symbol, return the offset of the input
  // section within the output section.  ADDEND is the addend being
  // applied to the input section.
  Address
  local_section_offset(Addend addend) const;

  // Get the value of the symbol referred to by a Rel relocation when
  // we are adding the given ADDEND.
  Address
  symbol_value(Addend addend) const;

  // If this relocation is against an input section, return the
  // relocatable object containing the input section.
  Sized_relobj<size, big_endian>*
  get_relobj() const
  {
    if (this->shndx_ == INVALID_CODE)
      return NULL;
    return this->u2_.relobj;
  }

  // Write the reloc entry to an output view.
  void
  write(unsigned char* pov) const;

  // Write the offset and info fields to Write_rel.
  template<typename Write_rel>
  void write_rel(Write_rel*) const;

  // This is used when sorting dynamic relocs.  Return -1 to sort this
  // reloc before R2, 0 to sort the same as R2, 1 to sort after R2.
  int
  compare(const Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>& r2)
    const;

  // Return whether this reloc should be sorted before the argument
  // when sorting dynamic relocs.
  bool
  sort_before(const Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>&
	      r2) const
  { return this->compare(r2) < 0; }

  // Return the symbol index.
  unsigned int
  get_symbol_index() const;

  // Return the output address.
  Address
  get_address() const;

 private:
  // Record that we need a dynamic symbol index.
  void
  set_needs_dynsym_index();

  // Codes for local_sym_index_.
  enum
  {
    // Global symbol.
    GSYM_CODE = -1U,
    // Output section.
    SECTION_CODE = -2U,
    // Target specific.
    TARGET_CODE = -3U,
    // Invalid uninitialized entry.
    INVALID_CODE = -4U
  };

  union
  {
    // For a local symbol or local section symbol
    // (this->local_sym_index_ >= 0), the object.  We will never
    // generate a relocation against a local symbol in a dynamic
    // object; that doesn't make sense.  And our callers will always
    // be templatized, so we use Sized_relobj here.
    Sized_relobj<size, big_endian>* relobj;
    // For a global symbol (this->local_sym_index_ == GSYM_CODE, the
    // symbol.  If this is NULL, it indicates a relocation against the
    // undefined 0 symbol.
    Symbol* gsym;
    // For a relocation against an output section
    // (this->local_sym_index_ == SECTION_CODE), the output section.
    Output_section* os;
    // For a target specific relocation, an argument to pass to the
    // target.
    void* arg;
  } u1_;
  union
  {
    // If this->shndx_ is not INVALID CODE, the object which holds the
    // input section being used to specify the reloc address.
    Sized_relobj<size, big_endian>* relobj;
    // If this->shndx_ is INVALID_CODE, the output data being used to
    // specify the reloc address.  This may be NULL if the reloc
    // address is absolute.
    Output_data* od;
  } u2_;
  // The address offset within the input section or the Output_data.
  Address address_;
  // This is GSYM_CODE for a global symbol, or SECTION_CODE for a
  // relocation against an output section, or TARGET_CODE for a target
  // specific relocation, or INVALID_CODE for an uninitialized value.
  // Otherwise, for a local symbol (this->is_section_symbol_ is
  // false), the local symbol index.  For a local section symbol
  // (this->is_section_symbol_ is true), the section index in the
  // input file.
  unsigned int local_sym_index_;
  // The reloc type--a processor specific code.
  unsigned int type_ : 28;
  // True if the relocation is a RELATIVE relocation.
  bool is_relative_ : 1;
  // True if the relocation is one which should not use
  // a symbol, but which obtains its addend from a symbol.
  bool is_symbolless_ : 1;
  // True if the relocation is against a section symbol.
  bool is_section_symbol_ : 1;
  // True if the addend should be the PLT offset.
  // (Used only for RELA, but stored here for space.)
  bool use_plt_offset_ : 1;
  // If the reloc address is an input section in an object, the
  // section index.  This is INVALID_CODE if the reloc address is
  // specified in some other way.
  unsigned int shndx_;
};

// The SHT_RELA version of Output_reloc<>.  This is just derived from
// the SHT_REL version of Output_reloc, but it adds an addend.

template<bool dynamic, int size, bool big_endian>
class Output_reloc<elfcpp::SHT_RELA, dynamic, size, big_endian>
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Addend;

  // An uninitialized entry.
  Output_reloc()
    : rel_()
  { }

  // A reloc against a global symbol.

  Output_reloc(Symbol* gsym, unsigned int type, Output_data* od,
	       Address address, Addend addend, bool is_relative,
	       bool is_symbolless, bool use_plt_offset)
    : rel_(gsym, type, od, address, is_relative, is_symbolless,
	   use_plt_offset),
      addend_(addend)
  { }

  Output_reloc(Symbol* gsym, unsigned int type,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend,
	       bool is_relative, bool is_symbolless, bool use_plt_offset)
    : rel_(gsym, type, relobj, shndx, address, is_relative,
	   is_symbolless, use_plt_offset), addend_(addend)
  { }

  // A reloc against a local symbol.

  Output_reloc(Sized_relobj<size, big_endian>* relobj,
	       unsigned int local_sym_index, unsigned int type,
	       Output_data* od, Address address,
	       Addend addend, bool is_relative,
	       bool is_symbolless, bool is_section_symbol,
	       bool use_plt_offset)
    : rel_(relobj, local_sym_index, type, od, address, is_relative,
	   is_symbolless, is_section_symbol, use_plt_offset),
      addend_(addend)
  { }

  Output_reloc(Sized_relobj<size, big_endian>* relobj,
	       unsigned int local_sym_index, unsigned int type,
	       unsigned int shndx, Address address,
	       Addend addend, bool is_relative,
	       bool is_symbolless, bool is_section_symbol,
	       bool use_plt_offset)
    : rel_(relobj, local_sym_index, type, shndx, address, is_relative,
	   is_symbolless, is_section_symbol, use_plt_offset),
      addend_(addend)
  { }

  // A reloc against the STT_SECTION symbol of an output section.

  Output_reloc(Output_section* os, unsigned int type, Output_data* od,
	       Address address, Addend addend, bool is_relative)
    : rel_(os, type, od, address, is_relative), addend_(addend)
  { }

  Output_reloc(Output_section* os, unsigned int type,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend,
	       bool is_relative)
    : rel_(os, type, relobj, shndx, address, is_relative), addend_(addend)
  { }

  // An absolute or relative relocation with no symbol.

  Output_reloc(unsigned int type, Output_data* od, Address address,
	       Addend addend, bool is_relative)
    : rel_(type, od, address, is_relative), addend_(addend)
  { }

  Output_reloc(unsigned int type, Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend,
	       bool is_relative)
    : rel_(type, relobj, shndx, address, is_relative), addend_(addend)
  { }

  // A target specific relocation.  The target will be called to get
  // the symbol index and the addend, passing ARG.  The type and
  // offset will be set as for other relocation types.

  Output_reloc(unsigned int type, void* arg, Output_data* od,
	       Address address, Addend addend)
    : rel_(type, arg, od, address), addend_(addend)
  { }

  Output_reloc(unsigned int type, void* arg,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend)
    : rel_(type, arg, relobj, shndx, address), addend_(addend)
  { }

  // Return whether this is a RELATIVE relocation.
  bool
  is_relative() const
  { return this->rel_.is_relative(); }

  // Return whether this is a relocation which should not use
  // a symbol, but which obtains its addend from a symbol.
  bool
  is_symbolless() const
  { return this->rel_.is_symbolless(); }

  // If this relocation is against an input section, return the
  // relocatable object containing the input section.
  Sized_relobj<size, big_endian>*
  get_relobj() const
  { return this->rel_.get_relobj(); }

  // Write the reloc entry to an output view.
  void
  write(unsigned char* pov) const;

  // Return whether this reloc should be sorted before the argument
  // when sorting dynamic relocs.
  bool
  sort_before(const Output_reloc<elfcpp::SHT_RELA, dynamic, size, big_endian>&
	      r2) const
  {
    int i = this->rel_.compare(r2.rel_);
    if (i < 0)
      return true;
    else if (i > 0)
      return false;
    else
      return this->addend_ < r2.addend_;
  }

 private:
  // The basic reloc.
  Output_reloc<elfcpp::SHT_REL, dynamic, size, big_endian> rel_;
  // The addend.
  Addend addend_;
};

// Output_data_reloc_generic is a non-template base class for
// Output_data_reloc_base.  This gives the generic code a way to hold
// a pointer to a reloc section.

class Output_data_reloc_generic : public Output_section_data_build
{
 public:
  Output_data_reloc_generic(int size, bool sort_relocs)
    : Output_section_data_build(Output_data::default_alignment_for_size(size)),
      relative_reloc_count_(0), sort_relocs_(sort_relocs)
  { }

  // Return the number of relative relocs in this section.
  size_t
  relative_reloc_count() const
  { return this->relative_reloc_count_; }

  // Whether we should sort the relocs.
  bool
  sort_relocs() const
  { return this->sort_relocs_; }

  // Add a reloc of type TYPE against the global symbol GSYM.  The
  // relocation applies to the data at offset ADDRESS within OD.
  virtual void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     uint64_t address, uint64_t addend) = 0;

  // Add a reloc of type TYPE against the global symbol GSYM.  The
  // relocation applies to data at offset ADDRESS within section SHNDX
  // of object file RELOBJ.  OD is the associated output section.
  virtual void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     Relobj* relobj, unsigned int shndx, uint64_t address,
		     uint64_t addend) = 0;

  // Add a reloc of type TYPE against the local symbol LOCAL_SYM_INDEX
  // in RELOBJ.  The relocation applies to the data at offset ADDRESS
  // within OD.
  virtual void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, uint64_t address,
		    uint64_t addend) = 0;

  // Add a reloc of type TYPE against the local symbol LOCAL_SYM_INDEX
  // in RELOBJ.  The relocation applies to the data at offset ADDRESS
  // within section SHNDX of RELOBJ.  OD is the associated output
  // section.
  virtual void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, unsigned int shndx,
		    uint64_t address, uint64_t addend) = 0;

  // Add a reloc of type TYPE against the STT_SECTION symbol of the
  // output section OS.  The relocation applies to the data at offset
  // ADDRESS within OD.
  virtual void
  add_output_section_generic(Output_section *os, unsigned int type,
			     Output_data* od, uint64_t address,
			     uint64_t addend) = 0;

  // Add a reloc of type TYPE against the STT_SECTION symbol of the
  // output section OS.  The relocation applies to the data at offset
  // ADDRESS within section SHNDX of RELOBJ.  OD is the associated
  // output section.
  virtual void
  add_output_section_generic(Output_section* os, unsigned int type,
			     Output_data* od, Relobj* relobj,
			     unsigned int shndx, uint64_t address,
			     uint64_t addend) = 0;

 protected:
  // Note that we've added another relative reloc.
  void
  bump_relative_reloc_count()
  { ++this->relative_reloc_count_; }

 private:
  // The number of relative relocs added to this section.  This is to
  // support DT_RELCOUNT.
  size_t relative_reloc_count_;
  // Whether to sort the relocations when writing them out, to make
  // the dynamic linker more efficient.
  bool sort_relocs_;
};

// Output_data_reloc is used to manage a section containing relocs.
// SH_TYPE is either elfcpp::SHT_REL or elfcpp::SHT_RELA.  DYNAMIC
// indicates whether this is a dynamic relocation or a normal
// relocation.  Output_data_reloc_base is a base class.
// Output_data_reloc is the real class, which we specialize based on
// the reloc type.

template<int sh_type, bool dynamic, int size, bool big_endian>
class Output_data_reloc_base : public Output_data_reloc_generic
{
 public:
  typedef Output_reloc<sh_type, dynamic, size, big_endian> Output_reloc_type;
  typedef typename Output_reloc_type::Address Address;
  static const int reloc_size =
    Reloc_types<sh_type, size, big_endian>::reloc_size;

  // Construct the section.
  Output_data_reloc_base(bool sort_relocs)
    : Output_data_reloc_generic(size, sort_relocs)
  { }

 protected:
  // Write out the data.
  void
  do_write(Output_file*);

  // Generic implementation of do_write, allowing a customized
  // class for writing the output relocation (e.g., for MIPS-64).
  template<class Output_reloc_writer>
  void
  do_write_generic(Output_file* of)
  {
    const off_t off = this->offset();
    const off_t oview_size = this->data_size();
    unsigned char* const oview = of->get_output_view(off, oview_size);

    if (this->sort_relocs())
      {
	gold_assert(dynamic);
	std::sort(this->relocs_.begin(), this->relocs_.end(),
		  Sort_relocs_comparison());
      }

    unsigned char* pov = oview;
    for (typename Relocs::const_iterator p = this->relocs_.begin();
	 p != this->relocs_.end();
	 ++p)
      {
	Output_reloc_writer::write(p, pov);
	pov += reloc_size;
      }

    gold_assert(pov - oview == oview_size);

    of->write_output_view(off, oview_size, oview);

    // We no longer need the relocation entries.
    this->relocs_.clear();
  }

  // Set the entry size and the link.
  void
  do_adjust_output_section(Output_section* os);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  {
    mapfile->print_output_data(this,
			       (dynamic
				? _("** dynamic relocs")
				: _("** relocs")));
  }

  // Add a relocation entry.
  void
  add(Output_data* od, const Output_reloc_type& reloc)
  {
    this->relocs_.push_back(reloc);
    this->set_current_data_size(this->relocs_.size() * reloc_size);
    if (dynamic)
      od->add_dynamic_reloc();
    if (reloc.is_relative())
      this->bump_relative_reloc_count();
    Sized_relobj<size, big_endian>* relobj = reloc.get_relobj();
    if (relobj != NULL)
      relobj->add_dyn_reloc(this->relocs_.size() - 1);
  }

 private:
  typedef std::vector<Output_reloc_type> Relocs;

  // The class used to sort the relocations.
  struct Sort_relocs_comparison
  {
    bool
    operator()(const Output_reloc_type& r1, const Output_reloc_type& r2) const
    { return r1.sort_before(r2); }
  };

  // The relocations in this section.
  Relocs relocs_;
};

// The class which callers actually create.

template<int sh_type, bool dynamic, int size, bool big_endian>
class Output_data_reloc;

// The SHT_REL version of Output_data_reloc.

template<bool dynamic, int size, bool big_endian>
class Output_data_reloc<elfcpp::SHT_REL, dynamic, size, big_endian>
  : public Output_data_reloc_base<elfcpp::SHT_REL, dynamic, size, big_endian>
{
 private:
  typedef Output_data_reloc_base<elfcpp::SHT_REL, dynamic, size,
				 big_endian> Base;

 public:
  typedef typename Base::Output_reloc_type Output_reloc_type;
  typedef typename Output_reloc_type::Address Address;

  Output_data_reloc(bool sr)
    : Output_data_reloc_base<elfcpp::SHT_REL, dynamic, size, big_endian>(sr)
  { }

  // Add a reloc against a global symbol.

  void
  add_global(Symbol* gsym, unsigned int type, Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address,
				    false, false, false));
  }

  void
  add_global(Symbol* gsym, unsigned int type, Output_data* od,
	     Sized_relobj<size, big_endian>* relobj,
	     unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    false, false, false));
  }

  void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     uint64_t address, uint64_t addend)
  {
    gold_assert(addend == 0);
    this->add(od, Output_reloc_type(gsym, type, od,
				    convert_types<Address, uint64_t>(address),
				    false, false, false));
  }

  void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     Relobj* relobj, unsigned int shndx, uint64_t address,
		     uint64_t addend)
  {
    gold_assert(addend == 0);
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(gsym, type, sized_relobj, shndx,
				    convert_types<Address, uint64_t>(address),
				    false, false, false));
  }

  // Add a RELATIVE reloc against a global symbol.  The final relocation
  // will not reference the symbol.

  void
  add_global_relative(Symbol* gsym, unsigned int type, Output_data* od,
		      Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address, true, true,
				    false));
  }

  void
  add_global_relative(Symbol* gsym, unsigned int type, Output_data* od,
		      Sized_relobj<size, big_endian>* relobj,
		      unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    true, true, false));
  }

  // Add a global relocation which does not use a symbol for the relocation,
  // but which gets its addend from a symbol.

  void
  add_symbolless_global_addend(Symbol* gsym, unsigned int type,
			       Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address, false, true,
				    false));
  }

  void
  add_symbolless_global_addend(Symbol* gsym, unsigned int type,
			       Output_data* od,
			       Sized_relobj<size, big_endian>* relobj,
			       unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    false, true, false));
  }

  // Add a reloc against a local symbol.

  void
  add_local(Sized_relobj<size, big_endian>* relobj,
	    unsigned int local_sym_index, unsigned int type,
	    Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od,
				    address, false, false, false, false));
  }

  void
  add_local(Sized_relobj<size, big_endian>* relobj,
	    unsigned int local_sym_index, unsigned int type,
	    Output_data* od, unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, false, false, false, false));
  }

  void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, uint64_t address,
		    uint64_t addend)
  {
    gold_assert(addend == 0);
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian> *>(relobj);
    this->add(od, Output_reloc_type(sized_relobj, local_sym_index, type, od,
				    convert_types<Address, uint64_t>(address),
				    false, false, false, false));
  }

  void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, unsigned int shndx,
		    uint64_t address, uint64_t addend)
  {
    gold_assert(addend == 0);
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(sized_relobj, local_sym_index, type, shndx,
				    convert_types<Address, uint64_t>(address),
				    false, false, false, false));
  }

  // Add a RELATIVE reloc against a local symbol.

  void
  add_local_relative(Sized_relobj<size, big_endian>* relobj,
		     unsigned int local_sym_index, unsigned int type,
		     Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od,
				    address, true, true, false, false));
  }

  void
  add_local_relative(Sized_relobj<size, big_endian>* relobj,
		     unsigned int local_sym_index, unsigned int type,
		     Output_data* od, unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, true, true, false, false));
  }

  void
  add_local_relative(Sized_relobj<size, big_endian>* relobj,
		     unsigned int local_sym_index, unsigned int type,
		     Output_data* od, unsigned int shndx, Address address,
		     bool use_plt_offset)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, true, true, false,
				    use_plt_offset));
  }

  // Add a local relocation which does not use a symbol for the relocation,
  // but which gets its addend from a symbol.

  void
  add_symbolless_local_addend(Sized_relobj<size, big_endian>* relobj,
			      unsigned int local_sym_index, unsigned int type,
			      Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od,
				    address, false, true, false, false));
  }

  void
  add_symbolless_local_addend(Sized_relobj<size, big_endian>* relobj,
			      unsigned int local_sym_index, unsigned int type,
			      Output_data* od, unsigned int shndx,
			      Address address)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, false, true, false, false));
  }

  // Add a reloc against a local section symbol.  This will be
  // converted into a reloc against the STT_SECTION symbol of the
  // output section.

  void
  add_local_section(Sized_relobj<size, big_endian>* relobj,
		    unsigned int input_shndx, unsigned int type,
		    Output_data* od, Address address)
  {
    this->add(od, Output_reloc_type(relobj, input_shndx, type, od,
				    address, false, false, true, false));
  }

  void
  add_local_section(Sized_relobj<size, big_endian>* relobj,
		    unsigned int input_shndx, unsigned int type,
		    Output_data* od, unsigned int shndx, Address address)
  {
    this->add(od, Output_reloc_type(relobj, input_shndx, type, shndx,
				    address, false, false, true, false));
  }

  // A reloc against the STT_SECTION symbol of an output section.
  // OS is the Output_section that the relocation refers to; OD is
  // the Output_data object being relocated.

  void
  add_output_section(Output_section* os, unsigned int type,
		     Output_data* od, Address address)
  { this->add(od, Output_reloc_type(os, type, od, address, false)); }

  void
  add_output_section(Output_section* os, unsigned int type, Output_data* od,
		     Sized_relobj<size, big_endian>* relobj,
		     unsigned int shndx, Address address)
  { this->add(od, Output_reloc_type(os, type, relobj, shndx, address, false)); }

  void
  add_output_section_generic(Output_section* os, unsigned int type,
			     Output_data* od, uint64_t address,
			     uint64_t addend)
  {
    gold_assert(addend == 0);
    this->add(od, Output_reloc_type(os, type, od,
				    convert_types<Address, uint64_t>(address),
				    false));
  }

  void
  add_output_section_generic(Output_section* os, unsigned int type,
			     Output_data* od, Relobj* relobj,
			     unsigned int shndx, uint64_t address,
			     uint64_t addend)
  {
    gold_assert(addend == 0);
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(os, type, sized_relobj, shndx,
				    convert_types<Address, uint64_t>(address),
				    false));
  }

  // As above, but the reloc TYPE is relative

  void
  add_output_section_relative(Output_section* os, unsigned int type,
			      Output_data* od, Address address)
  { this->add(od, Output_reloc_type(os, type, od, address, true)); }

  void
  add_output_section_relative(Output_section* os, unsigned int type,
			      Output_data* od,
			      Sized_relobj<size, big_endian>* relobj,
			      unsigned int shndx, Address address)
  { this->add(od, Output_reloc_type(os, type, relobj, shndx, address, true)); }

  // Add an absolute relocation.

  void
  add_absolute(unsigned int type, Output_data* od, Address address)
  { this->add(od, Output_reloc_type(type, od, address, false)); }

  void
  add_absolute(unsigned int type, Output_data* od,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address)
  { this->add(od, Output_reloc_type(type, relobj, shndx, address, false)); }

  // Add a relative relocation

  void
  add_relative(unsigned int type, Output_data* od, Address address)
  { this->add(od, Output_reloc_type(type, od, address, true)); }

  void
  add_relative(unsigned int type, Output_data* od,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address)
  { this->add(od, Output_reloc_type(type, relobj, shndx, address, true)); }

  // Add a target specific relocation.  A target which calls this must
  // define the reloc_symbol_index and reloc_addend virtual functions.

  void
  add_target_specific(unsigned int type, void* arg, Output_data* od,
		      Address address)
  { this->add(od, Output_reloc_type(type, arg, od, address)); }

  void
  add_target_specific(unsigned int type, void* arg, Output_data* od,
		      Sized_relobj<size, big_endian>* relobj,
		      unsigned int shndx, Address address)
  { this->add(od, Output_reloc_type(type, arg, relobj, shndx, address)); }
};

// The SHT_RELA version of Output_data_reloc.

template<bool dynamic, int size, bool big_endian>
class Output_data_reloc<elfcpp::SHT_RELA, dynamic, size, big_endian>
  : public Output_data_reloc_base<elfcpp::SHT_RELA, dynamic, size, big_endian>
{
 private:
  typedef Output_data_reloc_base<elfcpp::SHT_RELA, dynamic, size,
				 big_endian> Base;

 public:
  typedef typename Base::Output_reloc_type Output_reloc_type;
  typedef typename Output_reloc_type::Address Address;
  typedef typename Output_reloc_type::Addend Addend;

  Output_data_reloc(bool sr)
    : Output_data_reloc_base<elfcpp::SHT_RELA, dynamic, size, big_endian>(sr)
  { }

  // Add a reloc against a global symbol.

  void
  add_global(Symbol* gsym, unsigned int type, Output_data* od,
	     Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address, addend,
				    false, false, false));
  }

  void
  add_global(Symbol* gsym, unsigned int type, Output_data* od,
	     Sized_relobj<size, big_endian>* relobj,
	     unsigned int shndx, Address address,
	     Addend addend)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    addend, false, false, false));
  }

  void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     uint64_t address, uint64_t addend)
  {
    this->add(od, Output_reloc_type(gsym, type, od,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false, false, false));
  }

  void
  add_global_generic(Symbol* gsym, unsigned int type, Output_data* od,
		     Relobj* relobj, unsigned int shndx, uint64_t address,
		     uint64_t addend)
  {
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(gsym, type, sized_relobj, shndx,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false, false, false));
  }

  // Add a RELATIVE reloc against a global symbol.  The final output
  // relocation will not reference the symbol, but we must keep the symbol
  // information long enough to set the addend of the relocation correctly
  // when it is written.

  void
  add_global_relative(Symbol* gsym, unsigned int type, Output_data* od,
		      Address address, Addend addend, bool use_plt_offset)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address, addend, true,
				    true, use_plt_offset));
  }

  void
  add_global_relative(Symbol* gsym, unsigned int type, Output_data* od,
		      Sized_relobj<size, big_endian>* relobj,
		      unsigned int shndx, Address address, Addend addend,
		      bool use_plt_offset)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    addend, true, true, use_plt_offset));
  }

  // Add a global relocation which does not use a symbol for the relocation,
  // but which gets its addend from a symbol.

  void
  add_symbolless_global_addend(Symbol* gsym, unsigned int type, Output_data* od,
			       Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(gsym, type, od, address, addend,
				    false, true, false));
  }

  void
  add_symbolless_global_addend(Symbol* gsym, unsigned int type,
			       Output_data* od,
			       Sized_relobj<size, big_endian>* relobj,
			       unsigned int shndx, Address address,
			       Addend addend)
  {
    this->add(od, Output_reloc_type(gsym, type, relobj, shndx, address,
				    addend, false, true, false));
  }

  // Add a reloc against a local symbol.

  void
  add_local(Sized_relobj<size, big_endian>* relobj,
	    unsigned int local_sym_index, unsigned int type,
	    Output_data* od, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od, address,
				    addend, false, false, false, false));
  }

  void
  add_local(Sized_relobj<size, big_endian>* relobj,
	    unsigned int local_sym_index, unsigned int type,
	    Output_data* od, unsigned int shndx, Address address,
	    Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, addend, false, false, false,
				    false));
  }

  void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, uint64_t address,
		    uint64_t addend)
  {
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian> *>(relobj);
    this->add(od, Output_reloc_type(sized_relobj, local_sym_index, type, od,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false, false, false, false));
  }

  void
  add_local_generic(Relobj* relobj, unsigned int local_sym_index,
		    unsigned int type, Output_data* od, unsigned int shndx,
		    uint64_t address, uint64_t addend)
  {
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(sized_relobj, local_sym_index, type, shndx,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false, false, false, false));
  }

  // Add a RELATIVE reloc against a local symbol.

  void
  add_local_relative(Sized_relobj<size, big_endian>* relobj,
		     unsigned int local_sym_index, unsigned int type,
		     Output_data* od, Address address, Addend addend,
		     bool use_plt_offset)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od, address,
				    addend, true, true, false,
				    use_plt_offset));
  }

  void
  add_local_relative(Sized_relobj<size, big_endian>* relobj,
		     unsigned int local_sym_index, unsigned int type,
		     Output_data* od, unsigned int shndx, Address address,
		     Addend addend, bool use_plt_offset)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, addend, true, true, false,
				    use_plt_offset));
  }

  // Add a local relocation which does not use a symbol for the relocation,
  // but which gets it's addend from a symbol.

  void
  add_symbolless_local_addend(Sized_relobj<size, big_endian>* relobj,
			      unsigned int local_sym_index, unsigned int type,
			      Output_data* od, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, od, address,
				    addend, false, true, false, false));
  }

  void
  add_symbolless_local_addend(Sized_relobj<size, big_endian>* relobj,
			      unsigned int local_sym_index, unsigned int type,
			      Output_data* od, unsigned int shndx,
			      Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, local_sym_index, type, shndx,
				    address, addend, false, true, false,
				    false));
  }

  // Add a reloc against a local section symbol.  This will be
  // converted into a reloc against the STT_SECTION symbol of the
  // output section.

  void
  add_local_section(Sized_relobj<size, big_endian>* relobj,
		    unsigned int input_shndx, unsigned int type,
		    Output_data* od, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, input_shndx, type, od, address,
				    addend, false, false, true, false));
  }

  void
  add_local_section(Sized_relobj<size, big_endian>* relobj,
		    unsigned int input_shndx, unsigned int type,
		    Output_data* od, unsigned int shndx, Address address,
		    Addend addend)
  {
    this->add(od, Output_reloc_type(relobj, input_shndx, type, shndx,
				    address, addend, false, false, true,
				    false));
  }

  // A reloc against the STT_SECTION symbol of an output section.

  void
  add_output_section(Output_section* os, unsigned int type, Output_data* od,
		     Address address, Addend addend)
  { this->add(od, Output_reloc_type(os, type, od, address, addend, false)); }

  void
  add_output_section(Output_section* os, unsigned int type, Output_data* od,
		     Sized_relobj<size, big_endian>* relobj,
		     unsigned int shndx, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(os, type, relobj, shndx, address,
				    addend, false));
  }

  void
  add_output_section_generic(Output_section* os, unsigned int type,
			     Output_data* od, uint64_t address,
			     uint64_t addend)
  {
    this->add(od, Output_reloc_type(os, type, od,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false));
  }

  void
  add_output_section_generic(Output_section* os, unsigned int type,
			     Output_data* od, Relobj* relobj,
			     unsigned int shndx, uint64_t address,
			     uint64_t addend)
  {
    Sized_relobj<size, big_endian>* sized_relobj =
      static_cast<Sized_relobj<size, big_endian>*>(relobj);
    this->add(od, Output_reloc_type(os, type, sized_relobj, shndx,
				    convert_types<Address, uint64_t>(address),
				    convert_types<Addend, uint64_t>(addend),
				    false));
  }

  // As above, but the reloc TYPE is relative

  void
  add_output_section_relative(Output_section* os, unsigned int type,
			      Output_data* od, Address address, Addend addend)
  { this->add(od, Output_reloc_type(os, type, od, address, addend, true)); }

  void
  add_output_section_relative(Output_section* os, unsigned int type,
			      Output_data* od,
			      Sized_relobj<size, big_endian>* relobj,
			      unsigned int shndx, Address address,
			      Addend addend)
  {
    this->add(od, Output_reloc_type(os, type, relobj, shndx,
				    address, addend, true));
  }

  // Add an absolute relocation.

  void
  add_absolute(unsigned int type, Output_data* od, Address address,
	       Addend addend)
  { this->add(od, Output_reloc_type(type, od, address, addend, false)); }

  void
  add_absolute(unsigned int type, Output_data* od,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(type, relobj, shndx, address, addend,
				    false));
  }

  // Add a relative relocation

  void
  add_relative(unsigned int type, Output_data* od, Address address,
	       Addend addend)
  { this->add(od, Output_reloc_type(type, od, address, addend, true)); }

  void
  add_relative(unsigned int type, Output_data* od,
	       Sized_relobj<size, big_endian>* relobj,
	       unsigned int shndx, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(type, relobj, shndx, address, addend,
				    true));
  }

  // Add a target specific relocation.  A target which calls this must
  // define the reloc_symbol_index and reloc_addend virtual functions.

  void
  add_target_specific(unsigned int type, void* arg, Output_data* od,
		      Address address, Addend addend)
  { this->add(od, Output_reloc_type(type, arg, od, address, addend)); }

  void
  add_target_specific(unsigned int type, void* arg, Output_data* od,
		      Sized_relobj<size, big_endian>* relobj,
		      unsigned int shndx, Address address, Addend addend)
  {
    this->add(od, Output_reloc_type(type, arg, relobj, shndx, address,
				    addend));
  }
};

// Output_relocatable_relocs represents a relocation section in a
// relocatable link.  The actual data is written out in the target
// hook relocate_relocs.  This just saves space for it.

template<int sh_type, int size, bool big_endian>
class Output_relocatable_relocs : public Output_section_data
{
 public:
  Output_relocatable_relocs(Relocatable_relocs* rr)
    : Output_section_data(Output_data::default_alignment_for_size(size)),
      rr_(rr)
  { }

  void
  set_final_data_size();

  // Write out the data.  There is nothing to do here.
  void
  do_write(Output_file*)
  { }

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** relocs")); }

 private:
  // The relocs associated with this input section.
  Relocatable_relocs* rr_;
};

// Handle a GROUP section.

template<int size, bool big_endian>
class Output_data_group : public Output_section_data
{
 public:
  // The constructor clears *INPUT_SHNDXES.
  Output_data_group(Sized_relobj_file<size, big_endian>* relobj,
		    section_size_type entry_count,
		    elfcpp::Elf_Word flags,
		    std::vector<unsigned int>* input_shndxes);

  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** group")); }

  // Set final data size.
  void
  set_final_data_size()
  { this->set_data_size((this->input_shndxes_.size() + 1) * 4); }

 private:
  // The input object.
  Sized_relobj_file<size, big_endian>* relobj_;
  // The group flag word.
  elfcpp::Elf_Word flags_;
  // The section indexes of the input sections in this group.
  std::vector<unsigned int> input_shndxes_;
};

// Output_data_got is used to manage a GOT.  Each entry in the GOT is
// for one symbol--either a global symbol or a local symbol in an
// object.  The target specific code adds entries to the GOT as
// needed.  The GOT_SIZE template parameter is the size in bits of a
// GOT entry, typically 32 or 64.

class Output_data_got_base : public Output_section_data_build
{
 public:
  Output_data_got_base(uint64_t align)
    : Output_section_data_build(align)
  { }

  Output_data_got_base(off_t data_size, uint64_t align)
    : Output_section_data_build(data_size, align)
  { }

  // Reserve the slot at index I in the GOT.
  void
  reserve_slot(unsigned int i)
  { this->do_reserve_slot(i); }

 protected:
  // Reserve the slot at index I in the GOT.
  virtual void
  do_reserve_slot(unsigned int i) = 0;
};

template<int got_size, bool big_endian>
class Output_data_got : public Output_data_got_base
{
 public:
  typedef typename elfcpp::Elf_types<got_size>::Elf_Addr Valtype;

  Output_data_got()
    : Output_data_got_base(Output_data::default_alignment_for_size(got_size)),
      entries_(), free_list_()
  { }

  Output_data_got(off_t data_size)
    : Output_data_got_base(data_size,
			   Output_data::default_alignment_for_size(got_size)),
      entries_(), free_list_()
  {
    // For an incremental update, we have an existing GOT section.
    // Initialize the list of entries and the free list.
    this->entries_.resize(data_size / (got_size / 8));
    this->free_list_.init(data_size, false);
  }

  // Add an entry for a global symbol GSYM plus ADDEND to the GOT.
  // Return true if this is a new GOT entry, false if the symbol plus
  // addend was already in the GOT.
  bool
  add_global(Symbol* gsym, unsigned int got_type, uint64_t addend = 0);

  // Like add_global, but use the PLT offset of the global symbol if
  // it has one.
  bool
  add_global_plt(Symbol* gsym, unsigned int got_type, uint64_t addend = 0);

  // Like add_global, but for a TLS symbol where the value will be
  // offset using Target::tls_offset_for_global.
  bool
  add_global_tls(Symbol* gsym, unsigned int got_type, uint64_t addend = 0)
  { return this->add_global_plt(gsym, got_type, addend); }

  // Add an entry for a global symbol GSYM plus ADDEND to the GOT, and
  // add a dynamic relocation of type R_TYPE for the GOT entry.
  void
  add_global_with_rel(Symbol* gsym, unsigned int got_type,
		      Output_data_reloc_generic* rel_dyn, unsigned int r_type,
		      uint64_t addend = 0);

  // Add a pair of entries for a global symbol GSYM plus ADDEND to the
  // GOT, and add dynamic relocations of type R_TYPE_1 and R_TYPE_2,
  // respectively.
  void
  add_global_pair_with_rel(Symbol* gsym, unsigned int got_type,
			   Output_data_reloc_generic* rel_dyn,
			   unsigned int r_type_1, unsigned int r_type_2,
			   uint64_t addend = 0);

  // Add an entry for a local symbol plus ADDEND to the GOT.  This returns
  // true if this is a new GOT entry, false if the symbol already has a GOT
  // entry.
  bool
  add_local(Relobj* object, unsigned int sym_index, unsigned int got_type,
	    uint64_t addend = 0);

  // Like add_local, but use the PLT offset of the local symbol if it
  // has one.
  bool
  add_local_plt(Relobj* object, unsigned int sym_index, unsigned int got_type,
		uint64_t addend = 0);

  // Like add_local, but for a TLS symbol where the value will be
  // offset using Target::tls_offset_for_local.
  bool
  add_local_tls(Relobj* object, unsigned int sym_index, unsigned int got_type,
		uint64_t addend = 0)
  { return this->add_local_plt(object, sym_index, got_type, addend); }

  // Add an entry for a local symbol plus ADDEND to the GOT, and add a dynamic
  // relocation of type R_TYPE for the GOT entry.
  void
  add_local_with_rel(Relobj* object, unsigned int sym_index,
		     unsigned int got_type, Output_data_reloc_generic* rel_dyn,
		     unsigned int r_type, uint64_t addend = 0);

  // Add a pair of entries for a local symbol plus ADDEND to the GOT, and add
  // a dynamic relocation of type R_TYPE using the section symbol of
  // the output section to which input section SHNDX maps, on the first.
  // The first got entry will have a value of zero, the second the
  // value of the local symbol.
  void
  add_local_pair_with_rel(Relobj* object, unsigned int sym_index,
			  unsigned int shndx, unsigned int got_type,
			  Output_data_reloc_generic* rel_dyn,
			  unsigned int r_type, uint64_t addend = 0);

  // Add a pair of entries for a local symbol plus ADDEND to the GOT,
  // and add a dynamic relocation of type R_TYPE using STN_UNDEF on
  // the first.  The first got entry will have a value of zero, the
  // second the value of the local symbol plus ADDEND offset by
  // Target::tls_offset_for_local.
  void
  add_local_tls_pair(Relobj* object, unsigned int sym_index,
		     unsigned int got_type,
		     Output_data_reloc_generic* rel_dyn,
		     unsigned int r_type, uint64_t addend = 0);

  // Add a constant to the GOT.  This returns the offset of the new
  // entry from the start of the GOT.
  unsigned int
  add_constant(Valtype constant)
  { return this->add_got_entry(Got_entry(constant)); }

  // Add a pair of constants to the GOT.  This returns the offset of
  // the new entry from the start of the GOT.
  unsigned int
  add_constant_pair(Valtype c1, Valtype c2)
  { return this->add_got_entry_pair(Got_entry(c1), Got_entry(c2)); }

  // Replace GOT entry I with a new constant.
  void
  replace_constant(unsigned int i, Valtype constant)
  {
    this->replace_got_entry(i, Got_entry(constant));
  }

  // Reserve a slot in the GOT for a local symbol plus ADDEND.
  void
  reserve_local(unsigned int i, Relobj* object, unsigned int sym_index,
		unsigned int got_type, uint64_t addend = 0);

  // Reserve a slot in the GOT for a global symbol plus ADDEND.
  void
  reserve_global(unsigned int i, Symbol* gsym, unsigned int got_type,
		 uint64_t addend = 0);

 protected:
  // Write out the GOT table.
  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** GOT")); }

  // Reserve the slot at index I in the GOT.
  virtual void
  do_reserve_slot(unsigned int i)
  { this->free_list_.remove(i * got_size / 8, (i + 1) * got_size / 8); }

  // Return the number of words in the GOT.
  unsigned int
  num_entries () const
  { return this->entries_.size(); }

  // Return the offset into the GOT of GOT entry I.
  unsigned int
  got_offset(unsigned int i) const
  { return i * (got_size / 8); }

 private:
  // This POD class holds a single GOT entry.
  class Got_entry
  {
   public:
    // Create a zero entry.
    Got_entry()
      : local_sym_index_(RESERVED_CODE), use_plt_or_tls_offset_(false),
	addend_(0)
    { this->u_.constant = 0; }

    // Create a global symbol entry.
    Got_entry(Symbol* gsym, bool use_plt_or_tls_offset, uint64_t addend)
      : local_sym_index_(GSYM_CODE),
	use_plt_or_tls_offset_(use_plt_or_tls_offset), addend_(addend)
    { this->u_.gsym = gsym; }

    // Create a local symbol entry.
    Got_entry(Relobj* object, unsigned int local_sym_index,
	      bool use_plt_or_tls_offset, uint64_t addend)
      : local_sym_index_(local_sym_index),
	use_plt_or_tls_offset_(use_plt_or_tls_offset), addend_(addend)
    {
      gold_assert(local_sym_index != GSYM_CODE
		  && local_sym_index != CONSTANT_CODE
		  && local_sym_index != RESERVED_CODE
		  && local_sym_index == this->local_sym_index_);
      this->u_.object = object;
    }

    // Create a constant entry.  The constant is a host value--it will
    // be swapped, if necessary, when it is written out.
    explicit Got_entry(Valtype constant)
      : local_sym_index_(CONSTANT_CODE), use_plt_or_tls_offset_(false)
    { this->u_.constant = constant; }

    // Write the GOT entry to an output view.
    void
    write(Output_data_got_base* got, unsigned int got_indx,
	  unsigned char* pov) const;

   private:
    enum
    {
      GSYM_CODE = 0x7fffffff,
      CONSTANT_CODE = 0x7ffffffe,
      RESERVED_CODE = 0x7ffffffd
    };

    union
    {
      // For a local symbol, the object.
      Relobj* object;
      // For a global symbol, the symbol.
      Symbol* gsym;
      // For a constant, the constant.
      Valtype constant;
    } u_;
    // For a local symbol, the local symbol index.  This is GSYM_CODE
    // for a global symbol, or CONSTANT_CODE for a constant.
    unsigned int local_sym_index_ : 31;
    // Whether to use the PLT offset of the symbol if it has one.
    // For TLS symbols, whether to offset the symbol value.
    bool use_plt_or_tls_offset_ : 1;
    // The addend.
    uint64_t addend_;
  };

  typedef std::vector<Got_entry> Got_entries;

  // Create a new GOT entry and return its offset.
  unsigned int
  add_got_entry(Got_entry got_entry);

  // Create a pair of new GOT entries and return the offset of the first.
  unsigned int
  add_got_entry_pair(Got_entry got_entry_1, Got_entry got_entry_2);

  // Replace GOT entry I with a new value.
  void
  replace_got_entry(unsigned int i, Got_entry got_entry);

  // Return the offset into the GOT of the last entry added.
  unsigned int
  last_got_offset() const
  { return this->got_offset(this->num_entries() - 1); }

  // Set the size of the section.
  void
  set_got_size()
  { this->set_current_data_size(this->got_offset(this->num_entries())); }

  // The list of GOT entries.
  Got_entries entries_;

  // List of available regions within the section, for incremental
  // update links.
  Free_list free_list_;
};

// Output_data_dynamic is used to hold the data in SHT_DYNAMIC
// section.

class Output_data_dynamic : public Output_section_data
{
 public:
  Output_data_dynamic(Stringpool* pool)
    : Output_section_data(Output_data::default_alignment()),
      entries_(), pool_(pool)
  { }

  // Add a new dynamic entry with a fixed numeric value.
  void
  add_constant(elfcpp::DT tag, unsigned int val)
  { this->add_entry(Dynamic_entry(tag, val)); }

  // Add a new dynamic entry with the address of output data.
  void
  add_section_address(elfcpp::DT tag, const Output_data* od)
  { this->add_entry(Dynamic_entry(tag, od, false)); }

  // Add a new dynamic entry with the address of output data
  // plus a constant offset.
  void
  add_section_plus_offset(elfcpp::DT tag, const Output_data* od,
			  unsigned int offset)
  { this->add_entry(Dynamic_entry(tag, od, offset)); }

  // Add a new dynamic entry with the size of output data.
  void
  add_section_size(elfcpp::DT tag, const Output_data* od)
  { this->add_entry(Dynamic_entry(tag, od, true)); }

  // Add a new dynamic entry with the total size of two output datas.
  void
  add_section_size(elfcpp::DT tag, const Output_data* od,
		   const Output_data* od2)
  { this->add_entry(Dynamic_entry(tag, od, od2)); }

  // Add a new dynamic entry with the address of a symbol.
  void
  add_symbol(elfcpp::DT tag, const Symbol* sym)
  { this->add_entry(Dynamic_entry(tag, sym)); }

  // Add a new dynamic entry with a string.
  void
  add_string(elfcpp::DT tag, const char* str)
  { this->add_entry(Dynamic_entry(tag, this->pool_->add(str, true, NULL))); }

  void
  add_string(elfcpp::DT tag, const std::string& str)
  { this->add_string(tag, str.c_str()); }

  // Add a new dynamic entry with custom value.
  void
  add_custom(elfcpp::DT tag)
  { this->add_entry(Dynamic_entry(tag)); }

  // Get a dynamic entry offset.
  unsigned int
  get_entry_offset(elfcpp::DT tag) const;

 protected:
  // Adjust the output section to set the entry size.
  void
  do_adjust_output_section(Output_section*);

  // Set the final data size.
  void
  set_final_data_size();

  // Write out the dynamic entries.
  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** dynamic")); }

 private:
  // This POD class holds a single dynamic entry.
  class Dynamic_entry
  {
   public:
    // Create an entry with a fixed numeric value.
    Dynamic_entry(elfcpp::DT tag, unsigned int val)
      : tag_(tag), offset_(DYNAMIC_NUMBER)
    { this->u_.val = val; }

    // Create an entry with the size or address of a section.
    Dynamic_entry(elfcpp::DT tag, const Output_data* od, bool section_size)
      : tag_(tag),
	offset_(section_size
		? DYNAMIC_SECTION_SIZE
		: DYNAMIC_SECTION_ADDRESS)
    {
      this->u_.od = od;
      this->od2 = NULL;
    }

    // Create an entry with the size of two sections.
    Dynamic_entry(elfcpp::DT tag, const Output_data* od, const Output_data* od2)
      : tag_(tag),
	offset_(DYNAMIC_SECTION_SIZE)
    {
      this->u_.od = od;
      this->od2 = od2;
    }

    // Create an entry with the address of a section plus a constant offset.
    Dynamic_entry(elfcpp::DT tag, const Output_data* od, unsigned int offset)
      : tag_(tag),
	offset_(offset)
    { this->u_.od = od; }

    // Create an entry with the address of a symbol.
    Dynamic_entry(elfcpp::DT tag, const Symbol* sym)
      : tag_(tag), offset_(DYNAMIC_SYMBOL)
    { this->u_.sym = sym; }

    // Create an entry with a string.
    Dynamic_entry(elfcpp::DT tag, const char* str)
      : tag_(tag), offset_(DYNAMIC_STRING)
    { this->u_.str = str; }

    // Create an entry with a custom value.
    Dynamic_entry(elfcpp::DT tag)
      : tag_(tag), offset_(DYNAMIC_CUSTOM)
    { }

    // Return the tag of this entry.
    elfcpp::DT
    tag() const
    { return this->tag_; }

    // Write the dynamic entry to an output view.
    template<int size, bool big_endian>
    void
    write(unsigned char* pov, const Stringpool*) const;

   private:
    // Classification is encoded in the OFFSET field.
    enum Classification
    {
      // Section address.
      DYNAMIC_SECTION_ADDRESS = 0,
      // Number.
      DYNAMIC_NUMBER = -1U,
      // Section size.
      DYNAMIC_SECTION_SIZE = -2U,
      // Symbol address.
      DYNAMIC_SYMBOL = -3U,
      // String.
      DYNAMIC_STRING = -4U,
      // Custom value.
      DYNAMIC_CUSTOM = -5U
      // Any other value indicates a section address plus OFFSET.
    };

    union
    {
      // For DYNAMIC_NUMBER.
      unsigned int val;
      // For DYNAMIC_SECTION_SIZE and section address plus OFFSET.
      const Output_data* od;
      // For DYNAMIC_SYMBOL.
      const Symbol* sym;
      // For DYNAMIC_STRING.
      const char* str;
    } u_;
    // For DYNAMIC_SYMBOL with two sections.
    const Output_data* od2;
    // The dynamic tag.
    elfcpp::DT tag_;
    // The type of entry (Classification) or offset within a section.
    unsigned int offset_;
  };

  // Add an entry to the list.
  void
  add_entry(const Dynamic_entry& entry)
  { this->entries_.push_back(entry); }

  // Sized version of write function.
  template<int size, bool big_endian>
  void
  sized_write(Output_file* of);

  // The type of the list of entries.
  typedef std::vector<Dynamic_entry> Dynamic_entries;

  // The entries.
  Dynamic_entries entries_;
  // The pool used for strings.
  Stringpool* pool_;
};

// Output_symtab_xindex is used to handle SHT_SYMTAB_SHNDX sections,
// which may be required if the object file has more than
// SHN_LORESERVE sections.

class Output_symtab_xindex : public Output_section_data
{
 public:
  Output_symtab_xindex(size_t symcount)
    : Output_section_data(symcount * 4, 4, true),
      entries_()
  { }

  // Add an entry: symbol number SYMNDX has section SHNDX.
  void
  add(unsigned int symndx, unsigned int shndx)
  { this->entries_.push_back(std::make_pair(symndx, shndx)); }

 protected:
  void
  do_write(Output_file*);

  // Write to a map file.
  void
  do_print_to_mapfile(Mapfile* mapfile) const
  { mapfile->print_output_data(this, _("** symtab xindex")); }

 private:
  template<bool big_endian>
  void
  endian_do_write(unsigned char*);

  // It is likely that most symbols will not require entries.  Rather
  // than keep a vector for all symbols, we keep pairs of symbol index
  // and section index.
  typedef std::vector<std::pair<unsigned int, unsigned int> > Xindex_entries;

  // The entries we need.
  Xindex_entries entries_;
};

// A relaxed input section.
class Output_relaxed_input_section : public Output_section_data_build
{
 public:
  // We would like to call relobj->section_addralign(shndx) to get the
  // alignment but we do not want the constructor to fail.  So callers
  // are repsonsible for ensuring that.
  Output_relaxed_input_section(Relobj* relobj, unsigned int shndx,
			       uint64_t addralign)
    : Output_section_data_build(addralign), relobj_(relobj), shndx_(shndx)
  { }

  // Return the Relobj of this relaxed input section.
  Relobj*
  relobj() const
  { return this->relobj_; }

  // Return the section index of this relaxed input section.
  unsigned int
  shndx() const
  { return this->shndx_; }

 protected:
  void
  set_relobj(Relobj* relobj)
  { this->relobj_ = relobj; }

  void
  set_shndx(unsigned int shndx)
  { this->shndx_ = shndx; }

 private:
  Relobj* relobj_;
  unsigned int shndx_;
};

// This class describes properties of merge data sections.  It is used
// as a key type for maps.
class Merge_section_properties
{
 public:
  Merge_section_properties(bool is_string, uint64_t entsize,
			     uint64_t addralign)
    : is_string_(is_string), entsize_(entsize), addralign_(addralign)
  { }

  // Whether this equals to another Merge_section_properties MSP.
  bool
  eq(const Merge_section_properties& msp) const
  {
    return ((this->is_string_ == msp.is_string_)
	    && (this->entsize_ == msp.entsize_)
	    && (this->addralign_ == msp.addralign_));
  }

  // Compute a hash value for this using 64-bit FNV-1a hash.
  size_t
  hash_value() const
  {
    uint64_t h = 14695981039346656037ULL;	// FNV offset basis.
    uint64_t prime = 1099511628211ULL;
    h = (h ^ static_cast<uint64_t>(this->is_string_)) * prime;
    h = (h ^ static_cast<uint64_t>(this->entsize_)) * prime;
    h = (h ^ static_cast<uint64_t>(this->addralign_)) * prime;
    return h;
  }

  // Functors for associative containers.
  struct equal_to
  {
    bool
    operator()(const Merge_section_properties& msp1,
	       const Merge_section_properties& msp2) const
    { return msp1.eq(msp2); }
  };

  struct hash
  {
    size_t
    operator()(const Merge_section_properties& msp) const
    { return msp.hash_value(); }
  };

 private:
  // Whether this merge data section is for strings.
  bool is_string_;
  // Entsize of this merge data section.
  uint64_t entsize_;
  // Address alignment.
  uint64_t addralign_;
};

// This class is used to speed up look up of special input sections in an
// Output_section.

class Output_section_lookup_maps
{
 public:
  Output_section_lookup_maps()
    : is_valid_(true), merge_sections_by_properties_(),
      relaxed_input_sections_by_id_()
  { }

  // Whether the maps are valid.
  bool
  is_valid() const
  { return this->is_valid_; }

  // Invalidate the maps.
  void
  invalidate()
  { this->is_valid_ = false; }

  // Clear the maps.
  void
  clear()
  {
    this->merge_sections_by_properties_.clear();
    this->relaxed_input_sections_by_id_.clear();
    // A cleared map is valid.
    this->is_valid_ = true;
  }

  // Find a merge section by merge section properties.  Return NULL if none
  // is found.
  Output_merge_base*
  find_merge_section(const Merge_section_properties& msp) const
  {
    gold_assert(this->is_valid_);
    Merge_sections_by_properties::const_iterator p =
      this->merge_sections_by_properties_.find(msp);
    return p != this->merge_sections_by_properties_.end() ? p->second : NULL;
  }

  // Add a merge section pointed by POMB with properties MSP.
  void
  add_merge_section(const Merge_section_properties& msp,
		    Output_merge_base* pomb)
  {
    std::pair<Merge_section_properties, Output_merge_base*> value(msp, pomb);
    std::pair<Merge_sections_by_properties::iterator, bool> result =
      this->merge_sections_by_properties_.insert(value);
    gold_assert(result.second);
  }

  // Find a relaxed input section of OBJECT with index SHNDX.
  Output_relaxed_input_section*
  find_relaxed_input_section(const Relobj* object, unsigned int shndx) const
  {
    gold_assert(this->is_valid_);
    Relaxed_input_sections_by_id::const_iterator p =
      this->relaxed_input_sections_by_id_.find(Const_section_id(object, shndx));
    return p != this->relaxed_input_sections_by_id_.end() ? p->second : NULL;
  }

  // Add a relaxed input section pointed by POMB and whose original input
  // section is in OBJECT with index SHNDX.
  void
  add_relaxed_input_section(const Relobj* relobj, unsigned int shndx,
			    Output_relaxed_input_section* poris)
  {
    Const_section_id csid(relobj, shndx);
    std::pair<Const_section_id, Output_relaxed_input_section*>
      value(csid, poris);
    std::pair<Relaxed_input_sections_by_id::iterator, bool> result =
      this->relaxed_input_sections_by_id_.insert(value);
    gold_assert(result.second);
  }

 private:
  typedef Unordered_map<Merge_section_properties, Output_merge_base*,
			Merge_section_properties::hash,
			Merge_section_properties::equal_to>
    Merge_sections_by_properties;

  typedef Unordered_map<Const_section_id, Output_relaxed_input_section*,
			Const_section_id_hash>
    Relaxed_input_sections_by_id;

  // Whether this is valid
  bool is_valid_;
  // Merge sections by merge section properties.
  Merge_sections_by_properties merge_sections_by_properties_;
  // Relaxed sections by section IDs.
  Relaxed_input_sections_by_id relaxed_input_sections_by_id_;
};

// This abstract base class defines the interface for the
// types of methods used to fill free space left in an output
// section during an incremental link.  These methods are used
// to insert dummy compilation units into debug info so that
// debug info consumers can scan the debug info serially.

class Output_fill
{
 public:
  Output_fill()
    : is_big_endian_(parameters->target().is_big_endian())
  { }

  virtual
  ~Output_fill()
  { }

  // Return the smallest size chunk of free space that can be
  // filled with a dummy compilation unit.
  size_t
  minimum_hole_size() const
  { return this->do_minimum_hole_size(); }

  // Write a fill pattern of length LEN at offset OFF in the file.
  void
  write(Output_file* of, off_t off, size_t len) const
  { this->do_write(of, off, len); }

 protected:
  virtual size_t
  do_minimum_hole_size() const = 0;

  virtual void
  do_write(Output_file* of, off_t off, size_t len) const = 0;

  bool
  is_big_endian() const
  { return this->is_big_endian_; }

 private:
  bool is_big_endian_;
};

// Fill method that introduces a dummy compilation unit in
// a .debug_info or .debug_types section.

class Output_fill_debug_info : public Output_fill
{
 public:
  Output_fill_debug_info(bool is_debug_types)
    : is_debug_types_(is_debug_types)
  { }

 protected:
  virtual size_t
  do_minimum_hole_size() const;

  virtual void
  do_write(Output_file* of, off_t off, size_t len) const;

 private:
  // Version of the header.
  static const int version = 4;
  // True if this is a .debug_types section.
  bool is_debug_types_;
};

// Fill method that introduces a dummy compilation unit in
// a .debug_line section.

class Output_fill_debug_line : public Output_fill
{
 public:
  Output_fill_debug_line()
  { }

 protected:
  virtual size_t
  do_minimum_hole_size() const;

  virtual void
  do_write(Output_file* of, off_t off, size_t len) const;

 private:
  // Version of the header.  We write a DWARF-3 header because it's smaller
  // and many tools have not yet been updated to understand the DWARF-4 header.
  static const int version = 3;
  // Length of the portion of the header that follows the header_length
  // field.  This includes the following fields:
  // minimum_instruction_length, default_is_stmt, line_base, line_range,
  // opcode_base, standard_opcode_lengths[], include_directories, filenames.
  // The standard_opcode_lengths array is 12 bytes long, and the
  // include_directories and filenames fields each contain only a single
  // null byte.
  static const size_t header_length = 19;
};

// An output section.  We don't expect to have too many output
// sections, so we don't bother to do a template on the size.

class Output_section : public Output_data
{
 public:
  // Create an output section, giving the name, type, and flags.
  Output_section(const char* name, elfcpp::Elf_Word, elfcpp::Elf_Xword);
  virtual ~Output_section();

  // Add a new input section SHNDX, named NAME, with header SHDR, from
  // object OBJECT.  RELOC_SHNDX is the index of a relocation section
  // which applies to this section, or 0 if none, or -1 if more than
  // one.  HAVE_SECTIONS_SCRIPT is true if we have a SECTIONS clause
  // in a linker script; in that case we need to keep track of input
  // sections associated with an output section.  Return the offset
  // within the output section.
  template<int size, bool big_endian>
  off_t
  add_input_section(Layout* layout, Sized_relobj_file<size, big_endian>* object,
		    unsigned int shndx, const char* name,
		    const elfcpp::Shdr<size, big_endian>& shdr,
		    unsigned int reloc_shndx, bool have_sections_script);

  // Add generated data POSD to this output section.
  void
  add_output_section_data(Output_section_data* posd);

  // Add a relaxed input section PORIS called NAME to this output section
  // with LAYOUT.
  void
  add_relaxed_input_section(Layout* layout,
			    Output_relaxed_input_section* poris,
			    const std::string& name);

  // Return the section name.
  const char*
  name() const
  { return this->name_; }

  // Return the section type.
  elfcpp::Elf_Word
  type() const
  { return this->type_; }

  // Return the section flags.
  elfcpp::Elf_Xword
  flags() const
  { return this->flags_; }

  typedef std::map<Section_id, unsigned int> Section_layout_order;

  void
  update_section_layout(const Section_layout_order* order_map);

  // Update the output section flags based on input section flags.
  void
  update_flags_for_input_section(elfcpp::Elf_Xword flags);

  // Set the output section flags.
  void
  set_flags(elfcpp::Elf_Xword flags)
  { this->flags_ = flags; }

  // Return the entsize field.
  uint64_t
  entsize() const
  { return this->entsize_; }

  // Set the entsize field.
  void
  set_entsize(uint64_t v);

  // Set the load address.
  void
  set_load_address(uint64_t load_address)
  {
    this->load_address_ = load_address;
    this->has_load_address_ = true;
  }

  // Set the link field to the output section index of a section.
  void
  set_link_section(const Output_data* od)
  {
    gold_assert(this->link_ == 0
		&& !this->should_link_to_symtab_
		&& !this->should_link_to_dynsym_);
    this->link_section_ = od;
  }

  // Set the link field to a constant.
  void
  set_link(unsigned int v)
  {
    gold_assert(this->link_section_ == NULL
		&& !this->should_link_to_symtab_
		&& !this->should_link_to_dynsym_);
    this->link_ = v;
  }

  // Record that this section should link to the normal symbol table.
  void
  set_should_link_to_symtab()
  {
    gold_assert(this->link_section_ == NULL
		&& this->link_ == 0
		&& !this->should_link_to_dynsym_);
    this->should_link_to_symtab_ = true;
  }

  // Record that this section should link to the dynamic symbol table.
  void
  set_should_link_to_dynsym()
  {
    gold_assert(this->link_section_ == NULL
		&& this->link_ == 0
		&& !this->should_link_to_symtab_);
    this->should_link_to_dynsym_ = true;
  }

  // Return the info field.
  unsigned int
  info() const
  {
    gold_assert(this->info_section_ == NULL
		&& this->info_symndx_ == NULL);
    return this->info_;
  }

  // Set the info field to the output section index of a section.
  void
  set_info_section(const Output_section* os)
  {
    gold_assert((this->info_section_ == NULL
		 || (this->info_section_ == os
		     && this->info_uses_section_index_))
		&& this->info_symndx_ == NULL
		&& this->info_ == 0);
    this->info_section_ = os;
    this->info_uses_section_index_= true;
  }

  // Set the info field to the symbol table index of a symbol.
  void
  set_info_symndx(const Symbol* sym)
  {
    gold_assert(this->info_section_ == NULL
		&& (this->info_symndx_ == NULL
		    || this->info_symndx_ == sym)
		&& this->info_ == 0);
    this->info_symndx_ = sym;
  }

  // Set the info field to the symbol table index of a section symbol.
  void
  set_info_section_symndx(const Output_section* os)
  {
    gold_assert((this->info_section_ == NULL
		 || (this->info_section_ == os
		     && !this->info_uses_section_index_))
		&& this->info_symndx_ == NULL
		&& this->info_ == 0);
    this->info_section_ = os;
    this->info_uses_section_index_ = false;
  }

  // Set the info field to a constant.
  void
  set_info(unsigned int v)
  {
    gold_assert(this->info_section_ == NULL
		&& this->info_symndx_ == NULL
		&& (this->info_ == 0
		    || this->info_ == v));
    this->info_ = v;
  }

  // Set the addralign field.
  void
  set_addralign(uint64_t v)
  { this->addralign_ = v; }

  void
  checkpoint_set_addralign(uint64_t val)
  {
    if (this->checkpoint_ != NULL)
      this->checkpoint_->set_addralign(val);
  }

  // Whether the output section index has been set.
  bool
  has_out_shndx() const
  { return this->out_shndx_ != -1U; }

  // Indicate that we need a symtab index.
  void
  set_needs_symtab_index()
  { this->needs_symtab_index_ = true; }

  // Return whether we need a symtab index.
  bool
  needs_symtab_index() const
  { return this->needs_symtab_index_; }

  // Get the symtab index.
  unsigned int
  symtab_index() const
  {
    gold_assert(this->symtab_index_ != 0);
    return this->symtab_index_;
  }

  // Set the symtab index.
  void
  set_symtab_index(unsigned int index)
  {
    gold_assert(index != 0);
    this->symtab_index_ = index;
  }

  // Indicate that we need a dynsym index.
  void
  set_needs_dynsym_index()
  { this->needs_dynsym_index_ = true; }

  // Return whether we need a dynsym index.
  bool
  needs_dynsym_index() const
  { return this->needs_dynsym_index_; }

  // Get the dynsym index.
  unsigned int
  dynsym_index() const
  {
    gold_assert(this->dynsym_index_ != 0);
    return this->dynsym_index_;
  }

  // Set the dynsym index.
  void
  set_dynsym_index(unsigned int index)
  {
    gold_assert(index != 0);
    this->dynsym_index_ = index;
  }

  // Sort the attached input sections.
  void
  sort_attached_input_sections();

  // Return whether the input sections sections attachd to this output
  // section may require sorting.  This is used to handle constructor
  // priorities compatibly with GNU ld.
  bool
  may_sort_attached_input_sections() const
  { return this->may_sort_attached_input_sections_; }

  // Record that the input sections attached to this output section
  // may require sorting.
  void
  set_may_sort_attached_input_sections()
  { this->may_sort_attached_input_sections_ = true; }

   // Returns true if input sections must be sorted according to the
  // order in which their name appear in the --section-ordering-file.
  bool
  input_section_order_specified()
  { return this->input_section_order_specified_; }

  // Record that input sections must be sorted as some of their names
  // match the patterns specified through --section-ordering-file.
  void
  set_input_section_order_specified()
  { this->input_section_order_specified_ = true; }

  // Return whether the input sections attached to this output section
  // require sorting.  This is used to handle constructor priorities
  // compatibly with GNU ld.
  bool
  must_sort_attached_input_sections() const
  { return this->must_sort_attached_input_sections_; }

  // Record that the input sections attached to this output section
  // require sorting.
  void
  set_must_sort_attached_input_sections()
  { this->must_sort_attached_input_sections_ = true; }

  // Get the order in which this section appears in the PT_LOAD output
  // segment.
  Output_section_order
  order() const
  { return this->order_; }

  // Set the order for this section.
  void
  set_order(Output_section_order order)
  { this->order_ = order; }

  // Return whether this section holds relro data--data which has
  // dynamic relocations but which may be marked read-only after the
  // dynamic relocations have been completed.
  bool
  is_relro() const
  { return this->is_relro_; }

  // Record that this section holds relro data.
  void
  set_is_relro()
  { this->is_relro_ = true; }

  // Record that this section does not hold relro data.
  void
  clear_is_relro()
  { this->is_relro_ = false; }

  // True if this is a small section: a section which holds small
  // variables.
  bool
  is_small_section() const
  { return this->is_small_section_; }

  // Record that this is a small section.
  void
  set_is_small_section()
  { this->is_small_section_ = true; }

  // True if this is a large section: a section which holds large
  // variables.
  bool
  is_large_section() const
  { return this->is_large_section_; }

  // Record that this is a large section.
  void
  set_is_large_section()
  { this->is_large_section_ = true; }

  // True if this is a large data (not BSS) section.
  bool
  is_large_data_section()
  { return this->is_large_section_ && this->type_ != elfcpp::SHT_NOBITS; }

  // Return whether this section should be written after all the input
  // sections are complete.
  bool
  after_input_sections() const
  { return this->after_input_sections_; }

  // Record that this section should be written after all the input
  // sections are complete.
  void
  set_after_input_sections()
  { this->after_input_sections_ = true; }

  // Return whether this section requires postprocessing after all
  // relocations have been applied.
  bool
  requires_postprocessing() const
  { return this->requires_postprocessing_; }

  bool
  is_unique_segment() const
  { return this->is_unique_segment_; }

  void
  set_is_unique_segment()
  { this->is_unique_segment_ = true; }

  uint64_t extra_segment_flags() const
  { return this->extra_segment_flags_; }

  void
  set_extra_segment_flags(uint64_t flags)
  { this->extra_segment_flags_ = flags; }

  uint64_t segment_alignment() const
  { return this->segment_alignment_; }

  void
  set_segment_alignment(uint64_t align)
  { this->segment_alignment_ = align; }

  // If a section requires postprocessing, return the buffer to use.
  unsigned char*
  postprocessing_buffer() const
  {
    gold_assert(this->postprocessing_buffer_ != NULL);
    return this->postprocessing_buffer_;
  }

  // If a section requires postprocessing, create the buffer to use.
  void
  create_postprocessing_buffer();

  // If a section requires postprocessing, this is the size of the
  // buffer to which relocations should be applied.
  off_t
  postprocessing_buffer_size() const
  { return this->current_data_size_for_child(); }

  // Modify the section name.  This is only permitted for an
  // unallocated section, and only before the size has been finalized.
  // Otherwise the name will not get into Layout::namepool_.
  void
  set_name(const char* newname)
  {
    gold_assert((this->flags_ & elfcpp::SHF_ALLOC) == 0);
    gold_assert(!this->is_data_size_valid());
    this->name_ = newname;
  }

  // Return whether the offset OFFSET in the input section SHNDX in
  // object OBJECT is being included in the link.
  bool
  is_input_address_mapped(const Relobj* object, unsigned int shndx,
			  off_t offset) const;

  // Return the offset within the output section of OFFSET relative to
  // the start of input section SHNDX in object OBJECT.
  section_offset_type
  output_offset(const Relobj* object, unsigned int shndx,
		section_offset_type offset) const;

  // Return the output virtual address of OFFSET relative to the start
  // of input section SHNDX in object OBJECT.
  uint64_t
  output_address(const Relobj* object, unsigned int shndx,
		 off_t offset) const;

  // Look for the merged section for input section SHNDX in object
  // OBJECT.  If found, return true, and set *ADDR to the address of
  // the start of the merged section.  This is not necessary the
  // output offset corresponding to input offset 0 in the section,
  // since the section may be mapped arbitrarily.
  bool
  find_starting_output_address(const Relobj* object, unsigned int shndx,
			       uint64_t* addr) const;

  // Record that this output section was found in the SECTIONS clause
  // of a linker script.
  void
  set_found_in_sections_clause()
  { this->found_in_sections_clause_ = true; }

  // Return whether this output section was found in the SECTIONS
  // clause of a linker script.
  bool
  found_in_sections_clause() const
  { return this->found_in_sections_clause_; }

  // Write the section header into *OPHDR.
  template<int size, bool big_endian>
  void
  write_header(const Layout*, const Stringpool*,
	       elfcpp::Shdr_write<size, big_endian>*) const;

  // The next few calls are for linker script support.

  // In some cases we need to keep a list of the input sections
  // associated with this output section.  We only need the list if we
  // might have to change the offsets of the input section within the
  // output section after we add the input section.  The ordinary
  // input sections will be written out when we process the object
  // file, and as such we don't need to track them here.  We do need
  // to track Output_section_data objects here.  We store instances of
  // this structure in a std::vector, so it must be a POD.  There can
  // be many instances of this structure, so we use a union to save
  // some space.
  class Input_section
  {
   public:
    Input_section()
      : shndx_(0), p2align_(0)
    {
      this->u1_.data_size = 0;
      this->u2_.object = NULL;
    }

    // For an ordinary input section.
    Input_section(Relobj* object, unsigned int shndx, off_t data_size,
		  uint64_t addralign)
      : shndx_(shndx),
	p2align_(ffsll(static_cast<long long>(addralign))),
	section_order_index_(0)
    {
      gold_assert(shndx != OUTPUT_SECTION_CODE
		  && shndx != MERGE_DATA_SECTION_CODE
		  && shndx != MERGE_STRING_SECTION_CODE
		  && shndx != RELAXED_INPUT_SECTION_CODE);
      this->u1_.data_size = data_size;
      this->u2_.object = object;
    }

    // For a non-merge output section.
    Input_section(Output_section_data* posd)
      : shndx_(OUTPUT_SECTION_CODE), p2align_(0),
	section_order_index_(0)
    {
      this->u1_.data_size = 0;
      this->u2_.posd = posd;
    }

    // For a merge section.
    Input_section(Output_section_data* posd, bool is_string, uint64_t entsize)
      : shndx_(is_string
	       ? MERGE_STRING_SECTION_CODE
	       : MERGE_DATA_SECTION_CODE),
	p2align_(0),
	section_order_index_(0)
    {
      this->u1_.entsize = entsize;
      this->u2_.posd = posd;
    }

    // For a relaxed input section.
    Input_section(Output_relaxed_input_section* psection)
      : shndx_(RELAXED_INPUT_SECTION_CODE), p2align_(0),
	section_order_index_(0)
    {
      this->u1_.data_size = 0;
      this->u2_.poris = psection;
    }

    unsigned int
    section_order_index() const
    {
      return this->section_order_index_;
    }

    void
    set_section_order_index(unsigned int number)
    {
      this->section_order_index_ = number;
    }

    // The required alignment.
    uint64_t
    addralign() const
    {
      if (this->p2align_ != 0)
	return static_cast<uint64_t>(1) << (this->p2align_ - 1);
      else if (!this->is_input_section())
	return this->u2_.posd->addralign();
      else
	return 0;
    }

    // Set the required alignment, which must be either 0 or a power of 2.
    // For input sections that are sub-classes of Output_section_data, a
    // alignment of zero means asking the underlying object for alignment.
    void
    set_addralign(uint64_t addralign)
    {
      if (addralign == 0)
	this->p2align_ = 0;
      else
	{
	  gold_assert((addralign & (addralign - 1)) == 0);
	  this->p2align_ = ffsll(static_cast<long long>(addralign));
	}
    }

    // Return the current required size, without finalization.
    off_t
    current_data_size() const;

    // Return the required size.
    off_t
    data_size() const;

    // Whether this is an input section.
    bool
    is_input_section() const
    {
      return (this->shndx_ != OUTPUT_SECTION_CODE
	      && this->shndx_ != MERGE_DATA_SECTION_CODE
	      && this->shndx_ != MERGE_STRING_SECTION_CODE
	      && this->shndx_ != RELAXED_INPUT_SECTION_CODE);
    }

    // Return whether this is a merge section which matches the
    // parameters.
    bool
    is_merge_section(bool is_string, uint64_t entsize,
		     uint64_t addralign) const
    {
      return (this->shndx_ == (is_string
			       ? MERGE_STRING_SECTION_CODE
			       : MERGE_DATA_SECTION_CODE)
	      && this->u1_.entsize == entsize
	      && this->addralign() == addralign);
    }

    // Return whether this is a merge section for some input section.
    bool
    is_merge_section() const
    {
      return (this->shndx_ == MERGE_DATA_SECTION_CODE
	      || this->shndx_ == MERGE_STRING_SECTION_CODE);
    }

    // Return whether this is a relaxed input section.
    bool
    is_relaxed_input_section() const
    { return this->shndx_ == RELAXED_INPUT_SECTION_CODE; }

    // Return whether this is a generic Output_section_data.
    bool
    is_output_section_data() const
    {
      return this->shndx_ == OUTPUT_SECTION_CODE;
    }

    // Return the object for an input section.
    Relobj*
    relobj() const;

    // Return the input section index for an input section.
    unsigned int
    shndx() const;

    // For non-input-sections, return the associated Output_section_data
    // object.
    Output_section_data*
    output_section_data() const
    {
      gold_assert(!this->is_input_section());
      return this->u2_.posd;
    }

    // For a merge section, return the Output_merge_base pointer.
    Output_merge_base*
    output_merge_base() const
    {
      gold_assert(this->is_merge_section());
      return this->u2_.pomb;
    }

    // Return the Output_relaxed_input_section object.
    Output_relaxed_input_section*
    relaxed_input_section() const
    {
      gold_assert(this->is_relaxed_input_section());
      return this->u2_.poris;
    }

    // Set the output section.
    void
    set_output_section(Output_section* os)
    {
      gold_assert(!this->is_input_section());
      Output_section_data* posd =
	this->is_relaxed_input_section() ? this->u2_.poris : this->u2_.posd;
      posd->set_output_section(os);
    }

    // Set the address and file offset.  This is called during
    // Layout::finalize.  SECTION_FILE_OFFSET is the file offset of
    // the enclosing section.
    void
    set_address_and_file_offset(uint64_t address, off_t file_offset,
				off_t section_file_offset);

    // Reset the address and file offset.
    void
    reset_address_and_file_offset();

    // Finalize the data size.
    void
    finalize_data_size();

    // Add an input section, for SHF_MERGE sections.
    bool
    add_input_section(Relobj* object, unsigned int shndx)
    {
      gold_assert(this->shndx_ == MERGE_DATA_SECTION_CODE
		  || this->shndx_ == MERGE_STRING_SECTION_CODE);
      return this->u2_.posd->add_input_section(object, shndx);
    }

    // Given an input OBJECT, an input section index SHNDX within that
    // object, and an OFFSET relative to the start of that input
    // section, return whether or not the output offset is known.  If
    // this function returns true, it sets *POUTPUT to the offset in
    // the output section, relative to the start of the input section
    // in the output section.  *POUTPUT may be different from OFFSET
    // for a merged section.
    bool
    output_offset(const Relobj* object, unsigned int shndx,
		  section_offset_type offset,
		  section_offset_type* poutput) const;

    // Write out the data.  This does nothing for an input section.
    void
    write(Output_file*);

    // Write the data to a buffer.  This does nothing for an input
    // section.
    void
    write_to_buffer(unsigned char*);

    // Print to a map file.
    void
    print_to_mapfile(Mapfile*) const;

    // Print statistics about merge sections to stderr.
    void
    print_merge_stats(const char* section_name)
    {
      if (this->shndx_ == MERGE_DATA_SECTION_CODE
	  || this->shndx_ == MERGE_STRING_SECTION_CODE)
	this->u2_.posd->print_merge_stats(section_name);
    }

   private:
    // Code values which appear in shndx_.  If the value is not one of
    // these codes, it is the input section index in the object file.
    enum
    {
      // An Output_section_data.
      OUTPUT_SECTION_CODE = -1U,
      // An Output_section_data for an SHF_MERGE section with
      // SHF_STRINGS not set.
      MERGE_DATA_SECTION_CODE = -2U,
      // An Output_section_data for an SHF_MERGE section with
      // SHF_STRINGS set.
      MERGE_STRING_SECTION_CODE = -3U,
      // An Output_section_data for a relaxed input section.
      RELAXED_INPUT_SECTION_CODE = -4U
    };

    // For an ordinary input section, this is the section index in the
    // input file.  For an Output_section_data, this is
    // OUTPUT_SECTION_CODE or MERGE_DATA_SECTION_CODE or
    // MERGE_STRING_SECTION_CODE.
    unsigned int shndx_;
    // The required alignment, stored as a power of 2.
    unsigned int p2align_;
    union
    {
      // For an ordinary input section, the section size.
      off_t data_size;
      // For OUTPUT_SECTION_CODE or RELAXED_INPUT_SECTION_CODE, this is not
      // used.  For MERGE_DATA_SECTION_CODE or MERGE_STRING_SECTION_CODE, the
      // entity size.
      uint64_t entsize;
    } u1_;
    union
    {
      // For an ordinary input section, the object which holds the
      // input section.
      Relobj* object;
      // For OUTPUT_SECTION_CODE or MERGE_DATA_SECTION_CODE or
      // MERGE_STRING_SECTION_CODE, the data.
      Output_section_data* posd;
      Output_merge_base* pomb;
      // For RELAXED_INPUT_SECTION_CODE, the data.
      Output_relaxed_input_section* poris;
    } u2_;
    // The line number of the pattern it matches in the --section-ordering-file
    // file.  It is 0 if does not match any pattern.
    unsigned int section_order_index_;
  };

  // Store the list of input sections for this Output_section into the
  // list passed in.  This removes the input sections, leaving only
  // any Output_section_data elements.  This returns the size of those
  // Output_section_data elements.  ADDRESS is the address of this
  // output section.  FILL is the fill value to use, in case there are
  // any spaces between the remaining Output_section_data elements.
  uint64_t
  get_input_sections(uint64_t address, const std::string& fill,
		     std::list<Input_section>*);

  // Add a script input section.  A script input section can either be
  // a plain input section or a sub-class of Output_section_data.
  void
  add_script_input_section(const Input_section& input_section);

  // Set the current size of the output section.
  void
  set_current_data_size(off_t size)
  { this->set_current_data_size_for_child(size); }

  // End of linker script support.

  // Save states before doing section layout.
  // This is used for relaxation.
  void
  save_states();

  // Restore states prior to section layout.
  void
  restore_states();

  // Discard states.
  void
  discard_states();

  // Convert existing input sections to relaxed input sections.
  void
  convert_input_sections_to_relaxed_sections(
      const std::vector<Output_relaxed_input_section*>& sections);

  // Find a relaxed input section to an input section in OBJECT
  // with index SHNDX.  Return NULL if none is found.
  const Output_relaxed_input_section*
  find_relaxed_input_section(const Relobj* object, unsigned int shndx) const;

  // Whether section offsets need adjustment due to relaxation.
  bool
  section_offsets_need_adjustment() const
  { return this->section_offsets_need_adjustment_; }

  // Set section_offsets_need_adjustment to be true.
  void
  set_section_offsets_need_adjustment()
  { this->section_offsets_need_adjustment_ = true; }

  // Set section_offsets_need_adjustment to be false.
  void
  clear_section_offsets_need_adjustment()
  { this->section_offsets_need_adjustment_ = false; }

  // Adjust section offsets of input sections in this.  This is
  // requires if relaxation caused some input sections to change sizes.
  void
  adjust_section_offsets();

  // Whether this is a NOLOAD section.
  bool
  is_noload() const
  { return this->is_noload_; }

  // Set NOLOAD flag.
  void
  set_is_noload()
  { this->is_noload_ = true; }

  // Print merge statistics to stderr.
  void
  print_merge_stats();

  // Set a fixed layout for the section.  Used for incremental update links.
  void
  set_fixed_layout(uint64_t sh_addr, off_t sh_offset, off_t sh_size,
		   uint64_t sh_addralign);

  // Return TRUE if the section has a fixed layout.
  bool
  has_fixed_layout() const
  { return this->has_fixed_layout_; }

  // Set flag to allow patch space for this section.  Used for full
  // incremental links.
  void
  set_is_patch_space_allowed()
  { this->is_patch_space_allowed_ = true; }

  // Set a fill method to use for free space left in the output section
  // during incremental links.
  void
  set_free_space_fill(Output_fill* free_space_fill)
  {
    this->free_space_fill_ = free_space_fill;
    this->free_list_.set_min_hole_size(free_space_fill->minimum_hole_size());
  }

  // Reserve space within the fixed layout for the section.  Used for
  // incremental update links.
  void
  reserve(uint64_t sh_offset, uint64_t sh_size);

  // Allocate space from the free list for the section.  Used for
  // incremental update links.
  off_t
  allocate(off_t len, uint64_t addralign);

  typedef std::vector<Input_section> Input_section_list;

  // Allow access to the input sections.
  const Input_section_list&
  input_sections() const
  { return this->input_sections_; }

  Input_section_list&
  input_sections()
  { return this->input_sections_; }

  // For -r and --emit-relocs, we need to keep track of the associated
  // relocation section.
  Output_section*
  reloc_section() const
  { return this->reloc_section_; }

  void
  set_reloc_section(Output_section* os)
  { this->reloc_section_ = os; }

 protected:
  // Return the output section--i.e., the object itself.
  Output_section*
  do_output_section()
  { return this; }

  const Output_section*
  do_output_section() const
  { return this; }

  // Return the section index in the output file.
  unsigned int
  do_out_shndx() const
  {
    gold_assert(this->out_shndx_ != -1U);
    return this->out_shndx_;
  }

  // Set the output section index.
  void
  do_set_out_shndx(unsigned int shndx)
  {
    gold_assert(this->out_shndx_ == -1U || this->out_shndx_ == shndx);
    this->out_shndx_ = shndx;
  }

  // Update the data size of the Output_section.  For a typical
  // Output_section, there is nothing to do, but if there are any
  // Output_section_data objects we need to do a trial layout
  // here.
  virtual void
  update_data_size();

  // Set the final data size of the Output_section.  For a typical
  // Output_section, there is nothing to do, but if there are any
  // Output_section_data objects we need to set their final addresses
  // here.
  virtual void
  set_final_data_size();

  // Reset the address and file offset.
  void
  do_reset_address_and_file_offset();

  // Return true if address and file offset already have reset values. In
  // other words, calling reset_address_and_file_offset will not change them.
  bool
  do_address_and_file_offset_have_reset_values() const;

  // Write the data to the file.  For a typical Output_section, this
  // does nothing: the data is written out by calling Object::Relocate
  // on each input object.  But if there are any Output_section_data
  // objects we do need to write them out here.
  virtual void
  do_write(Output_file*);

  // Return the address alignment--function required by parent class.
  uint64_t
  do_addralign() const
  { return this->addralign_; }

  // Return whether there is a load address.
  bool
  do_has_load_address() const
  { return this->has_load_address_; }

  // Return the load address.
  uint64_t
  do_load_address() const
  {
    gold_assert(this->has_load_address_);
    return this->load_address_;
  }

  // Return whether this is an Output_section.
  bool
  do_is_section() const
  { return true; }

  // Return whether this is a section of the specified type.
  bool
  do_is_section_type(elfcpp::Elf_Word type) const
  { return this->type_ == type; }

  // Return whether the specified section flag is set.
  bool
  do_is_section_flag_set(elfcpp::Elf_Xword flag) const
  { return (this->flags_ & flag) != 0; }

  // Set the TLS offset.  Called only for SHT_TLS sections.
  void
  do_set_tls_offset(uint64_t tls_base);

  // Return the TLS offset, relative to the base of the TLS segment.
  // Valid only for SHT_TLS sections.
  uint64_t
  do_tls_offset() const
  { return this->tls_offset_; }

  // This may be implemented by a child class.
  virtual void
  do_finalize_name(Layout*)
  { }

  // Print to the map file.
  virtual void
  do_print_to_mapfile(Mapfile*) const;

  // Record that this section requires postprocessing after all
  // relocations have been applied.  This is called by a child class.
  void
  set_requires_postprocessing()
  {
    this->requires_postprocessing_ = true;
    this->after_input_sections_ = true;
  }

  // Write all the data of an Output_section into the postprocessing
  // buffer.
  void
  write_to_postprocessing_buffer();

  // Whether this always keeps an input section list
  bool
  always_keeps_input_sections() const
  { return this->always_keeps_input_sections_; }

  // Always keep an input section list.
  void
  set_always_keeps_input_sections()
  {
    gold_assert(this->current_data_size_for_child() == 0);
    this->always_keeps_input_sections_ = true;
  }

 private:
  // We only save enough information to undo the effects of section layout.
  class Checkpoint_output_section
  {
   public:
    Checkpoint_output_section(uint64_t addralign, elfcpp::Elf_Xword flags,
			      const Input_section_list& input_sections,
			      off_t first_input_offset,
			      bool attached_input_sections_are_sorted)
      : addralign_(addralign), flags_(flags),
	input_sections_(input_sections),
	input_sections_size_(input_sections_.size()),
	input_sections_copy_(), first_input_offset_(first_input_offset),
	attached_input_sections_are_sorted_(attached_input_sections_are_sorted)
    { }

    virtual
    ~Checkpoint_output_section()
    { }

    // Return the address alignment.
    uint64_t
    addralign() const
    { return this->addralign_; }

    void
    set_addralign(uint64_t val)
    { this->addralign_ = val; }

    // Return the section flags.
    elfcpp::Elf_Xword
    flags() const
    { return this->flags_; }

    // Return a reference to the input section list copy.
    Input_section_list*
    input_sections()
    { return &this->input_sections_copy_; }

    // Return the size of input_sections at the time when checkpoint is
    // taken.
    size_t
    input_sections_size() const
    { return this->input_sections_size_; }

    // Whether input sections are copied.
    bool
    input_sections_saved() const
    { return this->input_sections_copy_.size() == this->input_sections_size_; }

    off_t
    first_input_offset() const
    { return this->first_input_offset_; }

    bool
    attached_input_sections_are_sorted() const
    { return this->attached_input_sections_are_sorted_; }

    // Save input sections.
    void
    save_input_sections()
    {
      this->input_sections_copy_.reserve(this->input_sections_size_);
      this->input_sections_copy_.clear();
      Input_section_list::const_iterator p = this->input_sections_.begin();
      gold_assert(this->input_sections_size_ >= this->input_sections_.size());
      for(size_t i = 0; i < this->input_sections_size_ ; i++, ++p)
	this->input_sections_copy_.push_back(*p);
    }

   private:
    // The section alignment.
    uint64_t addralign_;
    // The section flags.
    elfcpp::Elf_Xword flags_;
    // Reference to the input sections to be checkpointed.
    const Input_section_list& input_sections_;
    // Size of the checkpointed portion of input_sections_;
    size_t input_sections_size_;
    // Copy of input sections.
    Input_section_list input_sections_copy_;
    // The offset of the first entry in input_sections_.
    off_t first_input_offset_;
    // True if the input sections attached to this output section have
    // already been sorted.
    bool attached_input_sections_are_sorted_;
  };

  // This class is used to sort the input sections.
  class Input_section_sort_entry;

  // This is the sort comparison function for ctors and dtors.
  struct Input_section_sort_compare
  {
    bool
    operator()(const Input_section_sort_entry&,
	       const Input_section_sort_entry&) const;
  };

  // This is the sort comparison function for .init_array and .fini_array.
  struct Input_section_sort_init_fini_compare
  {
    bool
    operator()(const Input_section_sort_entry&,
	       const Input_section_sort_entry&) const;
  };

  // This is the sort comparison function when a section order is specified
  // from an input file.
  struct Input_section_sort_section_order_index_compare
  {
    bool
    operator()(const Input_section_sort_entry&,
	       const Input_section_sort_entry&) const;
  };

  // This is the sort comparison function for .text to sort sections with
  // prefixes .text.{unlikely,exit,startup,hot} before other sections.
  struct Input_section_sort_section_prefix_special_ordering_compare
  {
    bool
    operator()(const Input_section_sort_entry&,
	       const Input_section_sort_entry&) const;
  };

  // This is the sort comparison function for sorting sections by name.
  struct Input_section_sort_section_name_compare
  {
    bool
    operator()(const Input_section_sort_entry&,
	       const Input_section_sort_entry&) const;
  };

  // Fill data.  This is used to fill in data between input sections.
  // It is also used for data statements (BYTE, WORD, etc.) in linker
  // scripts.  When we have to keep track of the input sections, we
  // can use an Output_data_const, but we don't want to have to keep
  // track of input sections just to implement fills.
  class Fill
  {
   public:
    Fill(off_t section_offset, off_t length)
      : section_offset_(section_offset),
	length_(convert_to_section_size_type(length))
    { }

    // Return section offset.
    off_t
    section_offset() const
    { return this->section_offset_; }

    // Return fill length.
    section_size_type
    length() const
    { return this->length_; }

   private:
    // The offset within the output section.
    off_t section_offset_;
    // The length of the space to fill.
    section_size_type length_;
  };

  typedef std::vector<Fill> Fill_list;

  // Map used during relaxation of existing sections.  This map
  // a section id an input section list index.  We assume that
  // Input_section_list is a vector.
  typedef Unordered_map<Section_id, size_t, Section_id_hash> Relaxation_map;

  // Add a new output section by Input_section.
  void
  add_output_section_data(Input_section*);

  // Add an SHF_MERGE input section.  Returns true if the section was
  // handled.  If KEEPS_INPUT_SECTIONS is true, the output merge section
  // stores information about the merged input sections.
  bool
  add_merge_input_section(Relobj* object, unsigned int shndx, uint64_t flags,
			  uint64_t entsize, uint64_t addralign,
			  bool keeps_input_sections);

  // Add an output SHF_MERGE section POSD to this output section.
  // IS_STRING indicates whether it is a SHF_STRINGS section, and
  // ENTSIZE is the entity size.  This returns the entry added to
  // input_sections_.
  void
  add_output_merge_section(Output_section_data* posd, bool is_string,
			   uint64_t entsize);

  // Find the merge section into which an input section with index SHNDX in
  // OBJECT has been added.  Return NULL if none found.
  const Output_section_data*
  find_merge_section(const Relobj* object, unsigned int shndx) const;

  // Build a relaxation map.
  void
  build_relaxation_map(
      const Input_section_list& input_sections,
      size_t limit,
      Relaxation_map* map) const;

  // Convert input sections in an input section list into relaxed sections.
  void
  convert_input_sections_in_list_to_relaxed_sections(
      const std::vector<Output_relaxed_input_section*>& relaxed_sections,
      const Relaxation_map& map,
      Input_section_list* input_sections);

  // Build the lookup maps for merge and relaxed input sections.
  void
  build_lookup_maps() const;

  // Most of these fields are only valid after layout.

  // The name of the section.  This will point into a Stringpool.
  const char* name_;
  // The section address is in the parent class.
  // The section alignment.
  uint64_t addralign_;
  // The section entry size.
  uint64_t entsize_;
  // The load address.  This is only used when using a linker script
  // with a SECTIONS clause.  The has_load_address_ field indicates
  // whether this field is valid.
  uint64_t load_address_;
  // The file offset is in the parent class.
  // Set the section link field to the index of this section.
  const Output_data* link_section_;
  // If link_section_ is NULL, this is the link field.
  unsigned int link_;
  // Set the section info field to the index of this section.
  const Output_section* info_section_;
  // If info_section_ is NULL, set the info field to the symbol table
  // index of this symbol.
  const Symbol* info_symndx_;
  // If info_section_ and info_symndx_ are NULL, this is the section
  // info field.
  unsigned int info_;
  // The section type.
  const elfcpp::Elf_Word type_;
  // The section flags.
  elfcpp::Elf_Xword flags_;
  // The order of this section in the output segment.
  Output_section_order order_;
  // The section index.
  unsigned int out_shndx_;
  // If there is a STT_SECTION for this output section in the normal
  // symbol table, this is the symbol index.  This starts out as zero.
  // It is initialized in Layout::finalize() to be the index, or -1U
  // if there isn't one.
  unsigned int symtab_index_;
  // If there is a STT_SECTION for this output section in the dynamic
  // symbol table, this is the symbol index.  This starts out as zero.
  // It is initialized in Layout::finalize() to be the index, or -1U
  // if there isn't one.
  unsigned int dynsym_index_;
  // The input sections.  This will be empty in cases where we don't
  // need to keep track of them.
  Input_section_list input_sections_;
  // The offset of the first entry in input_sections_.
  off_t first_input_offset_;
  // The fill data.  This is separate from input_sections_ because we
  // often will need fill sections without needing to keep track of
  // input sections.
  Fill_list fills_;
  // If the section requires postprocessing, this buffer holds the
  // section contents during relocation.
  unsigned char* postprocessing_buffer_;
  // Whether this output section needs a STT_SECTION symbol in the
  // normal symbol table.  This will be true if there is a relocation
  // which needs it.
  bool needs_symtab_index_ : 1;
  // Whether this output section needs a STT_SECTION symbol in the
  // dynamic symbol table.  This will be true if there is a dynamic
  // relocation which needs it.
  bool needs_dynsym_index_ : 1;
  // Whether the link field of this output section should point to the
  // normal symbol table.
  bool should_link_to_symtab_ : 1;
  // Whether the link field of this output section should point to the
  // dynamic symbol table.
  bool should_link_to_dynsym_ : 1;
  // Whether this section should be written after all the input
  // sections are complete.
  bool after_input_sections_ : 1;
  // Whether this section requires post processing after all
  // relocations have been applied.
  bool requires_postprocessing_ : 1;
  // Whether an input section was mapped to this output section
  // because of a SECTIONS clause in a linker script.
  bool found_in_sections_clause_ : 1;
  // Whether this section has an explicitly specified load address.
  bool has_load_address_ : 1;
  // True if the info_section_ field means the section index of the
  // section, false if it means the symbol index of the corresponding
  // section symbol.
  bool info_uses_section_index_ : 1;
  // True if input sections attached to this output section have to be
  // sorted according to a specified order.
  bool input_section_order_specified_ : 1;
  // True if the input sections attached to this output section may
  // need sorting.
  bool may_sort_attached_input_sections_ : 1;
  // True if the input sections attached to this output section must
  // be sorted.
  bool must_sort_attached_input_sections_ : 1;
  // True if the input sections attached to this output section have
  // already been sorted.
  bool attached_input_sections_are_sorted_ : 1;
  // True if this section holds relro data.
  bool is_relro_ : 1;
  // True if this is a small section.
  bool is_small_section_ : 1;
  // True if this is a large section.
  bool is_large_section_ : 1;
  // Whether code-fills are generated at write.
  bool generate_code_fills_at_write_ : 1;
  // Whether the entry size field should be zero.
  bool is_entsize_zero_ : 1;
  // Whether section offsets need adjustment due to relaxation.
  bool section_offsets_need_adjustment_ : 1;
  // Whether this is a NOLOAD section.
  bool is_noload_ : 1;
  // Whether this always keeps input section.
  bool always_keeps_input_sections_ : 1;
  // Whether this section has a fixed layout, for incremental update links.
  bool has_fixed_layout_ : 1;
  // True if we can add patch space to this section.
  bool is_patch_space_allowed_ : 1;
  // True if this output section goes into a unique segment.
  bool is_unique_segment_ : 1;
  // For SHT_TLS sections, the offset of this section relative to the base
  // of the TLS segment.
  uint64_t tls_offset_;
  // Additional segment flags, specified via linker plugin, when mapping some
  // input sections to unique segments.
  uint64_t extra_segment_flags_;
  // Segment alignment specified via linker plugin, when mapping some
  // input sections to unique segments.
  uint64_t segment_alignment_;
  // Saved checkpoint.
  Checkpoint_output_section* checkpoint_;
  // Fast lookup maps for merged and relaxed input sections.
  Output_section_lookup_maps* lookup_maps_;
  // List of available regions within the section, for incremental
  // update links.
  Free_list free_list_;
  // Method for filling chunks of free space.
  Output_fill* free_space_fill_;
  // Amount added as patch space for incremental linking.
  off_t patch_space_;
  // Associated relocation section, when emitting relocations.
  Output_section* reloc_section_;
};

// An output segment.  PT_LOAD segments are built from collections of
// output sections.  Other segments typically point within PT_LOAD
// segments, and are built directly as needed.
//
// NOTE: We want to use the copy constructor for this class.  During
// relaxation, we may try built the segments multiple times.  We do
// that by copying the original segment list before lay-out, doing
// a trial lay-out and roll-back to the saved copied if we need to
// to the lay-out again.

class Output_segment
{
 public:
  // Create an output segment, specifying the type and flags.
  Output_segment(elfcpp::Elf_Word, elfcpp::Elf_Word);

  // Return the virtual address.
  uint64_t
  vaddr() const
  { return this->vaddr_; }

  // Return the physical address.
  uint64_t
  paddr() const
  { return this->paddr_; }

  // Return the segment type.
  elfcpp::Elf_Word
  type() const
  { return this->type_; }

  // Return the segment flags.
  elfcpp::Elf_Word
  flags() const
  { return this->flags_; }

  // Return the memory size.
  uint64_t
  memsz() const
  { return this->memsz_; }

  // Return the file size.
  off_t
  filesz() const
  { return this->filesz_; }

  // Return the file offset.
  off_t
  offset() const
  { return this->offset_; }

  // Return the segment alignment.
  uint64_t
  align() const
  { return this->align_; }

  // Set the segment alignment.
  void
  set_align(uint64_t align)
  { this->align_ = align; }

  // Whether this is a segment created to hold large data sections.
  bool
  is_large_data_segment() const
  { return this->is_large_data_segment_; }

  // Record that this is a segment created to hold large data
  // sections.
  void
  set_is_large_data_segment()
  { this->is_large_data_segment_ = true; }

  bool
  is_unique_segment() const
  { return this->is_unique_segment_; }

  // Mark segment as unique, happens when linker plugins request that
  // certain input sections be mapped to unique segments.
  void
  set_is_unique_segment()
  { this->is_unique_segment_ = true; }

  // Return the maximum alignment of the Output_data.
  uint64_t
  maximum_alignment();

  // Add the Output_section OS to this PT_LOAD segment.  SEG_FLAGS is
  // the segment flags to use.
  void
  add_output_section_to_load(Layout* layout, Output_section* os,
			     elfcpp::Elf_Word seg_flags);

  // Add the Output_section OS to this non-PT_LOAD segment.  SEG_FLAGS
  // is the segment flags to use.
  void
  add_output_section_to_nonload(Output_section* os,
				elfcpp::Elf_Word seg_flags);

  // Remove an Output_section from this segment.  It is an error if it
  // is not present.
  void
  remove_output_section(Output_section* os);

  // Add an Output_data (which need not be an Output_section) to the
  // start of this segment.
  void
  add_initial_output_data(Output_data*);

  // Return true if this segment has any sections which hold actual
  // data, rather than being a BSS section.
  bool
  has_any_data_sections() const;

  // Whether this segment has a dynamic relocs.
  bool
  has_dynamic_reloc() const;

  // Return the first section.
  Output_section*
  first_section() const;

  // Return the address of the first section.
  uint64_t
  first_section_load_address() const
  {
    const Output_section* os = this->first_section();
    gold_assert(os != NULL);
    return os->has_load_address() ? os->load_address() : os->address();
  }

  // Return whether the addresses have been set already.
  bool
  are_addresses_set() const
  { return this->are_addresses_set_; }

  // Set the addresses.
  void
  set_addresses(uint64_t vaddr, uint64_t paddr)
  {
    this->vaddr_ = vaddr;
    this->paddr_ = paddr;
    this->are_addresses_set_ = true;
  }

  // Update the flags for the flags of an output section added to this
  // segment.
  void
  update_flags_for_output_section(elfcpp::Elf_Xword flags)
  {
    // The ELF ABI specifies that a PT_TLS segment should always have
    // PF_R as the flags.
    if (this->type() != elfcpp::PT_TLS)
      this->flags_ |= flags;
  }

  // Set the segment flags.  This is only used if we have a PHDRS
  // clause which explicitly specifies the flags.
  void
  set_flags(elfcpp::Elf_Word flags)
  { this->flags_ = flags; }

  // Set the address of the segment to ADDR and the offset to *POFF
  // and set the addresses and offsets of all contained output
  // sections accordingly.  Set the section indexes of all contained
  // output sections starting with *PSHNDX.  If RESET is true, first
  // reset the addresses of the contained sections.  Return the
  // address of the immediately following segment.  Update *POFF and
  // *PSHNDX.  This should only be called for a PT_LOAD segment.
  uint64_t
  set_section_addresses(const Target*, Layout*, bool reset, uint64_t addr,
			unsigned int* increase_relro, bool* has_relro,
			off_t* poff, unsigned int* pshndx);

  // Set the minimum alignment of this segment.  This may be adjusted
  // upward based on the section alignments.
  void
  set_minimum_p_align(uint64_t align)
  {
    if (align > this->min_p_align_)
      this->min_p_align_ = align;
  }

  // Set the memory size of this segment.
  void
  set_size(uint64_t size)
  {
    this->memsz_ = size;
  }

  // Set the offset of this segment based on the section.  This should
  // only be called for a non-PT_LOAD segment.
  void
  set_offset(unsigned int increase);

  // Set the TLS offsets of the sections contained in the PT_TLS segment.
  void
  set_tls_offsets();

  // Return the number of output sections.
  unsigned int
  output_section_count() const;

  // Return the section attached to the list segment with the lowest
  // load address.  This is used when handling a PHDRS clause in a
  // linker script.
  Output_section*
  section_with_lowest_load_address() const;

  // Write the segment header into *OPHDR.
  template<int size, bool big_endian>
  void
  write_header(elfcpp::Phdr_write<size, big_endian>*);

  // Write the section headers of associated sections into V.
  template<int size, bool big_endian>
  unsigned char*
  write_section_headers(const Layout*, const Stringpool*, unsigned char* v,
			unsigned int* pshndx) const;

  // Print the output sections in the map file.
  void
  print_sections_to_mapfile(Mapfile*) const;

 private:
  typedef std::vector<Output_data*> Output_data_list;

  // Find the maximum alignment in an Output_data_list.
  static uint64_t
  maximum_alignment_list(const Output_data_list*);

  // Return whether the first data section is a relro section.
  bool
  is_first_section_relro() const;

  // Set the section addresses in an Output_data_list.
  uint64_t
  set_section_list_addresses(Layout*, bool reset, Output_data_list*,
			     uint64_t addr, off_t* poff, off_t* fpoff,
			     unsigned int* pshndx, bool* in_tls);

  // Return the number of Output_sections in an Output_data_list.
  unsigned int
  output_section_count_list(const Output_data_list*) const;

  // Return whether an Output_data_list has a dynamic reloc.
  bool
  has_dynamic_reloc_list(const Output_data_list*) const;

  // Find the section with the lowest load address in an
  // Output_data_list.
  void
  lowest_load_address_in_list(const Output_data_list* pdl,
			      Output_section** found,
			      uint64_t* found_lma) const;

  // Find the first and last entries by address.
  void
  find_first_and_last_list(const Output_data_list* pdl,
			   const Output_data** pfirst,
			   const Output_data** plast) const;

  // Write the section headers in the list into V.
  template<int size, bool big_endian>
  unsigned char*
  write_section_headers_list(const Layout*, const Stringpool*,
			     const Output_data_list*, unsigned char* v,
			     unsigned int* pshdx) const;

  // Print a section list to the mapfile.
  void
  print_section_list_to_mapfile(Mapfile*, const Output_data_list*) const;

  // NOTE: We want to use the copy constructor.  Currently, shallow copy
  // works for us so we do not need to write our own copy constructor.

  // The list of output data attached to this segment.
  Output_data_list output_lists_[ORDER_MAX];
  // The segment virtual address.
  uint64_t vaddr_;
  // The segment physical address.
  uint64_t paddr_;
  // The size of the segment in memory.
  uint64_t memsz_;
  // The segment alignment.
  uint64_t align_;
  // The maximum section alignment.  The is_max_align_known_ field
  // indicates whether this has been finalized.
  uint64_t max_align_;
  // The required minimum value for the p_align field.  This is used
  // for PT_LOAD segments.  Note that this does not mean that
  // addresses should be aligned to this value; it means the p_paddr
  // and p_vaddr fields must be congruent modulo this value.  For
  // non-PT_LOAD segments, the dynamic linker works more efficiently
  // if the p_align field has the more conventional value, although it
  // can align as needed.
  uint64_t min_p_align_;
  // The offset of the segment data within the file.
  off_t offset_;
  // The size of the segment data in the file.
  off_t filesz_;
  // The segment type;
  elfcpp::Elf_Word type_;
  // The segment flags.
  elfcpp::Elf_Word flags_;
  // Whether we have finalized max_align_.
  bool is_max_align_known_ : 1;
  // Whether vaddr and paddr were set by a linker script.
  bool are_addresses_set_ : 1;
  // Whether this segment holds large data sections.
  bool is_large_data_segment_ : 1;
  // Whether this was marked as a unique segment via a linker plugin.
  bool is_unique_segment_ : 1;
};

} // End namespace gold.

#endif // !defined(GOLD_OUTPUT_H)
