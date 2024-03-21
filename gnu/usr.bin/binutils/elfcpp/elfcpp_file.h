// elfcpp_file.h -- file access for elfcpp   -*- C++ -*-

// Copyright (C) 2006-2023 Free Software Foundation, Inc.
// Written by Ian Lance Taylor <iant@google.com>.

// This file is part of elfcpp.
   
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

// This header file defines the class Elf_file which can be used to
// read useful data from an ELF file.  The functions here are all
// templates which take a file interface object as a parameter.  This
// type must have a subtype View.  This type must support two methods:
//     View view(off_t file_offset, off_t data_size)
// returns a View for the specified part of the file.
//     void error(const char* printf_format, ...)
// prints an error message and does not return.  The subtype View must
// support a method
//     const unsigned char* data()
// which returns a pointer to a buffer containing the requested data.
// This general interface is used to read data from the file.  Objects
// of type View will never survive longer than the elfcpp function.

// Some of these functions must return a reference to part of the
// file.  To use these, the file interface must support a subtype
// Location:
//    Location(off_t file_offset, off_t data_size)
// To use this in conjunction with the accessors types Shdr, etc., the
// file interface should support an overload of view:
//    View view(Location)
// This permits writing
//    elfcpp::Shdr shdr(file, ef.section_header(n));

#ifndef ELFCPP_FILE_H
#define ELFCPP_FILE_H

#include <string>
#include <cstdio>
#include <cstring>

#include "elfcpp.h"

namespace elfcpp
{

// A simple helper class to recognize if a file has an ELF header.

class Elf_recognizer
{
 public:
  // Maximum header size.  The user should try to read this much of
  // the file when using this class.

  static const int max_header_size = Elf_sizes<64>::ehdr_size;

  // Checks if the file contains the ELF magic.  Other header fields
  // are not checked.

  static bool
  is_elf_file(const unsigned char* ehdr_buf, int size);

  // Check if EHDR_BUF/BUFSIZE is a valid header of a 32-bit or
  // 64-bit, little-endian or big-endian ELF file.  Assumes
  // is_elf_file() has been checked to be true.  If the header is not
  // valid, *ERROR contains a human-readable error message.  If is is,
  // *SIZE is set to either 32 or 64, *BIG_ENDIAN is set to indicate
  // whether the file is big-endian.

  static bool
  is_valid_header(const unsigned char* ehdr_buf, off_t bufsize,
                  int* size, bool* big_endian,
                  std::string* error);
};

// This object is used to read an ELF file.
//   SIZE: The size of file, 32 or 64.
//   BIG_ENDIAN: Whether the file is in big-endian format.
//   FILE: A file reading type as described above.

template<int size, bool big_endian, typename File>
class Elf_file
{
 private:
  typedef Elf_file<size, big_endian, File> This;

 public:
  static const int ehdr_size = Elf_sizes<size>::ehdr_size;
  static const int phdr_size = Elf_sizes<size>::phdr_size;
  static const int shdr_size = Elf_sizes<size>::shdr_size;
  static const int sym_size = Elf_sizes<size>::sym_size;
  static const int rel_size = Elf_sizes<size>::rel_size;
  static const int rela_size = Elf_sizes<size>::rela_size;

  typedef Ehdr<size, big_endian> Ef_ehdr;
  typedef Phdr<size, big_endian> Ef_phdr;
  typedef Shdr<size, big_endian> Ef_shdr;
  typedef Sym<size, big_endian> Ef_sym;

  // Construct an Elf_file given an ELF file header.
  Elf_file(File* file, const Ef_ehdr& ehdr)
  { this->construct(file, ehdr); }

  // Construct an ELF file.
  inline
  Elf_file(File* file);

  // Return the file offset to the section headers.
  off_t
  shoff() const
  { return this->shoff_; }

  // Find the first section with an sh_type field equal to TYPE and
  // return its index.  Returns SHN_UNDEF if there is no such section.
  unsigned int
  find_section_by_type(unsigned int type);

  // Return the number of sections.
  unsigned int
  shnum()
  {
    this->initialize_shnum();
    return this->shnum_;
  }

  unsigned int
  shnum() const
  {
    if (this->shnum_ == 0 && this->shoff_ != 0)
      this->file_->error(_("ELF file has not been initialized yet"
			   " (internal error)"));
    return this->shnum_;
  }

  // Return the section index of the section name string table.
  unsigned int
  shstrndx()
  {
    this->initialize_shnum();
    return this->shstrndx_;
  }

  unsigned int
  shstrndx() const
  {
    if (this->shstrndx_ == SHN_XINDEX && this->shoff_ != 0)
      {
	this->file_->error(_("ELF file has not been initialized yet"
			     " (internal error)"));
	return 0;
      }
    return this->shstrndx_;
  }

