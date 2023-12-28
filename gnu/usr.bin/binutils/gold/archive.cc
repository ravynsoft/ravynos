// archive.cc -- archive support for gold

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

#include <cerrno>
#include <cstring>
#include <climits>
#include <vector>
#include "libiberty.h"
#include "filenames.h"

#include "elfcpp.h"
#include "options.h"
#include "mapfile.h"
#include "fileread.h"
#include "readsyms.h"
#include "symtab.h"
#include "object.h"
#include "layout.h"
#include "archive.h"
#include "plugin.h"
#include "incremental.h"

namespace gold
{

// Library_base methods.

// Determine whether a definition of SYM_NAME should cause an archive
// library member to be included in the link.  Returns SHOULD_INCLUDE_YES
// if the symbol is referenced but not defined, SHOULD_INCLUDE_NO if the
// symbol is already defined, and SHOULD_INCLUDE_UNKNOWN if the symbol is
// neither referenced nor defined.

Library_base::Should_include
Library_base::should_include_member(Symbol_table* symtab, Layout* layout,
				    const char* sym_name, Symbol** symp,
				    std::string* why, char** tmpbufp,
				    size_t* tmpbuflen)
{
  // In an object file, and therefore in an archive map, an
  // '@' in the name separates the symbol name from the
  // version name.  If there are two '@' characters, this is
  // the default version.
  char* tmpbuf = *tmpbufp;
  const char* ver = strchr(sym_name, '@');
  bool def = false;
  if (ver != NULL)
    {
      size_t symlen = ver - sym_name;
      if (symlen + 1 > *tmpbuflen)
        {
          tmpbuf = static_cast<char*>(xrealloc(tmpbuf, symlen + 1));
          *tmpbufp = tmpbuf;
          *tmpbuflen = symlen + 1;
        }
      memcpy(tmpbuf, sym_name, symlen);
      tmpbuf[symlen] = '\0';
      sym_name = tmpbuf;

      ++ver;
      if (*ver == '@')
        {
          ++ver;
          def = true;
        }
    }

  Symbol* sym = symtab->lookup(sym_name, ver);
  if (def
      && ver != NULL
      && (sym == NULL
          || !sym->is_undefined()
          || sym->binding() == elfcpp::STB_WEAK))
    sym = symtab->lookup(sym_name, NULL);

  *symp = sym;

  if (sym != NULL)
    {
      if (!sym->is_undefined())
	return Library_base::SHOULD_INCLUDE_NO;

      // PR 12001: Do not include an archive when the undefined
      // symbol has actually been defined on the command line.
      if (layout->script_options()->is_pending_assignment(sym_name))
	return Library_base::SHOULD_INCLUDE_NO;

      // If the symbol is weak undefined, we still need to check
      // for other reasons (like a -u option).
      if (sym->binding() != elfcpp::STB_WEAK)
	return Library_base::SHOULD_INCLUDE_YES;
    }

  // Check whether the symbol was named in a -u option.
  if (parameters->options().is_undefined(sym_name))
    {
      *why = "-u ";
      *why += sym_name;
      return Library_base::SHOULD_INCLUDE_YES;
    }

  if (layout->script_options()->is_referenced(sym_name))
    {
      size_t alc = 100 + strlen(sym_name);
      char* buf = new char[alc];
      snprintf(buf, alc, _("script or expression reference to %s"),
	       sym_name);
      *why = buf;
      delete[] buf;
      return Library_base::SHOULD_INCLUDE_YES;
    }

  if (!parameters->options().relocatable())
    {
      const char* entry_sym = parameters->entry();
      if (entry_sym != NULL && strcmp(sym_name, entry_sym) == 0)
	{
	  *why = "entry symbol ";
	  *why += sym_name;
	  return Library_base::SHOULD_INCLUDE_YES;
	}
    }

  return Library_base::SHOULD_INCLUDE_UNKNOWN;
}

// The header of an entry in the archive.  This is all readable text,
// padded with spaces where necessary.  If the contents of an archive
// are all text file, the entire archive is readable.

struct Archive::Archive_header
{
  // The entry name.
  char ar_name[16];
  // The file modification time.
  char ar_date[12];
  // The user's UID in decimal.
  char ar_uid[6];
  // The user's GID in decimal.
  char ar_gid[6];
  // The file mode in octal.
  char ar_mode[8];
  // The file size in decimal.
  char ar_size[10];
  // The final magic code.
  char ar_fmag[2];
};

// Class Archive static variables.
unsigned int Archive::total_archives;
unsigned int Archive::total_members;
unsigned int Archive::total_members_loaded;

// Archive methods.

const char Archive::armag[sarmag] =
{
  '!', '<', 'a', 'r', 'c', 'h', '>', '\n'
};

const char Archive::armagt[sarmag] =
{
  '!', '<', 't', 'h', 'i', 'n', '>', '\n'
};

const char Archive::arfmag[2] = { '`', '\n' };

const char Archive::sym64name[7] = { '/', 'S', 'Y', 'M', '6', '4', '/' };

Archive::Archive(const std::string& name, Input_file* input_file,
                 bool is_thin_archive, Dirsearch* dirpath, Task* task)
  : Library_base(task), name_(name), input_file_(input_file), armap_(),
    armap_names_(), extended_names_(), armap_checked_(), seen_offsets_(),
    members_(), is_thin_archive_(is_thin_archive), included_member_(false),
    nested_archives_(), dirpath_(dirpath), num_members_(0),
    included_all_members_(false)
{
  this->no_export_ =
    parameters->options().check_excluded_libs(input_file->found_name());
}

// Set up the archive: read the symbol map and the extended name
// table.

void
Archive::setup()
{
  // We need to ignore empty archives.
  if (this->input_file_->file().filesize() == sarmag)
    return;

  // The first member of the archive should be the symbol table.
  std::string armap_name;
  off_t header_size = this->read_header(sarmag, false, &armap_name, NULL);
  if (header_size == -1)
    return;

  section_size_type armap_size = convert_to_section_size_type(header_size);
  off_t off = sarmag;
  if (armap_name.empty())
    {
      this->read_armap<32>(sarmag + sizeof(Archive_header), armap_size);
      off = sarmag + sizeof(Archive_header) + armap_size;
    }
  else if (armap_name == "/SYM64/")
    {
      this->read_armap<64>(sarmag + sizeof(Archive_header), armap_size);
      off = sarmag + sizeof(Archive_header) + armap_size;
    }
  else if (!this->input_file_->options().whole_archive())
    gold_error(_("%s: no archive symbol table (run ranlib)"),
	       this->name().c_str());

  // See if there is an extended name table.  We cache these views
  // because it is likely that we will want to read the following
  // header in the add_symbols routine.
  if ((off & 1) != 0)
    ++off;
  std::string xname;
  header_size = this->read_header(off, true, &xname, NULL);
  if (header_size == -1)
    return;

  section_size_type extended_size = convert_to_section_size_type(header_size);
  if (xname == "/")
    {
      const unsigned char* p = this->get_view(off + sizeof(Archive_header),
                                              extended_size, false, true);
      const char* px = reinterpret_cast<const char*>(p);
      this->extended_names_.assign(px, extended_size);
    }
  bool preread_syms = (parameters->options().threads()
                       && parameters->options().preread_archive_symbols());
#ifndef ENABLE_THREADS
  preread_syms = false;
#else
  if (parameters->options().has_plugins())
    preread_syms = false;
#endif
  if (preread_syms)
    this->read_all_symbols();
}

// Unlock any nested archives.

void
Archive::unlock_nested_archives()
{
  for (Nested_archive_table::iterator p = this->nested_archives_.begin();
       p != this->nested_archives_.end();
       ++p)
    {
      p->second->unlock(this->task_);
    }
}

// Read the archive symbol map.

template<int mapsize>
void
Archive::read_armap(off_t start, section_size_type size)
{
  // To count the total number of archive members, we'll just count
  // the number of times the file offset changes.  Since most archives
  // group the symbols in the armap by object, this ought to give us
  // an accurate count.
  off_t last_seen_offset = -1;

  // Read in the entire armap.
  const unsigned char* p = this->get_view(start, size, true, false);

  // Numbers in the armap are always big-endian.
  typedef typename elfcpp::Elf_types<mapsize>::Elf_Addr Entry_type;
  const Entry_type* pword = reinterpret_cast<const Entry_type*>(p);
  unsigned long nsyms = convert_types<unsigned long, Entry_type>(
    elfcpp::Swap<mapsize, true>::readval(pword));
  ++pword;

  // Note that the addition is in units of sizeof(elfcpp::Elf_Word).
  const char* pnames = reinterpret_cast<const char*>(pword + nsyms);
  section_size_type names_size =
    reinterpret_cast<const char*>(p) + size - pnames;
  this->armap_names_.assign(pnames, names_size);

  this->armap_.resize(nsyms);

  section_offset_type name_offset = 0;
  for (unsigned long i = 0; i < nsyms; ++i)
    {
      this->armap_[i].name_offset = name_offset;
      this->armap_[i].file_offset = convert_types<off_t, Entry_type>(
        elfcpp::Swap<mapsize, true>::readval(pword));
      name_offset += strlen(pnames + name_offset) + 1;
      ++pword;
      if (this->armap_[i].file_offset != last_seen_offset)
        {
          last_seen_offset = this->armap_[i].file_offset;
          ++this->num_members_;
        }
    }

  if (static_cast<section_size_type>(name_offset) > names_size)
    gold_error(_("%s: bad archive symbol table names"),
	       this->name().c_str());

  // This array keeps track of which symbols are for archive elements
  // which we have already included in the link.
  this->armap_checked_.resize(nsyms);
}

// Read the header of an archive member at OFF.  Fail if something
// goes wrong.  Return the size of the member.  Set *PNAME to the name
// of the member.

off_t
Archive::read_header(off_t off, bool cache, std::string* pname,
                     off_t* nested_off)
{
  const unsigned char* p = this->get_view(off, sizeof(Archive_header), true,
					  cache);
  const Archive_header* hdr = reinterpret_cast<const Archive_header*>(p);
  return this->interpret_header(hdr, off,  pname, nested_off);
}

// Interpret the header of HDR, the header of the archive member at
// file offset OFF.  Return the size of the member, or -1 if something
// has gone wrong.  Set *PNAME to the name of the member.

off_t
Archive::interpret_header(const Archive_header* hdr, off_t off,
                          std::string* pname, off_t* nested_off) const
{
  if (memcmp(hdr->ar_fmag, arfmag, sizeof arfmag) != 0)
    {
      gold_error(_("%s: malformed archive header at %zu"),
		 this->name().c_str(), static_cast<size_t>(off));
      return -1;
    }

  const int size_string_size = sizeof hdr->ar_size;
  char size_string[size_string_size + 1];
  memcpy(size_string, hdr->ar_size, size_string_size);
  char* ps = size_string + size_string_size;
  while (ps[-1] == ' ')
    --ps;
  *ps = '\0';

  errno = 0;
  char* end;
  off_t member_size = strtol(size_string, &end, 10);
  if (*end != '\0'
      || member_size < 0
      || (member_size == LONG_MAX && errno == ERANGE))
    {
      gold_error(_("%s: malformed archive header size at %zu"),
		 this->name().c_str(), static_cast<size_t>(off));
      return -1;
    }

  if (hdr->ar_name[0] != '/')
    {
      const char* name_end = strchr(hdr->ar_name, '/');
      if (name_end == NULL
	  || name_end - hdr->ar_name >= static_cast<int>(sizeof hdr->ar_name))
	{
	  gold_error(_("%s: malformed archive header name at %zu"),
		     this->name().c_str(), static_cast<size_t>(off));
	  return -1;
	}
      pname->assign(hdr->ar_name, name_end - hdr->ar_name);
      if (nested_off != NULL)
        *nested_off = 0;
    }
  else if (hdr->ar_name[1] == ' ')
    {
      // This is the symbol table.
      if (!pname->empty())
	pname->clear();
    }
  else if (memcmp(hdr->ar_name, sym64name, sizeof sym64name) == 0)
    {
      // This is the symbol table, 64-bit version.
      pname->assign(sym64name, sizeof sym64name);
    }
  else if (hdr->ar_name[1] == '/')
    {
      // This is the extended name table.
      pname->assign(1, '/');
    }
  else
    {
      errno = 0;
      long x = strtol(hdr->ar_name + 1, &end, 10);
      long y = 0;
      if (*end == ':')
        y = strtol(end + 1, &end, 10);
      if (*end != ' '
	  || x < 0
	  || (x == LONG_MAX && errno == ERANGE)
	  || static_cast<size_t>(x) >= this->extended_names_.size())
	{
	  gold_error(_("%s: bad extended name index at %zu"),
		     this->name().c_str(), static_cast<size_t>(off));
	  return -1;
	}

      const char* name = this->extended_names_.data() + x;
      const char* name_end = strchr(name, '\n');
      if (static_cast<size_t>(name_end - name) > this->extended_names_.size()
	  || name_end[-1] != '/')
	{
	  gold_error(_("%s: bad extended name entry at header %zu"),
		     this->name().c_str(), static_cast<size_t>(off));
	  return -1;
	}
      pname->assign(name, name_end - 1 - name);
      if (nested_off != NULL)
        *nested_off = y;
    }

  return member_size;
}

// An archive member iterator.

class Archive::const_iterator
{
 public:
  // The header of an archive member.  This is what this iterator
  // points to.
  struct Header
  {
    // The name of the member.
    std::string name;
    // The file offset of the member.
    off_t off;
    // The file offset of a nested archive member.
    off_t nested_off;
    // The size of the member.
    off_t size;
  };

