// reloc.h -- relocate input files for gold   -*- C++ -*-

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

#ifndef GOLD_RELOC_H
#define GOLD_RELOC_H

#include <vector>
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif

#include "elfcpp.h"
#include "workqueue.h"

namespace gold
{

class General_options;
class Object;
class Relobj;
struct Read_relocs_data;
class Symbol;
class Layout;
class Output_data;
class Output_section;

template<int size>
class Sized_symbol;

template<int size, bool big_endian>
class Sized_relobj_file;

template<int size>
class Symbol_value;

template<int sh_type, bool dynamic, int size, bool big_endian>
class Output_data_reloc;

// A class to read the relocations for an object file, and then queue
// up a task to see if they require any GOT/PLT/COPY relocations in
// the symbol table.

class Read_relocs : public Task
{
 public:
  //   THIS_BLOCKER and NEXT_BLOCKER are passed along to a Scan_relocs
  // or Gc_process_relocs task, so that they run in a deterministic
  // order.
  Read_relocs(Symbol_table* symtab, Layout* layout, Relobj* object,
	      Task_token* this_blocker, Task_token* next_blocker)
    : symtab_(symtab), layout_(layout), object_(object),
      this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const;

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Relobj* object_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// Process the relocs to figure out which sections are garbage.
// Very similar to scan relocs.

class Gc_process_relocs : public Task
{
 public:
  // THIS_BLOCKER prevents this task from running until the previous
  // one is finished.  NEXT_BLOCKER prevents the next task from
  // running.
  Gc_process_relocs(Symbol_table* symtab, Layout* layout, Relobj* object,
		    Read_relocs_data* rd, Task_token* this_blocker,
		    Task_token* next_blocker)
    : symtab_(symtab), layout_(layout), object_(object), rd_(rd),
      this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Gc_process_relocs();

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const;

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Relobj* object_;
  Read_relocs_data* rd_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// Scan the relocations for an object to see if they require any
// GOT/PLT/COPY relocations.

class Scan_relocs : public Task
{
 public:
  // THIS_BLOCKER prevents this task from running until the previous
  // one is finished.  NEXT_BLOCKER prevents the next task from
  // running.
  Scan_relocs(Symbol_table* symtab, Layout* layout, Relobj* object,
	      Read_relocs_data* rd, Task_token* this_blocker,
	      Task_token* next_blocker)
    : symtab_(symtab), layout_(layout), object_(object), rd_(rd),
      this_blocker_(this_blocker), next_blocker_(next_blocker)
  { }

  ~Scan_relocs();

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const;

 private:
  Symbol_table* symtab_;
  Layout* layout_;
  Relobj* object_;
  Read_relocs_data* rd_;
  Task_token* this_blocker_;
  Task_token* next_blocker_;
};

// A class to perform all the relocations for an object file.

class Relocate_task : public Task
{
 public:
  Relocate_task(const Symbol_table* symtab, const Layout* layout,
		Relobj* object, Output_file* of,
		Task_token* input_sections_blocker,
		Task_token* output_sections_blocker, Task_token* final_blocker)
    : symtab_(symtab), layout_(layout), object_(object), of_(of),
      input_sections_blocker_(input_sections_blocker),
      output_sections_blocker_(output_sections_blocker),
      final_blocker_(final_blocker)
  { }

  // The standard Task methods.

  Task_token*
  is_runnable();

  void
  locks(Task_locker*);

  void
  run(Workqueue*);

  std::string
  get_name() const;

