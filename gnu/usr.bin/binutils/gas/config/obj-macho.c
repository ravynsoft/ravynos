/* Mach-O object file format
   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Here we handle the mach-o directives that are common to all architectures.

   Most significant are mach-o named sections and a variety of symbol type
   decorations.  */

/* Mach-O supports multiple, named segments each of which may contain
   multiple named sections.  Thus the concept of subsectioning is
   handled by (say) having a __TEXT segment with appropriate flags from
   which subsections are generated like __text, __const etc.

   The well-known as short-hand section switch directives like .text, .data
   etc. are mapped onto predefined segment/section pairs using facilities
   supplied by the mach-o port of bfd.

   A number of additional mach-o short-hand section switch directives are
   also defined.  */

#define OBJ_HEADER "obj-macho.h"

#include "as.h"
#include "subsegs.h"
#include "symbols.h"
#include "write.h"
#include "mach-o.h"
#include "mach-o/loader.h"
#include "obj-macho.h"

#include <string.h>

/* Forward decls.  */
static segT obj_mach_o_segT_from_bfd_name (const char *, int);

/* TODO: Implement "-dynamic"/"-static" command line options.  */

static int obj_mach_o_is_static;

/* TODO: Implement the "-n" command line option to suppress the initial
   switch to the text segment.  */

static int obj_mach_o_start_with_text_section = 1;

/* Allow for special re-ordering on output.  */

static int obj_mach_o_seen_objc_section;

/* Start-up: At present, just create the sections we want.  */
void
mach_o_begin (void)
{
  /* Mach-O only defines the .text section by default, and even this can
     be suppressed by a flag.  In the latter event, the first code MUST
     be a section definition.  */
  if (obj_mach_o_start_with_text_section)
    {
      text_section = obj_mach_o_segT_from_bfd_name (TEXT_SECTION_NAME, 1);
      subseg_set (text_section, 0);
      if (obj_mach_o_is_static)
	{
	  bfd_mach_o_section *mo_sec
			= bfd_mach_o_get_mach_o_section (text_section);
	  mo_sec->flags &= ~BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS;
	}
    }
}

/* Remember the subsections_by_symbols state in case we need to reset
   the file flags.  */

static int obj_mach_o_subsections_by_symbols;

/* This will put at most 16 characters (terminated by a ',' or newline) from
   the input stream into dest.  If there are more than 16 chars before the
   delimiter, a warning is given and the string is truncated.  On completion of
   this function, input_line_pointer will point to the char after the ',' or
   to the newline.

   It trims leading and trailing space.  */

static int
collect_16char_name (char *dest, const char *msg, int require_comma)
{
  char c, *namstart;

  SKIP_WHITESPACE ();
  namstart = input_line_pointer;

  while ( (c = *input_line_pointer) != ','
	 && !is_end_of_line[(unsigned char) c])
    input_line_pointer++;

  {
      int len = input_line_pointer - namstart; /* could be zero.  */
      /* lose any trailing space.  */
      while (len > 0 && namstart[len-1] == ' ')
        len--;
      if (len > 16)
        {
          *input_line_pointer = '\0'; /* make a temp string.  */
	  as_bad (_("the %s name '%s' is too long (maximum 16 characters)"),
		     msg, namstart);
	  *input_line_pointer = c; /* restore for printing.  */
	  len = 16;
	}
      if (len > 0)
        memcpy (dest, namstart, len);
  }

  if (c != ',' && require_comma)
    {
      as_bad (_("expected a %s name followed by a `,'"), msg);
      return 1;
    }

  return 0;
}

static int
obj_mach_o_get_section_names (char *seg, char *sec,
			      unsigned segl, unsigned secl)
{
  /* Zero-length segment and section names are allowed.  */
  /* Parse segment name.  */
  memset (seg, 0, segl);
  if (collect_16char_name (seg, _("segment"), 1))
    {
      ignore_rest_of_line ();
      return 0;
    }
  input_line_pointer++; /* Skip the terminating ',' */

  /* Parse section name, which can be empty.  */
  memset (sec, 0, secl);
  collect_16char_name (sec, _("section"), 0);
  return 1;
}

/* Build (or get) a section from the mach-o description - which includes
   optional definitions for type, attributes, alignment and stub size.

   BFD supplies default values for sections which have a canonical name.  */

#define SECT_TYPE_SPECIFIED 0x0001
#define SECT_ATTR_SPECIFIED 0x0002
#define SECT_ALGN_SPECIFIED 0x0004
#define SECT_STUB_SPECIFIED 0x0008

static segT
obj_mach_o_make_or_get_sect (char * segname, char * sectname,
			     unsigned int specified_mask,
			     unsigned int usectype, unsigned int usecattr,
			     unsigned int ualign, offsetT stub_size)
{
  unsigned int sectype, secattr, secalign;
  flagword oldflags, flags;
  const char *name;
  segT sec;
  bfd_mach_o_section *msect;
  const mach_o_section_name_xlat *xlat;

  /* This provides default bfd flags and default mach-o section type and
     attributes along with the canonical name.  */
  xlat = bfd_mach_o_section_data_for_mach_sect (stdoutput, segname, sectname);

  /* TODO: more checking of whether overrides are actually allowed.  */

  if (xlat != NULL)
    {
      name = xstrdup (xlat->bfd_name);
      sectype = xlat->macho_sectype;
      if (specified_mask & SECT_TYPE_SPECIFIED)
	{
	  if ((sectype == BFD_MACH_O_S_ZEROFILL
	       || sectype == BFD_MACH_O_S_GB_ZEROFILL)
	      && sectype != usectype)
	    as_bad (_("cannot override zerofill section type for `%s,%s'"),
		    segname, sectname);
	  else
	    sectype = usectype;
	}
      secattr = xlat->macho_secattr;
      secalign = xlat->sectalign;
      flags = xlat->bfd_flags;
    }
  else
    {
      /* There is no normal BFD section name for this section.  Create one.
         The name created doesn't really matter as it will never be written
         on disk.  */
      name = concat (segname, ".", sectname, (char *) NULL);
      if (specified_mask & SECT_TYPE_SPECIFIED)
	sectype = usectype;
      else
	sectype = BFD_MACH_O_S_REGULAR;
      secattr = BFD_MACH_O_S_ATTR_NONE;
      secalign = 0;
      flags = SEC_NO_FLAGS;
    }

  /* For now, just use what the user provided.  */

  if (specified_mask & SECT_ATTR_SPECIFIED)
    secattr = usecattr;

  if (specified_mask & SECT_ALGN_SPECIFIED)
    secalign = ualign;

  /* Sub-segments don't exists as is on Mach-O.  */
  sec = subseg_new (name, 0);

  oldflags = bfd_section_flags (sec);
  msect = bfd_mach_o_get_mach_o_section (sec);

  if (oldflags == SEC_NO_FLAGS)
    {
      /* In the absence of canonical information, try to determine CODE and
	 DEBUG section flags from the mach-o section data.  */
      if (flags == SEC_NO_FLAGS
	  && (specified_mask & SECT_ATTR_SPECIFIED)
	  && (secattr & BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS))
	flags |= SEC_CODE;

      if (flags == SEC_NO_FLAGS
	  && (specified_mask & SECT_ATTR_SPECIFIED)
	  && (secattr & BFD_MACH_O_S_ATTR_DEBUG))
	flags |= SEC_DEBUGGING;

      /* New, so just use the defaults or what's specified.  */
      if (!bfd_set_section_flags (sec, flags))
	as_warn (_("failed to set flags for \"%s\": %s"),
		 bfd_section_name (sec),
		 bfd_errmsg (bfd_get_error ()));

      strncpy (msect->segname, segname, BFD_MACH_O_SEGNAME_SIZE);
      msect->segname[BFD_MACH_O_SEGNAME_SIZE] = 0;
      strncpy (msect->sectname, sectname, BFD_MACH_O_SECTNAME_SIZE);
      msect->sectname[BFD_MACH_O_SECTNAME_SIZE] = 0;

      msect->align = secalign;
      msect->flags = sectype | secattr;

      if (sectype == BFD_MACH_O_S_ZEROFILL
	  || sectype == BFD_MACH_O_S_GB_ZEROFILL)
        seg_info (sec)->bss = 1;
    }
  else if (flags != SEC_NO_FLAGS)
    {
      if (flags != oldflags
	  || msect->flags != (secattr | sectype))
	as_warn (_("Ignoring changed section attributes for %s"), name);
    }

  if (specified_mask & SECT_STUB_SPECIFIED)
    /* At present, the stub size is not supplied from the BFD tables.  */
    msect->reserved2 = stub_size;

  return sec;
}

