// dynobj.cc -- dynamic object support for gold

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

#include <vector>
#include <cstring>

#include "elfcpp.h"
#include "parameters.h"
#include "script.h"
#include "symtab.h"
#include "dynobj.h"

namespace gold
{

// Class Dynobj.

// Sets up the default soname_ to use, in the (rare) cases we never
// see a DT_SONAME entry.

Dynobj::Dynobj(const std::string& name, Input_file* input_file, off_t offset)
  : Object(name, input_file, true, offset),
    needed_(),
    unknown_needed_(UNKNOWN_NEEDED_UNSET)
{
  // This will be overridden by a DT_SONAME entry, hopefully.  But if
  // we never see a DT_SONAME entry, our rule is to use the dynamic
  // object's filename.  The only exception is when the dynamic object
  // is part of an archive (so the filename is the archive's
  // filename).  In that case, we use just the dynobj's name-in-archive.
  if (input_file == NULL)
    this->soname_ = name;
  else
    {
      this->soname_ = input_file->found_name();
      if (this->offset() != 0)
	{
	  std::string::size_type open_paren = this->name().find('(');
	  std::string::size_type close_paren = this->name().find(')');
	  if (open_paren != std::string::npos
	      && close_paren != std::string::npos)
	    {
	      // It's an archive, and name() is of the form 'foo.a(bar.so)'.
	      open_paren += 1;
	      this->soname_ = this->name().substr(open_paren,
						  close_paren - open_paren);
	    }
	}
    }
}

// Class Sized_dynobj.

template<int size, bool big_endian>
Sized_dynobj<size, big_endian>::Sized_dynobj(
    const std::string& name,
    Input_file* input_file,
    off_t offset,
    const elfcpp::Ehdr<size, big_endian>& ehdr)
  : Dynobj(name, input_file, offset),
    elf_file_(this, ehdr),
    dynsym_shndx_(-1U),
    symbols_(NULL),
    defined_count_(0)
{
}

// Set up the object.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::setup()
{
  const unsigned int shnum = this->elf_file_.shnum();
  this->set_shnum(shnum);
}

// Find the SHT_DYNSYM section and the various version sections, and
// the dynamic section, given the section headers.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::find_dynsym_sections(
    const unsigned char* pshdrs,
    unsigned int* pversym_shndx,
    unsigned int* pverdef_shndx,
    unsigned int* pverneed_shndx,
    unsigned int* pdynamic_shndx)
{
  *pversym_shndx = -1U;
  *pverdef_shndx = -1U;
  *pverneed_shndx = -1U;
  *pdynamic_shndx = -1U;

  unsigned int symtab_shndx = 0;
  unsigned int xindex_shndx = 0;
  unsigned int xindex_link = 0;
  const unsigned int shnum = this->shnum();
  const unsigned char* p = pshdrs;
  for (unsigned int i = 0; i < shnum; ++i, p += This::shdr_size)
    {
      typename This::Shdr shdr(p);

      unsigned int* pi;
      switch (shdr.get_sh_type())
	{
	case elfcpp::SHT_DYNSYM:
	  this->dynsym_shndx_ = i;
	  if (xindex_shndx > 0 && xindex_link == i)
	    {
	      Xindex* xindex = new Xindex(this->elf_file_.large_shndx_offset());
	      xindex->read_symtab_xindex<size, big_endian>(this, xindex_shndx,
							   pshdrs);
	      this->set_xindex(xindex);
	    }
	  pi = NULL;
	  break;
	case elfcpp::SHT_SYMTAB:
	  symtab_shndx = i;
	  pi = NULL;
	  break;
	case elfcpp::SHT_GNU_versym:
	  pi = pversym_shndx;
	  break;
	case elfcpp::SHT_GNU_verdef:
	  pi = pverdef_shndx;
	  break;
	case elfcpp::SHT_GNU_verneed:
	  pi = pverneed_shndx;
	  break;
	case elfcpp::SHT_DYNAMIC:
	  pi = pdynamic_shndx;
	  break;
	case elfcpp::SHT_SYMTAB_SHNDX:
	  xindex_shndx = i;
	  xindex_link = this->adjust_shndx(shdr.get_sh_link());
	  if (xindex_link == this->dynsym_shndx_)
	    {
	      Xindex* xindex = new Xindex(this->elf_file_.large_shndx_offset());
	      xindex->read_symtab_xindex<size, big_endian>(this, xindex_shndx,
							   pshdrs);
	      this->set_xindex(xindex);
	    }
	  pi = NULL;
	  break;
	default:
	  pi = NULL;
	  break;
	}

      if (pi == NULL)
	continue;

      if (*pi != -1U)
	this->error(_("unexpected duplicate type %u section: %u, %u"),
		    shdr.get_sh_type(), *pi, i);

      *pi = i;
    }

  // If there is no dynamic symbol table, use the normal symbol table.
  // On some SVR4 systems, a shared library is stored in an archive.
  // The version stored in the archive only has a normal symbol table.
  // It has an SONAME entry which points to another copy in the file
  // system which has a dynamic symbol table as usual.  This is way of
  // addressing the issues which glibc addresses using GROUP with
  // libc_nonshared.a.
  if (this->dynsym_shndx_ == -1U && symtab_shndx != 0)
    {
      this->dynsym_shndx_ = symtab_shndx;
      if (xindex_shndx > 0 && xindex_link == symtab_shndx)
	{
	  Xindex* xindex = new Xindex(this->elf_file_.large_shndx_offset());
	  xindex->read_symtab_xindex<size, big_endian>(this, xindex_shndx,
						       pshdrs);
	  this->set_xindex(xindex);
	}
    }
}

// Read the contents of section SHNDX.  PSHDRS points to the section
// headers.  TYPE is the expected section type.  LINK is the expected
// section link.  Store the data in *VIEW and *VIEW_SIZE.  The
// section's sh_info field is stored in *VIEW_INFO.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::read_dynsym_section(
    const unsigned char* pshdrs,
    unsigned int shndx,
    elfcpp::SHT type,
    unsigned int link,
    File_view** view,
    section_size_type* view_size,
    unsigned int* view_info)
{
  if (shndx == -1U)
    {
      *view = NULL;
      *view_size = 0;
      *view_info = 0;
      return;
    }

  typename This::Shdr shdr(pshdrs + shndx * This::shdr_size);

  gold_assert(shdr.get_sh_type() == type);

  if (this->adjust_shndx(shdr.get_sh_link()) != link)
    this->error(_("unexpected link in section %u header: %u != %u"),
	        shndx, this->adjust_shndx(shdr.get_sh_link()), link);

  *view = this->get_lasting_view(shdr.get_sh_offset(), shdr.get_sh_size(),
				 true, false);
  *view_size = convert_to_section_size_type(shdr.get_sh_size());
  *view_info = shdr.get_sh_info();
}

