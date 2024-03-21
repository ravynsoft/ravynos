// resolve.cc -- symbol resolution for gold

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

#include "elfcpp.h"
#include "target.h"
#include "object.h"
#include "symtab.h"
#include "plugin.h"

namespace gold
{

// Symbol methods used in this file.

// This symbol is being overridden by another symbol whose version is
// VERSION.  Update the VERSION_ field accordingly.

inline void
Symbol::override_version(const char* version)
{
  if (version == NULL)
    {
      // This is the case where this symbol is NAME/VERSION, and the
      // version was not marked as hidden.  That makes it the default
      // version, so we create NAME/NULL.  Later we see another symbol
      // NAME/NULL, and that symbol is overriding this one.  In this
      // case, since NAME/VERSION is the default, we make NAME/NULL
      // override NAME/VERSION as well.  They are already the same
      // Symbol structure.  Setting the VERSION_ field to NULL ensures
      // that it will be output with the correct, empty, version.
      this->version_ = version;
    }
  else
    {
      // This is the case where this symbol is NAME/VERSION_ONE, and
      // now we see NAME/VERSION_TWO, and NAME/VERSION_TWO is
      // overriding NAME.  If VERSION_ONE and VERSION_TWO are
      // different, then this can only happen when VERSION_ONE is NULL
      // and VERSION_TWO is not hidden.
      gold_assert(this->version_ == version || this->version_ == NULL);
      this->version_ = version;
    }
}

// This symbol is being overidden by another symbol whose visibility
// is VISIBILITY.  Updated the VISIBILITY_ field accordingly.

inline void
Symbol::override_visibility(elfcpp::STV visibility)
{
  // The rule for combining visibility is that we always choose the
  // most constrained visibility.  In order of increasing constraint,
  // visibility goes PROTECTED, HIDDEN, INTERNAL.  This is the reverse
  // of the numeric values, so the effect is that we always want the
  // smallest non-zero value.
  if (visibility != elfcpp::STV_DEFAULT)
    {
      if (this->visibility_ == elfcpp::STV_DEFAULT)
	this->visibility_ = visibility;
      else if (this->visibility_ > visibility)
	this->visibility_ = visibility;
    }
}

// Override the fields in Symbol.

template<int size, bool big_endian>
void
Symbol::override_base(const elfcpp::Sym<size, big_endian>& sym,
		      unsigned int st_shndx, bool is_ordinary,
		      Object* object, const char* version)
{
  gold_assert(this->source_ == FROM_OBJECT);
  this->u1_.object = object;
  this->override_version(version);
  this->u2_.shndx = st_shndx;
  this->is_ordinary_shndx_ = is_ordinary;
  // Don't override st_type from plugin placeholder symbols.
  if (object->pluginobj() == NULL)
    this->type_ = sym.get_st_type();
  this->binding_ = sym.get_st_bind();
  this->override_visibility(sym.get_st_visibility());
  this->nonvis_ = sym.get_st_nonvis();
  if (object->is_dynamic())
    this->in_dyn_ = true;
  else
    this->in_reg_ = true;
}

// Override the fields in Sized_symbol.

template<int size>
template<bool big_endian>
void
Sized_symbol<size>::override(const elfcpp::Sym<size, big_endian>& sym,
			     unsigned st_shndx, bool is_ordinary,
			     Object* object, const char* version)
{
  this->override_base(sym, st_shndx, is_ordinary, object, version);
  this->value_ = sym.get_st_value();
  this->symsize_ = sym.get_st_size();
}

// Override TOSYM with symbol FROMSYM, defined in OBJECT, with version
// VERSION.  This handles all aliases of TOSYM.

template<int size, bool big_endian>
void
Symbol_table::override(Sized_symbol<size>* tosym,
		       const elfcpp::Sym<size, big_endian>& fromsym,
		       unsigned int st_shndx, bool is_ordinary,
		       Object* object, const char* version)
{
  tosym->override(fromsym, st_shndx, is_ordinary, object, version);
  if (tosym->has_alias())
    {
      Symbol* sym = this->weak_aliases_[tosym];
      gold_assert(sym != NULL);
      Sized_symbol<size>* ssym = this->get_sized_symbol<size>(sym);
      do
	{
	  ssym->override(fromsym, st_shndx, is_ordinary, object, version);
	  sym = this->weak_aliases_[ssym];
	  gold_assert(sym != NULL);
	  ssym = this->get_sized_symbol<size>(sym);
	}
      while (ssym != tosym);
    }
}

// The resolve functions build a little code for each symbol.
// Bit 0: 0 for global, 1 for weak.
// Bit 1: 0 for regular object, 1 for shared object
// Bits 2-3: 0 for normal, 1 for undefined, 2 for common
// This gives us values from 0 to 11.

static const int global_or_weak_shift = 0;
static const unsigned int global_flag = 0 << global_or_weak_shift;
static const unsigned int weak_flag = 1 << global_or_weak_shift;

static const int regular_or_dynamic_shift = 1;
static const unsigned int regular_flag = 0 << regular_or_dynamic_shift;
static const unsigned int dynamic_flag = 1 << regular_or_dynamic_shift;

static const int def_undef_or_common_shift = 2;
static const unsigned int def_flag = 0 << def_undef_or_common_shift;
static const unsigned int undef_flag = 1 << def_undef_or_common_shift;
static const unsigned int common_flag = 2 << def_undef_or_common_shift;

// This convenience function combines all the flags based on facts
// about the symbol.

static unsigned int
symbol_to_bits(elfcpp::STB binding, bool is_dynamic,
	       unsigned int shndx, bool is_ordinary)
{
  unsigned int bits;

  switch (binding)
    {
    case elfcpp::STB_GLOBAL:
    case elfcpp::STB_GNU_UNIQUE:
      bits = global_flag;
      break;

    case elfcpp::STB_WEAK:
      bits = weak_flag;
      break;

    case elfcpp::STB_LOCAL:
      // We should only see externally visible symbols in the symbol
      // table.
      gold_error(_("invalid STB_LOCAL symbol in external symbols"));
      bits = global_flag;
      break;

    default:
      // Any target which wants to handle STB_LOOS, etc., needs to
      // define a resolve method.
      gold_error(_("unsupported symbol binding %d"), static_cast<int>(binding));
      bits = global_flag;
    }

  if (is_dynamic)
    bits |= dynamic_flag;
  else
    bits |= regular_flag;

  switch (shndx)
    {
    case elfcpp::SHN_UNDEF:
      bits |= undef_flag;
      break;

    case elfcpp::SHN_COMMON:
      if (!is_ordinary)
	bits |= common_flag;
      break;

    default:
      if (!is_ordinary && Symbol::is_common_shndx(shndx))
	bits |= common_flag;
      else
        bits |= def_flag;
      break;
    }

  return bits;
}

// Resolve a symbol.  This is called the second and subsequent times
// we see a symbol.  TO is the pre-existing symbol.  ST_SHNDX is the
// section index for SYM, possibly adjusted for many sections.
// IS_ORDINARY is whether ST_SHNDX is a normal section index rather
// than a special code.  ORIG_ST_SHNDX is the original section index,
// before any munging because of discarded sections, except that all
// non-ordinary section indexes are mapped to SHN_UNDEF.  VERSION is
// the version of SYM.

template<int size, bool big_endian>
void
Symbol_table::resolve(Sized_symbol<size>* to,
		      const elfcpp::Sym<size, big_endian>& sym,
		      unsigned int st_shndx, bool is_ordinary,
		      unsigned int orig_st_shndx,
		      Object* object, const char* version,
		      bool is_default_version)
{
  bool to_is_ordinary;
  const unsigned int to_shndx = to->shndx(&to_is_ordinary);

  // It's possible for a symbol to be defined in an object file
  // using .symver to give it a version, and for there to also be
  // a linker script giving that symbol the same version.  We
  // don't want to give a multiple-definition error for this
  // harmless redefinition.
  if (to->source() == Symbol::FROM_OBJECT
      && to->object() == object
      && to->is_defined()
      && is_ordinary
      && to_is_ordinary
      && to_shndx == st_shndx
      && to->value() == sym.get_st_value())
    return;

  // Likewise for an absolute symbol defined twice with the same value.
  if (!is_ordinary
      && st_shndx == elfcpp::SHN_ABS
      && !to_is_ordinary
      && to_shndx == elfcpp::SHN_ABS
      && to->value() == sym.get_st_value())
    return;

  if (parameters->target().has_resolve())
    {
      Sized_target<size, big_endian>* sized_target;
      sized_target = parameters->sized_target<size, big_endian>();
      if (sized_target->resolve(to, sym, object, version))
	return;
    }

  if (!object->is_dynamic())
    {
      if (sym.get_st_type() == elfcpp::STT_COMMON
	  && (is_ordinary || !Symbol::is_common_shndx(st_shndx)))
	{
	  gold_warning(_("STT_COMMON symbol '%s' in %s "
			 "is not in a common section"),
		       to->demangled_name().c_str(),
		       to->object()->name().c_str());
	  return;
	}
      // Record that we've seen this symbol in a regular object.
      to->set_in_reg();
    }
  else if (st_shndx == elfcpp::SHN_UNDEF
           && (to->visibility() == elfcpp::STV_HIDDEN
               || to->visibility() == elfcpp::STV_INTERNAL))
    {
      // The symbol is hidden, so a reference from a shared object
      // cannot bind to it.  We tried issuing a warning in this case,
      // but that produces false positives when the symbol is
      // actually resolved in a different shared object (PR 15574).
      return;
    }
  else
    {
      // Record that we've seen this symbol in a dynamic object.
      to->set_in_dyn();
    }

  // Record if we've seen this symbol in a real ELF object (i.e., the
  // symbol is referenced from outside the world known to the plugin).
  if (object->pluginobj() == NULL && !object->is_dynamic())
    to->set_in_real_elf();

  // If we're processing replacement files, allow new symbols to override
  // the placeholders from the plugin objects.
  // Treat common symbols specially since it is possible that an ELF
  // file increased the size of the alignment.
  if (to->source() == Symbol::FROM_OBJECT)
    {
      Pluginobj* obj = to->object()->pluginobj();
      if (obj != NULL
          && parameters->options().plugins()->in_replacement_phase())
        {
	  bool adjust_common = false;
	  typename Sized_symbol<size>::Size_type tosize = 0;
	  typename Sized_symbol<size>::Value_type tovalue = 0;
	  if (to->is_common()
	      && !is_ordinary && Symbol::is_common_shndx(st_shndx))
	    {
	      adjust_common = true;
	      tosize = to->symsize();
	      tovalue = to->value();
	    }
	  this->override(to, sym, st_shndx, is_ordinary, object, version);
	  if (adjust_common)
	    {
	      if (tosize > to->symsize())
		to->set_symsize(tosize);
	      if (tovalue > to->value())
		to->set_value(tovalue);
	    }
	  return;
        }
    }

  // A new weak undefined reference, merging with an old weak
  // reference, could be a One Definition Rule (ODR) violation --
  // especially if the types or sizes of the references differ.  We'll
  // store such pairs and look them up later to make sure they
  // actually refer to the same lines of code.  We also check
  // combinations of weak and strong, which might occur if one case is
  // inline and the other is not.  (Note: not all ODR violations can
  // be found this way, and not everything this finds is an ODR
  // violation.  But it's helpful to warn about.)
  if (parameters->options().detect_odr_violations()
      && (sym.get_st_bind() == elfcpp::STB_WEAK
	  || to->binding() == elfcpp::STB_WEAK)
      && orig_st_shndx != elfcpp::SHN_UNDEF
      && to_is_ordinary
      && to_shndx != elfcpp::SHN_UNDEF
      && sym.get_st_size() != 0    // Ignore weird 0-sized symbols.
      && to->symsize() != 0
      && (sym.get_st_type() != to->type()
          || sym.get_st_size() != to->symsize())
      // C does not have a concept of ODR, so we only need to do this
      // on C++ symbols.  These have (mangled) names starting with _Z.
      && to->name()[0] == '_' && to->name()[1] == 'Z')
    {
      Symbol_location fromloc
          = { object, orig_st_shndx, static_cast<off_t>(sym.get_st_value()) };
      Symbol_location toloc = { to->object(), to_shndx,
				static_cast<off_t>(to->value()) };
      this->candidate_odr_violations_[to->name()].insert(fromloc);
      this->candidate_odr_violations_[to->name()].insert(toloc);
    }

  // Plugins don't provide a symbol type, so adopt the existing type
  // if the FROM symbol is from a plugin.
  elfcpp::STT fromtype = (object->pluginobj() != NULL
			  ? to->type()
			  : sym.get_st_type());
  unsigned int frombits = symbol_to_bits(sym.get_st_bind(),
                                         object->is_dynamic(),
					 st_shndx, is_ordinary);

  bool adjust_common_sizes;
  bool adjust_dyndef;
  typename Sized_symbol<size>::Size_type tosize = to->symsize();
  if (Symbol_table::should_override(to, frombits, fromtype, OBJECT,
				    object, &adjust_common_sizes,
				    &adjust_dyndef, is_default_version))
    {
      elfcpp::STB orig_tobinding = to->binding();
      typename Sized_symbol<size>::Value_type tovalue = to->value();
      this->override(to, sym, st_shndx, is_ordinary, object, version);
      if (adjust_common_sizes)
	{
	  if (tosize > to->symsize())
	    to->set_symsize(tosize);
	  if (tovalue > to->value())
	    to->set_value(tovalue);
	}
      if (adjust_dyndef)
	{
	  // We are overriding an UNDEF or WEAK UNDEF with a DYN DEF.
	  // Remember which kind of UNDEF it was for future reference.
	  to->set_undef_binding(orig_tobinding);
	}
    }
  else
    {
      if (adjust_common_sizes)
	{
	  if (sym.get_st_size() > tosize)
	    to->set_symsize(sym.get_st_size());
	  if (sym.get_st_value() > to->value())
	    to->set_value(sym.get_st_value());
	}
      if (adjust_dyndef)
	{
	  // We are keeping a DYN DEF after seeing an UNDEF or WEAK UNDEF.
	  // Remember which kind of UNDEF it was.
	  to->set_undef_binding(sym.get_st_bind());
	}
      // The ELF ABI says that even for a reference to a symbol we
      // merge the visibility.
      to->override_visibility(sym.get_st_visibility());
    }

  // If we have a non-WEAK reference from a regular object to a
  // dynamic object, mark the dynamic object as needed.
  if (to->is_from_dynobj() && to->in_reg() && !to->is_undef_binding_weak())
    to->object()->set_is_needed();

  if (adjust_common_sizes && parameters->options().warn_common())
    {
      if (tosize > sym.get_st_size())
	Symbol_table::report_resolve_problem(false,
					     _("common of '%s' overriding "
					       "smaller common"),
					     to, OBJECT, object);
      else if (tosize < sym.get_st_size())
	Symbol_table::report_resolve_problem(false,
					     _("common of '%s' overidden by "
					       "larger common"),
					     to, OBJECT, object);
      else
	Symbol_table::report_resolve_problem(false,
					     _("multiple common of '%s'"),
					     to, OBJECT, object);
    }
}

// Handle the core of symbol resolution.  This is called with the
// existing symbol, TO, and a bitflag describing the new symbol.  This
// returns true if we should override the existing symbol with the new
// one, and returns false otherwise.  It sets *ADJUST_COMMON_SIZES to
// true if we should set the symbol size to the maximum of the TO and
// FROM sizes.  It handles error conditions.

bool
Symbol_table::should_override(const Symbol* to, unsigned int frombits,
			      elfcpp::STT fromtype, Defined defined,
			      Object* object, bool* adjust_common_sizes,
			      bool* adjust_dyndef, bool is_default_version)
{
  *adjust_common_sizes = false;
  *adjust_dyndef = false;

  unsigned int tobits;
  if (to->source() == Symbol::IS_UNDEFINED)
    tobits = symbol_to_bits(to->binding(), false, elfcpp::SHN_UNDEF, true);
  else if (to->source() != Symbol::FROM_OBJECT)
    tobits = symbol_to_bits(to->binding(), false, elfcpp::SHN_ABS, false);
  else
    {
      bool is_ordinary;
      unsigned int shndx = to->shndx(&is_ordinary);
      tobits = symbol_to_bits(to->binding(),
			      to->object()->is_dynamic(),
			      shndx,
			      is_ordinary);
    }

  if ((to->type() == elfcpp::STT_TLS) ^ (fromtype == elfcpp::STT_TLS)
      && !to->is_placeholder())
    Symbol_table::report_resolve_problem(true,
					 _("symbol '%s' used as both __thread "
					   "and non-__thread"),
					 to, defined, object);

  // We use a giant switch table for symbol resolution.  This code is
  // unwieldy, but: 1) it is efficient; 2) we definitely handle all
  // cases; 3) it is easy to change the handling of a particular case.
  // The alternative would be a series of conditionals, but it is easy
  // to get the ordering wrong.  This could also be done as a table,
  // but that is no easier to understand than this large switch
  // statement.

  // These are the values generated by the bit codes.
  enum
  {
    DEF =              global_flag | regular_flag | def_flag,
    WEAK_DEF =         weak_flag   | regular_flag | def_flag,
    DYN_DEF =          global_flag | dynamic_flag | def_flag,
    DYN_WEAK_DEF =     weak_flag   | dynamic_flag | def_flag,
    UNDEF =            global_flag | regular_flag | undef_flag,
    WEAK_UNDEF =       weak_flag   | regular_flag | undef_flag,
    DYN_UNDEF =        global_flag | dynamic_flag | undef_flag,
    DYN_WEAK_UNDEF =   weak_flag   | dynamic_flag | undef_flag,
    COMMON =           global_flag | regular_flag | common_flag,
    WEAK_COMMON =      weak_flag   | regular_flag | common_flag,
    DYN_COMMON =       global_flag | dynamic_flag | common_flag,
    DYN_WEAK_COMMON =  weak_flag   | dynamic_flag | common_flag
  };

  switch (tobits * 16 + frombits)
    {
    case DEF * 16 + DEF:
      // Two definitions of the same symbol.

      // If either symbol is defined by an object included using
      // --just-symbols, then don't warn.  This is for compatibility
      // with the GNU linker.  FIXME: This is a hack.
      if ((to->source() == Symbol::FROM_OBJECT && to->object()->just_symbols())
          || (object != NULL && object->just_symbols()))
        return false;

      if (!parameters->options().muldefs())
	Symbol_table::report_resolve_problem(true,
					     _("multiple definition of '%s'"),
					     to, defined, object);
      return false;

    case WEAK_DEF * 16 + DEF:
      // We've seen a weak definition, and now we see a strong
      // definition.  In the original SVR4 linker, this was treated as
      // a multiple definition error.  In the Solaris linker and the
      // GNU linker, a weak definition followed by a regular
      // definition causes the weak definition to be overridden.  We
      // are currently compatible with the GNU linker.  In the future
      // we should add a target specific option to change this.
      // FIXME.
      return true;

    case DYN_DEF * 16 + DEF:
    case DYN_WEAK_DEF * 16 + DEF:
      // We've seen a definition in a dynamic object, and now we see a
      // definition in a regular object.  The definition in the
      // regular object overrides the definition in the dynamic
      // object.
      return true;

    case UNDEF * 16 + DEF:
    case WEAK_UNDEF * 16 + DEF:
    case DYN_UNDEF * 16 + DEF:
    case DYN_WEAK_UNDEF * 16 + DEF:
      // We've seen an undefined reference, and now we see a
      // definition.  We use the definition.
      return true;

    case COMMON * 16 + DEF:
    case WEAK_COMMON * 16 + DEF:
    case DYN_COMMON * 16 + DEF:
    case DYN_WEAK_COMMON * 16 + DEF:
      // We've seen a common symbol and now we see a definition.  The
      // definition overrides.
      if (parameters->options().warn_common())
	Symbol_table::report_resolve_problem(false,
					     _("definition of '%s' overriding "
					       "common"),
					     to, defined, object);
      return true;

    case DEF * 16 + WEAK_DEF:
    case WEAK_DEF * 16 + WEAK_DEF:
      // We've seen a definition and now we see a weak definition.  We
      // ignore the new weak definition.
      return false;

    case DYN_DEF * 16 + WEAK_DEF:
    case DYN_WEAK_DEF * 16 + WEAK_DEF:
      // We've seen a dynamic definition and now we see a regular weak
      // definition.  The regular weak definition overrides.
      return true;

    case UNDEF * 16 + WEAK_DEF:
    case WEAK_UNDEF * 16 + WEAK_DEF:
    case DYN_UNDEF * 16 + WEAK_DEF:
    case DYN_WEAK_UNDEF * 16 + WEAK_DEF:
      // A weak definition of a currently undefined symbol.
      return true;

    case COMMON * 16 + WEAK_DEF:
    case WEAK_COMMON * 16 + WEAK_DEF:
      // A weak definition does not override a common definition.
      return false;

    case DYN_COMMON * 16 + WEAK_DEF:
    case DYN_WEAK_COMMON * 16 + WEAK_DEF:
      // A weak definition does override a definition in a dynamic
      // object.
      if (parameters->options().warn_common())
	Symbol_table::report_resolve_problem(false,
					     _("definition of '%s' overriding "
					       "dynamic common definition"),
					     to, defined, object);
      return true;

    case DEF * 16 + DYN_DEF:
    case WEAK_DEF * 16 + DYN_DEF:
      // Ignore a dynamic definition if we already have a definition.
      return false;

    case DYN_DEF * 16 + DYN_DEF:
    case DYN_WEAK_DEF * 16 + DYN_DEF:
      // Ignore a dynamic definition if we already have a definition,
      // unless the existing definition is an unversioned definition
      // in the same dynamic object, and the new definition is a
      // default version.
      if (to->object() == object
          && to->version() == NULL
          && is_default_version)
        return true;
      // Or, if the existing definition is in an unused --as-needed library,
      // and the reference is weak, let the new definition override.
      if (to->in_reg()
	  && to->is_undef_binding_weak()
	  && to->object()->as_needed()
	  && !to->object()->is_needed())
	return true;
      return false;

    case UNDEF * 16 + DYN_DEF:
    case DYN_UNDEF * 16 + DYN_DEF:
    case DYN_WEAK_UNDEF * 16 + DYN_DEF:
      // Use a dynamic definition if we have a reference.
      return true;

    case WEAK_UNDEF * 16 + DYN_DEF:
      // When overriding a weak undef by a dynamic definition,
      // we need to remember that the original undef was weak.
      *adjust_dyndef = true;
      return true;

    case COMMON * 16 + DYN_DEF:
    case WEAK_COMMON * 16 + DYN_DEF:
      // Ignore a dynamic definition if we already have a common
      // definition.
      return false;

    case DEF * 16 + DYN_WEAK_DEF:
    case WEAK_DEF * 16 + DYN_WEAK_DEF:
      // Ignore a weak dynamic definition if we already have a
      // definition.
      return false;

    case UNDEF * 16 + DYN_WEAK_DEF:
      // When overriding an undef by a dynamic weak definition,
      // we need to remember that the original undef was not weak.
      *adjust_dyndef = true;
      return true;

    case DYN_UNDEF * 16 + DYN_WEAK_DEF:
    case DYN_WEAK_UNDEF * 16 + DYN_WEAK_DEF:
      // Use a weak dynamic definition if we have a reference.
      return true;

    case WEAK_UNDEF * 16 + DYN_WEAK_DEF:
      // When overriding a weak undef by a dynamic definition,
      // we need to remember that the original undef was weak.
      *adjust_dyndef = true;
      return true;

    case COMMON * 16 + DYN_WEAK_DEF:
    case WEAK_COMMON * 16 + DYN_WEAK_DEF:
      // Ignore a weak dynamic definition if we already have a common
      // definition.
      return false;

    case DYN_COMMON * 16 + DYN_DEF:
    case DYN_WEAK_COMMON * 16 + DYN_DEF:
    case DYN_DEF * 16 + DYN_WEAK_DEF:
    case DYN_WEAK_DEF * 16 + DYN_WEAK_DEF:
    case DYN_COMMON * 16 + DYN_WEAK_DEF:
    case DYN_WEAK_COMMON * 16 + DYN_WEAK_DEF:
      // If the existing definition is in an unused --as-needed library,
      // and the reference is weak, let a new dynamic definition override.
      if (to->in_reg()
	  && to->is_undef_binding_weak()
	  && to->object()->as_needed()
	  && !to->object()->is_needed())
	return true;
      return false;

    case DEF * 16 + UNDEF:
    case WEAK_DEF * 16 + UNDEF:
    case UNDEF * 16 + UNDEF:
      // A new undefined reference tells us nothing.
      return false;

    case DYN_DEF * 16 + UNDEF:
    case DYN_WEAK_DEF * 16 + UNDEF:
      // For a dynamic def, we need to remember which kind of undef we see.
      *adjust_dyndef = true;
      return false;

    case WEAK_UNDEF * 16 + UNDEF:
    case DYN_UNDEF * 16 + UNDEF:
    case DYN_WEAK_UNDEF * 16 + UNDEF:
      // A strong undef overrides a dynamic or weak undef.
      return true;

    case COMMON * 16 + UNDEF:
    case WEAK_COMMON * 16 + UNDEF:
    case DYN_COMMON * 16 + UNDEF:
    case DYN_WEAK_COMMON * 16 + UNDEF:
      // A new undefined reference tells us nothing.
      return false;

    case DEF * 16 + WEAK_UNDEF:
    case WEAK_DEF * 16 + WEAK_UNDEF:
    case UNDEF * 16 + WEAK_UNDEF:
    case WEAK_UNDEF * 16 + WEAK_UNDEF:
    case DYN_UNDEF * 16 + WEAK_UNDEF:
    case COMMON * 16 + WEAK_UNDEF:
    case WEAK_COMMON * 16 + WEAK_UNDEF:
    case DYN_COMMON * 16 + WEAK_UNDEF:
    case DYN_WEAK_COMMON * 16 + WEAK_UNDEF:
      // A new weak undefined reference tells us nothing unless the
      // exisiting symbol is a dynamic weak reference.
      return false;

    case DYN_WEAK_UNDEF * 16 + WEAK_UNDEF:
      // A new weak reference overrides an existing dynamic weak reference.
      // This is necessary because a dynamic weak reference remembers
      // the old binding, which may not be weak.  If we keeps the existing
      // dynamic weak reference, the weakness may be dropped in the output.
      return true;

    case DYN_DEF * 16 + WEAK_UNDEF:
    case DYN_WEAK_DEF * 16 + WEAK_UNDEF:
      // For a dynamic def, we need to remember which kind of undef we see.
      *adjust_dyndef = true;
      return false;

    case DEF * 16 + DYN_UNDEF:
    case WEAK_DEF * 16 + DYN_UNDEF:
    case DYN_DEF * 16 + DYN_UNDEF:
    case DYN_WEAK_DEF * 16 + DYN_UNDEF:
    case UNDEF * 16 + DYN_UNDEF:
    case WEAK_UNDEF * 16 + DYN_UNDEF:
    case DYN_UNDEF * 16 + DYN_UNDEF:
    case DYN_WEAK_UNDEF * 16 + DYN_UNDEF:
    case COMMON * 16 + DYN_UNDEF:
    case WEAK_COMMON * 16 + DYN_UNDEF:
    case DYN_COMMON * 16 + DYN_UNDEF:
    case DYN_WEAK_COMMON * 16 + DYN_UNDEF:
      // A new dynamic undefined reference tells us nothing.
      return false;

    case DEF * 16 + DYN_WEAK_UNDEF:
    case WEAK_DEF * 16 + DYN_WEAK_UNDEF:
    case DYN_DEF * 16 + DYN_WEAK_UNDEF:
    case DYN_WEAK_DEF * 16 + DYN_WEAK_UNDEF:
    case UNDEF * 16 + DYN_WEAK_UNDEF:
    case WEAK_UNDEF * 16 + DYN_WEAK_UNDEF:
    case DYN_UNDEF * 16 + DYN_WEAK_UNDEF:
    case DYN_WEAK_UNDEF * 16 + DYN_WEAK_UNDEF:
    case COMMON * 16 + DYN_WEAK_UNDEF:
    case WEAK_COMMON * 16 + DYN_WEAK_UNDEF:
    case DYN_COMMON * 16 + DYN_WEAK_UNDEF:
    case DYN_WEAK_COMMON * 16 + DYN_WEAK_UNDEF:
      // A new weak dynamic undefined reference tells us nothing.
      return false;

    case DEF * 16 + COMMON:
      // A common symbol does not override a definition.
      if (parameters->options().warn_common())
	Symbol_table::report_resolve_problem(false,
					     _("common '%s' overridden by "
					       "previous definition"),
					     to, defined, object);
      return false;

    case WEAK_DEF * 16 + COMMON:
    case DYN_DEF * 16 + COMMON:
    case DYN_WEAK_DEF * 16 + COMMON:
      // A common symbol does override a weak definition or a dynamic
      // definition.
      return true;

    case UNDEF * 16 + COMMON:
    case WEAK_UNDEF * 16 + COMMON:
    case DYN_UNDEF * 16 + COMMON:
    case DYN_WEAK_UNDEF * 16 + COMMON:
      // A common symbol is a definition for a reference.
      return true;

    case COMMON * 16 + COMMON:
      // Set the size to the maximum.
      *adjust_common_sizes = true;
      return false;

    case WEAK_COMMON * 16 + COMMON:
      // I'm not sure just what a weak common symbol means, but
      // presumably it can be overridden by a regular common symbol.
      return true;

    case DYN_COMMON * 16 + COMMON:
    case DYN_WEAK_COMMON * 16 + COMMON:
      // Use the real common symbol, but adjust the size if necessary.
      *adjust_common_sizes = true;
      return true;

    case DEF * 16 + WEAK_COMMON:
    case WEAK_DEF * 16 + WEAK_COMMON:
    case DYN_DEF * 16 + WEAK_COMMON:
    case DYN_WEAK_DEF * 16 + WEAK_COMMON:
      // Whatever a weak common symbol is, it won't override a
      // definition.
      return false;

    case UNDEF * 16 + WEAK_COMMON:
    case WEAK_UNDEF * 16 + WEAK_COMMON:
    case DYN_UNDEF * 16 + WEAK_COMMON:
    case DYN_WEAK_UNDEF * 16 + WEAK_COMMON:
      // A weak common symbol is better than an undefined symbol.
      return true;

    case COMMON * 16 + WEAK_COMMON:
    case WEAK_COMMON * 16 + WEAK_COMMON:
    case DYN_COMMON * 16 + WEAK_COMMON:
    case DYN_WEAK_COMMON * 16 + WEAK_COMMON:
      // Ignore a weak common symbol in the presence of a real common
      // symbol.
      return false;

    case DEF * 16 + DYN_COMMON:
    case WEAK_DEF * 16 + DYN_COMMON:
    case DYN_DEF * 16 + DYN_COMMON:
    case DYN_WEAK_DEF * 16 + DYN_COMMON:
      // Ignore a dynamic common symbol in the presence of a
      // definition.
      return false;

    case UNDEF * 16 + DYN_COMMON:
    case WEAK_UNDEF * 16 + DYN_COMMON:
    case DYN_UNDEF * 16 + DYN_COMMON:
    case DYN_WEAK_UNDEF * 16 + DYN_COMMON:
      // A dynamic common symbol is a definition of sorts.
      return true;

    case COMMON * 16 + DYN_COMMON:
    case WEAK_COMMON * 16 + DYN_COMMON:
    case DYN_COMMON * 16 + DYN_COMMON:
    case DYN_WEAK_COMMON * 16 + DYN_COMMON:
      // Set the size to the maximum.
      *adjust_common_sizes = true;
      return false;

    case DEF * 16 + DYN_WEAK_COMMON:
    case WEAK_DEF * 16 + DYN_WEAK_COMMON:
    case DYN_DEF * 16 + DYN_WEAK_COMMON:
    case DYN_WEAK_DEF * 16 + DYN_WEAK_COMMON:
      // A common symbol is ignored in the face of a definition.
      return false;

    case UNDEF * 16 + DYN_WEAK_COMMON:
    case WEAK_UNDEF * 16 + DYN_WEAK_COMMON:
    case DYN_UNDEF * 16 + DYN_WEAK_COMMON:
    case DYN_WEAK_UNDEF * 16 + DYN_WEAK_COMMON:
      // I guess a weak common symbol is better than a definition.
      return true;

    case COMMON * 16 + DYN_WEAK_COMMON:
    case WEAK_COMMON * 16 + DYN_WEAK_COMMON:
    case DYN_COMMON * 16 + DYN_WEAK_COMMON:
    case DYN_WEAK_COMMON * 16 + DYN_WEAK_COMMON:
      // Set the size to the maximum.
      *adjust_common_sizes = true;
      return false;

    default:
      gold_unreachable();
    }
}

// Issue an error or warning due to symbol resolution.  IS_ERROR
// indicates an error rather than a warning.  MSG is the error
// message; it is expected to have a %s for the symbol name.  TO is
// the existing symbol.  DEFINED/OBJECT is where the new symbol was
// found.

// FIXME: We should have better location information here.  When the
// symbol is defined, we should be able to pull the location from the
// debug info if there is any.

void
Symbol_table::report_resolve_problem(bool is_error, const char* msg,
				     const Symbol* to, Defined defined,
				     Object* object)
{
  std::string demangled(to->demangled_name());
  size_t len = strlen(msg) + demangled.length() + 10;
  char* buf = new char[len];
  snprintf(buf, len, msg, demangled.c_str());

  const char* objname;
  switch (defined)
    {
    case OBJECT:
      objname = object->name().c_str();
      break;
    case COPY:
      objname = _("COPY reloc");
      break;
    case DEFSYM:
    case UNDEFINED:
      objname = _("command line");
      break;
    case SCRIPT:
      objname = _("linker script");
      break;
    case PREDEFINED:
    case INCREMENTAL_BASE:
      objname = _("linker defined");
      break;
    default:
      gold_unreachable();
    }

  if (is_error)
    gold_error("%s: %s", objname, buf);
  else
    gold_warning("%s: %s", objname, buf);

  delete[] buf;

  if (to->source() == Symbol::FROM_OBJECT)
    objname = to->object()->name().c_str();
  else
    objname = _("command line");
  gold_info("%s: %s: previous definition here", program_name, objname);
}

// Completely override existing symbol.  Everything bar name_,
// version_, and is_forced_local_ flag are copied.  version_ is
// cleared if from->version_ is clear.  Returns true if this symbol
// should be forced local.
bool
Symbol::clone(const Symbol* from)
{
  // Don't allow cloning after dynamic linking info is attached to symbols.
  // We aren't prepared to merge such.
  gold_assert(!this->has_symtab_index() && !from->has_symtab_index());
  gold_assert(!this->has_dynsym_index() && !from->has_dynsym_index());
  gold_assert(this->got_offset_list() == NULL
	      && from->got_offset_list() == NULL);
  gold_assert(!this->has_plt_offset() && !from->has_plt_offset());

  if (!from->version_)
    this->version_ = from->version_;
  this->u1_ = from->u1_;
  this->u2_ = from->u2_;
  this->type_ = from->type_;
  this->binding_ = from->binding_;
  this->visibility_ = from->visibility_;
  this->nonvis_ = from->nonvis_;
  this->source_ = from->source_;
  this->is_def_ = from->is_def_;
  this->is_forwarder_ = from->is_forwarder_;
  this->has_alias_ = from->has_alias_;
  this->needs_dynsym_entry_ = from->needs_dynsym_entry_;
  this->in_reg_ = from->in_reg_;
  this->in_dyn_ = from->in_dyn_;
  this->needs_dynsym_value_ = from->needs_dynsym_value_;
  this->has_warning_ = from->has_warning_;
  this->is_copied_from_dynobj_ = from->is_copied_from_dynobj_;
  this->is_ordinary_shndx_ = from->is_ordinary_shndx_;
  this->in_real_elf_ = from->in_real_elf_;
  this->is_defined_in_discarded_section_
    = from->is_defined_in_discarded_section_;
  this->undef_binding_set_ = from->undef_binding_set_;
  this->undef_binding_weak_ = from->undef_binding_weak_;
  this->is_predefined_ = from->is_predefined_;
  this->is_protected_ = from->is_protected_;
  this->non_zero_localentry_ = from->non_zero_localentry_;

  return !this->is_forced_local_ && from->is_forced_local_;
}

template <int size>
bool
Sized_symbol<size>::clone(const Sized_symbol<size>* from)
{
  this->value_ = from->value_;
  this->symsize_ = from->symsize_;
  return Symbol::clone(from);
}

// A special case of should_override which is only called for a strong
// defined symbol from a regular object file.  This is used when
// defining special symbols.

bool
Symbol_table::should_override_with_special(const Symbol* to,
					   elfcpp::STT fromtype,
					   Defined defined)
{
  bool adjust_common_sizes;
  bool adjust_dyn_def;
  unsigned int frombits = global_flag | regular_flag | def_flag;
  bool ret = Symbol_table::should_override(to, frombits, fromtype, defined,
					   NULL, &adjust_common_sizes,
					   &adjust_dyn_def, false);
  gold_assert(!adjust_common_sizes && !adjust_dyn_def);
  return ret;
}

// Override symbol base with a special symbol.

void
Symbol::override_base_with_special(const Symbol* from)
{
  bool same_name = this->name_ == from->name_;
  gold_assert(same_name || this->has_alias());

  // If we are overriding an undef, remember the original binding.
  if (this->is_undefined())
    this->set_undef_binding(this->binding_);

  this->source_ = from->source_;
  switch (from->source_)
    {
    case FROM_OBJECT:
    case IN_OUTPUT_DATA:
    case IN_OUTPUT_SEGMENT:
      this->u1_ = from->u1_;
      this->u2_ = from->u2_;
      break;
    case IS_CONSTANT:
    case IS_UNDEFINED:
      break;
    default:
      gold_unreachable();
      break;
    }

  if (same_name)
    {
      // When overriding a versioned symbol with a special symbol, we
      // may be changing the version.  This will happen if we see a
      // special symbol such as "_end" defined in a shared object with
      // one version (from a version script), but we want to define it
      // here with a different version (from a different version
      // script).
      this->version_ = from->version_;
    }
  this->type_ = from->type_;
  this->binding_ = from->binding_;
  this->override_visibility(from->visibility_);
  this->nonvis_ = from->nonvis_;

  // Special symbols are always considered to be regular symbols.
  this->in_reg_ = true;

  if (from->needs_dynsym_entry_)
    this->needs_dynsym_entry_ = true;
  if (from->needs_dynsym_value_)
    this->needs_dynsym_value_ = true;

  this->is_predefined_ = from->is_predefined_;

  // We shouldn't see these flags.  If we do, we need to handle them
  // somehow.
  gold_assert(!from->is_forwarder_);
  gold_assert(!from->has_plt_offset());
  gold_assert(!from->has_warning_);
  gold_assert(!from->is_copied_from_dynobj_);
  gold_assert(!from->is_forced_local_);
}

// Override a symbol with a special symbol.

template<int size>
void
Sized_symbol<size>::override_with_special(const Sized_symbol<size>* from)
{
  this->override_base_with_special(from);
  this->value_ = from->value_;
  this->symsize_ = from->symsize_;
}

// Override TOSYM with the special symbol FROMSYM.  This handles all
// aliases of TOSYM.

template<int size>
void
Symbol_table::override_with_special(Sized_symbol<size>* tosym,
				    const Sized_symbol<size>* fromsym)
{
  tosym->override_with_special(fromsym);
  if (tosym->has_alias())
    {
      Symbol* sym = this->weak_aliases_[tosym];
      gold_assert(sym != NULL);
      Sized_symbol<size>* ssym = this->get_sized_symbol<size>(sym);
      do
	{
	  ssym->override_with_special(fromsym);
	  sym = this->weak_aliases_[ssym];
	  gold_assert(sym != NULL);
	  ssym = this->get_sized_symbol<size>(sym);
	}
      while (ssym != tosym);
    }
  if (tosym->binding() == elfcpp::STB_LOCAL
      || ((tosym->visibility() == elfcpp::STV_HIDDEN
	   || tosym->visibility() == elfcpp::STV_INTERNAL)
	  && (tosym->binding() == elfcpp::STB_GLOBAL
	      || tosym->binding() == elfcpp::STB_GNU_UNIQUE
	      || tosym->binding() == elfcpp::STB_WEAK)
	  && !parameters->options().relocatable()))
    this->force_local(tosym);
}

// Instantiate the templates we need.  We could use the configure
// script to restrict this to only the ones needed for implemented
// targets.

// We have to instantiate both big and little endian versions because
// these are used by other templates that depends on size only.

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Symbol_table::resolve<32, false>(
    Sized_symbol<32>* to,
    const elfcpp::Sym<32, false>& sym,
    unsigned int st_shndx,
    bool is_ordinary,
    unsigned int orig_st_shndx,
    Object* object,
    const char* version,
    bool is_default_version);

template
void
Symbol_table::resolve<32, true>(
    Sized_symbol<32>* to,
    const elfcpp::Sym<32, true>& sym,
    unsigned int st_shndx,
    bool is_ordinary,
    unsigned int orig_st_shndx,
    Object* object,
    const char* version,
    bool is_default_version);
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Symbol_table::resolve<64, false>(
    Sized_symbol<64>* to,
    const elfcpp::Sym<64, false>& sym,
    unsigned int st_shndx,
    bool is_ordinary,
    unsigned int orig_st_shndx,
    Object* object,
    const char* version,
    bool is_default_version);

template
void
Symbol_table::resolve<64, true>(
    Sized_symbol<64>* to,
    const elfcpp::Sym<64, true>& sym,
    unsigned int st_shndx,
    bool is_ordinary,
    unsigned int orig_st_shndx,
    Object* object,
    const char* version,
    bool is_default_version);
#endif

#if defined(HAVE_TARGET_32_LITTLE) || defined(HAVE_TARGET_32_BIG)
template
void
Symbol_table::override_with_special<32>(Sized_symbol<32>*,
					const Sized_symbol<32>*);
#endif

#if defined(HAVE_TARGET_64_LITTLE) || defined(HAVE_TARGET_64_BIG)
template
void
Symbol_table::override_with_special<64>(Sized_symbol<64>*,
					const Sized_symbol<64>*);
#endif

template
bool
Sized_symbol<32>::clone(const Sized_symbol<32>*);

template
bool
Sized_symbol<64>::clone(const Sized_symbol<64>*);
} // End namespace gold.