/* .section

   The '.section' specification syntax looks like:
   .section <segment> , <section> [, type [, attribs [, size]]]

   White space is allowed everywhere between elements.

   <segment> and <section> may be from 0 to 16 chars in length - they may
   contain spaces but leading and trailing space will be trimmed.  It is
   mandatory that they be present (or that zero-length names are indicated
   by ",,").

   There is only a single section type for any entry.

   There may be multiple attributes, they are delimited by `+'.

   Not all section types and attributes are accepted by the Darwin system
   assemblers as user-specifiable - although, at present, we do here.  */

static void
obj_mach_o_section (int ignore ATTRIBUTE_UNUSED)
{
  unsigned int sectype = BFD_MACH_O_S_REGULAR;
  unsigned int specified_mask = 0;
  unsigned int secattr = 0;
  offsetT sizeof_stub = 0;
  segT new_seg;
  char segname[17];
  char sectname[17];

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* Get the User's segment and section names.  */
  if (! obj_mach_o_get_section_names (segname, sectname, 17, 17))
    return;

  /* Parse section type, if present.  */
  if (*input_line_pointer == ',')
    {
      char *p;
      char c;
      char tmpc;
      int len;
      input_line_pointer++;
      SKIP_WHITESPACE ();
      p = input_line_pointer;
      while ((c = *input_line_pointer) != ','
	      && !is_end_of_line[(unsigned char) c])
	input_line_pointer++;

      len = input_line_pointer - p;
      /* strip trailing spaces.  */
      while (len > 0 && p[len-1] == ' ')
	len--;
      tmpc = p[len];

      /* Temporarily make a string from the token.  */
      p[len] = 0;
      sectype = bfd_mach_o_get_section_type_from_name (stdoutput, p);
      if (sectype > 255) /* Max Section ID == 255.  */
        {
          as_bad (_("unknown or invalid section type '%s'"), p);
	  p[len] = tmpc;
	  ignore_rest_of_line ();
	  return;
        }
      else
	specified_mask |= SECT_TYPE_SPECIFIED;
      /* Restore.  */
      p[len] = tmpc;

      /* Parse attributes.
	 TODO: check validity of attributes for section type.  */
      if ((specified_mask & SECT_TYPE_SPECIFIED)
	  && c == ',')
        {
          do
            {
              int attr;

	      /* Skip initial `,' and subsequent `+'.  */
              input_line_pointer++;
	      SKIP_WHITESPACE ();
	      p = input_line_pointer;
	      while ((c = *input_line_pointer) != '+'
		      && c != ','
		      && !is_end_of_line[(unsigned char) c])
		input_line_pointer++;

	      len = input_line_pointer - p;
	      /* strip trailing spaces.  */
	      while (len > 0 && p[len-1] == ' ')
		len--;
	      tmpc = p[len];

	      /* Temporarily make a string from the token.  */
	      p[len] ='\0';
              attr = bfd_mach_o_get_section_attribute_from_name (p);
	      if (attr == -1)
		{
                  as_bad (_("unknown or invalid section attribute '%s'"), p);
		  p[len] = tmpc;
		  ignore_rest_of_line ();
		  return;
                }
              else
		{
		  specified_mask |= SECT_ATTR_SPECIFIED;
                  secattr |= attr;
		}
	      /* Restore.  */
	      p[len] = tmpc;
            }
          while (*input_line_pointer == '+');

          /* Parse sizeof_stub.  */
          if ((specified_mask & SECT_ATTR_SPECIFIED)
	      && *input_line_pointer == ',')
            {
              if (sectype != BFD_MACH_O_S_SYMBOL_STUBS)
                {
		  as_bad (_("unexpected section size information"));
		  ignore_rest_of_line ();
		  return;
		}

	      input_line_pointer++;
              sizeof_stub = get_absolute_expression ();
              specified_mask |= SECT_STUB_SPECIFIED;
            }
          else if ((specified_mask & SECT_ATTR_SPECIFIED)
		   && sectype == BFD_MACH_O_S_SYMBOL_STUBS)
            {
              as_bad (_("missing sizeof_stub expression"));
	      ignore_rest_of_line ();
	      return;
            }
        }
    }

  new_seg = obj_mach_o_make_or_get_sect (segname, sectname, specified_mask,
					 sectype, secattr, 0 /*align */,
					 sizeof_stub);
  if (new_seg != NULL)
    {
      subseg_set (new_seg, 0);
      demand_empty_rest_of_line ();
    }
}

/* .zerofill segname, sectname [, symbolname, size [, align]]

   Zerofill switches, temporarily, to a sect of type 'zerofill'.

   If a variable name is given, it defines that in the section.
   Otherwise it just creates the section if it doesn't exist.  */

static void
obj_mach_o_zerofill (int ignore ATTRIBUTE_UNUSED)
{
  char segname[17];
  char sectname[17];
  segT old_seg = now_seg;
  segT new_seg;
  symbolS *sym = NULL;
  unsigned int align = 0;
  unsigned int specified_mask = 0;
  offsetT size = 0;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* Get the User's segment and section names.  */
  if (! obj_mach_o_get_section_names (segname, sectname, 17, 17))
    return;

  /* Parse variable definition, if present.  */
  if (*input_line_pointer == ',')
    {
      /* Parse symbol, size [.align]
         We follow the method of s_common_internal, with the difference
         that the symbol cannot be a duplicate-common.  */
      char *name;
      char c;
      char *p;
      expressionS exp;

      input_line_pointer++; /* Skip ',' */
      SKIP_WHITESPACE ();
      c = get_symbol_name (&name);
      /* Just after name is now '\0'.  */
      p = input_line_pointer;
      *p = c;

      if (name == p)
	{
	  as_bad (_("expected symbol name"));
	  ignore_rest_of_line ();
	  goto done;
	}

      SKIP_WHITESPACE_AFTER_NAME ();
      if (*input_line_pointer == ',')
	input_line_pointer++;

      expression_and_evaluate (&exp);
      if (exp.X_op != O_constant
	  && exp.X_op != O_absent)
	{
	    as_bad (_("bad or irreducible absolute expression"));
	  ignore_rest_of_line ();
	  goto done;
	}
      else if (exp.X_op == O_absent)
	{
	  as_bad (_("missing size expression"));
	  ignore_rest_of_line ();
	  goto done;
	}

      size = exp.X_add_number;
      size &= ((valueT) 2 << (stdoutput->arch_info->bits_per_address - 1)) - 1;
      if (exp.X_add_number != size || !exp.X_unsigned)
	{
	  as_warn (_("size (%ld) out of range, ignored"),
		   (long) exp.X_add_number);
	  ignore_rest_of_line ();
	  goto done;
	}

     *p = 0; /* Make the name into a c string for err messages.  */
     sym = symbol_find_or_make (name);
     if (S_IS_DEFINED (sym) || symbol_equated_p (sym))
	{
	  as_bad (_("symbol `%s' is already defined"), name);
	  *p = c;
	  ignore_rest_of_line ();
	   goto done;
	}

      size = S_GET_VALUE (sym);
      if (size == 0)
	size = exp.X_add_number;
      else if (size != exp.X_add_number)
	as_warn (_("size of \"%s\" is already %ld; not changing to %ld"),
		   name, (long) size, (long) exp.X_add_number);

      *p = c;  /* Restore the termination char.  */

      SKIP_WHITESPACE ();
      if (*input_line_pointer == ',')
	{
	  align = (unsigned int) parse_align (0);
	  if (align == (unsigned int) -1)
	    {
	      as_warn (_("align value not recognized, using size"));
	      align = size;
	    }
	  if (align > 15)
	    {
	      as_warn (_("Alignment (%lu) too large: 15 assumed."),
			(unsigned long)align);
	      align = 15;
	    }
	  specified_mask |= SECT_ALGN_SPECIFIED;
	}
    }
 /* else just a section definition.  */

  specified_mask |= SECT_TYPE_SPECIFIED;
  new_seg = obj_mach_o_make_or_get_sect (segname, sectname, specified_mask,
					 BFD_MACH_O_S_ZEROFILL,
					 BFD_MACH_O_S_ATTR_NONE,
					 align, (offsetT) 0 /*stub size*/);
  if (new_seg == NULL)
    return;

  /* In case the user specifies the bss section by mach-o name.
     Create it on demand */
  if (strcmp (new_seg->name, BSS_SECTION_NAME) == 0
      && bss_section == NULL)
    bss_section = new_seg;

  subseg_set (new_seg, 0);

  if (sym != NULL)
    {
      char *pfrag;

      if (align)
	{
	  record_alignment (new_seg, align);
	  frag_align (align, 0, 0);
	}

      /* Detach from old frag.  */
      if (S_GET_SEGMENT (sym) == new_seg)
	symbol_get_frag (sym)->fr_symbol = NULL;

      symbol_set_frag (sym, frag_now);
      pfrag = frag_var (rs_org, 1, 1, 0, sym, size, NULL);
      *pfrag = 0;

      S_SET_SEGMENT (sym, new_seg);
      if (new_seg == bss_section)
	S_CLEAR_EXTERNAL (sym);
    }

 done:
  /* switch back to the section that was current before the .zerofill.  */
  subseg_set (old_seg, 0);
}