  const_iterator(Archive* archive, off_t off)
    : archive_(archive), off_(off)
  { this->read_next_header(); }

  const Header&
  operator*() const
  { return this->header_; }

  const Header*
  operator->() const
  { return &this->header_; }

  const_iterator&
  operator++()
  {
    if (this->off_ == this->archive_->file().filesize())
      return *this;
    this->off_ += sizeof(Archive_header);
    if (!this->archive_->is_thin_archive())
      this->off_ += this->header_.size;
    if ((this->off_ & 1) != 0)
      ++this->off_;
    this->read_next_header();
    return *this;
  }

  const_iterator
  operator++(int)
  {
    const_iterator ret = *this;
    ++*this;
    return ret;
  }

  bool
  operator==(const const_iterator p) const
  { return this->off_ == p->off; }

  bool
  operator!=(const const_iterator p) const
  { return this->off_ != p->off; }

 private:
  void
  read_next_header();

  // The underlying archive.
  Archive* archive_;
  // The current offset in the file.
  off_t off_;
  // The current archive header.
  Header header_;
};

// Read the next archive header.

void
Archive::const_iterator::read_next_header()
{
  off_t filesize = this->archive_->file().filesize();
  while (true)
    {
      if (filesize - this->off_ < static_cast<off_t>(sizeof(Archive_header)))
	{
	  if (filesize != this->off_)
	    {
	      gold_error(_("%s: short archive header at %zu"),
			 this->archive_->filename().c_str(),
			 static_cast<size_t>(this->off_));
	      this->off_ = filesize;
	    }
	  this->header_.off = filesize;
	  return;
	}

      unsigned char buf[sizeof(Archive_header)];
      this->archive_->file().read(this->off_, sizeof(Archive_header), buf);

      const Archive_header* hdr = reinterpret_cast<const Archive_header*>(buf);
      off_t size = this->archive_->interpret_header(hdr, this->off_,
						    &this->header_.name,
						    &this->header_.nested_off);
      if (size == -1)
	{
	  this->header_.off = filesize;
	  return;
	}

      this->header_.size = size;
      this->header_.off = this->off_;

      // Skip special members.
      if (!this->header_.name.empty()
	  && this->header_.name != "/"
	  && this->header_.name != "/SYM64/")
	return;

      this->off_ += sizeof(Archive_header) + this->header_.size;
      if ((this->off_ & 1) != 0)
	++this->off_;
    }
}

// Initial iterator.

Archive::const_iterator
Archive::begin()
{
  return Archive::const_iterator(this, sarmag);
}

// Final iterator.

Archive::const_iterator
Archive::end()
{
  return Archive::const_iterator(this, this->input_file_->file().filesize());
}

// Get the file and offset for an archive member, which may be an
// external member of a thin archive.  Set *INPUT_FILE to the
// file containing the actual member, *MEMOFF to the offset
// within that file (0 if not a nested archive), and *MEMBER_NAME
// to the name of the archive member.  Return TRUE on success.

bool
Archive::get_file_and_offset(off_t off, Input_file** input_file, off_t* memoff,
                             off_t* memsize, std::string* member_name)
{
  off_t nested_off;

  *memsize = this->read_header(off, false, member_name, &nested_off);
  if (*memsize == -1)
    return false;

  *input_file = this->input_file_;
  *memoff = off + static_cast<off_t>(sizeof(Archive_header));

  if (!this->is_thin_archive_)
    return true;

  // Adjust a relative pathname so that it is relative
  // to the directory containing the archive.
  if (!IS_ABSOLUTE_PATH(member_name->c_str()))
    {
      const char* arch_path = this->filename().c_str();
      const char* basename = lbasename(arch_path);
      if (basename > arch_path)
        member_name->replace(0, 0,
                             this->filename().substr(0, basename - arch_path));
    }

  if (nested_off > 0)
    {
      // This is a member of a nested archive.  Open the containing
      // archive if we don't already have it open, then do a recursive
      // call to include the member from that archive.
      Archive* arch;
      Nested_archive_table::const_iterator p =
        this->nested_archives_.find(*member_name);
      if (p != this->nested_archives_.end())
        arch = p->second;
      else
        {
          Input_file_argument* input_file_arg =
            new Input_file_argument(member_name->c_str(),
                                    Input_file_argument::INPUT_FILE_TYPE_FILE,
                                    "", false, parameters->options());
          *input_file = new Input_file(input_file_arg);
	  int dummy = 0;
          if (!(*input_file)->open(*this->dirpath_, this->task_, &dummy))
            return false;
          arch = new Archive(*member_name, *input_file, false, this->dirpath_,
                             this->task_);
          arch->setup();
          std::pair<Nested_archive_table::iterator, bool> ins =
            this->nested_archives_.insert(std::make_pair(*member_name, arch));
          gold_assert(ins.second);
        }
      return arch->get_file_and_offset(nested_off, input_file, memoff,
				       memsize, member_name);
    }

  // This is an external member of a thin archive.  Open the
  // file as a regular relocatable object file.
  Input_file_argument* input_file_arg =
      new Input_file_argument(member_name->c_str(),
                              Input_file_argument::INPUT_FILE_TYPE_FILE,
                              "", false, this->input_file_->options());
  *input_file = new Input_file(input_file_arg);
  int dummy = 0;
  if (!(*input_file)->open(*this->dirpath_, this->task_, &dummy))
    return false;

  *memoff = 0;
  *memsize = (*input_file)->file().filesize();
  return true;
}

// Return an ELF object for the member at offset OFF.  If
// PUNCONFIGURED is not NULL, then if the ELF object has an
// unsupported target type, set *PUNCONFIGURED to true and return
// NULL.

Object*
Archive::get_elf_object_for_member(off_t off, bool* punconfigured)
{
  if (punconfigured != NULL)
    *punconfigured = false;

  Input_file* input_file;
  off_t memoff;
  off_t memsize;
  std::string member_name;
  if (!this->get_file_and_offset(off, &input_file, &memoff, &memsize,
				 &member_name))
    return NULL;

  const unsigned char* ehdr;
  int read_size;
  Object *obj = NULL;
  bool is_elf_obj = false;
  bool unclaimed = false;

  if (is_elf_object(input_file, memoff, &ehdr, &read_size))
    {
      obj = make_elf_object((std::string(this->input_file_->filename())
			     + "(" + member_name + ")"),
			    input_file, memoff, ehdr, read_size,
			    punconfigured);
      is_elf_obj = true;
    }

  if (parameters->options().has_plugins())
    {
      Object* plugin_obj
	= parameters->options().plugins()->claim_file(input_file,
						      memoff,
						      memsize,
						      obj);
      if (plugin_obj != NULL)
        {
          // The input file was claimed by a plugin, and its symbols
          // have been provided by the plugin.
	  // Delete its elf object.
	  if (obj != NULL)
	    delete obj;
          return plugin_obj;
        }

      unclaimed = true;
    }

  if (!is_elf_obj)
    {
      if (unclaimed)
	gold_error(_("%s: plugin failed to claim member %s at %zu"),
		   this->name().c_str(), member_name.c_str(),
		   static_cast<size_t>(off));
      else
	gold_error(_("%s: member %s at %zu is not an ELF object"),
		   this->name().c_str(), member_name.c_str(),
		   static_cast<size_t>(off));
      return NULL;
    }

  if (obj == NULL)
    return NULL;
  obj->set_no_export(this->no_export());
  return obj;
}

// Read the symbols from all the archive members in the link.

void
Archive::read_all_symbols()
{
  for (Archive::const_iterator p = this->begin();
       p != this->end();
       ++p)
    this->read_symbols(p->off);
}

// Read the symbols from an archive member in the link.  OFF is the file
// offset of the member header.

void
Archive::read_symbols(off_t off)
{
  Object* obj = this->get_elf_object_for_member(off, NULL);
  if (obj == NULL)
    return;

  Read_symbols_data* sd = new Read_symbols_data;
  obj->read_symbols(sd);
  Archive_member member(obj, sd);
  this->members_[off] = member;
}

// Select members from the archive and add them to the link.  We walk
// through the elements in the archive map, and look each one up in
// the symbol table.  If it exists as a strong undefined symbol, we
// pull in the corresponding element.  We have to do this in a loop,
// since pulling in one element may create new undefined symbols which
// may be satisfied by other objects in the archive.  Return true in
// the normal case, false if the first member we tried to add from
// this archive had an incompatible target.

bool
Archive::add_symbols(Symbol_table* symtab, Layout* layout,
		     Input_objects* input_objects, Mapfile* mapfile)
{
  ++Archive::total_archives;

  if (this->input_file_->options().whole_archive())
    return this->include_all_members(symtab, layout, input_objects,
				     mapfile);

  Archive::total_members += this->num_members_;

  input_objects->archive_start(this);

  const size_t armap_size = this->armap_.size();

  // This is a quick optimization, since we usually see many symbols
  // in a row with the same offset.  last_seen_offset holds the last
  // offset we saw that was present in the seen_offsets_ set.
  off_t last_seen_offset = -1;

  // Track which symbols in the symbol table we've already found to be
  // defined.

  char* tmpbuf = NULL;
  size_t tmpbuflen = 0;
  bool added_new_object;
  do
    {
      added_new_object = false;
      for (size_t i = 0; i < armap_size; ++i)
	{
          if (this->armap_checked_[i])
            continue;
	  if (this->armap_[i].file_offset == last_seen_offset)
            {
              this->armap_checked_[i] = true;
              continue;
            }
	  if (this->seen_offsets_.find(this->armap_[i].file_offset)
              != this->seen_offsets_.end())
	    {
              this->armap_checked_[i] = true;
	      last_seen_offset = this->armap_[i].file_offset;
	      continue;
	    }

	  const char* sym_name = (this->armap_names_.data()
				  + this->armap_[i].name_offset);

          Symbol* sym;
          std::string why;
          Archive::Should_include t =
	    Archive::should_include_member(symtab, layout, sym_name, &sym,
					   &why, &tmpbuf, &tmpbuflen);

	  if (t == Archive::SHOULD_INCLUDE_NO
              || t == Archive::SHOULD_INCLUDE_YES)
	    this->armap_checked_[i] = true;

	  if (t != Archive::SHOULD_INCLUDE_YES)
	    continue;

	  // We want to include this object in the link.
	  last_seen_offset = this->armap_[i].file_offset;
	  this->seen_offsets_.insert(last_seen_offset);

	  if (!this->include_member(symtab, layout, input_objects,
				    last_seen_offset, mapfile, sym,
				    why.c_str()))
	    {
	      if (tmpbuf != NULL)
		free(tmpbuf);
	      return false;
	    }

	  added_new_object = true;
	}
    }
  while (added_new_object);

  if (tmpbuf != NULL)
    free(tmpbuf);

  input_objects->archive_stop(this);

  return true;
}

// Return whether the archive includes a member which defines the
// symbol SYM.

bool
Archive::defines_symbol(Symbol* sym) const
{
  const char* symname = sym->name();
  size_t symname_len = strlen(symname);
  size_t armap_size = this->armap_.size();
  for (size_t i = 0; i < armap_size; ++i)
    {
      if (this->armap_checked_[i])
	continue;
      const char* archive_symname = (this->armap_names_.data()
				     + this->armap_[i].name_offset);
      if (strncmp(archive_symname, symname, symname_len) != 0)
	continue;
      char c = archive_symname[symname_len];
      if (c == '\0' && sym->version() == NULL)
	return true;
      if (c == '@')
	{
	  const char* ver = archive_symname + symname_len + 1;
	  if (*ver == '@')
	    {
	      if (sym->version() == NULL)
		return true;
	      ++ver;
	    }
	  if (sym->version() != NULL && strcmp(sym->version(), ver) == 0)
	    return true;
	}
    }
  return false;
}

// Include all the archive members in the link.  This is for --whole-archive.

bool
Archive::include_all_members(Symbol_table* symtab, Layout* layout,
                             Input_objects* input_objects, Mapfile* mapfile)
{
  // Don't include the same archive twice.  This can happen if
  // --whole-archive is nested inside --start-group (PR gold/12163).
  if (this->included_all_members_)
    return true;

  this->included_all_members_ = true;

  input_objects->archive_start(this);

  if (this->members_.size() > 0)
    {
      std::map<off_t, Archive_member>::const_iterator p;
      for (p = this->members_.begin();
           p != this->members_.end();
           ++p)
        {
          if (!this->include_member(symtab, layout, input_objects, p->first,
				    mapfile, NULL, "--whole-archive"))
	    return false;
          ++Archive::total_members;
        }
    }
  else
    {
      for (Archive::const_iterator p = this->begin();
           p != this->end();
           ++p)
        {
          if (!this->include_member(symtab, layout, input_objects, p->off,
				    mapfile, NULL, "--whole-archive"))
	    return false;
          ++Archive::total_members;
        }
    }

  input_objects->archive_stop(this);

  return true;
}

// Return the number of members in the archive.  This is only used for
// reports.

size_t
Archive::count_members()
{
  size_t ret = 0;
  for (Archive::const_iterator p = this->begin();
       p != this->end();
       ++p)
    ++ret;
  return ret;
}

// RAII class to ensure we unlock the object if it's a member of a
// thin archive. We can't use Task_lock_obj in Archive::include_member
// because the object file is already locked when it's opened by
// get_elf_object_for_member.

class Thin_archive_object_unlocker
{
 public:
  Thin_archive_object_unlocker(const Task *task, Object* obj)
    : task_(task), obj_(obj)
  { }