  // Return the value to subtract from section indexes >=
  // SHN_LORESERVE.  See the comment in initialize_shnum.
  int
  large_shndx_offset()
  {
    this->initialize_shnum();
    return this->large_shndx_offset_;
  }

  int
  large_shndx_offset() const
  {
    if (this->shstrndx_ == SHN_XINDEX && this->shoff_ != 0)
      this->file_->error(_("ELF file has not been initialized yet"
			   " (internal error)"));
    return this->large_shndx_offset_;
  }

  // Return the location of the header of section SHNDX.
  typename File::Location
  section_header(unsigned int shndx)
  {
    return typename File::Location(this->section_header_offset(shndx),
				   shdr_size);
  }

  // Return the name of section SHNDX.
  std::string
  section_name(unsigned int shndx) const;

  // Return the location of the contents of section SHNDX.
  typename File::Location
  section_contents(unsigned int shndx);

  // Return the size of section SHNDX.
  typename Elf_types<size>::Elf_WXword
  section_size(unsigned int shndx);

  // Return the flags of section SHNDX.
  typename Elf_types<size>::Elf_WXword
  section_flags(unsigned int shndx);

  // Return the address of section SHNDX.
  typename Elf_types<size>::Elf_Addr
  section_addr(unsigned int shndx);

  // Return the type of section SHNDX.
  Elf_Word
  section_type(unsigned int shndx);

  // Return the link field of section SHNDX.
  Elf_Word
  section_link(unsigned int shndx);

  // Return the info field of section SHNDX.
  Elf_Word
  section_info(unsigned int shndx);

  // Return the addralign field of section SHNDX.
  typename Elf_types<size>::Elf_WXword
  section_addralign(unsigned int shndx);

 private:
  // Shared constructor code.
  void
  construct(File* file, const Ef_ehdr& ehdr);

  // Initialize shnum_ and shstrndx_.
  void
  initialize_shnum();

  // Return the file offset of the header of section SHNDX.
  off_t
  section_header_offset(unsigned int shndx) const;

  // The file we are reading.
  File* file_;
  // The file offset to the section headers.
  off_t shoff_;
  // The number of sections.
  unsigned int shnum_;
  // The section index of the section name string table.
  unsigned int shstrndx_;
  // Offset to add to sections larger than SHN_LORESERVE.
  int large_shndx_offset_;
};

// A small wrapper around SHT_STRTAB data mapped to memory. It checks that the
// index is not out of bounds and the string is NULL-terminated.

class Elf_strtab
{
 public:
  // Construct an Elf_strtab for a section with contents *P and size SIZE.
  Elf_strtab(const unsigned char* p, size_t size);

  // Return the file offset to the section headers.
  bool
  get_c_string(size_t offset, const char** cstring) const
  {
    if (offset >= this->usable_size_)
      return false;
    *cstring = this->base_ + offset;
    return true;
  }