static segT
obj_mach_o_segT_from_bfd_name (const char *nam, int must_succeed)
{
  const mach_o_section_name_xlat *xlat;
  const char *segn;
  segT sec;

  /* BFD has tables of flags and default attributes for all the sections that
     have a 'canonical' name.  */
  xlat = bfd_mach_o_section_data_for_bfd_name (stdoutput, nam, &segn);
  if (xlat == NULL)
    {
      if (must_succeed)
	as_fatal (_("BFD is out of sync with GAS, "
		     "unhandled well-known section type `%s'"), nam);
      return NULL;
    }

  sec = bfd_get_section_by_name (stdoutput, nam);
  if (sec == NULL)
    {
      bfd_mach_o_section *msect;

      sec = subseg_force_new (xlat->bfd_name, 0);

      /* Set default type, attributes and alignment.  */
      msect = bfd_mach_o_get_mach_o_section (sec);
      msect->flags = xlat->macho_sectype | xlat->macho_secattr;
      msect->align = xlat->sectalign;

      if ((msect->flags & BFD_MACH_O_SECTION_TYPE_MASK)
	  == BFD_MACH_O_S_ZEROFILL)
	seg_info (sec)->bss = 1;
    }

  return sec;
}

static const char * const known_sections[] =
{
  /*  0 */ NULL,
  /* __TEXT */
  /*  1 */ ".const",
  /*  2 */ ".static_const",
  /*  3 */ ".cstring",
  /*  4 */ ".literal4",
  /*  5 */ ".literal8",
  /*  6 */ ".literal16",
  /*  7 */ ".constructor",
  /*  8 */ ".destructor",
  /*  9 */ ".eh_frame",
  /* __DATA */
  /* 10 */ ".const_data",
  /* 11 */ ".static_data",
  /* 12 */ ".mod_init_func",
  /* 13 */ ".mod_term_func",
  /* 14 */ ".dyld",
  /* 15 */ ".cfstring"
};

/* Interface for a known non-optional section directive.  */

static void
obj_mach_o_known_section (int sect_index)
{
  segT section;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  section = obj_mach_o_segT_from_bfd_name (known_sections[sect_index], 1);
  if (section != NULL)
    subseg_set (section, 0);

  /* else, we leave the section as it was; there was a fatal error anyway.  */
}

static const char * const objc_sections[] =
{
  /*  0 */ NULL,
  /*  1 */ ".objc_class",
  /*  2 */ ".objc_meta_class",
  /*  3 */ ".objc_cat_cls_meth",
  /*  4 */ ".objc_cat_inst_meth",
  /*  5 */ ".objc_protocol",
  /*  6 */ ".objc_string_object",
  /*  7 */ ".objc_cls_meth",
  /*  8 */ ".objc_inst_meth",
  /*  9 */ ".objc_cls_refs",
  /* 10 */ ".objc_message_refs",
  /* 11 */ ".objc_symbols",
  /* 12 */ ".objc_category",
  /* 13 */ ".objc_class_vars",
  /* 14 */ ".objc_instance_vars",
  /* 15 */ ".objc_module_info",
  /* 16 */ ".cstring", /* objc_class_names Alias for .cstring */
  /* 17 */ ".cstring", /* Alias objc_meth_var_types for .cstring */
  /* 18 */ ".cstring", /* objc_meth_var_names Alias for .cstring */
  /* 19 */ ".objc_selector_strs",
  /* 20 */ ".objc_image_info", /* extension.  */
  /* 21 */ ".objc_selector_fixup", /* extension.  */
  /* 22 */ ".objc1_class_ext", /* ObjC-1 extension.  */
  /* 23 */ ".objc1_property_list", /* ObjC-1 extension.  */
  /* 24 */ ".objc1_protocol_ext" /* ObjC-1 extension.  */
};

/* This currently does the same as known_sections, but kept separate for
   ease of maintenance.  */

static void
obj_mach_o_objc_section (int sect_index)
{
  segT section;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  section = obj_mach_o_segT_from_bfd_name (objc_sections[sect_index], 1);
  if (section != NULL)
    {
      obj_mach_o_seen_objc_section = 1; /* We need to ensure that certain
					   sections are present and in the
					   right order.  */
      subseg_set (section, 0);
    }

  /* else, we leave the section as it was; there was a fatal error anyway.  */
}

/* Debug section directives.  */

static const char * const debug_sections[] =
{
  /*  0 */ NULL,
  /* __DWARF */
  /*  1 */ ".debug_frame",
  /*  2 */ ".debug_info",
  /*  3 */ ".debug_abbrev",
  /*  4 */ ".debug_aranges",
  /*  5 */ ".debug_macinfo",
  /*  6 */ ".debug_line",
  /*  7 */ ".debug_loc",
  /*  8 */ ".debug_pubnames",
  /*  9 */ ".debug_pubtypes",
  /* 10 */ ".debug_str",
  /* 11 */ ".debug_ranges",
  /* 12 */ ".debug_macro"
};

/* ??? Maybe these should be conditional on gdwarf-*.
   It`s also likely that we will need to be able to set them from the cfi
   code.  */

static void
obj_mach_o_debug_section (int sect_index)
{
  segT section;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  section = obj_mach_o_segT_from_bfd_name (debug_sections[sect_index], 1);
  if (section != NULL)
    subseg_set (section, 0);

  /* else, we leave the section as it was; there was a fatal error anyway.  */
}

/* This could be moved to the tc-xx files, but there is so little dependency
   there, that the code might as well be shared.  */

struct opt_tgt_sect
{
 const char *name;
 unsigned x86_val;
 unsigned ppc_val;
};

/* The extensions here are for specific sections that are generated by GCC
   and Darwin system tools, but don't have directives in the `system as'.  */

static const struct opt_tgt_sect tgt_sections[] =
{
  /*  0 */ { NULL, 0, 0},
  /*  1 */ { ".lazy_symbol_pointer", 0, 0},
  /*  2 */ { ".lazy_symbol_pointer2", 0, 0}, /* X86 - extension */
  /*  3 */ { ".lazy_symbol_pointer3", 0, 0}, /* X86 - extension */
  /*  4 */ { ".non_lazy_symbol_pointer", 0, 0},
  /*  5 */ { ".non_lazy_symbol_pointer_x86", 0, 0}, /* X86 - extension */
  /*  6 */ { ".symbol_stub", 16, 20},
  /*  7 */ { ".symbol_stub1", 0, 16}, /* PPC - extension */
  /*  8 */ { ".picsymbol_stub", 26, 36},
  /*  9 */ { ".picsymbol_stub1", 0, 32}, /* PPC - extension */
  /* 10 */ { ".picsymbol_stub2", 25, 0}, /* X86 - extension */
  /* 11 */ { ".picsymbol_stub3", 5, 0}, /* X86 - extension  */
};

/* Interface for an optional section directive.  */

static void
obj_mach_o_opt_tgt_section (int sect_index)
{
  const struct opt_tgt_sect *tgtsct = &tgt_sections[sect_index];
  segT section;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  section = obj_mach_o_segT_from_bfd_name (tgtsct->name, 0);
  if (section == NULL)
    {
      as_bad (_("%s is not used for the selected target"), tgtsct->name);
      /* Leave the section as it is.  */
    }
  else
    {
      bfd_mach_o_section *mo_sec = bfd_mach_o_get_mach_o_section (section);
      subseg_set (section, 0);
#if defined (TC_I386)
      mo_sec->reserved2 = tgtsct->x86_val;
#elif defined (TC_PPC)
      mo_sec->reserved2 = tgtsct->ppc_val;
#else
      mo_sec->reserved2 = 0;
#endif
    }
}

/* We don't necessarily have the three 'base' sections on mach-o.
   Normally, we would start up with only the 'text' section defined.
   However, even that can be suppressed with (TODO) c/l option "-n".
   Thus, we have to be able to create all three sections on-demand.  */