// Read the dynamic tags.  Set the soname field if this shared object
// has a DT_SONAME tag.  Record the DT_NEEDED tags.  PSHDRS points to
// the section headers.  DYNAMIC_SHNDX is the section index of the
// SHT_DYNAMIC section.  STRTAB_SHNDX, STRTAB, and STRTAB_SIZE are the
// section index and contents of a string table which may be the one
// associated with the SHT_DYNAMIC section.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::read_dynamic(const unsigned char* pshdrs,
					     unsigned int dynamic_shndx,
					     unsigned int strtab_shndx,
					     const unsigned char* strtabu,
					     off_t strtab_size)
{
  typename This::Shdr dynamicshdr(pshdrs + dynamic_shndx * This::shdr_size);
  gold_assert(dynamicshdr.get_sh_type() == elfcpp::SHT_DYNAMIC);

  const off_t dynamic_size = dynamicshdr.get_sh_size();
  const unsigned char* pdynamic = this->get_view(dynamicshdr.get_sh_offset(),
						 dynamic_size, true, false);

  const unsigned int link = this->adjust_shndx(dynamicshdr.get_sh_link());
  if (link != strtab_shndx)
    {
      if (link >= this->shnum())
	{
	  this->error(_("DYNAMIC section %u link out of range: %u"),
		      dynamic_shndx, link);
	  return;
	}

      typename This::Shdr strtabshdr(pshdrs + link * This::shdr_size);
      if (strtabshdr.get_sh_type() != elfcpp::SHT_STRTAB)
	{
	  this->error(_("DYNAMIC section %u link %u is not a strtab"),
		      dynamic_shndx, link);
	  return;
	}

      strtab_size = strtabshdr.get_sh_size();
      strtabu = this->get_view(strtabshdr.get_sh_offset(), strtab_size, false,
			       false);
    }

  const char* const strtab = reinterpret_cast<const char*>(strtabu);

  for (const unsigned char* p = pdynamic;
       p < pdynamic + dynamic_size;
       p += This::dyn_size)
    {
      typename This::Dyn dyn(p);

      switch (dyn.get_d_tag())
	{
	case elfcpp::DT_NULL:
	  // We should always see DT_NULL at the end of the dynamic
	  // tags.
	  return;

	case elfcpp::DT_SONAME:
	  {
	    off_t val = dyn.get_d_val();
	    if (val >= strtab_size)
	      this->error(_("DT_SONAME value out of range: %lld >= %lld"),
			  static_cast<long long>(val),
			  static_cast<long long>(strtab_size));
	    else
	      this->set_soname_string(strtab + val);
	  }
	  break;

	case elfcpp::DT_NEEDED:
	  {
	    off_t val = dyn.get_d_val();
	    if (val >= strtab_size)
	      this->error(_("DT_NEEDED value out of range: %lld >= %lld"),
			  static_cast<long long>(val),
			  static_cast<long long>(strtab_size));
	    else
	      this->add_needed(strtab + val);
	  }
	  break;

	default:
	  break;
	}
    }

  this->error(_("missing DT_NULL in dynamic segment"));
}

// Read the symbols and sections from a dynamic object.  We read the
// dynamic symbols, not the normal symbols.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_read_symbols(Read_symbols_data* sd)
{
  this->base_read_symbols(sd);
}

// Read the symbols and sections from a dynamic object.  We read the
// dynamic symbols, not the normal symbols.  This is common code for
// all target-specific overrides of do_read_symbols().

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::base_read_symbols(Read_symbols_data* sd)
{
  this->read_section_data(&this->elf_file_, sd);

  const unsigned char* const pshdrs = sd->section_headers->data();

  unsigned int versym_shndx;
  unsigned int verdef_shndx;
  unsigned int verneed_shndx;
  unsigned int dynamic_shndx;
  this->find_dynsym_sections(pshdrs, &versym_shndx, &verdef_shndx,
			     &verneed_shndx, &dynamic_shndx);

  unsigned int strtab_shndx = -1U;

  sd->symbols = NULL;
  sd->symbols_size = 0;
  sd->external_symbols_offset = 0;
  sd->symbol_names = NULL;
  sd->symbol_names_size = 0;
  sd->versym = NULL;
  sd->versym_size = 0;
  sd->verdef = NULL;
  sd->verdef_size = 0;
  sd->verdef_info = 0;
  sd->verneed = NULL;
  sd->verneed_size = 0;
  sd->verneed_info = 0;

  const unsigned char* namesu = sd->section_names->data();
  const char* names = reinterpret_cast<const char*>(namesu);
  if (memmem(names, sd->section_names_size, ".zdebug_", 8) != NULL)
    {
      Compressed_section_map* compressed_sections =
	  build_compressed_section_map<size, big_endian>(
	      pshdrs, this->shnum(), names, sd->section_names_size, this, true);
      if (compressed_sections != NULL)
        this->set_compressed_sections(compressed_sections);
    }

  if (this->dynsym_shndx_ != -1U)
    {
      // Get the dynamic symbols.
      typename This::Shdr dynsymshdr(pshdrs
				     + this->dynsym_shndx_ * This::shdr_size);

      sd->symbols = this->get_lasting_view(dynsymshdr.get_sh_offset(),
					   dynsymshdr.get_sh_size(), true,
					   false);
      sd->symbols_size =
	convert_to_section_size_type(dynsymshdr.get_sh_size());

      // Get the symbol names.
      strtab_shndx = this->adjust_shndx(dynsymshdr.get_sh_link());
      if (strtab_shndx >= this->shnum())
	{
	  this->error(_("invalid dynamic symbol table name index: %u"),
		      strtab_shndx);
	  return;
	}
      typename This::Shdr strtabshdr(pshdrs + strtab_shndx * This::shdr_size);
      if (strtabshdr.get_sh_type() != elfcpp::SHT_STRTAB)
	{
	  this->error(_("dynamic symbol table name section "
			"has wrong type: %u"),
		      static_cast<unsigned int>(strtabshdr.get_sh_type()));
	  return;
	}

      sd->symbol_names = this->get_lasting_view(strtabshdr.get_sh_offset(),
						strtabshdr.get_sh_size(),
						false, false);
      sd->symbol_names_size =
	convert_to_section_size_type(strtabshdr.get_sh_size());

      // Get the version information.

      unsigned int dummy;
      this->read_dynsym_section(pshdrs, versym_shndx, elfcpp::SHT_GNU_versym,
				this->dynsym_shndx_,
				&sd->versym, &sd->versym_size, &dummy);

      // We require that the version definition and need section link
      // to the same string table as the dynamic symbol table.  This
      // is not a technical requirement, but it always happens in
      // practice.  We could change this if necessary.

      this->read_dynsym_section(pshdrs, verdef_shndx, elfcpp::SHT_GNU_verdef,
				strtab_shndx, &sd->verdef, &sd->verdef_size,
				&sd->verdef_info);

      this->read_dynsym_section(pshdrs, verneed_shndx, elfcpp::SHT_GNU_verneed,
				strtab_shndx, &sd->verneed, &sd->verneed_size,
				&sd->verneed_info);
    }

  // Read the SHT_DYNAMIC section to find whether this shared object
  // has a DT_SONAME tag and to record any DT_NEEDED tags.  This
  // doesn't really have anything to do with reading the symbols, but
  // this is a convenient place to do it.
  if (dynamic_shndx != -1U)
    this->read_dynamic(pshdrs, dynamic_shndx, strtab_shndx,
		       (sd->symbol_names == NULL
			? NULL
			: sd->symbol_names->data()),
		       sd->symbol_names_size);
}

// Return the Xindex structure to use for object with lots of
// sections.

template<int size, bool big_endian>
Xindex*
Sized_dynobj<size, big_endian>::do_initialize_xindex()
{
  gold_assert(this->dynsym_shndx_ != -1U);
  Xindex* xindex = new Xindex(this->elf_file_.large_shndx_offset());
  xindex->initialize_symtab_xindex<size, big_endian>(this, this->dynsym_shndx_);
  return xindex;
}