 private:
  // Contents of the section mapped to memory.
  const char* base_;
  // One larger that the position of the last NULL character in the section.
  // For valid SHT_STRTAB sections, this is the size of the section.
  size_t usable_size_;
};

// Inline function definitions.

// Check for presence of the ELF magic number.

inline bool
Elf_recognizer::is_elf_file(const unsigned char* ehdr_buf, int size)
{
  if (size < 4)
    return false;

  static unsigned char elfmagic[4] =
    {
      elfcpp::ELFMAG0, elfcpp::ELFMAG1,
      elfcpp::ELFMAG2, elfcpp::ELFMAG3
    };
  return memcmp(ehdr_buf, elfmagic, 4) == 0;
}

namespace
{

// Print a number to a string.

inline std::string
internal_printf_int(const char* format, int arg)
{
  char buf[256];
  snprintf(buf, sizeof(buf), format, arg);
  return std::string(buf);
}

}  // End anonymous namespace.

// Check the validity of the ELF header.

inline bool
Elf_recognizer::is_valid_header(
    const unsigned char* ehdr_buf,
    off_t bufsize,
    int* size,
    bool* big_endian,
    std::string* error)
{
  if (bufsize < elfcpp::EI_NIDENT)
    {
      *error = _("ELF file too short");
      return false;
    }

  int v = ehdr_buf[elfcpp::EI_VERSION];
  if (v != elfcpp::EV_CURRENT)
    {
      if (v == elfcpp::EV_NONE)
        *error = _("invalid ELF version 0");
      else
        *error = internal_printf_int(_("unsupported ELF version %d"), v);
      return false;
    }

  int c = ehdr_buf[elfcpp::EI_CLASS];
  if (c == elfcpp::ELFCLASSNONE)
    {
      *error = _("invalid ELF class 0");
      return false;
    }
  else if (c != elfcpp::ELFCLASS32
           && c != elfcpp::ELFCLASS64)
    {
      *error = internal_printf_int(_("unsupported ELF class %d"), c);
      return false;
    }

  int d = ehdr_buf[elfcpp::EI_DATA];
  if (d == elfcpp::ELFDATANONE)
    {
      *error = _("invalid ELF data encoding");
      return false;
    }
  else if (d != elfcpp::ELFDATA2LSB
           && d != elfcpp::ELFDATA2MSB)
    {
      *error = internal_printf_int(_("unsupported ELF data encoding %d"), d);
      return false;
    }

  *big_endian = (d == elfcpp::ELFDATA2MSB);

  if (c == elfcpp::ELFCLASS32)
    {
      if (bufsize < elfcpp::Elf_sizes<32>::ehdr_size)
        {
          *error = _("ELF file too short");
          return false;
        }
      *size = 32;
    }
  else
    {
      if (bufsize < elfcpp::Elf_sizes<64>::ehdr_size)
        {
          *error = _("ELF file too short");
          return false;
        }
      *size = 64;
    }

  return true;
}

// Template function definitions.

// Construct an Elf_file given an ELF file header.

template<int size, bool big_endian, typename File>
void
Elf_file<size, big_endian, File>::construct(File* file, const Ef_ehdr& ehdr)
{
  this->file_ = file;
  this->shoff_ = ehdr.get_e_shoff();
  this->shnum_ = ehdr.get_e_shnum();
  this->shstrndx_ = ehdr.get_e_shstrndx();
  this->large_shndx_offset_ = 0;
  if (ehdr.get_e_ehsize() != This::ehdr_size)
    file->error(_("bad e_ehsize (%d != %d)"),
		ehdr.get_e_ehsize(), This::ehdr_size);
  if (ehdr.get_e_shentsize() != This::shdr_size)
    file->error(_("bad e_shentsize (%d != %d)"),
		ehdr.get_e_shentsize(), This::shdr_size);
}

// Construct an ELF file.

template<int size, bool big_endian, typename File>
inline
Elf_file<size, big_endian, File>::Elf_file(File* file)
{
  typename File::View v(file->view(file_header_offset, This::ehdr_size));
  this->construct(file, Ef_ehdr(v.data()));
}

// Initialize the shnum_ and shstrndx_ fields, handling overflow.

template<int size, bool big_endian, typename File>
void
Elf_file<size, big_endian, File>::initialize_shnum()
{
  if ((this->shnum_ == 0 || this->shstrndx_ == SHN_XINDEX)
      && this->shoff_ != 0)
    {
      typename File::View v(this->file_->view(this->shoff_, This::shdr_size));
      Ef_shdr shdr(v.data());

      if (this->shnum_ == 0)
	this->shnum_ = shdr.get_sh_size();

      if (this->shstrndx_ == SHN_XINDEX)
	{
	  this->shstrndx_ = shdr.get_sh_link();

	  // Versions of the GNU binutils between 2.12 and 2.18 did
	  // not handle objects with more than SHN_LORESERVE sections
	  // correctly.  All large section indexes were offset by
	  // 0x100.  Some information can be found here:
	  // http://sourceware.org/bugzilla/show_bug.cgi?id=5900 .
	  // Fortunately these object files are easy to detect, as the
	  // GNU binutils always put the section header string table
	  // near the end of the list of sections.  Thus if the
	  // section header string table index is larger than the
	  // number of sections, then we know we have to subtract
	  // 0x100 to get the real section index.
	  if (this->shstrndx_ >= this->shnum_)
	    {
	      if (this->shstrndx_ >= elfcpp::SHN_LORESERVE + 0x100)
		{
		  this->large_shndx_offset_ = - 0x100;
		  this->shstrndx_ -= 0x100;
		}
	      if (this->shstrndx_ >= this->shnum_)
		this->file_->error(_("bad shstrndx: %u >= %u"),
				   this->shstrndx_, this->shnum_);
	    }
	}
    }
}

// Find section with sh_type equal to TYPE and return its index.
// Returns SHN_UNDEF if not found.

template<int size, bool big_endian, typename File>
unsigned int
Elf_file<size, big_endian, File>::find_section_by_type(unsigned int type)
{
  unsigned int shnum = this->shnum();
  typename File::View v(this->file_->view(this->shoff_,
					  This::shdr_size * shnum));
  for (unsigned int i = 0; i < shnum; i++)
    {
      Ef_shdr shdr(v.data() + This::shdr_size * i);
      if (shdr.get_sh_type() == type)
        return i;
    }
  return SHN_UNDEF;
}

// Return the file offset of the section header of section SHNDX.

template<int size, bool big_endian, typename File>
off_t
Elf_file<size, big_endian, File>::section_header_offset(unsigned int shndx) const
{
  if (shndx >= this->shnum())
    this->file_->error(_("section_header_offset: bad shndx %u >= %u"),
		       shndx, this->shnum());
  return this->shoff_ + This::shdr_size * shndx;
}

// Return the name of section SHNDX.

template<int size, bool big_endian, typename File>
std::string
Elf_file<size, big_endian, File>::section_name(unsigned int shndx) const
{
  File* const file = this->file_;

  // Get the section name offset.
  unsigned int sh_name;
  {
    typename File::View v(file->view(this->section_header_offset(shndx),
				     This::shdr_size));
    Ef_shdr shdr(v.data());
    sh_name = shdr.get_sh_name();
  }

  // Get the file offset for the section name string table data.
  off_t shstr_off;
  typename Elf_types<size>::Elf_WXword shstr_size;
  {
    const unsigned int shstrndx = this->shstrndx_;
    typename File::View v(file->view(this->section_header_offset(shstrndx),
				     This::shdr_size));
    Ef_shdr shstr_shdr(v.data());
    shstr_off = shstr_shdr.get_sh_offset();
    shstr_size = shstr_shdr.get_sh_size();
  }

  if (sh_name >= shstr_size)
    file->error(_("bad section name offset for section %u: %u"),
		shndx, sh_name);

  typename File::View v(file->view(shstr_off, shstr_size));

  const unsigned char* datau = v.data();
  const char* data = reinterpret_cast<const char*>(datau);
  const void* p = ::memchr(data + sh_name, '\0', shstr_size - sh_name);
  if (p == NULL)
    file->error(_("missing null terminator for name of section %u"),
		shndx);

  size_t len = static_cast<const char*>(p) - (data + sh_name);

  return std::string(data + sh_name, len);
}

// Return the contents of section SHNDX.

template<int size, bool big_endian, typename File>
typename File::Location
Elf_file<size, big_endian, File>::section_contents(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_contents: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));
  Ef_shdr shdr(v.data());
  return typename File::Location(shdr.get_sh_offset(), shdr.get_sh_size());
}