static void
obj_mach_o_base_section (int sect_index)
{
  segT section;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  /* We don't support numeric (or any other) qualifications on the
     well-known section shorthands.  */
  demand_empty_rest_of_line ();

  switch (sect_index)
    {
      /* Handle the three sections that are globally known within GAS.
	 For Mach-O, these are created on demand rather than at startup.  */
      case 1:
	if (text_section == NULL)
	  text_section = obj_mach_o_segT_from_bfd_name (TEXT_SECTION_NAME, 1);
	if (obj_mach_o_is_static)
	  {
	    bfd_mach_o_section *mo_sec
		= bfd_mach_o_get_mach_o_section (text_section);
	    mo_sec->flags &= ~BFD_MACH_O_S_ATTR_PURE_INSTRUCTIONS;
	  }
	section = text_section;
	break;
      case 2:
	if (data_section == NULL)
	  data_section = obj_mach_o_segT_from_bfd_name (DATA_SECTION_NAME, 1);
	section = data_section;
	break;
      case 3:
        /* ??? maybe this achieves very little, as an addition.  */
	if (bss_section == NULL)
	  {
	    bss_section = obj_mach_o_segT_from_bfd_name (BSS_SECTION_NAME, 1);
	    seg_info (bss_section)->bss = 1;
	  }
	section = bss_section;
	break;
      default:
        as_fatal (_("internal error: base section index out of range"));
        return;
	break;
    }
  subseg_set (section, 0);
}

/* This finishes off parsing a .comm or .lcomm statement, which both can have
   an (optional) alignment field.  It also allows us to create the bss section
   on demand.  */

static symbolS *
obj_mach_o_common_parse (int is_local, symbolS *symbolP,
			 addressT size)
{
  addressT align = 0;
  bfd_mach_o_asymbol *s;

  SKIP_WHITESPACE ();

  /* Both comm and lcomm take an optional alignment, as a power
     of two between 1 and 15.  */
  if (*input_line_pointer == ',')
    {
      /* We expect a power of 2.  */
      align = parse_align (0);
      if (align == (addressT) -1)
	return NULL;
      if (align > 15)
	{
	  as_warn (_("Alignment (%lu) too large: 15 assumed."),
		  (unsigned long)align);
	  align = 15;
	}
    }

  s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (symbolP);
  if (is_local)
    {
      /* Create the BSS section on demand.  */
      if (bss_section == NULL)
	{
	  bss_section = obj_mach_o_segT_from_bfd_name (BSS_SECTION_NAME, 1);
	  seg_info (bss_section)->bss = 1;
	}
      bss_alloc (symbolP, size, align);
      s->n_type = BFD_MACH_O_N_SECT;
      S_CLEAR_EXTERNAL (symbolP);
    }
  else
    {
      S_SET_VALUE (symbolP, size);
      S_SET_ALIGN (symbolP, align);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT (symbolP, bfd_com_section_ptr);
      s->n_type = BFD_MACH_O_N_UNDF | BFD_MACH_O_N_EXT;
    }

  /* This is a data object (whatever we choose that to mean).  */
  s->symbol.flags |= BSF_OBJECT;

  /* We've set symbol qualifiers, so validate if you can.  */
  s->symbol.udata.i = SYM_MACHO_FIELDS_NOT_VALIDATED;

  return symbolP;
}

static void
obj_mach_o_comm (int is_local)
{
  s_comm_internal (is_local, obj_mach_o_common_parse);
}

/* Set properties that apply to the whole file.  At present, the only
   one defined, is subsections_via_symbols.  */

typedef enum obj_mach_o_file_properties {
  OBJ_MACH_O_FILE_PROP_NONE = 0,
  OBJ_MACH_O_FILE_PROP_SUBSECTS_VIA_SYMS,
  OBJ_MACH_O_FILE_PROP_MAX
} obj_mach_o_file_properties;

static void
obj_mach_o_fileprop (int prop)
{
  if (prop < 0 || prop >= OBJ_MACH_O_FILE_PROP_MAX)
    as_fatal (_("internal error: bad file property ID %d"), prop);

  switch ((obj_mach_o_file_properties) prop)
    {
      case OBJ_MACH_O_FILE_PROP_SUBSECTS_VIA_SYMS:
        obj_mach_o_subsections_by_symbols = 1;
	if (!bfd_set_private_flags (stdoutput,
				    BFD_MACH_O_MH_SUBSECTIONS_VIA_SYMBOLS))
	  as_bad (_("failed to set subsections by symbols"));
	demand_empty_rest_of_line ();
	break;
      default:
	break;
    }
}

/* Temporary markers for symbol reference data.
   Lazy will remain in place.  */
#define LAZY 0x01
#define REFE 0x02

/* We have a bunch of qualifiers that may be applied to symbols.
   .globl is handled here so that we might make sure that conflicting qualifiers
   are caught where possible.  */

typedef enum obj_mach_o_symbol_type {
  OBJ_MACH_O_SYM_UNK = 0,
  OBJ_MACH_O_SYM_LOCAL = 1,
  OBJ_MACH_O_SYM_GLOBL = 2,
  OBJ_MACH_O_SYM_REFERENCE = 3,
  OBJ_MACH_O_SYM_WEAK_REF = 4,
  OBJ_MACH_O_SYM_LAZY_REF = 5,
  OBJ_MACH_O_SYM_WEAK_DEF = 6,
  OBJ_MACH_O_SYM_PRIV_EXT = 7,
  OBJ_MACH_O_SYM_NO_DEAD_STRIP = 8,
  OBJ_MACH_O_SYM_WEAK = 9
} obj_mach_o_symbol_type;

/* Set Mach-O-specific symbol qualifiers. */

static int
obj_mach_o_set_symbol_qualifier (symbolS *sym, int type)
{
  int is_defined;
  bfd_mach_o_asymbol *s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (sym);
  bfd_mach_o_section *sec;
  int sectype = -1;

  /* If the symbol is defined, then we can do more rigorous checking on
     the validity of the qualifiers.  Otherwise, we are stuck with waiting
     until it's defined - or until write the file.

     In certain cases (e.g. when a symbol qualifier is intended to introduce
     an undefined symbol in a stubs section) we should check that the current
     section is appropriate to the qualifier.  */

  is_defined = s->symbol.section != bfd_und_section_ptr;
  if (is_defined)
    sec = bfd_mach_o_get_mach_o_section (s->symbol.section) ;
  else
    sec = bfd_mach_o_get_mach_o_section (now_seg) ;

  if (sec != NULL)
    sectype = sec->flags & BFD_MACH_O_SECTION_TYPE_MASK;

  switch ((obj_mach_o_symbol_type) type)
    {
      case OBJ_MACH_O_SYM_LOCAL:
	/* This is an extension over the system tools.  */
        if (s->n_type & (BFD_MACH_O_N_PEXT | BFD_MACH_O_N_EXT))
	  {
	    as_bad (_("'%s' previously declared as '%s'."), s->symbol.name,
		      (s->n_type & BFD_MACH_O_N_PEXT) ? "private extern"
						      : "global" );
	    s->symbol.udata.i = SYM_MACHO_FIELDS_UNSET;
	    return 1;
	  }
	else
	  {
	    s->n_type &= ~BFD_MACH_O_N_EXT;
	    S_CLEAR_EXTERNAL (sym);
	  }
	break;

      case OBJ_MACH_O_SYM_PRIV_EXT:
	s->n_type |= BFD_MACH_O_N_PEXT ;
	s->n_desc &= ~LAZY; /* The native tool switches this off too.  */
	/* We follow the system tools in marking PEXT as also global.  */
	/* Fall through.  */

      case OBJ_MACH_O_SYM_GLOBL:
	/* It's not an error to define a symbol and then make it global.  */
	s->n_type |= BFD_MACH_O_N_EXT;
	S_SET_EXTERNAL (sym);
	break;

      case OBJ_MACH_O_SYM_REFERENCE:
        if (is_defined)
          s->n_desc |= BFD_MACH_O_N_NO_DEAD_STRIP;
        else
          s->n_desc |= (REFE | BFD_MACH_O_N_NO_DEAD_STRIP);
	break;

      case OBJ_MACH_O_SYM_LAZY_REF:
        if (is_defined)
          s->n_desc |= BFD_MACH_O_N_NO_DEAD_STRIP;
        else
          s->n_desc |= (REFE | LAZY | BFD_MACH_O_N_NO_DEAD_STRIP);
	break;

      /* Force ld to retain the symbol - even if it appears unused.  */
      case OBJ_MACH_O_SYM_NO_DEAD_STRIP:
	s->n_desc |= BFD_MACH_O_N_NO_DEAD_STRIP ;
	break;

      /* Mach-O's idea of weak ...  */
      case OBJ_MACH_O_SYM_WEAK_REF:
	s->n_desc |= BFD_MACH_O_N_WEAK_REF ;
	break;

      case OBJ_MACH_O_SYM_WEAK_DEF:
	if (is_defined && sectype != BFD_MACH_O_S_COALESCED)
	  {
	    as_bad (_("'%s' can't be a weak_definition (currently only"
		      " supported in sections of type coalesced)"),
		      s->symbol.name);
	    s->symbol.udata.i = SYM_MACHO_FIELDS_UNSET;
	    return 1;
	  }
	else
	  s->n_desc |= BFD_MACH_O_N_WEAK_DEF;
	break;

      case OBJ_MACH_O_SYM_WEAK:
        /* A generic 'weak' - we try to figure out what it means at
	   symbol frob time.  */
	S_SET_WEAK (sym);
	break;

      default:
	break;
    }

    /* We've seen some kind of qualifier - check validity if or when the entity
     is defined.  */
  s->symbol.udata.i = SYM_MACHO_FIELDS_NOT_VALIDATED;
  return 0;
}