  ~Thin_archive_object_unlocker()
  {
    if (this->obj_->offset() == 0)
      this->obj_->unlock(this->task_);
  }

 private:
  Thin_archive_object_unlocker(const Thin_archive_object_unlocker&);
  Thin_archive_object_unlocker& operator=(const Thin_archive_object_unlocker&);

  const Task* task_;
  Object* obj_;
};

// Include an archive member in the link.  OFF is the file offset of
// the member header.  WHY is the reason we are including this member.
// Return true if we added the member or if we had an error, return
// false if this was the first member we tried to add from this
// archive and it had an incompatible format.

bool
Archive::include_member(Symbol_table* symtab, Layout* layout,
			Input_objects* input_objects, off_t off,
			Mapfile* mapfile, Symbol* sym, const char* why)
{
  ++Archive::total_members_loaded;

  std::map<off_t, Archive_member>::const_iterator p = this->members_.find(off);
  if (p != this->members_.end())
    {
      Object* obj = p->second.obj_;

      Read_symbols_data* sd = p->second.sd_;
      if (mapfile != NULL)
        mapfile->report_include_archive_member(obj->name(), sym, why);
      if (input_objects->add_object(obj))
        {
          obj->layout(symtab, layout, sd);
          obj->add_symbols(symtab, sd, layout);
	  this->included_member_ = true;
        }
      delete sd;
      return true;
    }

  // If this is the first object we are including from this archive,
  // and we searched for this archive, most likely because it was
  // found via a -l option, then if the target is incompatible we want
  // to move on to the next archive found in the search path.
  bool unconfigured = false;
  bool* punconfigured = NULL;
  if (!this->included_member_ && this->searched_for())
    punconfigured = &unconfigured;

  Object* obj = this->get_elf_object_for_member(off, punconfigured);
  if (obj == NULL)
    {
      // Return false to search for another archive, true if we found
      // an error.
      return unconfigured ? false : true;
    }

  // If the object is an external member of a thin archive,
  // unlock it when we're done here.
  Thin_archive_object_unlocker unlocker(this->task_, obj);

  if (mapfile != NULL)
    mapfile->report_include_archive_member(obj->name(), sym, why);

  Pluginobj* pluginobj = obj->pluginobj();
  if (pluginobj != NULL)
    {
      pluginobj->add_symbols(symtab, NULL, layout);
      this->included_member_ = true;
      return true;
    }

  if (!input_objects->add_object(obj))
    {
      delete obj;
      return true;
    }

  if (layout->incremental_inputs() != NULL)
    layout->incremental_inputs()->report_object(obj, 0, this, NULL);

  {
    Read_symbols_data sd;
    obj->read_symbols(&sd);
    obj->layout(symtab, layout, &sd);
    obj->add_symbols(symtab, &sd, layout);
  }

  this->included_member_ = true;
  return true;
}

// Iterate over all unused symbols, and call the visitor class V for each.

void
Archive::do_for_all_unused_symbols(Symbol_visitor_base* v) const
{
  for (std::vector<Armap_entry>::const_iterator p = this->armap_.begin();
       p != this->armap_.end();
       ++p)
    {
      if (this->seen_offsets_.find(p->file_offset)
          == this->seen_offsets_.end())
        v->visit(this->armap_names_.data() + p->name_offset);
    }
}

// Print statistical information to stderr.  This is used for --stats.

void
Archive::print_stats()
{
  fprintf(stderr, _("%s: archive libraries: %u\n"),
          program_name, Archive::total_archives);
  fprintf(stderr, _("%s: total archive members: %u\n"),
          program_name, Archive::total_members);
  fprintf(stderr, _("%s: loaded archive members: %u\n"),
          program_name, Archive::total_members_loaded);
}

// Add_archive_symbols methods.

Add_archive_symbols::~Add_archive_symbols()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

// Return whether we can add the archive symbols.  We are blocked by
// this_blocker_.  We block next_blocker_.  We also lock the file.

Task_token*
Add_archive_symbols::is_runnable()
{
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Add_archive_symbols::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
  tl->add(this, this->archive_->token());
}

void
Add_archive_symbols::run(Workqueue* workqueue)
{
  // For an incremental link, begin recording layout information.
  Incremental_inputs* incremental_inputs = this->layout_->incremental_inputs();
  if (incremental_inputs != NULL)
    {
      unsigned int arg_serial = this->input_argument_->file().arg_serial();
      Script_info* script_info = this->input_argument_->script_info();
      incremental_inputs->report_archive_begin(this->archive_, arg_serial,
					       script_info);
    }

  bool added = this->archive_->add_symbols(this->symtab_, this->layout_,
					   this->input_objects_,
					   this->mapfile_);
  this->archive_->unlock_nested_archives();

  this->archive_->release();
  this->archive_->clear_uncached_views();

  if (!added)
    {
      // This archive holds object files which are incompatible with
      // our output file.
      Read_symbols::incompatible_warning(this->input_argument_,
					 this->archive_->input_file());
      Read_symbols::requeue(workqueue, this->input_objects_, this->symtab_,
			    this->layout_, this->dirpath_, this->dirindex_,
			    this->mapfile_, this->input_argument_,
			    this->input_group_, this->next_blocker_);
      delete this->archive_;
      return;
    }

  if (this->input_group_ != NULL)
    this->input_group_->add_archive(this->archive_);
  else
    {
      // For an incremental link, finish recording the layout information.
      if (incremental_inputs != NULL)
	incremental_inputs->report_archive_end(this->archive_);

      if (!parameters->options().has_plugins()
	  || this->archive_->input_file()->options().whole_archive())
	{
	  // We no longer need to know about this archive.
	  delete this->archive_;
	}
      else
	{
	  // The plugin interface may want to rescan this archive.
	  parameters->options().plugins()->save_archive(this->archive_);
	}

      this->archive_ = NULL;
    }
}

// Class Lib_group static variables.
unsigned int Lib_group::total_lib_groups;
unsigned int Lib_group::total_members;
unsigned int Lib_group::total_members_loaded;

Lib_group::Lib_group(const Input_file_lib* lib, Task* task)
  : Library_base(task), members_()
{
  this->members_.resize(lib->size());
}

const std::string&
Lib_group::do_filename() const
{
  std::string *filename = new std::string("/group/");
  return *filename;
}

// Select members from the lib group and add them to the link.  We walk
// through the members, and check if each one up should be included.
// If the object says it should be included, we do so.  We have to do
// this in a loop, since including one member may create new undefined
// symbols which may be satisfied by other members.

void
Lib_group::add_symbols(Symbol_table* symtab, Layout* layout,
                       Input_objects* input_objects)
{
  ++Lib_group::total_lib_groups;

  Lib_group::total_members += this->members_.size();

  bool added_new_object;
  do
    {
      added_new_object = false;
      unsigned int i = 0;
      while (i < this->members_.size())
	{
	  const Archive_member& member = this->members_[i];
	  Object* obj = member.obj_;
	  std::string why;

          // Skip files with no symbols. Plugin objects have
          // member.sd_ == NULL.
          if (obj != NULL
	      && (member.sd_ == NULL || member.sd_->symbol_names != NULL))
            {
	      Archive::Should_include t = obj->should_include_member(symtab,
								     layout,
								     member.sd_,
								     &why);

	      if (t != Archive::SHOULD_INCLUDE_YES)
		{
		  ++i;
		  continue;
		}

	      this->include_member(symtab, layout, input_objects, member);

	      added_new_object = true;
	    }
          else
            {
              if (member.sd_ != NULL)
		{
		  // The file must be locked in order to destroy the views
		  // associated with it.
		  gold_assert(obj != NULL);
		  obj->lock(this->task_);
		  delete member.sd_;
		  obj->unlock(this->task_);
		}
            }

	  this->members_[i] = this->members_.back();
	  this->members_.pop_back();
	}
    }
  while (added_new_object);
}

// Include a lib group member in the link.

void
Lib_group::include_member(Symbol_table* symtab, Layout* layout,
			  Input_objects* input_objects,
			  const Archive_member& member)
{
  ++Lib_group::total_members_loaded;

  Object* obj = member.obj_;
  gold_assert(obj != NULL);

  Pluginobj* pluginobj = obj->pluginobj();
  if (pluginobj != NULL)
    {
      pluginobj->add_symbols(symtab, NULL, layout);
      return;
    }

  Read_symbols_data* sd = member.sd_;
  gold_assert(sd != NULL);
  obj->lock(this->task_);
  if (input_objects->add_object(obj))
    {
      if (layout->incremental_inputs() != NULL)
	layout->incremental_inputs()->report_object(obj, member.arg_serial_,
						    this, NULL);
      obj->layout(symtab, layout, sd);
      obj->add_symbols(symtab, sd, layout);
    }
  delete sd;
  // Unlock the file for the next task.
  obj->unlock(this->task_);
}

// Iterate over all unused symbols, and call the visitor class V for each.

void
Lib_group::do_for_all_unused_symbols(Symbol_visitor_base* v) const
{
  // Files are removed from the members list when used, so all the
  // files remaining on the list are unused.
  for (std::vector<Archive_member>::const_iterator p = this->members_.begin();
       p != this->members_.end();
       ++p)
    {
      Object* obj = p->obj_;
      obj->for_all_global_symbols(p->sd_, v);
    }
}

// Print statistical information to stderr.  This is used for --stats.

void
Lib_group::print_stats()
{
  fprintf(stderr, _("%s: lib groups: %u\n"),
          program_name, Lib_group::total_lib_groups);
  fprintf(stderr, _("%s: total lib groups members: %u\n"),
          program_name, Lib_group::total_members);
  fprintf(stderr, _("%s: loaded lib groups members: %u\n"),
          program_name, Lib_group::total_members_loaded);
}

Task_token*
Add_lib_group_symbols::is_runnable()
{
  if (this->readsyms_blocker_ != NULL && this->readsyms_blocker_->is_blocked())
    return this->readsyms_blocker_;
  if (this->this_blocker_ != NULL && this->this_blocker_->is_blocked())
    return this->this_blocker_;
  return NULL;
}

void
Add_lib_group_symbols::locks(Task_locker* tl)
{
  tl->add(this, this->next_blocker_);
}

void
Add_lib_group_symbols::run(Workqueue*)
{
  // For an incremental link, begin recording layout information.
  Incremental_inputs* incremental_inputs = this->layout_->incremental_inputs();
  if (incremental_inputs != NULL)
    incremental_inputs->report_archive_begin(this->lib_, 0, NULL);

  this->lib_->add_symbols(this->symtab_, this->layout_, this->input_objects_);

  if (incremental_inputs != NULL)
    incremental_inputs->report_archive_end(this->lib_);
}

Add_lib_group_symbols::~Add_lib_group_symbols()
{
  if (this->this_blocker_ != NULL)
    delete this->this_blocker_;
  // next_blocker_ is deleted by the task associated with the next
  // input file.
}

} // End namespace gold.