// Get the size of section SHNDX.

template<int size, bool big_endian, typename File>
typename Elf_types<size>::Elf_WXword
Elf_file<size, big_endian, File>::section_size(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_size: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_size();
}

// Return the section flags of section SHNDX.

template<int size, bool big_endian, typename File>
typename Elf_types<size>::Elf_WXword
Elf_file<size, big_endian, File>::section_flags(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_flags: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_flags();
}

// Return the address of section SHNDX.

template<int size, bool big_endian, typename File>
typename Elf_types<size>::Elf_Addr
Elf_file<size, big_endian, File>::section_addr(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_flags: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_addr();
}

// Return the type of section SHNDX.

template<int size, bool big_endian, typename File>
Elf_Word
Elf_file<size, big_endian, File>::section_type(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_type: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_type();
}

// Return the sh_link field of section SHNDX.

template<int size, bool big_endian, typename File>
Elf_Word
Elf_file<size, big_endian, File>::section_link(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_link: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_link();
}

// Return the sh_info field of section SHNDX.

template<int size, bool big_endian, typename File>
Elf_Word
Elf_file<size, big_endian, File>::section_info(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_info: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_info();
}

// Return the sh_addralign field of section SHNDX.

template<int size, bool big_endian, typename File>
typename Elf_types<size>::Elf_WXword
Elf_file<size, big_endian, File>::section_addralign(unsigned int shndx)
{
  File* const file = this->file_;

  if (shndx >= this->shnum())
    file->error(_("section_addralign: bad shndx %u >= %u"),
		shndx, this->shnum());

  typename File::View v(file->view(this->section_header_offset(shndx),
				   This::shdr_size));

  Ef_shdr shdr(v.data());
  return shdr.get_sh_addralign();
}

inline
Elf_strtab::Elf_strtab(const unsigned char* p, size_t size)
{
  // Check if the section is NUL-terminated. If it isn't, we ignore
  // the last part to make sure we don't return non-NUL-terminated
  // strings.
  while (size > 0 && p[size - 1] != 0)
    size--;
  this->base_ = reinterpret_cast<const char*>(p);
  this->usable_size_ = size;
}

} // End namespace elfcpp.

#endif // !defined(ELFCPP_FILE_H)