/* Respond to symbol qualifiers.
   All of the form:
   .<qualifier> symbol [, symbol]*
   a list of symbols is an extension over the Darwin system as.  */

static void
obj_mach_o_sym_qual (int ntype)
{
  char *name;
  char c;
  symbolS *symbolP;

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  do
    {
      c = get_symbol_name (&name);
      symbolP = symbol_find_or_make (name);
      obj_mach_o_set_symbol_qualifier (symbolP, ntype);
      *input_line_pointer = c;
      SKIP_WHITESPACE_AFTER_NAME ();
      c = *input_line_pointer;
      if (c == ',')
	{
	  input_line_pointer++;
	  SKIP_WHITESPACE ();
	  if (is_end_of_line[(unsigned char) *input_line_pointer])
	    c = '\n';
	}
    }
  while (c == ',');

  demand_empty_rest_of_line ();
}

typedef struct obj_mach_o_indirect_sym
{
  symbolS *sym;
  segT sect;
  struct obj_mach_o_indirect_sym *next;
} obj_mach_o_indirect_sym;

/* We store in order an maintain a pointer to the last one - to save reversing
   later.  */
obj_mach_o_indirect_sym *indirect_syms;
obj_mach_o_indirect_sym *indirect_syms_tail;

static void
obj_mach_o_indirect_symbol (int arg ATTRIBUTE_UNUSED)
{
  bfd_mach_o_section *sec = bfd_mach_o_get_mach_o_section (now_seg);

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (obj_mach_o_is_static)
    as_bad (_("use of .indirect_symbols requires `-dynamic'"));

  switch (sec->flags & BFD_MACH_O_SECTION_TYPE_MASK)
    {
      case BFD_MACH_O_S_SYMBOL_STUBS:
      case BFD_MACH_O_S_LAZY_SYMBOL_POINTERS:
      case BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS:
        {
          obj_mach_o_indirect_sym *isym;
	  char *name;
	  char c = get_symbol_name (&name);
	  symbolS *sym = symbol_find_or_make (name);
	  unsigned int elsize =
			bfd_mach_o_section_get_entry_size (stdoutput, sec);

	  if (elsize == 0)
	    {
	      as_bad (_("attempt to add an indirect_symbol to a stub or"
			" reference section with a zero-sized element at %s"),
			name);
	      (void) restore_line_pointer (c);
	      ignore_rest_of_line ();
	      return;
	    }
	  (void) restore_line_pointer (c);

	  /* The indirect symbols are validated after the symbol table is
	     frozen, we must make sure that if a local symbol is used as an
	     indirect, it is promoted to a 'real' one.  Fetching the bfd sym
	     achieves this.  */
	  symbol_get_bfdsym (sym);
	  isym = XNEW (obj_mach_o_indirect_sym);

	  /* Just record the data for now, we will validate it when we
	     compute the output in obj_mach_o_set_indirect_symbols.  */
	  isym->sym = sym;
	  isym->sect = now_seg;
	  isym->next = NULL;
	  if (indirect_syms == NULL)
	    indirect_syms = isym;
	  else
	    indirect_syms_tail->next = isym;
	  indirect_syms_tail = isym;
	}
        break;

      default:
	as_bad (_("an .indirect_symbol must be in a symbol pointer"
		  " or stub section."));
	ignore_rest_of_line ();
	return;
    }
  demand_empty_rest_of_line ();
}

const pseudo_typeS mach_o_pseudo_table[] =
{
  /* Section directives.  */
  { "comm", obj_mach_o_comm, 0 },
  { "lcomm", obj_mach_o_comm, 1 },

  { "text", obj_mach_o_base_section, 1},
  { "data", obj_mach_o_base_section, 2},
  { "bss", obj_mach_o_base_section, 3},   /* extension */

  { "const", obj_mach_o_known_section, 1},
  { "static_const", obj_mach_o_known_section, 2},
  { "cstring", obj_mach_o_known_section, 3},
  { "literal4", obj_mach_o_known_section, 4},
  { "literal8", obj_mach_o_known_section, 5},
  { "literal16", obj_mach_o_known_section, 6},
  { "constructor", obj_mach_o_known_section, 7},
  { "destructor", obj_mach_o_known_section, 8},
  { "eh_frame", obj_mach_o_known_section, 9},

  { "const_data", obj_mach_o_known_section, 10},
  { "static_data", obj_mach_o_known_section, 11},
  { "mod_init_func", obj_mach_o_known_section, 12},
  { "mod_term_func", obj_mach_o_known_section, 13},
  { "dyld", obj_mach_o_known_section, 14},
  { "cfstring", obj_mach_o_known_section, 15},

  { "objc_class", obj_mach_o_objc_section, 1},
  { "objc_meta_class", obj_mach_o_objc_section, 2},
  { "objc_cat_cls_meth", obj_mach_o_objc_section, 3},
  { "objc_cat_inst_meth", obj_mach_o_objc_section, 4},
  { "objc_protocol", obj_mach_o_objc_section, 5},
  { "objc_string_object", obj_mach_o_objc_section, 6},
  { "objc_cls_meth", obj_mach_o_objc_section, 7},
  { "objc_inst_meth", obj_mach_o_objc_section, 8},
  { "objc_cls_refs", obj_mach_o_objc_section, 9},
  { "objc_message_refs", obj_mach_o_objc_section, 10},
  { "objc_symbols", obj_mach_o_objc_section, 11},
  { "objc_category", obj_mach_o_objc_section, 12},
  { "objc_class_vars", obj_mach_o_objc_section, 13},
  { "objc_instance_vars", obj_mach_o_objc_section, 14},
  { "objc_module_info", obj_mach_o_objc_section, 15},
  { "objc_class_names", obj_mach_o_objc_section, 16}, /* Alias for .cstring */
  { "objc_meth_var_types", obj_mach_o_objc_section, 17}, /* Alias for .cstring */
  { "objc_meth_var_names", obj_mach_o_objc_section, 18}, /* Alias for .cstring */
  { "objc_selector_strs", obj_mach_o_objc_section, 19},
  { "objc_image_info", obj_mach_o_objc_section, 20}, /* extension.  */
  { "objc_selector_fixup", obj_mach_o_objc_section, 21}, /* extension.  */
  { "objc1_class_ext", obj_mach_o_objc_section, 22}, /* ObjC-1 extension.  */
  { "objc1_property_list", obj_mach_o_objc_section, 23}, /* ObjC-1 extension.  */
  { "objc1_protocol_ext", obj_mach_o_objc_section, 24}, /* ObjC-1 extension.  */

  { "debug_frame", obj_mach_o_debug_section, 1}, /* extension.  */
  { "debug_info", obj_mach_o_debug_section, 2}, /* extension.  */
  { "debug_abbrev", obj_mach_o_debug_section, 3}, /* extension.  */
  { "debug_aranges", obj_mach_o_debug_section, 4}, /* extension.  */
  { "debug_macinfo", obj_mach_o_debug_section, 5}, /* extension.  */
  { "debug_line", obj_mach_o_debug_section, 6}, /* extension.  */
  { "debug_loc", obj_mach_o_debug_section, 7}, /* extension.  */
  { "debug_pubnames", obj_mach_o_debug_section, 8}, /* extension.  */
  { "debug_pubtypes", obj_mach_o_debug_section, 9}, /* extension.  */
  { "debug_str", obj_mach_o_debug_section, 10}, /* extension.  */
  { "debug_ranges", obj_mach_o_debug_section, 11}, /* extension.  */
  { "debug_macro", obj_mach_o_debug_section, 12}, /* extension.  */

  { "lazy_symbol_pointer", obj_mach_o_opt_tgt_section, 1},
  { "lazy_symbol_pointer2", obj_mach_o_opt_tgt_section, 2}, /* extension.  */
  { "lazy_symbol_pointer3", obj_mach_o_opt_tgt_section, 3}, /* extension.  */
  { "non_lazy_symbol_pointer", obj_mach_o_opt_tgt_section, 4},
  { "non_lazy_symbol_pointer_x86", obj_mach_o_opt_tgt_section, 5}, /* extension.  */
  { "symbol_stub", obj_mach_o_opt_tgt_section, 6},
  { "symbol_stub1", obj_mach_o_opt_tgt_section, 7}, /* extension.  */
  { "picsymbol_stub", obj_mach_o_opt_tgt_section, 8}, /* extension.  */
  { "picsymbol_stub1", obj_mach_o_opt_tgt_section, 9}, /* extension.  */
  { "picsymbol_stub2", obj_mach_o_opt_tgt_section, 4}, /* extension.  */
  { "picsymbol_stub3", obj_mach_o_opt_tgt_section, 4}, /* extension.  */

  { "section", obj_mach_o_section, 0},
  { "zerofill", obj_mach_o_zerofill, 0},

  /* Symbol qualifiers.  */
  {"local",		obj_mach_o_sym_qual, OBJ_MACH_O_SYM_LOCAL},
  {"globl",		obj_mach_o_sym_qual, OBJ_MACH_O_SYM_GLOBL},
  {"reference",		obj_mach_o_sym_qual, OBJ_MACH_O_SYM_REFERENCE},
  {"weak_reference",	obj_mach_o_sym_qual, OBJ_MACH_O_SYM_WEAK_REF},
  {"lazy_reference",	obj_mach_o_sym_qual, OBJ_MACH_O_SYM_LAZY_REF},
  {"weak_definition",	obj_mach_o_sym_qual, OBJ_MACH_O_SYM_WEAK_DEF},
  {"private_extern",	obj_mach_o_sym_qual, OBJ_MACH_O_SYM_PRIV_EXT},
  {"no_dead_strip",	obj_mach_o_sym_qual, OBJ_MACH_O_SYM_NO_DEAD_STRIP},
  {"weak",		obj_mach_o_sym_qual, OBJ_MACH_O_SYM_WEAK}, /* ext */

  { "indirect_symbol",	obj_mach_o_indirect_symbol, 0},

  /* File flags.  */
  { "subsections_via_symbols", obj_mach_o_fileprop,
			       OBJ_MACH_O_FILE_PROP_SUBSECTS_VIA_SYMS},

  {NULL, NULL, 0}
};