// Lay out the input sections for a dynamic object.  We don't want to
// include sections from a dynamic object, so all that we actually do
// here is check for .gnu.warning and .note.GNU-split-stack sections.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_layout(Symbol_table* symtab,
					  Layout*,
					  Read_symbols_data* sd)
{
  const unsigned int shnum = this->shnum();
  if (shnum == 0)
    return;

  // Get the section headers.
  const unsigned char* pshdrs = sd->section_headers->data();

  // Get the section names.
  const unsigned char* pnamesu = sd->section_names->data();
  const char* pnames = reinterpret_cast<const char*>(pnamesu);

  // Skip the first, dummy, section.
  pshdrs += This::shdr_size;
  for (unsigned int i = 1; i < shnum; ++i, pshdrs += This::shdr_size)
    {
      typename This::Shdr shdr(pshdrs);

      if (shdr.get_sh_name() >= sd->section_names_size)
	{
	  this->error(_("bad section name offset for section %u: %lu"),
		      i, static_cast<unsigned long>(shdr.get_sh_name()));
	  return;
	}

      const char* name = pnames + shdr.get_sh_name();

      this->handle_gnu_warning_section(name, i, symtab);
      this->handle_split_stack_section(name);
    }

  delete sd->section_headers;
  sd->section_headers = NULL;
  delete sd->section_names;
  sd->section_names = NULL;
}

// Add an entry to the vector mapping version numbers to version
// strings.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::set_version_map(
    Version_map* version_map,
    unsigned int ndx,
    const char* name) const
{
  if (ndx >= version_map->size())
    version_map->resize(ndx + 1);
  if ((*version_map)[ndx] != NULL)
    this->error(_("duplicate definition for version %u"), ndx);
  (*version_map)[ndx] = name;
}

// Add mappings for the version definitions to VERSION_MAP.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::make_verdef_map(
    Read_symbols_data* sd,
    Version_map* version_map) const
{
  if (sd->verdef == NULL)
    return;

  const char* names = reinterpret_cast<const char*>(sd->symbol_names->data());
  section_size_type names_size = sd->symbol_names_size;

  const unsigned char* pverdef = sd->verdef->data();
  section_size_type verdef_size = sd->verdef_size;
  const unsigned int count = sd->verdef_info;

  const unsigned char* p = pverdef;
  for (unsigned int i = 0; i < count; ++i)
    {
      elfcpp::Verdef<size, big_endian> verdef(p);

      if (verdef.get_vd_version() != elfcpp::VER_DEF_CURRENT)
	{
	  this->error(_("unexpected verdef version %u"),
		      verdef.get_vd_version());
	  return;
	}

      const section_size_type vd_ndx = verdef.get_vd_ndx();

      // The GNU linker clears the VERSYM_HIDDEN bit.  I'm not
      // sure why.

      // The first Verdaux holds the name of this version.  Subsequent
      // ones are versions that this one depends upon, which we don't
      // care about here.
      const section_size_type vd_cnt = verdef.get_vd_cnt();
      if (vd_cnt < 1)
	{
	  this->error(_("verdef vd_cnt field too small: %u"),
                      static_cast<unsigned int>(vd_cnt));
	  return;
	}

      const section_size_type vd_aux = verdef.get_vd_aux();
      if ((p - pverdef) + vd_aux >= verdef_size)
	{
	  this->error(_("verdef vd_aux field out of range: %u"),
                      static_cast<unsigned int>(vd_aux));
	  return;
	}

      const unsigned char* pvda = p + vd_aux;
      elfcpp::Verdaux<size, big_endian> verdaux(pvda);

      const section_size_type vda_name = verdaux.get_vda_name();
      if (vda_name >= names_size)
	{
	  this->error(_("verdaux vda_name field out of range: %u"),
                      static_cast<unsigned int>(vda_name));
	  return;
	}

      this->set_version_map(version_map, vd_ndx, names + vda_name);

      const section_size_type vd_next = verdef.get_vd_next();
      if ((p - pverdef) + vd_next >= verdef_size)
	{
	  this->error(_("verdef vd_next field out of range: %u"),
                      static_cast<unsigned int>(vd_next));
	  return;
	}

      p += vd_next;
    }
}

// Add mappings for the required versions to VERSION_MAP.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::make_verneed_map(
    Read_symbols_data* sd,
    Version_map* version_map) const
{
  if (sd->verneed == NULL)
    return;

  const char* names = reinterpret_cast<const char*>(sd->symbol_names->data());
  section_size_type names_size = sd->symbol_names_size;

  const unsigned char* pverneed = sd->verneed->data();
  const section_size_type verneed_size = sd->verneed_size;
  const unsigned int count = sd->verneed_info;

  const unsigned char* p = pverneed;
  for (unsigned int i = 0; i < count; ++i)
    {
      elfcpp::Verneed<size, big_endian> verneed(p);

      if (verneed.get_vn_version() != elfcpp::VER_NEED_CURRENT)
	{
	  this->error(_("unexpected verneed version %u"),
		      verneed.get_vn_version());
	  return;
	}

      const section_size_type vn_aux = verneed.get_vn_aux();

      if ((p - pverneed) + vn_aux >= verneed_size)
	{
	  this->error(_("verneed vn_aux field out of range: %u"),
                      static_cast<unsigned int>(vn_aux));
	  return;
	}

      const unsigned int vn_cnt = verneed.get_vn_cnt();
      const unsigned char* pvna = p + vn_aux;
      for (unsigned int j = 0; j < vn_cnt; ++j)
	{
	  elfcpp::Vernaux<size, big_endian> vernaux(pvna);

	  const unsigned int vna_name = vernaux.get_vna_name();
	  if (vna_name >= names_size)
	    {
	      this->error(_("vernaux vna_name field out of range: %u"),
			  static_cast<unsigned int>(vna_name));
	      return;
	    }

	  this->set_version_map(version_map, vernaux.get_vna_other(),
				names + vna_name);

	  const section_size_type vna_next = vernaux.get_vna_next();
	  if ((pvna - pverneed) + vna_next >= verneed_size)
	    {
	      this->error(_("verneed vna_next field out of range: %u"),
			  static_cast<unsigned int>(vna_next));
	      return;
	    }

	  pvna += vna_next;
	}

      const section_size_type vn_next = verneed.get_vn_next();
      if ((p - pverneed) + vn_next >= verneed_size)
	{
	  this->error(_("verneed vn_next field out of range: %u"),
                      static_cast<unsigned int>(vn_next));
	  return;
	}

      p += vn_next;
    }
}

// Create a vector mapping version numbers to version strings.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::make_version_map(
    Read_symbols_data* sd,
    Version_map* version_map) const
{
  if (sd->verdef == NULL && sd->verneed == NULL)
    return;

  // A guess at the maximum version number we will see.  If this is
  // wrong we will be less efficient but still correct.
  version_map->reserve(sd->verdef_info + sd->verneed_info * 10);

  this->make_verdef_map(sd, version_map);
  this->make_verneed_map(sd, version_map);
}