 private:
  const Symbol_table* symtab_;
  const Layout* layout_;
  Relobj* object_;
  Output_file* of_;
  Task_token* input_sections_blocker_;
  Task_token* output_sections_blocker_;
  Task_token* final_blocker_;
};

// During a relocatable link, this class records how relocations
// should be handled for a single input reloc section.  An instance of
// this class is created while scanning relocs, and it is used while
// processing relocs.

class Relocatable_relocs
{
 public:
  // We use a vector of unsigned char to indicate how the input relocs
  // should be handled.  Each element is one of the following values.
  // We create this vector when we initially scan the relocations.
  enum Reloc_strategy
  {
    // Copy the input reloc.  Don't modify it other than updating the
    // r_offset field and the r_sym part of the r_info field.
    RELOC_COPY,
    // Copy the input reloc which is against an STT_SECTION symbol.
    // Update the r_offset and r_sym part of the r_info field.  Adjust
    // the addend by subtracting the value of the old local symbol and
    // adding the value of the new local symbol.  The addend is in the
    // SHT_RELA reloc and the contents of the data section do not need
    // to be changed.
    RELOC_ADJUST_FOR_SECTION_RELA,
    // Like RELOC_ADJUST_FOR_SECTION_RELA but the addend should not be
    // adjusted.
    RELOC_ADJUST_FOR_SECTION_0,
    // Like RELOC_ADJUST_FOR_SECTION_RELA but the contents of the
    // section need to be changed.  The number indicates the number of
    // bytes in the addend in the section contents.
    RELOC_ADJUST_FOR_SECTION_1,
    RELOC_ADJUST_FOR_SECTION_2,
    RELOC_ADJUST_FOR_SECTION_4,
    RELOC_ADJUST_FOR_SECTION_8,
    // Like RELOC_ADJUST_FOR_SECTION_4 but for unaligned relocs.
    RELOC_ADJUST_FOR_SECTION_4_UNALIGNED,
    // Discard the input reloc--process it completely when relocating
    // the data section contents.
    RELOC_DISCARD,
    // An input reloc which is not discarded, but which requires
    // target specific processing in order to update it.
    RELOC_SPECIAL
  };

  Relocatable_relocs()
    : reloc_strategies_(), output_reloc_count_(0), posd_(NULL)
  { }

  // Record the number of relocs.
  void
  set_reloc_count(size_t reloc_count)
  { this->reloc_strategies_.reserve(reloc_count); }

  // Record what to do for the next reloc.
  void
  set_next_reloc_strategy(Reloc_strategy strategy)
  {
    this->reloc_strategies_.push_back(static_cast<unsigned char>(strategy));
    if (strategy != RELOC_DISCARD)
      ++this->output_reloc_count_;
  }

  // Record the Output_data associated with this reloc section.
  void
  set_output_data(Output_data* posd)
  {
    gold_assert(this->posd_ == NULL);
    this->posd_ = posd;
  }

  // Return the Output_data associated with this reloc section.
  Output_data*
  output_data() const
  { return this->posd_; }

  // Return what to do for reloc I.
  Reloc_strategy
  strategy(unsigned int i) const
  {
    gold_assert(i < this->reloc_strategies_.size());
    return static_cast<Reloc_strategy>(this->reloc_strategies_[i]);
  }

  // Set the strategy for reloc I.
  void
  set_strategy(unsigned int i, Reloc_strategy strategy)
  {
    gold_assert(i < this->reloc_strategies_.size());
    this->reloc_strategies_[i] = strategy;
  }

  // Return the number of relocations to create in the output file.
  size_t
  output_reloc_count() const
  { return this->output_reloc_count_; }

 private:
  typedef std::vector<unsigned char> Reloc_strategies;

  // The strategies for the input reloc.  There is one entry in this
  // vector for each relocation in the input section.
  Reloc_strategies reloc_strategies_;
  // The number of relocations to be created in the output file.
  size_t output_reloc_count_;
  // The output data structure associated with this relocation.
  Output_data* posd_;
};

template<int valsize>
class Bits;

// Standard relocation routines which are used on many targets.  Here
// SIZE and BIG_ENDIAN refer to the target, not the relocation type.

template<int size, bool big_endian>
class Relocate_functions
{
 public:
  typedef typename elfcpp::Elf_types<size>::Elf_Addr Address;
  typedef typename elfcpp::Elf_types<size>::Elf_Swxword Addendtype;

  enum Overflow_check
  {
    // No overflow checking.
    CHECK_NONE,
    // Check for overflow of a signed value.
    CHECK_SIGNED,
    // Check for overflow of an unsigned value.
    CHECK_UNSIGNED,
    // Check for overflow of a signed or unsigned value.
    // (i.e., no error if either signed or unsigned fits.)
    CHECK_SIGNED_OR_UNSIGNED
  };

  enum Reloc_status
  {
    RELOC_OK,
    RELOC_OVERFLOW
  };