/* Determine the default n_type value for a symbol from its section.  */

static unsigned
obj_mach_o_type_for_symbol (bfd_mach_o_asymbol *s)
{
  if (s->symbol.section == bfd_abs_section_ptr)
    return BFD_MACH_O_N_ABS;
  else if (s->symbol.section == bfd_com_section_ptr
	   || s->symbol.section == bfd_und_section_ptr)
    return BFD_MACH_O_N_UNDF;
  else
    return BFD_MACH_O_N_SECT;
}

void
obj_mach_o_frob_colon (const char *name)
{
  if (!bfd_is_local_label_name (stdoutput, name))
    {
      /* A non-local label will create a new subsection, so start a new
         frag.  */
      frag_wane (frag_now);
      frag_new (0);
    }
}

/* We need to check the correspondence between some kinds of symbols and their
   sections.  Common and BSS vars will seen via the obj_macho_comm() function.

   The earlier we can pick up a problem, the better the diagnostics will be.

   However, when symbol type information is attached, the symbol section will
   quite possibly be unknown.  So we are stuck with checking (most of the)
   validity at the time the file is written (unfortunately, then one doesn't
   get line number information in the diagnostic).  */

/* Here we pick up the case where symbol qualifiers have been applied that
   are possibly incompatible with the section etc. that the symbol is defined
   in.  */

void obj_mach_o_frob_label (struct symbol *sp)
{
  bfd_mach_o_asymbol *s;
  unsigned base_type;
  bfd_mach_o_section *sec;
  int sectype = -1;

  if (!bfd_is_local_label_name (stdoutput, S_GET_NAME (sp)))
    {
      /* If this is a non-local label, it should have started a new sub-
	 section.  */
      gas_assert (frag_now->obj_frag_data.subsection == NULL);
      frag_now->obj_frag_data.subsection = sp;
    }

  /* Leave local symbols alone.  */

  if (S_IS_LOCAL (sp))
    return;

  s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (sp);
  /* Leave debug symbols alone.  */
  if ((s->n_type & BFD_MACH_O_N_STAB) != 0)
    return;

  /* This is the base symbol type, that we mask in.  */
  base_type = obj_mach_o_type_for_symbol (s);

  sec = bfd_mach_o_get_mach_o_section (s->symbol.section);
  if (sec != NULL)
    sectype = sec->flags & BFD_MACH_O_SECTION_TYPE_MASK;

  /* If there is a pre-existing qualifier, we can make some checks about
     validity now.  */

  if(s->symbol.udata.i == SYM_MACHO_FIELDS_NOT_VALIDATED)
    {
      if ((s->n_desc & BFD_MACH_O_N_WEAK_DEF)
	  && sectype != BFD_MACH_O_S_COALESCED)
	{
	  as_bad (_("'%s' can't be a weak_definition (currently only supported"
		    " in sections of type coalesced)"), s->symbol.name);
	  /* Don't cascade errors.  */
	  s->symbol.udata.i = SYM_MACHO_FIELDS_UNSET;
	}

      /* Have we changed from an undefined to defined ref? */
      s->n_desc &= ~(REFE | LAZY);
    }

  s->n_type &= ~BFD_MACH_O_N_TYPE;
  s->n_type |= base_type;
}

/* This is the fall-back, we come here when we get to the end of the file and
   the symbol is not defined - or there are combinations of qualifiers required
   (e.g. global + weak_def).  */

int
obj_mach_o_frob_symbol (struct symbol *sp)
{
  bfd_mach_o_asymbol *s;
  unsigned base_type;
  bfd_mach_o_section *sec;
  int sectype = -1;

  /* Leave local symbols alone.  */
  if (S_IS_LOCAL (sp))
    return 0;

  s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (sp);
  /* Leave debug symbols alone.  */
  if ((s->n_type & BFD_MACH_O_N_STAB) != 0)
    return 0;

  base_type = obj_mach_o_type_for_symbol (s);
  sec = bfd_mach_o_get_mach_o_section (s->symbol.section);
  if (sec != NULL)
    sectype = sec->flags & BFD_MACH_O_SECTION_TYPE_MASK;

  if (s->symbol.section == bfd_und_section_ptr)
    {
      /* ??? Do we really gain much from implementing this as well as the
	 mach-o specific ones?  */
      if (s->symbol.flags & BSF_WEAK)
	s->n_desc |= BFD_MACH_O_N_WEAK_REF;

      /* Undefined syms, become extern.  */
      s->n_type |= BFD_MACH_O_N_EXT;
      S_SET_EXTERNAL (sp);
    }
  else if (s->symbol.section == bfd_com_section_ptr)
    {
      /* ... so do comm.  */
      s->n_type |= BFD_MACH_O_N_EXT;
      S_SET_EXTERNAL (sp);
    }
  else
    {
      if ((s->symbol.flags & BSF_WEAK)
	   && (sectype == BFD_MACH_O_S_COALESCED)
	   && (s->n_type & (BFD_MACH_O_N_PEXT | BFD_MACH_O_N_EXT)))
	s->n_desc |= BFD_MACH_O_N_WEAK_DEF;
/* ??? we should do this - but then that reveals that the semantics of weak
       are different from what's supported in mach-o object files.
      else
	as_bad (_("'%s' can't be a weak_definition."),
		s->symbol.name); */
    }

  if (s->symbol.udata.i == SYM_MACHO_FIELDS_UNSET)
    {
      /* Anything here that should be added that is non-standard.  */
      s->n_desc &= ~BFD_MACH_O_REFERENCE_MASK;
    }
  else if (s->symbol.udata.i == SYM_MACHO_FIELDS_NOT_VALIDATED)
    {
      /* Try to validate any combinations.  */
      if (s->n_desc & BFD_MACH_O_N_WEAK_DEF)
	{
	  if (s->symbol.section == bfd_und_section_ptr)
	    as_bad (_("'%s' can't be a weak_definition (since it is"
		      " undefined)"), s->symbol.name);
	  else if (sectype != BFD_MACH_O_S_COALESCED)
	    as_bad (_("'%s' can't be a weak_definition (currently only supported"
		      " in sections of type coalesced)"), s->symbol.name);
	  else if (! (s->n_type & (BFD_MACH_O_N_PEXT | BFD_MACH_O_N_EXT)))
	    as_bad (_("Non-global symbol: '%s' can't be a weak_definition."),
		    s->symbol.name);
	}

    }
  else
    as_bad (_("internal error: [%s] unexpected code [%lx] in frob symbol"),
	    s->symbol.name, (unsigned long)s->symbol.udata.i);

  s->n_type &= ~BFD_MACH_O_N_TYPE;
  s->n_type |= base_type;

  if (s->symbol.flags & BSF_GLOBAL)
    s->n_type |= BFD_MACH_O_N_EXT;

  /* This cuts both ways - we promote some things to external above.  */
  if (s->n_type & (BFD_MACH_O_N_PEXT | BFD_MACH_O_N_EXT))
    S_SET_EXTERNAL (sp);

  return 0;
}