// Add the dynamic symbols to the symbol table.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_add_symbols(Symbol_table* symtab,
					       Read_symbols_data* sd,
					       Layout*)
{
  if (sd->symbols == NULL)
    {
      gold_assert(sd->symbol_names == NULL);
      gold_assert(sd->versym == NULL && sd->verdef == NULL
		  && sd->verneed == NULL);
      return;
    }

  const int sym_size = This::sym_size;
  const size_t symcount = sd->symbols_size / sym_size;
  gold_assert(sd->external_symbols_offset == 0);
  if (symcount * sym_size != sd->symbols_size)
    {
      this->error(_("size of dynamic symbols is not multiple of symbol size"));
      return;
    }

  Version_map version_map;
  this->make_version_map(sd, &version_map);

  // If printing symbol counts or a cross reference table or
  // preparing for an incremental link, we want to track symbols.
  if (parameters->options().user_set_print_symbol_counts()
      || parameters->options().cref()
      || parameters->incremental())
    {
      this->symbols_ = new Symbols();
      this->symbols_->resize(symcount);
    }

  const char* sym_names =
    reinterpret_cast<const char*>(sd->symbol_names->data());
  symtab->add_from_dynobj(this, sd->symbols->data(), symcount,
			  sym_names, sd->symbol_names_size,
			  (sd->versym == NULL
			   ? NULL
			   : sd->versym->data()),
			  sd->versym_size,
			  &version_map,
			  this->symbols_,
			  &this->defined_count_);

  delete sd->symbols;
  sd->symbols = NULL;
  delete sd->symbol_names;
  sd->symbol_names = NULL;
  if (sd->versym != NULL)
    {
      delete sd->versym;
      sd->versym = NULL;
    }
  if (sd->verdef != NULL)
    {
      delete sd->verdef;
      sd->verdef = NULL;
    }
  if (sd->verneed != NULL)
    {
      delete sd->verneed;
      sd->verneed = NULL;
    }

  // This is normally the last time we will read any data from this
  // file.
  this->clear_view_cache_marks();
}

template<int size, bool big_endian>
Archive::Should_include
Sized_dynobj<size, big_endian>::do_should_include_member(Symbol_table*,
							 Layout*,
							 Read_symbols_data*,
							 std::string*)
{
  return Archive::SHOULD_INCLUDE_YES;
}

// Iterate over global symbols, calling a visitor class V for each.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_for_all_global_symbols(
    Read_symbols_data* sd,
    Library_base::Symbol_visitor_base* v)
{
  const char* sym_names =
      reinterpret_cast<const char*>(sd->symbol_names->data());
  const unsigned char* syms =
      sd->symbols->data() + sd->external_symbols_offset;
  const int sym_size = elfcpp::Elf_sizes<size>::sym_size;
  size_t symcount = ((sd->symbols_size - sd->external_symbols_offset)
                     / sym_size);
  const unsigned char* p = syms;

  for (size_t i = 0; i < symcount; ++i, p += sym_size)
    {
      elfcpp::Sym<size, big_endian> sym(p);
      if (sym.get_st_shndx() != elfcpp::SHN_UNDEF
	  && sym.get_st_bind() != elfcpp::STB_LOCAL)
	v->visit(sym_names + sym.get_st_name());
    }
}

// Iterate over local symbols, calling a visitor class V for each GOT offset
// associated with a local symbol.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_for_all_local_got_entries(
    Got_offset_list::Visitor*) const
{
}

// Get symbol counts.

template<int size, bool big_endian>
void
Sized_dynobj<size, big_endian>::do_get_global_symbol_counts(
    const Symbol_table*,
    size_t* defined,
    size_t* used) const
{
  *defined = this->defined_count_;
  size_t count = 0;
  for (typename Symbols::const_iterator p = this->symbols_->begin();
       p != this->symbols_->end();
       ++p)
    if (*p != NULL
	&& (*p)->source() == Symbol::FROM_OBJECT
	&& (*p)->object() == this
	&& (*p)->is_defined()
	&& (*p)->has_dynsym_index())
      ++count;
  *used = count;
}

// Given a vector of hash codes, compute the number of hash buckets to
// use.

unsigned int
Dynobj::compute_bucket_count(const std::vector<uint32_t>& hashcodes,
			     bool for_gnu_hash_table)
{
  // FIXME: Implement optional hash table optimization.

  // Array used to determine the number of hash table buckets to use
  // based on the number of symbols there are.  If there are fewer
  // than 3 symbols we use 1 bucket, fewer than 17 symbols we use 3
  // buckets, fewer than 37 we use 17 buckets, and so forth.  We never
  // use more than 262147 buckets.  This is straight from the old GNU
  // linker.
  static const unsigned int buckets[] =
  {
    1, 3, 17, 37, 67, 97, 131, 197, 263, 521, 1031, 2053, 4099, 8209,
    16411, 32771, 65537, 131101, 262147
  };
  const int buckets_count = sizeof buckets / sizeof buckets[0];

  unsigned int symcount = hashcodes.size();
  unsigned int ret = 1;
  const double full_fraction
    = 1.0 - parameters->options().hash_bucket_empty_fraction();
  for (int i = 0; i < buckets_count; ++i)
    {
      if (symcount < buckets[i] * full_fraction)
	break;
      ret = buckets[i];
    }

  if (for_gnu_hash_table && ret < 2)
    ret = 2;

  return ret;
}

// The standard ELF hash function.  This hash function must not
// change, as the dynamic linker uses it also.

uint32_t
Dynobj::elf_hash(const char* name)
{
  const unsigned char* nameu = reinterpret_cast<const unsigned char*>(name);
  uint32_t h = 0;
  unsigned char c;
  while ((c = *nameu++) != '\0')
    {
      h = (h << 4) + c;
      uint32_t g = h & 0xf0000000;
      if (g != 0)
	{
	  h ^= g >> 24;
	  // The ELF ABI says h &= ~g, but using xor is equivalent in
	  // this case (since g was set from h) and may save one
	  // instruction.
	  h ^= g;
	}
    }
  return h;
}

// Create a standard ELF hash table, setting *PPHASH and *PHASHLEN.
// DYNSYMS is a vector with all the global dynamic symbols.
// LOCAL_DYNSYM_COUNT is the number of local symbols in the dynamic
// symbol table.

void
Dynobj::create_elf_hash_table(const std::vector<Symbol*>& dynsyms,
			      unsigned int local_dynsym_count,
			      unsigned char** pphash,
			      unsigned int* phashlen)
{
  unsigned int dynsym_count = dynsyms.size();

  // Get the hash values for all the symbols.
  std::vector<uint32_t> dynsym_hashvals(dynsym_count);
  for (unsigned int i = 0; i < dynsym_count; ++i)
    dynsym_hashvals[i] = Dynobj::elf_hash(dynsyms[i]->name());

  const unsigned int bucketcount =
    Dynobj::compute_bucket_count(dynsym_hashvals, false);

  std::vector<uint32_t> bucket(bucketcount);
  std::vector<uint32_t> chain(local_dynsym_count + dynsym_count);

  for (unsigned int i = 0; i < dynsym_count; ++i)
    {
      unsigned int dynsym_index = dynsyms[i]->dynsym_index();
      unsigned int bucketpos = dynsym_hashvals[i] % bucketcount;
      chain[dynsym_index] = bucket[bucketpos];
      bucket[bucketpos] = dynsym_index;
    }

  int size = parameters->target().hash_entry_size();
  unsigned int hashlen = ((2
			   + bucketcount
			   + local_dynsym_count
			   + dynsym_count)
			  * size / 8);
  unsigned char* phash = new unsigned char[hashlen];

  bool big_endian = parameters->target().is_big_endian();
  if (size == 32)
    {
      if (big_endian)
	{
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
	  Dynobj::sized_create_elf_hash_table<32, true>(bucket, chain, phash,
							hashlen);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
	  Dynobj::sized_create_elf_hash_table<32, false>(bucket, chain, phash,
							 hashlen);
#else
	  gold_unreachable();
#endif
	}
    }
  else if (size == 64)
    {
      if (big_endian)
	{
#if defined(HAVE_TARGET_32_BIG) || defined(HAVE_TARGET_64_BIG)
	  Dynobj::sized_create_elf_hash_table<64, true>(bucket, chain, phash,
							hashlen);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_64_LITTLE)
	  Dynobj::sized_create_elf_hash_table<64, false>(bucket, chain, phash,
							 hashlen);
#else
	  gold_unreachable();
#endif
	}
    }
  else
    gold_unreachable();

  *pphash = phash;
  *phashlen = hashlen;
}