 private:
  // Check for overflow.
  template<int valsize>
  static inline Reloc_status
  check_overflow(Address value, Overflow_check check)
  {
    switch (check)
      {
      case CHECK_SIGNED:
        if (size == 32)
	  return (Bits<valsize>::has_overflow32(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
	else
	  return (Bits<valsize>::has_overflow(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
      case CHECK_UNSIGNED:
        if (size == 32)
	  return (Bits<valsize>::has_unsigned_overflow32(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
	else
	  return (Bits<valsize>::has_unsigned_overflow(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
      case CHECK_SIGNED_OR_UNSIGNED:
        if (size == 32)
	  return (Bits<valsize>::has_signed_unsigned_overflow32(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
	else
	  return (Bits<valsize>::has_signed_unsigned_overflow64(value)
		  ? RELOC_OVERFLOW
		  : RELOC_OK);
      case CHECK_NONE:
      default:
        return RELOC_OK;
      }
  }

  // Do a simple relocation with the addend in the section contents.
  // VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  rel(unsigned char* view, Address value, Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype addend = elfcpp::Swap<valsize, big_endian>::readval(wv);
    value += addend;
    elfcpp::Swap<valsize, big_endian>::
	writeval(wv, static_cast<Valtype>(value));
    return check_overflow<valsize>(value, check);
  }

  // Like the above but for relocs at unaligned addresses.
  template<int valsize>
  static inline Reloc_status
  rel_unaligned(unsigned char* view, Address value, Overflow_check check)
  {
    typedef typename elfcpp::Swap_unaligned<valsize, big_endian>::Valtype
	Valtype;
    Valtype addend = elfcpp::Swap_unaligned<valsize, big_endian>::readval(view);
    value += addend;
    elfcpp::Swap_unaligned<valsize, big_endian>::
	writeval(view, static_cast<Valtype>(value));
    return check_overflow<valsize>(value, check);
  }

  // Do a simple relocation using a Symbol_value with the addend in
  // the section contents.  VALSIZE is the size of the value to
  // relocate.
  template<int valsize>
  static inline Reloc_status
  rel(unsigned char* view,
      const Sized_relobj_file<size, big_endian>* object,
      const Symbol_value<size>* psymval,
      Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype addend = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Address value = psymval->value(object, addend);
    elfcpp::Swap<valsize, big_endian>::
	writeval(wv, static_cast<Valtype>(value));
    return check_overflow<valsize>(value, check);
  }

  // Like the above but for relocs at unaligned addresses.
  template<int valsize>
  static inline Reloc_status
  rel_unaligned(unsigned char* view,
                const Sized_relobj_file<size, big_endian>* object,
                const Symbol_value<size>* psymval,
                Overflow_check check)
  {
    typedef typename elfcpp::Swap_unaligned<valsize, big_endian>::Valtype
        Valtype;
    Valtype addend = elfcpp::Swap_unaligned<valsize, big_endian>::readval(view);
    Address value = psymval->value(object, addend);
    elfcpp::Swap_unaligned<valsize, big_endian>::writeval(view, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple relocation with the addend in the relocation.
  // VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  rela(unsigned char* view, Address value, Addendtype addend,
       Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    value += addend;
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple relocation using a symbol value with the addend in
  // the relocation.  VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  rela(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval,
       Addendtype addend,
       Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Address value = psymval->value(object, addend);
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple PC relative relocation with the addend in the section
  // contents.  VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  pcrel(unsigned char* view, Address value, Address address,
	Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype addend = elfcpp::Swap<valsize, big_endian>::readval(wv);
    value = value + addend - address;
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  // Like the above but for relocs at unaligned addresses.
  template<int valsize>
  static inline Reloc_status
  pcrel_unaligned(unsigned char* view, Address value, Address address,
		  Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype addend = elfcpp::Swap_unaligned<valsize, big_endian>::readval(view);
    value = value + addend - address;
    elfcpp::Swap_unaligned<valsize, big_endian>::writeval(view, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple PC relative relocation with a Symbol_value with the
  // addend in the section contents.  VALSIZE is the size of the
  // value.
  template<int valsize>
  static inline Reloc_status
  pcrel(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	Address address,
	Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Valtype addend = elfcpp::Swap<valsize, big_endian>::readval(wv);
    Address value = psymval->value(object, addend) - address;
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple PC relative relocation with the addend in the
  // relocation.  VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  pcrela(unsigned char* view, Address value, Addendtype addend, Address address,
	 Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    value = value + addend - address;
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  // Do a simple PC relative relocation with a Symbol_value with the
  // addend in the relocation.  VALSIZE is the size of the value.
  template<int valsize>
  static inline Reloc_status
  pcrela(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 Addendtype addend,
	 Address address,
	 Overflow_check check)
  {
    typedef typename elfcpp::Swap<valsize, big_endian>::Valtype Valtype;
    Valtype* wv = reinterpret_cast<Valtype*>(view);
    Address value = psymval->value(object, addend) - address;
    elfcpp::Swap<valsize, big_endian>::writeval(wv, value);
    return check_overflow<valsize>(value, check);
  }

  typedef Relocate_functions<size, big_endian> This;

 public:
  // Do a simple 8-bit REL relocation with the addend in the section
  // contents.
  static inline void
  rel8(unsigned char* view, Address value)
  { This::template rel<8>(view, value, CHECK_NONE); }

  static inline Reloc_status
  rel8_check(unsigned char* view, Address value, Overflow_check check)
  { return This::template rel<8>(view, value, check); }

  static inline void
  rel8(unsigned char* view,
       const Sized_relobj_file<size, big_endian>* object,
       const Symbol_value<size>* psymval)
  { This::template rel<8>(view, object, psymval, CHECK_NONE); }

  static inline Reloc_status
  rel8_check(unsigned char* view,
	     const Sized_relobj_file<size, big_endian>* object,
	     const Symbol_value<size>* psymval,
	     Overflow_check check)
  { return This::template rel<8>(view, object, psymval, check); }

  // Do an 8-bit RELA relocation with the addend in the relocation.
  static inline void
  rela8(unsigned char* view, Address value, Addendtype addend)
  { This::template rela<8>(view, value, addend, CHECK_NONE); }

  static inline Reloc_status
  rela8_check(unsigned char* view, Address value, Addendtype addend,
	      Overflow_check check)
  { return This::template rela<8>(view, value, addend, check); }

  static inline void
  rela8(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval,
	Addendtype addend)
  { This::template rela<8>(view, object, psymval, addend, CHECK_NONE); }

  static inline Reloc_status
  rela8_check(unsigned char* view,
	      const Sized_relobj_file<size, big_endian>* object,
	      const Symbol_value<size>* psymval,
	      Addendtype addend,
	      Overflow_check check)
  { return This::template rela<8>(view, object, psymval, addend, check); }

  // Do a simple 8-bit PC relative relocation with the addend in the
  // section contents.
  static inline void
  pcrel8(unsigned char* view, unsigned char value, Address address)
  { This::template pcrel<8>(view, value, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel8_check(unsigned char* view, unsigned char value, Address address,
	       Overflow_check check)
  { return This::template pcrel<8>(view, value, address, check); }

  static inline void
  pcrel8(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 Address address)
  { This::template pcrel<8>(view, object, psymval, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel8_check(unsigned char* view,
	       const Sized_relobj_file<size, big_endian>* object,
	       const Symbol_value<size>* psymval,
	       Address address,
	       Overflow_check check)
  { return This::template pcrel<8>(view, object, psymval, address, check); }

  // Do a simple 8-bit PC relative RELA relocation with the addend in
  // the reloc.
  static inline void
  pcrela8(unsigned char* view, Address value, Addendtype addend,
	  Address address)
  { This::template pcrela<8>(view, value, addend, address, CHECK_NONE); }

  static inline Reloc_status
  pcrela8_check(unsigned char* view, Address value, Addendtype addend,
		Address address, Overflow_check check)
  { return This::template pcrela<8>(view, value, addend, address, check); }

  static inline void
  pcrela8(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  Addendtype addend,
	  Address address)
  { This::template pcrela<8>(view, object, psymval, addend, address,
			     CHECK_NONE); }

  static inline Reloc_status
  pcrela8_check(unsigned char* view,
		const Sized_relobj_file<size, big_endian>* object,
		const Symbol_value<size>* psymval,
		Addendtype addend,
		Address address,
		Overflow_check check)
  { return This::template pcrela<8>(view, object, psymval, addend, address,
				    check); }

  // Do a simple 16-bit REL relocation with the addend in the section
  // contents.
  static inline void
  rel16(unsigned char* view, Address value)
  { This::template rel<16>(view, value, CHECK_NONE); }

  static inline Reloc_status
  rel16_check(unsigned char* view, Address value, Overflow_check check)
  { return This::template rel<16>(view, value, check); }

  static inline void
  rel16(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval)
  { This::template rel<16>(view, object, psymval, CHECK_NONE); }

  static inline Reloc_status
  rel16_check(unsigned char* view,
	      const Sized_relobj_file<size, big_endian>* object,
	      const Symbol_value<size>* psymval,
	      Overflow_check check)
  { return This::template rel<16>(view, object, psymval, check); }

  // Do an 16-bit RELA relocation with the addend in the relocation.
  static inline void
  rela16(unsigned char* view, Address value, Addendtype addend)
  { This::template rela<16>(view, value, addend, CHECK_NONE); }

  static inline Reloc_status
  rela16_check(unsigned char* view, Address value, Addendtype addend,
	       Overflow_check check)
  { return This::template rela<16>(view, value, addend, check); }

  static inline void
  rela16(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 Addendtype addend)
  { This::template rela<16>(view, object, psymval, addend, CHECK_NONE); }

  static inline Reloc_status
  rela16_check(unsigned char* view,
	       const Sized_relobj_file<size, big_endian>* object,
	       const Symbol_value<size>* psymval,
	       Addendtype addend,
	       Overflow_check check)
  { return This::template rela<16>(view, object, psymval, addend, check); }

  // Do a simple 16-bit PC relative REL relocation with the addend in
  // the section contents.
  static inline void
  pcrel16(unsigned char* view, Address value, Address address)
  { This::template pcrel<16>(view, value, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel16_check(unsigned char* view, Address value, Address address,
		Overflow_check check)
  { return This::template pcrel<16>(view, value, address, check); }

  static inline void
  pcrel16(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  Address address)
  { This::template pcrel<16>(view, object, psymval, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel16_check(unsigned char* view,
		const Sized_relobj_file<size, big_endian>* object,
		const Symbol_value<size>* psymval,
		Address address,
		Overflow_check check)
  { return This::template pcrel<16>(view, object, psymval, address, check); }

  // Do a simple 16-bit PC relative RELA relocation with the addend in
  // the reloc.
  static inline void
  pcrela16(unsigned char* view, Address value, Addendtype addend,
	   Address address)
  { This::template pcrela<16>(view, value, addend, address, CHECK_NONE); }

  static inline Reloc_status
  pcrela16_check(unsigned char* view, Address value, Addendtype addend,
		 Address address, Overflow_check check)
  { return This::template pcrela<16>(view, value, addend, address, check); }

  static inline void
  pcrela16(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   Addendtype addend,
	   Address address)
  { This::template pcrela<16>(view, object, psymval, addend, address,
			      CHECK_NONE); }

  static inline Reloc_status
  pcrela16_check(unsigned char* view,
		 const Sized_relobj_file<size, big_endian>* object,
		 const Symbol_value<size>* psymval,
		 Addendtype addend,
		 Address address,
		 Overflow_check check)
  { return This::template pcrela<16>(view, object, psymval, addend, address,
				     check); }

  // Do a simple 32-bit REL relocation with the addend in the section
  // contents.
  static inline void
  rel32(unsigned char* view, Address value)
  { This::template rel<32>(view, value, CHECK_NONE); }

  static inline Reloc_status
  rel32_check(unsigned char* view, Address value, Overflow_check check)
  { return This::template rel<32>(view, value, check); }

  // Like above but for relocs at unaligned addresses.
  static inline void
  rel32_unaligned(unsigned char* view, Address value)
  { This::template rel_unaligned<32>(view, value, CHECK_NONE); }

  static inline Reloc_status
  rel32_unaligned_check(unsigned char* view, Address value,
			Overflow_check check)
  { return This::template rel_unaligned<32>(view, value, check); }

  static inline void
  rel32(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval)
  { This::template rel<32>(view, object, psymval, CHECK_NONE); }

  static inline Reloc_status
  rel32_check(unsigned char* view,
	      const Sized_relobj_file<size, big_endian>* object,
	      const Symbol_value<size>* psymval,
	      Overflow_check check)
  { return This::template rel<32>(view, object, psymval, check); }

  // Like above but for relocs at unaligned addresses.
  static inline void
  rel32_unaligned(unsigned char* view,
	          const Sized_relobj_file<size, big_endian>* object,
	          const Symbol_value<size>* psymval)
  { This::template rel_unaligned<32>(view, object, psymval, CHECK_NONE); }

  static inline Reloc_status
  rel32_unaligned_check(unsigned char* view,
			const Sized_relobj_file<size, big_endian>* object,
			const Symbol_value<size>* psymval,
			Overflow_check check)
  { return This::template rel_unaligned<32>(view, object, psymval, check); }

  // Do a 32-bit RELA relocation with the addend in the relocation.
  static inline void
  rela32(unsigned char* view, Address value, Addendtype addend)
  { This::template rela<32>(view, value, addend, CHECK_NONE); }

  static inline Reloc_status
  rela32(unsigned char* view, Address value, Addendtype addend,
	 Overflow_check check)
  { return This::template rela<32>(view, value, addend, check); }

  static inline void
  rela32(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 Addendtype addend)
  { This::template rela<32>(view, object, psymval, addend, CHECK_NONE); }

  static inline Reloc_status
  rela32_check(unsigned char* view,
	       const Sized_relobj_file<size, big_endian>* object,
	       const Symbol_value<size>* psymval,
	       Addendtype addend,
	       Overflow_check check)
  { return This::template rela<32>(view, object, psymval, addend, check); }

  // Do a simple 32-bit PC relative REL relocation with the addend in
  // the section contents.
  static inline void
  pcrel32(unsigned char* view, Address value, Address address)
  { This::template pcrel<32>(view, value, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel32_check(unsigned char* view, Address value, Address address,
		Overflow_check check)
  { return This::template pcrel<32>(view, value, address, check); }

  // Unaligned version of the above.
  static inline void
  pcrel32_unaligned(unsigned char* view, Address value, Address address)
  { This::template pcrel_unaligned<32>(view, value, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel32_unaligned_check(unsigned char* view, Address value, Address address,
			  Overflow_check check)
  { return This::template pcrel_unaligned<32>(view, value, address, check); }

  static inline void
  pcrel32(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  Address address)
  { This::template pcrel<32>(view, object, psymval, address, CHECK_NONE); }

  static inline Reloc_status
  pcrel32_check(unsigned char* view,
		const Sized_relobj_file<size, big_endian>* object,
		const Symbol_value<size>* psymval,
		Address address,
		Overflow_check check)
  { return This::template pcrel<32>(view, object, psymval, address, check); }

  // Do a simple 32-bit PC relative RELA relocation with the addend in
  // the relocation.
  static inline void
  pcrela32(unsigned char* view, Address value, Addendtype addend,
           Address address)
  { This::template pcrela<32>(view, value, addend, address, CHECK_NONE); }

  static inline Reloc_status
  pcrela32_check(unsigned char* view, Address value, Addendtype addend,
           Address address, Overflow_check check)
  { return This::template pcrela<32>(view, value, addend, address, check); }

  static inline void
  pcrela32(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   Addendtype addend,
	   Address address)
  { This::template pcrela<32>(view, object, psymval, addend, address,
			      CHECK_NONE); }

  static inline Reloc_status
  pcrela32_check(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   Addendtype addend,
	   Address address,
	   Overflow_check check)
  { return This::template pcrela<32>(view, object, psymval, addend, address,
				     check); }

  // Do a simple 64-bit REL relocation with the addend in the section
  // contents.
  static inline void
  rel64(unsigned char* view, Address value)
  { This::template rel<64>(view, value, CHECK_NONE); }

  static inline void
  rel64(unsigned char* view,
	const Sized_relobj_file<size, big_endian>* object,
	const Symbol_value<size>* psymval)
  { This::template rel<64>(view, object, psymval, CHECK_NONE); }

  // Do a 64-bit RELA relocation with the addend in the relocation.
  static inline void
  rela64(unsigned char* view, Address value, Addendtype addend)
  { This::template rela<64>(view, value, addend, CHECK_NONE); }

  static inline void
  rela64(unsigned char* view,
	 const Sized_relobj_file<size, big_endian>* object,
	 const Symbol_value<size>* psymval,
	 Addendtype addend)
  { This::template rela<64>(view, object, psymval, addend, CHECK_NONE); }

  // Do a simple 64-bit PC relative REL relocation with the addend in
  // the section contents.
  static inline void
  pcrel64(unsigned char* view, Address value, Address address)
  { This::template pcrel<64>(view, value, address, CHECK_NONE); }

  static inline void
  pcrel64(unsigned char* view,
	  const Sized_relobj_file<size, big_endian>* object,
	  const Symbol_value<size>* psymval,
	  Address address)
  { This::template pcrel<64>(view, object, psymval, address, CHECK_NONE); }

  // Do a simple 64-bit PC relative RELA relocation with the addend in
  // the relocation.
  static inline void
  pcrela64(unsigned char* view, Address value, Addendtype addend,
	   Address address)
  { This::template pcrela<64>(view, value, addend, address, CHECK_NONE); }

  static inline void
  pcrela64(unsigned char* view,
	   const Sized_relobj_file<size, big_endian>* object,
	   const Symbol_value<size>* psymval,
	   Addendtype addend,
	   Address address)
  { This::template pcrela<64>(view, object, psymval, addend, address,
			      CHECK_NONE); }
};

// Convenience class for min and max values of a given BITS length.

template<int bits>
class Limits
{
 public:
  static const uint64_t MAX_UNSIGNED = (1ULL << bits) - 1;
  static const int64_t MAX_SIGNED = MAX_UNSIGNED >> 1;
  static const int64_t MIN_SIGNED = -MAX_SIGNED - 1;
};

template<>
class Limits<64>
{
 public:
  static const uint64_t MAX_UNSIGNED = ~0ULL;
  static const int64_t MAX_SIGNED = MAX_UNSIGNED >> 1;
  static const int64_t MIN_SIGNED = -MAX_SIGNED - 1;
};

// Integer manipulation functions used by various targets when
// performing relocations.

template<int bits>
class Bits
{
 public:
  // Sign extend an n-bit unsigned integer stored in a uint32_t into
  // an int32_t.  BITS must be between 1 and 32.
  static inline int32_t
  sign_extend32(uint32_t val)
  {
    gold_assert(bits > 0 && bits <= 32);
    if (bits == 32)
      return static_cast<int32_t>(val);
    uint32_t mask = (~static_cast<uint32_t>(0)) >> (32 - bits);
    val &= mask;
    uint32_t top_bit = 1U << (bits - 1);
    int32_t as_signed = static_cast<int32_t>(val);
    if ((val & top_bit) != 0)
      as_signed -= static_cast<int32_t>(top_bit * 2);
    return as_signed;    
  }

  // Return true if VAL (stored in a uint32_t) has overflowed a signed
  // value with BITS bits.
  static inline bool
  has_overflow32(uint32_t val)
  {
    gold_assert(bits > 0 && bits <= 32);
    if (bits == 32)
      return false;
    const int32_t max = static_cast<int32_t>(Limits<bits>::MAX_SIGNED);
    const int32_t min = static_cast<int32_t>(Limits<bits>::MIN_SIGNED);
    int32_t as_signed = static_cast<int32_t>(val);
    return as_signed > max || as_signed < min;
  }

  // Return true if VAL (stored in a uint32_t) has overflowed an unsigned
  // value with BITS bits.
  static inline bool
  has_unsigned_overflow32(uint32_t val)
  {
    gold_assert(bits > 0 && bits <= 32);
    if (bits == 32)
      return false;
    const uint32_t max = static_cast<uint32_t>(Limits<bits>::MAX_UNSIGNED);
    return val > max;
  }

  // Return true if VAL (stored in a uint32_t) has overflowed both a
  // signed and an unsigned value.  E.g.,
  // Bits<8>::has_signed_unsigned_overflow32 would check -128 <= VAL <
  // 255.
  static inline bool
  has_signed_unsigned_overflow32(uint32_t val)
  {
    gold_assert(bits > 0 && bits <= 32);
    if (bits == 32)
      return false;
    const int32_t max = static_cast<int32_t>(Limits<bits>::MAX_UNSIGNED);
    const int32_t min = static_cast<int32_t>(Limits<bits>::MIN_SIGNED);
    int32_t as_signed = static_cast<int32_t>(val);
    return as_signed > max || as_signed < min;
  }

  // Select bits from A and B using bits in MASK.  For each n in
  // [0..31], the n-th bit in the result is chosen from the n-th bits
  // of A and B.  A zero selects A and a one selects B.
  static inline uint32_t
  bit_select32(uint32_t a, uint32_t b, uint32_t mask)
  { return (a & ~mask) | (b & mask); }

  // Sign extend an n-bit unsigned integer stored in a uint64_t into
  // an int64_t.  BITS must be between 1 and 64.
  static inline int64_t
  sign_extend(uint64_t val)
  {
    gold_assert(bits > 0 && bits <= 64);
    if (bits == 64)
      return static_cast<int64_t>(val);
    uint64_t mask = (~static_cast<uint64_t>(0)) >> (64 - bits);
    val &= mask;
    uint64_t top_bit = static_cast<uint64_t>(1) << (bits - 1);
    int64_t as_signed = static_cast<int64_t>(val);
    if ((val & top_bit) != 0)
      as_signed -= static_cast<int64_t>(top_bit * 2);
    return as_signed;    
  }

  // Return true if VAL (stored in a uint64_t) has overflowed a signed
  // value with BITS bits.
  static inline bool
  has_overflow(uint64_t val)
  {
    gold_assert(bits > 0 && bits <= 64);
    if (bits == 64)
      return false;
    const int64_t max = Limits<bits>::MAX_SIGNED;
    const int64_t min = Limits<bits>::MIN_SIGNED;
    int64_t as_signed = static_cast<int64_t>(val);
    return as_signed > max || as_signed < min;
  }

  // Return true if VAL (stored in a uint64_t) has overflowed an unsigned
  // value with BITS bits.
  static inline bool
  has_unsigned_overflow(uint64_t val)
  {
    gold_assert(bits > 0 && bits <= 64);
    if (bits == 64)
      return false;
    const uint64_t max = Limits<bits>::MAX_UNSIGNED;
    return val > max;
  }

  // Return true if VAL (stored in a uint64_t) has overflowed both a
  // signed and an unsigned value.  E.g.,
  // Bits<8>::has_signed_unsigned_overflow would check -128 <= VAL <
  // 255.
  static inline bool
  has_signed_unsigned_overflow64(uint64_t val)
  {
    gold_assert(bits > 0 && bits <= 64);
    if (bits == 64)
      return false;
    const int64_t max = static_cast<int64_t>(Limits<bits>::MAX_UNSIGNED);
    const int64_t min = Limits<bits>::MIN_SIGNED;
    int64_t as_signed = static_cast<int64_t>(val);
    return as_signed > max || as_signed < min;
  }

  // Select bits from A and B using bits in MASK.  For each n in
  // [0..31], the n-th bit in the result is chosen from the n-th bits
  // of A and B.  A zero selects A and a one selects B.
  static inline uint64_t
  bit_select64(uint64_t a, uint64_t b, uint64_t mask)
  { return (a & ~mask) | (b & mask); }
};

// Track relocations while reading a section.  This lets you ask for
// the relocation at a certain offset, and see how relocs occur
// between points of interest.

template<int size, bool big_endian>
class Track_relocs
{
 public:
  Track_relocs()
    : prelocs_(NULL), len_(0), pos_(0), reloc_size_(0)
  { }

  // Initialize the Track_relocs object.  OBJECT is the object holding
  // the reloc section, RELOC_SHNDX is the section index of the reloc
  // section, and RELOC_TYPE is the type of the reloc section
  // (elfcpp::SHT_REL or elfcpp::SHT_RELA).  This returns false if
  // something went wrong.
  bool
  initialize(Object* object, unsigned int reloc_shndx,
	     unsigned int reloc_type);

  // Return the offset in the data section to which the next reloc
  // applies.  This returns -1 if there is no next reloc.
  off_t
  next_offset() const;

  // Return the symbol index of the next reloc.  This returns -1U if
  // there is no next reloc.
  unsigned int
  next_symndx() const;

  // Return the addend of the next reloc.  This returns 0 if there is
  // no next reloc.
  uint64_t
  next_addend() const;

  // Advance to OFFSET within the data section, and return the number
  // of relocs which would be skipped, excluding r_info==0 relocs.
  int
  advance(off_t offset);

  // Checkpoint the current position in the reloc section.
  section_size_type
  checkpoint() const
  { return this->pos_; }

  // Reset the position to CHECKPOINT.
  void
  reset(section_size_type checkpoint)
  { this->pos_ = checkpoint; }

 private:
  // The contents of the input object's reloc section.
  const unsigned char* prelocs_;
  // The length of the reloc section.
  section_size_type len_;
  // Our current position in the reloc section.
  section_size_type pos_;
  // The size of the relocs in the section.
  int reloc_size_;
};

} // End namespace gold.

#endif // !defined(GOLD_RELOC_H)