/* Support stabs for mach-o.  */

void
obj_mach_o_process_stab (int what, const char *string,
			 int type, int other, int desc)
{
  symbolS *symbolP;
  bfd_mach_o_asymbol *s;

  switch (what)
    {
      case 'd':
	symbolP = symbol_new ("", now_seg, frag_now, frag_now_fix ());
	/* Special stabd NULL name indicator.  */
	S_SET_NAME (symbolP, NULL);
	break;

      case 'n':
      case 's':
	symbolP = symbol_new (string, undefined_section,
			      &zero_address_frag, 0);
	pseudo_set (symbolP);
	break;

      default:
	as_bad(_("unrecognized stab type '%c'"), (char)what);
	abort ();
	break;
    }

  s = (bfd_mach_o_asymbol *) symbol_get_bfdsym (symbolP);
  s->n_type = type;
  s->n_desc = desc;
  /* For stabd, this will eventually get overwritten by the section number.  */
  s->n_sect = other;

  /* It's a debug symbol.  */
  s->symbol.flags |= BSF_DEBUGGING;

  /* We've set it - so check it, if you can, but don't try to create the
     flags.  */
  s->symbol.udata.i = SYM_MACHO_FIELDS_NOT_VALIDATED;
}

/* This is a place to check for any errors that we can't detect until we know
   what remains undefined at the end of assembly.  */

static void
obj_mach_o_check_before_writing (bfd *abfd ATTRIBUTE_UNUSED,
				 asection *sec,
				 void *unused ATTRIBUTE_UNUSED)
{
  fixS *fixP;
  struct frchain *frchp;
  segment_info_type *seginfo = seg_info (sec);

  if (seginfo == NULL)
    return;

  /* We are not allowed subtractions where either of the operands is
     undefined.  So look through the frags for any fixes to check.  */
  for (frchp = seginfo->frchainP; frchp != NULL; frchp = frchp->frch_next)
   for (fixP = frchp->fix_root; fixP != NULL; fixP = fixP->fx_next)
    {
      if (fixP->fx_addsy != NULL
	  && fixP->fx_subsy != NULL
	  && (! S_IS_DEFINED (fixP->fx_addsy)
	      || ! S_IS_DEFINED (fixP->fx_subsy)))
	{
	  segT add_symbol_segment = S_GET_SEGMENT (fixP->fx_addsy);
	  segT sub_symbol_segment = S_GET_SEGMENT (fixP->fx_subsy);

	  if (! S_IS_DEFINED (fixP->fx_addsy)
	      && S_IS_DEFINED (fixP->fx_subsy))
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
		_("`%s' can't be undefined in `%s' - `%s' {%s section}"),
		S_GET_NAME (fixP->fx_addsy), S_GET_NAME (fixP->fx_addsy),
		S_GET_NAME (fixP->fx_subsy), segment_name (sub_symbol_segment));
	    }
	  else if (! S_IS_DEFINED (fixP->fx_subsy)
		   && S_IS_DEFINED (fixP->fx_addsy))
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
		_("`%s' can't be undefined in `%s' {%s section} - `%s'"),
		S_GET_NAME (fixP->fx_subsy), S_GET_NAME (fixP->fx_addsy),
		segment_name (add_symbol_segment), S_GET_NAME (fixP->fx_subsy));
	    }
	  else
	    {
	      as_bad_where (fixP->fx_file, fixP->fx_line,
		_("`%s' and `%s' can't be undefined in `%s' - `%s'"),
		S_GET_NAME (fixP->fx_addsy), S_GET_NAME (fixP->fx_subsy),
		S_GET_NAME (fixP->fx_addsy), S_GET_NAME (fixP->fx_subsy));
	    }
	}
    }
}

/* Do any checks that we can't complete without knowing what's undefined.  */
void
obj_mach_o_pre_output_hook (void)
{
  bfd_map_over_sections (stdoutput, obj_mach_o_check_before_writing, (char *) 0);
}

/* Here we count up frags in each subsection (where a sub-section is defined
   as starting with a non-local symbol).
   Note that, if there are no non-local symbols in a section, all the frags will
   be attached as one anonymous subsection.  */

static void
obj_mach_o_set_subsections (bfd *abfd ATTRIBUTE_UNUSED,
                            asection *sec,
                            void *unused ATTRIBUTE_UNUSED)
{
  segment_info_type *seginfo = seg_info (sec);
  symbolS *cur_subsection = NULL;
  struct obj_mach_o_symbol_data *cur_subsection_data = NULL;
  fragS *frag;
  frchainS *chain;

  /* Protect against sections not created by gas.  */
  if (seginfo == NULL)
    return;

  /* Attach every frag to a subsection.  */
  for (chain = seginfo->frchainP; chain != NULL; chain = chain->frch_next)
    for (frag = chain->frch_root; frag != NULL; frag = frag->fr_next)
      {
        if (frag->obj_frag_data.subsection == NULL)
          frag->obj_frag_data.subsection = cur_subsection;
        else
          {
            cur_subsection = frag->obj_frag_data.subsection;
            cur_subsection_data = symbol_get_obj (cur_subsection);
            cur_subsection_data->subsection_size = 0;
          }
        if (cur_subsection_data != NULL)
          {
            /* Update subsection size.  */
            cur_subsection_data->subsection_size += frag->fr_fix;
          }
      }
}

/* Handle mach-o subsections-via-symbols counting up frags belonging to each
   sub-section.  */

void
obj_mach_o_pre_relax_hook (void)
{
  bfd_map_over_sections (stdoutput, obj_mach_o_set_subsections, (char *) 0);
}

/* Zerofill and GB Zerofill sections must be sorted to follow all other
   sections in their segments.

   The native 'as' leaves the sections physically in the order they appear in
   the source, and adjusts the section VMAs to meet the constraint.

   We follow this for now - if nothing else, it makes comparison easier.

   An alternative implementation would be to sort the sections as ld requires.
   It might be advantageous to implement such a scheme in the future (or even
   to make the style of section ordering user-selectable).  */

typedef struct obj_mach_o_set_vma_data
{
  bfd_vma vma;
  unsigned vma_pass;
  unsigned zerofill_seen;
  unsigned gb_zerofill_seen;
} obj_mach_o_set_vma_data;

/* We do (possibly) three passes through to set the vma, so that:

   zerofill sections get VMAs after all others in their segment
   GB zerofill get VMAs last.

   As we go, we notice if we see any Zerofill or GB Zerofill sections, so that
   we can skip the additional passes if there's nothing to do.  */

static void
obj_mach_o_set_section_vma (bfd *abfd ATTRIBUTE_UNUSED, asection *sec, void *v_p)
{
  bfd_mach_o_section *ms = bfd_mach_o_get_mach_o_section (sec);
  unsigned bfd_align = bfd_section_alignment (sec);
  obj_mach_o_set_vma_data *p = (struct obj_mach_o_set_vma_data *)v_p;
  unsigned sectype = (ms->flags & BFD_MACH_O_SECTION_TYPE_MASK);
  unsigned zf;

  zf = 0;
  if (sectype == BFD_MACH_O_S_ZEROFILL)
    {
      zf = 1;
      p->zerofill_seen = zf;
    }
  else if (sectype == BFD_MACH_O_S_GB_ZEROFILL)
    {
      zf = 2;
      p->gb_zerofill_seen = zf;
    }

  if (p->vma_pass != zf)
    return;

  /* We know the section size now - so make a vma for the section just
     based on order.  */
  ms->size = bfd_section_size (sec);

  /* Make sure that the align agrees, and set to the largest value chosen.  */
  ms->align = ms->align > bfd_align ? ms->align : bfd_align;
  bfd_set_section_alignment (sec, ms->align);

  p->vma += (1 << ms->align) - 1;
  p->vma &= ~((1 << ms->align) - 1);
  ms->addr = p->vma;
  bfd_set_section_vma (sec, p->vma);
  p->vma += ms->size;
}

/* (potentially) three passes over the sections, setting VMA.  We skip the
  {gb}zerofill passes if we didn't see any of the relevant sections.  */