// Fill in an ELF hash table.

template<int size, bool big_endian>
void
Dynobj::sized_create_elf_hash_table(const std::vector<uint32_t>& bucket,
				    const std::vector<uint32_t>& chain,
				    unsigned char* phash,
				    unsigned int hashlen)
{
  unsigned char* p = phash;

  const unsigned int bucketcount = bucket.size();
  const unsigned int chaincount = chain.size();

  elfcpp::Swap<size, big_endian>::writeval(p, bucketcount);
  p += size / 8;
  elfcpp::Swap<size, big_endian>::writeval(p, chaincount);
  p += size / 8;

  for (unsigned int i = 0; i < bucketcount; ++i)
    {
      elfcpp::Swap<size, big_endian>::writeval(p, bucket[i]);
      p += size / 8;
    }

  for (unsigned int i = 0; i < chaincount; ++i)
    {
      elfcpp::Swap<size, big_endian>::writeval(p, chain[i]);
      p += size / 8;
    }

  gold_assert(static_cast<unsigned int>(p - phash) == hashlen);
}

// The hash function used for the GNU hash table.  This hash function
// must not change, as the dynamic linker uses it also.

uint32_t
Dynobj::gnu_hash(const char* name)
{
  const unsigned char* nameu = reinterpret_cast<const unsigned char*>(name);
  uint32_t h = 5381;
  unsigned char c;
  while ((c = *nameu++) != '\0')
    h = (h << 5) + h + c;
  return h;
}

// Create a GNU hash table, setting *PPHASH and *PHASHLEN.  GNU hash
// tables are an extension to ELF which are recognized by the GNU
// dynamic linker.  They are referenced using dynamic tag DT_GNU_HASH.
// TARGET is the target.  DYNSYMS is a vector with all the global
// symbols which will be going into the dynamic symbol table.
// LOCAL_DYNSYM_COUNT is the number of local symbols in the dynamic
// symbol table.

void
Dynobj::create_gnu_hash_table(const std::vector<Symbol*>& dynsyms,
			      unsigned int local_dynsym_count,
			      unsigned char** pphash,
			      unsigned int* phashlen)
{
  const unsigned int count = dynsyms.size();

  // Sort the dynamic symbols into two vectors.  Symbols which we do
  // not want to put into the hash table we store into
  // UNHASHED_DYNSYMS.  Symbols which we do want to store we put into
  // HASHED_DYNSYMS.  DYNSYM_HASHVALS is parallel to HASHED_DYNSYMS,
  // and records the hash codes.

  std::vector<Symbol*> unhashed_dynsyms;
  unhashed_dynsyms.reserve(count);

  std::vector<Symbol*> hashed_dynsyms;
  hashed_dynsyms.reserve(count);

  std::vector<uint32_t> dynsym_hashvals;
  dynsym_hashvals.reserve(count);
  
  for (unsigned int i = 0; i < count; ++i)
    {
      Symbol* sym = dynsyms[i];

      if (!sym->needs_dynsym_value()
	  && (sym->is_undefined()
	      || sym->is_from_dynobj()
	      || sym->is_forced_local()))
	unhashed_dynsyms.push_back(sym);
      else
	{
	  hashed_dynsyms.push_back(sym);
	  dynsym_hashvals.push_back(Dynobj::gnu_hash(sym->name()));
	}
    }

  // Put the unhashed symbols at the start of the global portion of
  // the dynamic symbol table.
  const unsigned int unhashed_count = unhashed_dynsyms.size();
  unsigned int unhashed_dynsym_index = local_dynsym_count;
  for (unsigned int i = 0; i < unhashed_count; ++i)
    {
      unhashed_dynsyms[i]->set_dynsym_index(unhashed_dynsym_index);
      ++unhashed_dynsym_index;
    }

  // For the actual data generation we call out to a templatized
  // function.
  int size = parameters->target().get_size();
  bool big_endian = parameters->target().is_big_endian();
  if (size == 32)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_32_BIG
	  Dynobj::sized_create_gnu_hash_table<32, true>(hashed_dynsyms,
							dynsym_hashvals,
							unhashed_dynsym_index,
							pphash,
							phashlen);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_32_LITTLE
	  Dynobj::sized_create_gnu_hash_table<32, false>(hashed_dynsyms,
							 dynsym_hashvals,
							 unhashed_dynsym_index,
							 pphash,
							 phashlen);
#else
	  gold_unreachable();
#endif
	}
    }
  else if (size == 64)
    {
      if (big_endian)
	{
#ifdef HAVE_TARGET_64_BIG
	  Dynobj::sized_create_gnu_hash_table<64, true>(hashed_dynsyms,
							dynsym_hashvals,
							unhashed_dynsym_index,
							pphash,
							phashlen);
#else
	  gold_unreachable();
#endif
	}
      else
	{
#ifdef HAVE_TARGET_64_LITTLE
	  Dynobj::sized_create_gnu_hash_table<64, false>(hashed_dynsyms,
							 dynsym_hashvals,
							 unhashed_dynsym_index,
							 pphash,
							 phashlen);
#else
	  gold_unreachable();
#endif
	}
    }
  else
    gold_unreachable();
}

// Create the actual data for a GNU hash table.  This is just a copy
// of the code from the old GNU linker.

template<int size, bool big_endian>
void
Dynobj::sized_create_gnu_hash_table(
    const std::vector<Symbol*>& hashed_dynsyms,
    const std::vector<uint32_t>& dynsym_hashvals,
    unsigned int unhashed_dynsym_count,
    unsigned char** pphash,
    unsigned int* phashlen)
{
  if (hashed_dynsyms.empty())
    {
      // Special case for the empty hash table.
      unsigned int hashlen = 5 * 4 + size / 8;
      unsigned char* phash = new unsigned char[hashlen];
      // One empty bucket.
      elfcpp::Swap<32, big_endian>::writeval(phash, 1);
      // Symbol index above unhashed symbols.
      elfcpp::Swap<32, big_endian>::writeval(phash + 4, unhashed_dynsym_count);
      // One word for bitmask.
      elfcpp::Swap<32, big_endian>::writeval(phash + 8, 1);
      // Only bloom filter.
      elfcpp::Swap<32, big_endian>::writeval(phash + 12, 0);
      // No valid hashes.
      elfcpp::Swap<size, big_endian>::writeval(phash + 16, 0);
      // No hashes in only bucket.
      elfcpp::Swap<32, big_endian>::writeval(phash + 16 + size / 8, 0);

      *phashlen = hashlen;
      *pphash = phash;

      return;
    }

  const unsigned int bucketcount =
    Dynobj::compute_bucket_count(dynsym_hashvals, true);

  const unsigned int nsyms = hashed_dynsyms.size();

  uint32_t maskbitslog2 = 1;
  uint32_t x = nsyms >> 1;
  while (x != 0)
    {
      ++maskbitslog2;
      x >>= 1;
    }
  if (maskbitslog2 < 3)
    maskbitslog2 = 5;
  else if (((1U << (maskbitslog2 - 2)) & nsyms) != 0)
    maskbitslog2 += 3;
  else
    maskbitslog2 += 2;

  uint32_t shift1;
  if (size == 32)
    shift1 = 5;
  else
    {
      if (maskbitslog2 == 5)
	maskbitslog2 = 6;
      shift1 = 6;
    }
  uint32_t mask = (1U << shift1) - 1U;
  uint32_t shift2 = maskbitslog2;
  uint32_t maskbits = 1U << maskbitslog2;
  uint32_t maskwords = 1U << (maskbitslog2 - shift1);

  typedef typename elfcpp::Elf_types<size>::Elf_WXword Word;
  std::vector<Word> bitmask(maskwords);
  std::vector<uint32_t> counts(bucketcount);
  std::vector<uint32_t> indx(bucketcount);
  uint32_t symindx = unhashed_dynsym_count;

  // Count the number of times each hash bucket is used.
  for (unsigned int i = 0; i < nsyms; ++i)
    ++counts[dynsym_hashvals[i] % bucketcount];

  unsigned int cnt = symindx;
  for (unsigned int i = 0; i < bucketcount; ++i)
    {
      indx[i] = cnt;
      cnt += counts[i];
    }

  unsigned int hashlen = (4 + bucketcount + nsyms) * 4;
  hashlen += maskbits / 8;
  unsigned char* phash = new unsigned char[hashlen];

  elfcpp::Swap<32, big_endian>::writeval(phash, bucketcount);
  elfcpp::Swap<32, big_endian>::writeval(phash + 4, symindx);
  elfcpp::Swap<32, big_endian>::writeval(phash + 8, maskwords);
  elfcpp::Swap<32, big_endian>::writeval(phash + 12, shift2);

  unsigned char* p = phash + 16 + maskbits / 8;
  for (unsigned int i = 0; i < bucketcount; ++i)
    {
      if (counts[i] == 0)
	elfcpp::Swap<32, big_endian>::writeval(p, 0);
      else
	elfcpp::Swap<32, big_endian>::writeval(p, indx[i]);
      p += 4;
    }

  for (unsigned int i = 0; i < nsyms; ++i)
    {
      Symbol* sym = hashed_dynsyms[i];
      uint32_t hashval = dynsym_hashvals[i];

      unsigned int bucket = hashval % bucketcount;
      unsigned int val = ((hashval >> shift1)
			  & ((maskbits >> shift1) - 1));
      bitmask[val] |= (static_cast<Word>(1U)) << (hashval & mask);
      bitmask[val] |= (static_cast<Word>(1U)) << ((hashval >> shift2) & mask);
      val = hashval & ~ 1U;
      if (counts[bucket] == 1)
	{
	  // Last element terminates the chain.
	  val |= 1;
	}
      elfcpp::Swap<32, big_endian>::writeval(p + (indx[bucket] - symindx) * 4,
					     val);
      --counts[bucket];

      sym->set_dynsym_index(indx[bucket]);
      ++indx[bucket];
    }

  p = phash + 16;
  for (unsigned int i = 0; i < maskwords; ++i)
    {
      elfcpp::Swap<size, big_endian>::writeval(p, bitmask[i]);
      p += size / 8;
    }

  *phashlen = hashlen;
  *pphash = phash;
}

// Verdef methods.

// Write this definition to a buffer for the output section.

template<int size, bool big_endian>
unsigned char*
Verdef::write(const Stringpool* dynpool, bool is_last, unsigned char* pb) const
{
  const int verdef_size = elfcpp::Elf_sizes<size>::verdef_size;
  const int verdaux_size = elfcpp::Elf_sizes<size>::verdaux_size;

  elfcpp::Verdef_write<size, big_endian> vd(pb);
  vd.set_vd_version(elfcpp::VER_DEF_CURRENT);
  vd.set_vd_flags((this->is_base_ ? elfcpp::VER_FLG_BASE : 0)
		  | (this->is_weak_ ? elfcpp::VER_FLG_WEAK : 0)
		  | (this->is_info_ ? elfcpp::VER_FLG_INFO : 0));
  vd.set_vd_ndx(this->index());
  vd.set_vd_cnt(1 + this->deps_.size());
  vd.set_vd_hash(Dynobj::elf_hash(this->name()));
  vd.set_vd_aux(verdef_size);
  vd.set_vd_next(is_last
		 ? 0
		 : verdef_size + (1 + this->deps_.size()) * verdaux_size);
  pb += verdef_size;

  elfcpp::Verdaux_write<size, big_endian> vda(pb);
  vda.set_vda_name(dynpool->get_offset(this->name()));
  vda.set_vda_next(this->deps_.empty() ? 0 : verdaux_size);
  pb += verdaux_size;

  Deps::const_iterator p;
  unsigned int i;
  for (p = this->deps_.begin(), i = 0;
       p != this->deps_.end();
       ++p, ++i)
    {
      elfcpp::Verdaux_write<size, big_endian> vda(pb);
      vda.set_vda_name(dynpool->get_offset(*p));
      vda.set_vda_next(i + 1 >= this->deps_.size() ? 0 : verdaux_size);
      pb += verdaux_size;
    }

  return pb;
}

// Verneed methods.

Verneed::~Verneed()
{
  for (Need_versions::iterator p = this->need_versions_.begin();
       p != this->need_versions_.end();
       ++p)
    delete *p;
}

// Add a new version to this file reference.

Verneed_version*
Verneed::add_name(const char* name)
{
  Verneed_version* vv = new Verneed_version(name);
  this->need_versions_.push_back(vv);
  return vv;
}

// Set the version indexes starting at INDEX.

unsigned int
Verneed::finalize(unsigned int index)
{
  for (Need_versions::iterator p = this->need_versions_.begin();
       p != this->need_versions_.end();
       ++p)
    {
      (*p)->set_index(index);
      ++index;
    }
  return index;
}

// Write this list of referenced versions to a buffer for the output
// section.

template<int size, bool big_endian>
unsigned char*
Verneed::write(const Stringpool* dynpool, bool is_last,
	       unsigned char* pb) const
{
  const int verneed_size = elfcpp::Elf_sizes<size>::verneed_size;
  const int vernaux_size = elfcpp::Elf_sizes<size>::vernaux_size;

  elfcpp::Verneed_write<size, big_endian> vn(pb);
  vn.set_vn_version(elfcpp::VER_NEED_CURRENT);
  vn.set_vn_cnt(this->need_versions_.size());
  vn.set_vn_file(dynpool->get_offset(this->filename()));
  vn.set_vn_aux(verneed_size);
  vn.set_vn_next(is_last
		 ? 0
		 : verneed_size + this->need_versions_.size() * vernaux_size);
  pb += verneed_size;

  Need_versions::const_iterator p;
  unsigned int i;
  for (p = this->need_versions_.begin(), i = 0;
       p != this->need_versions_.end();
       ++p, ++i)
    {
      elfcpp::Vernaux_write<size, big_endian> vna(pb);
      vna.set_vna_hash(Dynobj::elf_hash((*p)->version()));
      // FIXME: We need to sometimes set VER_FLG_WEAK here.
      vna.set_vna_flags(0);
      vna.set_vna_other((*p)->index());
      vna.set_vna_name(dynpool->get_offset((*p)->version()));
      vna.set_vna_next(i + 1 >= this->need_versions_.size()
		       ? 0
		       : vernaux_size);
      pb += vernaux_size;
    }

  return pb;
}