void obj_mach_o_post_relax_hook (void)
{
  obj_mach_o_set_vma_data d;

  memset (&d, 0, sizeof (d));

  bfd_map_over_sections (stdoutput, obj_mach_o_set_section_vma, (char *) &d);
  if ((d.vma_pass = d.zerofill_seen) != 0)
    bfd_map_over_sections (stdoutput, obj_mach_o_set_section_vma, (char *) &d);
  if ((d.vma_pass = d.gb_zerofill_seen) != 0)
    bfd_map_over_sections (stdoutput, obj_mach_o_set_section_vma, (char *) &d);
}

static void
obj_mach_o_set_indirect_symbols (bfd *abfd, asection *sec,
				 void *xxx ATTRIBUTE_UNUSED)
{
  bfd_vma sect_size = bfd_section_size (sec);
  bfd_mach_o_section *ms = bfd_mach_o_get_mach_o_section (sec);
  unsigned lazy = 0;

  /* See if we have any indirect syms to consider.  */
  if (indirect_syms == NULL)
    return;

  /* Process indirect symbols.
     Check for errors, if OK attach them as a flat array to the section
     for which they are defined.  */

  switch (ms->flags & BFD_MACH_O_SECTION_TYPE_MASK)
    {
      case BFD_MACH_O_S_SYMBOL_STUBS:
      case BFD_MACH_O_S_LAZY_SYMBOL_POINTERS:
	lazy = LAZY;
	/* Fall through.  */
      case BFD_MACH_O_S_NON_LAZY_SYMBOL_POINTERS:
	{
	  unsigned int nactual = 0;
	  unsigned int ncalc;
	  obj_mach_o_indirect_sym *isym;
	  obj_mach_o_indirect_sym *list = NULL;
	  obj_mach_o_indirect_sym *list_tail = NULL;
	  unsigned long eltsiz =
			bfd_mach_o_section_get_entry_size (abfd, ms);

	  for (isym = indirect_syms; isym != NULL; isym = isym->next)
	    {
	      if (isym->sect == sec)
		{
		  nactual++;
		  if (list == NULL)
		    list = isym;
		  else
		    list_tail->next = isym;
		  list_tail = isym;
		}
	    }

	  /* If none are in this section, stop here.  */
	  if (nactual == 0)
	    break;

	  /* If we somehow added indirect symbols to a section with a zero
	     entry size, we're dead ... */
	  gas_assert (eltsiz != 0);

	  ncalc = (unsigned int) (sect_size / eltsiz);
	  if (nactual != ncalc)
	    as_bad (_("the number of .indirect_symbols defined in section %s"
		      " does not match the number expected (%d defined, %d"
		      " expected)"), sec->name, nactual, ncalc);
	  else
	    {
	      unsigned n;
	      bfd_mach_o_asymbol *sym;

	      /* FIXME: It seems that there can be more indirect symbols
		 than is computed by the loop above.  So be paranoid and
		 allocate enough space for every symbol to be indirect.
		 See PR 21939 for an example of where this is needed.  */
	      if (nactual < bfd_get_symcount (abfd))
		nactual = bfd_get_symcount (abfd);

	      ms->indirect_syms =
			bfd_zalloc (abfd,
				    nactual * sizeof (bfd_mach_o_asymbol *));

	      if (ms->indirect_syms == NULL)
		as_fatal (_("internal error: failed to allocate %d indirect"
			    "symbol pointers"), nactual);

	      for (isym = list, n = 0; isym != NULL; isym = isym->next, n++)
		{
		  sym = (bfd_mach_o_asymbol *)symbol_get_bfdsym (isym->sym);
		  /* Array is init to NULL & NULL signals a local symbol
		     If the section is lazy-bound, we need to keep the
		     reference to the symbol, since dyld can override.

		     Absolute symbols are handled specially.  */
		  if (sym->symbol.section == bfd_abs_section_ptr)
		    {
		      if (n >= nactual)
			as_fatal (_("internal error: more indirect mach-o symbols than expected"));
		      ms->indirect_syms[n] = sym;
		    }
		  else if (S_IS_LOCAL (isym->sym) && ! lazy)
		    ;
		  else
		    {
		      if (sym == NULL)
		        ;
		      /* If the symbols is external ...  */
		      else if (S_IS_EXTERNAL (isym->sym)
			       || (sym->n_type & BFD_MACH_O_N_EXT)
			       || ! S_IS_DEFINED (isym->sym)
			       || lazy)
			{
			  sym->n_desc &= ~LAZY;
			  /* ... it can be lazy, if not defined or hidden.  */
			  if ((sym->n_type & BFD_MACH_O_N_TYPE)
			       == BFD_MACH_O_N_UNDF
			      && ! (sym->n_type & BFD_MACH_O_N_PEXT)
			      && (sym->n_type & BFD_MACH_O_N_EXT))
			    sym->n_desc |= lazy;
			  if (n >= nactual)
			    as_fatal (_("internal error: more indirect mach-o symbols than expected"));
			  ms->indirect_syms[n] = sym;
		        }
		    }
		}
	    }
	}
	break;

      default:
	break;
    }
}

/* The process of relocation could alter what's externally visible, thus we
   leave setting the indirect symbols until last.  */

void
obj_mach_o_frob_file_after_relocs (void)
{
  bfd_map_over_sections (stdoutput, obj_mach_o_set_indirect_symbols, (char *) 0);
}

/* Reverse relocations order to make ld happy.  */

void
obj_mach_o_reorder_section_relocs (asection *sec, arelent **rels, unsigned int n)
{
  unsigned int i;
  unsigned int max = n / 2;

  for (i = 0; i < max; i++)
    {
      arelent *r = rels[i];
      rels[i] = rels[n - i - 1];
      rels[n - i - 1] = r;
    }
  bfd_set_reloc (stdoutput, sec, rels, n);
}

/* Relocation rules are different in frame sections.  */

static int
obj_mach_o_is_frame_section (segT sec)
{
  int l;
  l = strlen (segment_name (sec));
  if ((l == 9 && startswith (segment_name (sec), ".eh_frame"))
       || (l == 12 && startswith (segment_name (sec), ".debug_frame")))
    return 1;
  return 0;
}

/* Unless we're in a frame section, we need to force relocs to be generated for
   local subtractions.  We might eliminate them later (if they are within the
   same sub-section) but we don't know that at the point that this decision is
   being made.  */

int
obj_mach_o_allow_local_subtract (expressionS * left ATTRIBUTE_UNUSED,
				 expressionS * right ATTRIBUTE_UNUSED,
				 segT seg)
{
  /* Don't interfere if it's one of the GAS internal sections.  */
  if (! SEG_NORMAL (seg))
    return 1;

  /* Allow in frame sections, otherwise emit a reloc.  */
  return obj_mach_o_is_frame_section (seg);
}

int
obj_mach_o_in_different_subsection (symbolS *a, symbolS *b)
{
  fragS *fa;
  fragS *fb;

  if (S_GET_SEGMENT (a) != S_GET_SEGMENT (b)
      || !S_IS_DEFINED (a)
      || !S_IS_DEFINED (b))
    {
      /* Not in the same segment, or undefined symbol.  */
      return 1;
    }

  fa = symbol_get_frag (a);
  fb = symbol_get_frag (b);
  if (fa == NULL || fb == NULL)
    {
      /* One of the symbols is not in a subsection.  */
      return 1;
    }

  return fa->obj_frag_data.subsection != fb->obj_frag_data.subsection;
}

int
obj_mach_o_force_reloc_sub_same (fixS *fix, segT seg)
{
  if (! SEG_NORMAL (seg))
    return 1;
  return obj_mach_o_in_different_subsection (fix->fx_addsy, fix->fx_subsy);
}

int
obj_mach_o_force_reloc_sub_local (fixS *fix, segT seg ATTRIBUTE_UNUSED)
{
  return obj_mach_o_in_different_subsection (fix->fx_addsy, fix->fx_subsy);
}

int
obj_mach_o_force_reloc (fixS *fix)
{
  if (generic_force_reloc (fix))
    return 1;

  /* Force a reloc if the target is not in the same subsection.
     FIXME: handle (a - b) where a and b belongs to the same subsection ?  */
  if (fix->fx_addsy != NULL)
    {
      symbolS *subsec = fix->fx_frag->obj_frag_data.subsection;
      symbolS *targ = fix->fx_addsy;

      /* There might be no subsections at all.  */
      if (subsec == NULL)
        return 0;

      if (S_GET_SEGMENT (targ) == absolute_section)
        return 0;

      return obj_mach_o_in_different_subsection (targ, subsec);
    }
  return 0;
}