// Versions methods.

Versions::Versions(const Version_script_info& version_script,
                   Stringpool* dynpool)
  : defs_(), needs_(), version_table_(),
    is_finalized_(false), version_script_(version_script),
    needs_base_version_(true)
{
  if (!this->version_script_.empty())
    {
      // Parse the version script, and insert each declared version into
      // defs_ and version_table_.
      std::vector<std::string> versions = this->version_script_.get_versions();

      if (this->needs_base_version_ && !versions.empty())
	this->define_base_version(dynpool);

      for (size_t k = 0; k < versions.size(); ++k)
        {
          Stringpool::Key version_key;
          const char* version = dynpool->add(versions[k].c_str(),
                                             true, &version_key);
          Verdef* const vd = new Verdef(
              version,
              this->version_script_.get_dependencies(version),
              false, false, false, false);
          this->defs_.push_back(vd);
          Key key(version_key, 0);
          this->version_table_.insert(std::make_pair(key, vd));
        }
    }
}

Versions::~Versions()
{
  for (Defs::iterator p = this->defs_.begin();
       p != this->defs_.end();
       ++p)
    delete *p;

  for (Needs::iterator p = this->needs_.begin();
       p != this->needs_.end();
       ++p)
    delete *p;
}

// Define the base version of a shared library.  The base version definition
// must be the first entry in defs_.  We insert it lazily so that defs_ is
// empty if no symbol versioning is used.  Then layout can just drop the
// version sections.

void
Versions::define_base_version(Stringpool* dynpool)
{
  // If we do any versioning at all,  we always need a base version, so
  // define that first.  Nothing explicitly declares itself as part of base,
  // so it doesn't need to be in version_table_.
  gold_assert(this->defs_.empty());
  const char* name = parameters->options().soname();
  if (name == NULL)
    name = parameters->options().output_file_name();
  name = dynpool->add(name, false, NULL);
  Verdef* vdbase = new Verdef(name, std::vector<std::string>(),
                              true, false, false, true);
  this->defs_.push_back(vdbase);
  this->needs_base_version_ = false;
}

// Return the dynamic object which a symbol refers to.

Dynobj*
Versions::get_dynobj_for_sym(const Symbol_table* symtab,
			     const Symbol* sym) const
{
  if (sym->is_copied_from_dynobj())
    return symtab->get_copy_source(sym);
  else
    {
      Object* object = sym->object();
      gold_assert(object->is_dynamic());
      return static_cast<Dynobj*>(object);
    }
}

// Record version information for a symbol going into the dynamic
// symbol table.

void
Versions::record_version(const Symbol_table* symtab,
			 Stringpool* dynpool, const Symbol* sym)
{
  gold_assert(!this->is_finalized_);
  gold_assert(sym->version() != NULL);

  // A symbol defined as "sym@" is bound to an unspecified base version.
  if (sym->version()[0] == '\0')
    return;

  Stringpool::Key version_key;
  const char* version = dynpool->add(sym->version(), false, &version_key);

  if (!sym->is_from_dynobj() && !sym->is_copied_from_dynobj())
    {
      this->add_def(dynpool, sym, version, version_key);
    }
  else
    {
      // This is a version reference.
      Dynobj* dynobj = this->get_dynobj_for_sym(symtab, sym);
      this->add_need(dynpool, dynobj->soname(), version, version_key);
    }
}

// We've found a symbol SYM defined in version VERSION.

void
Versions::add_def(Stringpool* dynpool, const Symbol* sym, const char* version,
		  Stringpool::Key version_key)
{
  Key k(version_key, 0);
  Version_base* const vbnull = NULL;
  std::pair<Version_table::iterator, bool> ins =
    this->version_table_.insert(std::make_pair(k, vbnull));

  if (!ins.second)
    {
      // We already have an entry for this version.
      Version_base* vb = ins.first->second;

      // We have now seen a symbol in this version, so it is not
      // weak.
      gold_assert(vb != NULL);
      vb->clear_weak();
    }
  else
    {
      // If we are creating a shared object, it is an error to
      // find a definition of a symbol with a version which is not
      // in the version script.
      if (parameters->options().shared())
	gold_error(_("symbol %s has undefined version %s"),
		   sym->demangled_name().c_str(), version);

      // When creating a regular executable, automatically define
      // a new version.
      if (this->needs_base_version_)
	this->define_base_version(dynpool);
      Verdef* vd = new Verdef(version, std::vector<std::string>(),
                              false, false, false, false);
      this->defs_.push_back(vd);
      ins.first->second = vd;
    }
}

// Add a reference to version NAME in file FILENAME.

void
Versions::add_need(Stringpool* dynpool, const char* filename, const char* name,
		   Stringpool::Key name_key)
{
  Stringpool::Key filename_key;
  filename = dynpool->add(filename, true, &filename_key);

  Key k(name_key, filename_key);
  Version_base* const vbnull = NULL;
  std::pair<Version_table::iterator, bool> ins =
    this->version_table_.insert(std::make_pair(k, vbnull));

  if (!ins.second)
    {
      // We already have an entry for this filename/version.
      return;
    }

  // See whether we already have this filename.  We don't expect many
  // version references, so we just do a linear search.  This could be
  // replaced by a hash table.
  Verneed* vn = NULL;
  for (Needs::iterator p = this->needs_.begin();
       p != this->needs_.end();
       ++p)
    {
      if ((*p)->filename() == filename)
	{
	  vn = *p;
	  break;
	}
    }

  if (vn == NULL)
    {
      // Create base version definition lazily for shared library.
      if (parameters->options().shared() && this->needs_base_version_)
	this->define_base_version(dynpool);

      // We have a new filename.
      vn = new Verneed(filename);
      this->needs_.push_back(vn);
    }

  ins.first->second = vn->add_name(name);
}

// Set the version indexes.  Create a new dynamic version symbol for
// each new version definition.

unsigned int
Versions::finalize(Symbol_table* symtab, unsigned int dynsym_index,
		   std::vector<Symbol*>* syms)
{
  gold_assert(!this->is_finalized_);

  unsigned int vi = 1;

  for (Defs::iterator p = this->defs_.begin();
       p != this->defs_.end();
       ++p)
    {
      (*p)->set_index(vi);
      ++vi;

      // Create a version symbol if necessary.
      if (!(*p)->is_symbol_created())
	{
	  Symbol* vsym = symtab->define_as_constant((*p)->name(),
						    (*p)->name(),
						    Symbol_table::PREDEFINED,
						    0, 0,
						    elfcpp::STT_OBJECT,
						    elfcpp::STB_GLOBAL,
						    elfcpp::STV_DEFAULT, 0,
						    false, false);
	  vsym->set_needs_dynsym_entry();
          vsym->set_dynsym_index(dynsym_index);
	  vsym->set_is_default();
	  ++dynsym_index;
	  syms->push_back(vsym);
	  // The name is already in the dynamic pool.
	}
    }

  // Index 1 is used for global symbols.
  if (vi == 1)
    {
      gold_assert(this->defs_.empty());
      vi = 2;
    }

  for (Needs::iterator p = this->needs_.begin();
       p != this->needs_.end();
       ++p)
    vi = (*p)->finalize(vi);

  this->is_finalized_ = true;

  return dynsym_index;
}

// Return the version index to use for a symbol.  This does two hash
// table lookups: one in DYNPOOL and one in this->version_table_.
// Another approach alternative would be store a pointer in SYM, which
// would increase the size of the symbol table.  Or perhaps we could
// use a hash table from dynamic symbol pointer values to Version_base
// pointers.

unsigned int
Versions::version_index(const Symbol_table* symtab, const Stringpool* dynpool,
			const Symbol* sym) const
{
  Stringpool::Key version_key;
  const char* version = dynpool->find(sym->version(), &version_key);
  gold_assert(version != NULL);

  Key k;
  if (!sym->is_from_dynobj() && !sym->is_copied_from_dynobj())
    {
      k = Key(version_key, 0);
    }
  else
    {
      Dynobj* dynobj = this->get_dynobj_for_sym(symtab, sym);

      Stringpool::Key filename_key;
      const char* filename = dynpool->find(dynobj->soname(), &filename_key);
      gold_assert(filename != NULL);

      k = Key(version_key, filename_key);
    }

  Version_table::const_iterator p = this->version_table_.find(k);
  gold_assert(p != this->version_table_.end());

  return p->second->index();
}

// Return an allocated buffer holding the contents of the symbol
// version section.

template<int size, bool big_endian>
void
Versions::symbol_section_contents(const Symbol_table* symtab,
				  const Stringpool* dynpool,
				  unsigned int local_symcount,
				  const std::vector<Symbol*>& syms,
				  unsigned char** pp,
				  unsigned int* psize) const
{
  gold_assert(this->is_finalized_);

  unsigned int sz = (local_symcount + syms.size()) * 2;
  unsigned char* pbuf = new unsigned char[sz];

  for (unsigned int i = 0; i < local_symcount; ++i)
    elfcpp::Swap<16, big_endian>::writeval(pbuf + i * 2,
					   elfcpp::VER_NDX_LOCAL);

  for (std::vector<Symbol*>::const_iterator p = syms.begin();
       p != syms.end();
       ++p)
    {
      unsigned int version_index;
      const char* version = (*p)->version();
      if (version == NULL)
	{
	  if ((*p)->is_defined() && !(*p)->is_from_dynobj())
	    version_index = elfcpp::VER_NDX_GLOBAL;
	  else
	    version_index = elfcpp::VER_NDX_LOCAL;
	}
      else if (version[0] == '\0')
        version_index = elfcpp::VER_NDX_GLOBAL;
      else
	version_index = this->version_index(symtab, dynpool, *p);
      // If the symbol was defined as foo@V1 instead of foo@@V1, add
      // the hidden bit.
      if ((*p)->version() != NULL
	  && (*p)->is_defined()
	  && !(*p)->is_default()
	  && !(*p)->from_dyn())
        version_index |= elfcpp::VERSYM_HIDDEN;
      elfcpp::Swap<16, big_endian>::writeval(pbuf + (*p)->dynsym_index() * 2,
                                             version_index);
    }

  *pp = pbuf;
  *psize = sz;
}

// Return an allocated buffer holding the contents of the version
// definition section.

template<int size, bool big_endian>
void
Versions::def_section_contents(const Stringpool* dynpool,
			       unsigned char** pp, unsigned int* psize,
			       unsigned int* pentries) const
{
  gold_assert(this->is_finalized_);
  gold_assert(!this->defs_.empty());

  const int verdef_size = elfcpp::Elf_sizes<size>::verdef_size;
  const int verdaux_size = elfcpp::Elf_sizes<size>::verdaux_size;

  unsigned int sz = 0;
  for (Defs::const_iterator p = this->defs_.begin();
       p != this->defs_.end();
       ++p)
    {
      sz += verdef_size + verdaux_size;
      sz += (*p)->count_dependencies() * verdaux_size;
    }

  unsigned char* pbuf = new unsigned char[sz];

  unsigned char* pb = pbuf;
  Defs::const_iterator p;
  unsigned int i;
  for (p = this->defs_.begin(), i = 0;
       p != this->defs_.end();
       ++p, ++i)
    pb = (*p)->write<size, big_endian>(dynpool,
				       i + 1 >= this->defs_.size(),
				       pb);

  gold_assert(static_cast<unsigned int>(pb - pbuf) == sz);

  *pp = pbuf;
  *psize = sz;
  *pentries = this->defs_.size();
}

// Return an allocated buffer holding the contents of the version
// reference section.

template<int size, bool big_endian>
void
Versions::need_section_contents(const Stringpool* dynpool,
				unsigned char** pp, unsigned int* psize,
				unsigned int* pentries) const
{
  gold_assert(this->is_finalized_);
  gold_assert(!this->needs_.empty());

  const int verneed_size = elfcpp::Elf_sizes<size>::verneed_size;
  const int vernaux_size = elfcpp::Elf_sizes<size>::vernaux_size;

  unsigned int sz = 0;
  for (Needs::const_iterator p = this->needs_.begin();
       p != this->needs_.end();
       ++p)
    {
      sz += verneed_size;
      sz += (*p)->count_versions() * vernaux_size;
    }

  unsigned char* pbuf = new unsigned char[sz];

  unsigned char* pb = pbuf;
  Needs::const_iterator p;
  unsigned int i;
  for (p = this->needs_.begin(), i = 0;
       p != this->needs_.end();
       ++p, ++i)
    pb = (*p)->write<size, big_endian>(dynpool,
				       i + 1 >= this->needs_.size(),
				       pb);

  gold_assert(static_cast<unsigned int>(pb - pbuf) == sz);

  *pp = pbuf;
  *psize = sz;
  *pentries = this->needs_.size();
}

// Instantiate the templates we need.  We could use the configure
// script to restrict this to only the ones for implemented targets.

#ifdef HAVE_TARGET_32_LITTLE
template
class Sized_dynobj<32, false>;
#endif

#ifdef HAVE_TARGET_32_BIG
template
class Sized_dynobj<32, true>;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
class Sized_dynobj<64, false>;
#endif

#ifdef HAVE_TARGET_64_BIG
template
class Sized_dynobj<64, true>;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Versions::symbol_section_contents<32, false>(
    const Symbol_table*,
    const Stringpool*,
    unsigned int,
    const std::vector<Symbol*>&,
    unsigned char**,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Versions::symbol_section_contents<32, true>(
    const Symbol_table*,
    const Stringpool*,
    unsigned int,
    const std::vector<Symbol*>&,
    unsigned char**,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Versions::symbol_section_contents<64, false>(
    const Symbol_table*,
    const Stringpool*,
    unsigned int,
    const std::vector<Symbol*>&,
    unsigned char**,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Versions::symbol_section_contents<64, true>(
    const Symbol_table*,
    const Stringpool*,
    unsigned int,
    const std::vector<Symbol*>&,
    unsigned char**,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Versions::def_section_contents<32, false>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Versions::def_section_contents<32, true>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Versions::def_section_contents<64, false>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Versions::def_section_contents<64, true>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_32_LITTLE
template
void
Versions::need_section_contents<32, false>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_32_BIG
template
void
Versions::need_section_contents<32, true>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_LITTLE
template
void
Versions::need_section_contents<64, false>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

#ifdef HAVE_TARGET_64_BIG
template
void
Versions::need_section_contents<64, true>(
    const Stringpool*,
    unsigned char**,
    unsigned int*,
    unsigned int*) const;
#endif

} // End namespace gold.
