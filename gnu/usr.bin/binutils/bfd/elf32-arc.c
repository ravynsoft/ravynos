/* ARC-specific support for 32-bit ELF
   Copyright (C) 1994-2023 Free Software Foundation, Inc.
   Contributed by Cupertino Miranda (cmiranda@synopsys.com).

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/arc.h"
#include "libiberty.h"
#include "opcode/arc-func.h"
#include "opcode/arc.h"
#include "arc-plt.h"

#define FEATURE_LIST_NAME bfd_feature_list
#define CONFLICT_LIST bfd_conflict_list
#include "opcode/arc-attrs.h"

/* #define ARC_ENABLE_DEBUG 1  */
#ifdef ARC_ENABLE_DEBUG
static const char *
name_for_global_symbol (struct elf_link_hash_entry *h)
{
  static char *local_str = "(local)";
  if (h == NULL)
    return local_str;
  return h->root.root.string;
}
#define ARC_DEBUG(fmt, args...) fprintf (stderr, fmt, ##args)
#else
#define ARC_DEBUG(...)
#endif


#define ADD_RELA(BFD, SECTION, OFFSET, SYM_IDX, TYPE, ADDEND)		\
  {									\
    struct elf_link_hash_table *_htab = elf_hash_table (info);		\
    Elf_Internal_Rela _rel;						\
    bfd_byte * _loc;							\
									\
    if (_htab->dynamic_sections_created)				\
      {									\
	BFD_ASSERT (_htab->srel##SECTION &&_htab->srel##SECTION->contents); \
	_loc = _htab->srel##SECTION->contents				\
	  + ((_htab->srel##SECTION->reloc_count)			\
	     * sizeof (Elf32_External_Rela));				\
	_htab->srel##SECTION->reloc_count++;				\
	_rel.r_addend = ADDEND;						\
	_rel.r_offset = (_htab->s##SECTION)->output_section->vma	\
	  + (_htab->s##SECTION)->output_offset + OFFSET;		\
	BFD_ASSERT ((long) SYM_IDX != -1);				\
	_rel.r_info = ELF32_R_INFO (SYM_IDX, TYPE);			\
	bfd_elf32_swap_reloca_out (BFD, &_rel, _loc);			\
      }									\
  }

#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
      case VALUE: \
	return "R_" #TYPE; \
	break;

static ATTRIBUTE_UNUSED const char *
reloc_type_to_name (unsigned int type)
{
  switch (type)
    {
#include "elf/arc-reloc.def"

    default:
      return "UNKNOWN";
      break;
    }
}

#undef ARC_RELOC_HOWTO

/* Try to minimize the amount of space occupied by relocation tables
   on the ROM (not that the ROM won't be swamped by other ELF overhead).  */

#define USE_REL 1

/* Similar with bfd_get_32 but taking into account the
   middle-endianess of the ARC CPUs.  Only to be used in code
   sections.  */

static bfd_vma
bfd_get_32_me (bfd * abfd,const unsigned char * data)
{
  bfd_vma value = 0;

  if (bfd_big_endian (abfd))
    value = bfd_get_32 (abfd, data);
  else
    {
      value = ((bfd_get_8 (abfd, data) & 255) << 16);
      value |= ((bfd_get_8 (abfd, data + 1) & 255) << 24);
      value |= (bfd_get_8 (abfd, data + 2) & 255);
      value |= ((bfd_get_8 (abfd, data + 3) & 255) << 8);
    }

  return value;
}

static void
bfd_put_32_me (bfd *abfd, bfd_vma value,unsigned char *data)
{
  bfd_put_16 (abfd, (value & 0xffff0000) >> 16, data);
  bfd_put_16 (abfd, value & 0xffff, data + 2);
}

static ATTRIBUTE_UNUSED bool
is_reloc_PC_relative (reloc_howto_type *howto)
{
  return strstr (howto->name, "PC") != NULL;
}

static bool
is_reloc_SDA_relative (reloc_howto_type *howto)
{
  return strstr (howto->name, "SDA") != NULL;
}

static bool
is_reloc_for_GOT (reloc_howto_type * howto)
{
  if (strstr (howto->name, "TLS") != NULL)
    return false;
  return strstr (howto->name, "GOT") != NULL;
}

static bool
is_reloc_for_PLT (reloc_howto_type * howto)
{
  return strstr (howto->name, "PLT") != NULL;
}

static bool
is_reloc_for_TLS (reloc_howto_type *howto)
{
  return strstr (howto->name, "TLS") != NULL;
}

struct arc_relocation_data
{
  bfd_signed_vma reloc_offset;
  bfd_signed_vma reloc_addend;
  bfd_signed_vma got_offset_value;

  bfd_signed_vma sym_value;
  asection *sym_section;

  reloc_howto_type *howto;

  asection *input_section;

  bfd_signed_vma sdata_begin_symbol_vma;
  bool sdata_begin_symbol_vma_set;
  bfd_signed_vma got_symbol_vma;

  bool should_relocate;

  const char *symbol_name;
};

/* ARC ELF linker hash entry.  */
struct elf_arc_link_hash_entry
{
  struct elf_link_hash_entry root;

  struct got_entry *got_ents;
};


/* Should be included at this location due to static declarations
   defined before this point.  */
#include "arc-got.h"

#define arc_bfd_get_8(A,B,C) bfd_get_8(A,B)
#define arc_bfd_get_16(A,B,C) bfd_get_16(A,B)
#define arc_bfd_get_32(A,B,C) bfd_get_32(A,B)
#define arc_bfd_put_8(A,B,C,D) bfd_put_8(A,B,C)
#define arc_bfd_put_16(A,B,C,D) bfd_put_16(A,B,C)
#define arc_bfd_put_32(A,B,C,D) bfd_put_32(A,B,C)


static bfd_reloc_status_type
arc_elf_reloc (bfd *abfd ATTRIBUTE_UNUSED,
	       arelent *reloc_entry,
	       asymbol *symbol_in,
	       void *data ATTRIBUTE_UNUSED,
	       asection *input_section,
	       bfd *output_bfd,
	       char ** error_message ATTRIBUTE_UNUSED)
{
  if (output_bfd != NULL)
    {
      reloc_entry->address += input_section->output_offset;

      /* In case of relocateable link and if the reloc is against a
	 section symbol, the addend needs to be adjusted according to
	 where the section symbol winds up in the output section.  */
      if ((symbol_in->flags & BSF_SECTION_SYM) && symbol_in->section)
	reloc_entry->addend += symbol_in->section->output_offset;

      return bfd_reloc_ok;
    }

  return bfd_reloc_continue;
}


#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  TYPE = VALUE,

enum howto_list
{
#include "elf/arc-reloc.def"
  HOWTO_LIST_LAST
};

#undef ARC_RELOC_HOWTO

#define ARC_RELOC_HOWTO(TYPE, VALUE, RSIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  [TYPE] = HOWTO (R_##TYPE, 0, RSIZE, BITSIZE, false, 0,		\
		  complain_overflow_##OVERFLOW, arc_elf_reloc,		\
		  "R_" #TYPE, false, 0, 0, false),

static struct reloc_howto_struct elf_arc_howto_table[] =
{
#include "elf/arc-reloc.def"
/* Example of what is generated by the preprocessor.  Currently kept as an
   example.
 HOWTO (R_ARC_NONE, // Type.
    0, // Rightshift.
    4, // Size.
    32, // Bitsize.
    false, // PC_relative.
    0, // Bitpos.
    complain_overflow_bitfield, // Complain_on_overflow.
    bfd_elf_generic_reloc, // Special_function.
    "R_ARC_NONE", // Name.
    true, // Partial_inplace.
    0, // Src_mask.
    0, // Dst_mask.
    false), // PCrel_offset.
*/
};
#undef ARC_RELOC_HOWTO

static void
arc_elf_howto_init (void)
{
#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  elf_arc_howto_table[TYPE].pc_relative =				\
    (strstr (#FORMULA, " P ") != NULL || strstr (#FORMULA, " PDATA ") != NULL); \
  elf_arc_howto_table[TYPE].dst_mask = RELOC_FUNCTION(0, ~0);		\
  /* Only 32 bit data relocations should be marked as ME.  */		\
  if (strstr (#FORMULA, " ME ") != NULL)				\
    {									\
      BFD_ASSERT (SIZE == 4);						\
    }

#include "elf/arc-reloc.def"

}
#undef ARC_RELOC_HOWTO


#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  [TYPE] = VALUE,

const int howto_table_lookup[] =
{
#include "elf/arc-reloc.def"
};

#undef ARC_RELOC_HOWTO

static reloc_howto_type *
arc_elf_howto (unsigned int r_type)
{
  if (elf_arc_howto_table[R_ARC_32].dst_mask == 0)
    arc_elf_howto_init ();
  return &elf_arc_howto_table[r_type];
}

/* Map BFD reloc types to ARC ELF reloc types.  */

struct arc_reloc_map
{
  bfd_reloc_code_real_type  bfd_reloc_val;
  unsigned char		    elf_reloc_val;
};

/* ARC ELF linker hash table.  */
struct elf_arc_link_hash_table
{
  struct elf_link_hash_table elf;
};

static struct bfd_hash_entry *
elf_arc_link_hash_newfunc (struct bfd_hash_entry *entry,
			   struct bfd_hash_table *table,
			   const char *string)
{
  struct elf_arc_link_hash_entry * ret =
    (struct elf_arc_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = (struct elf_arc_link_hash_entry *)
	bfd_hash_allocate (table, sizeof (struct elf_arc_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_arc_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      ret->got_ents = NULL;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Destroy an ARC ELF linker hash table.  */
static void
elf_arc_link_hash_table_free (bfd *obfd)
{
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an ARC ELF linker hash table.  */

static struct bfd_link_hash_table *
arc_elf_link_hash_table_create (bfd *abfd)
{
  struct elf_arc_link_hash_table *ret;

  ret = (struct elf_arc_link_hash_table *) bfd_zmalloc (sizeof (*ret));
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd,
				      elf_arc_link_hash_newfunc,
				      sizeof (struct elf_arc_link_hash_entry),
				      ARC_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->elf.root.hash_table_free = elf_arc_link_hash_table_free;

  return &ret->elf.root;
}

#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  { BFD_RELOC_##TYPE, R_##TYPE },

static const struct arc_reloc_map arc_reloc_map[] =
{
#include "elf/arc-reloc.def"

  {BFD_RELOC_NONE,  R_ARC_NONE},
  {BFD_RELOC_8,  R_ARC_8},
  {BFD_RELOC_16, R_ARC_16},
  {BFD_RELOC_24, R_ARC_24},
  {BFD_RELOC_32, R_ARC_32},
};

#undef ARC_RELOC_HOWTO

typedef ATTRIBUTE_UNUSED unsigned (*replace_func) (unsigned, int ATTRIBUTE_UNUSED);

#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  case TYPE: \
    func = RELOC_FUNCTION; \
    break;

static replace_func
get_replace_function (bfd *abfd, unsigned int r_type)
{
  replace_func func = NULL;

  switch (r_type)
    {
      #include "elf/arc-reloc.def"
    }

  if (func == replace_bits24 && bfd_big_endian (abfd))
    func = replace_bits24_be;

  return func;
}
#undef ARC_RELOC_HOWTO

static reloc_howto_type *
arc_elf32_bfd_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = ARRAY_SIZE (arc_reloc_map); i--;)
    {
      if (arc_reloc_map[i].bfd_reloc_val == code)
	return arc_elf_howto (arc_reloc_map[i].elf_reloc_val);
    }

  return NULL;
}

/* Function to set the ELF flag bits.  */
static bool
arc_elf_set_private_flags (bfd *abfd, flagword flags)
{
  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = true;
  return true;
}

/* Print private flags.  */
static bool
arc_elf_print_private_bfd_data (bfd *abfd, void * ptr)
{
  FILE *file = (FILE *) ptr;
  flagword flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  fprintf (file, _("private flags = 0x%lx:"), (unsigned long) flags);

  switch (flags & EF_ARC_MACH_MSK)
    {
    case EF_ARC_CPU_ARCV2HS : fprintf (file, " -mcpu=ARCv2HS");    break;
    case EF_ARC_CPU_ARCV2EM : fprintf (file, " -mcpu=ARCv2EM");    break;
    case E_ARC_MACH_ARC600  : fprintf (file, " -mcpu=ARC600");     break;
    case E_ARC_MACH_ARC601  : fprintf (file, " -mcpu=ARC601");     break;
    case E_ARC_MACH_ARC700  : fprintf (file, " -mcpu=ARC700");     break;
    default:
      fprintf (file, "-mcpu=unknown");
      break;
    }

  switch (flags & EF_ARC_OSABI_MSK)
    {
    case E_ARC_OSABI_ORIG : fprintf (file, " (ABI:legacy)"); break;
    case E_ARC_OSABI_V2   : fprintf (file, " (ABI:v2)");     break;
    case E_ARC_OSABI_V3   : fprintf (file, " (ABI:v3)");     break;
    case E_ARC_OSABI_V4   : fprintf (file, " (ABI:v4)");     break;
    default:
      fprintf (file, " (ABI:unknown)");
      break;
    }

  fputc ('\n', file);
  return true;
}

/* Copy backend specific data from one object module to another.  */

static bool
arc_elf_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  BFD_ASSERT (!elf_flags_init (obfd)
	      || elf_elfheader (obfd)->e_flags == elf_elfheader (ibfd)->e_flags);

  elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
  elf_flags_init (obfd) = true;

  /* Copy object attributes.  */
  _bfd_elf_copy_obj_attributes (ibfd, obfd);

  return _bfd_elf_copy_private_bfd_data (ibfd, obfd);
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd * abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (elf_arc_howto_table); i++)
    if (elf_arc_howto_table[i].name != NULL
	&& strcasecmp (elf_arc_howto_table[i].name, r_name) == 0)
      return arc_elf_howto (i);

  return NULL;
}

/* Set the howto pointer for an ARC ELF reloc.  */

static bool
arc_info_to_howto_rel (bfd * abfd,
		       arelent * cache_ptr,
		       Elf_Internal_Rela * dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  if (r_type >= (unsigned int) R_ARC_max)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: unsupported relocation type %#x"),
			  abfd, r_type);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  cache_ptr->howto = arc_elf_howto (r_type);
  return true;
}

/* Extract CPU features from an NTBS.  */

static unsigned
arc_extract_features (const char *p)
{
  unsigned i, r = 0;

  if (!p)
    return 0;

  for (i = 0; i < ARRAY_SIZE (bfd_feature_list); i++)
    {
      char *t = strstr (p, bfd_feature_list[i].attr);
      unsigned l = strlen (bfd_feature_list[i].attr);
      if ((t != NULL)
	  && (t[l] == ','
	      || t[l] == '\0'))
	r |= bfd_feature_list[i].feature;
    }

  return r;
}

/* Concatenate two strings.  s1 can be NULL but not
   s2.  */

static char *
arc_stralloc (char * s1, const char * s2)
{
  char *p;

  /* Only s1 can be null.  */
  BFD_ASSERT (s2);

  p = s1 ? concat (s1, ",", s2, NULL) : (char *)s2;

  return p;
}

/* Merge ARC object attributes from IBFD into OBFD.  Raise an error if
   there are conflicting attributes.  */

static bool
arc_elf_merge_attributes (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  obj_attribute *in_attr;
  obj_attribute *out_attr;
  int i;
  bool result = true;
  const char *sec_name = get_elf_backend_data (ibfd)->obj_attrs_section;
  char *tagname = NULL;

  /* Skip the linker stubs file.  This preserves previous behavior
     of accepting unknown attributes in the first input file - but
     is that a bug?  */
  if (ibfd->flags & BFD_LINKER_CREATED)
    return true;

  /* Skip any input that hasn't attribute section.
     This enables to link object files without attribute section with
     any others.  */
  if (bfd_get_section_by_name (ibfd, sec_name) == NULL)
    return true;

  if (!elf_known_obj_attributes_proc (obfd)[0].i)
    {
      /* This is the first object.  Copy the attributes.  */
      _bfd_elf_copy_obj_attributes (ibfd, obfd);

      out_attr = elf_known_obj_attributes_proc (obfd);

      /* Use the Tag_null value to indicate the attributes have been
	 initialized.  */
      out_attr[0].i = 1;

      return true;
    }

  in_attr = elf_known_obj_attributes_proc (ibfd);
  out_attr = elf_known_obj_attributes_proc (obfd);

  for (i = LEAST_KNOWN_OBJ_ATTRIBUTE; i < NUM_KNOWN_OBJ_ATTRIBUTES; i++)
    {
      /* Merge this attribute with existing attributes.  */
      switch (i)
	{
	case Tag_ARC_PCS_config:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (in_attr[i].i != 0 && out_attr[i].i != in_attr[i].i)
	    {
	      const char *tagval[] = { "Absent", "Bare-metal/mwdt",
					"Bare-metal/newlib", "Linux/uclibc",
					"Linux/glibc" };
	      BFD_ASSERT (in_attr[i].i < 5);
	      BFD_ASSERT (out_attr[i].i < 5);
	      /* It's sometimes ok to mix different configs, so this is only
		 a warning.  */
	      _bfd_error_handler
		(_("warning: %pB: conflicting platform configuration "
		   "%s with %s"), ibfd,
		 tagval[in_attr[i].i],
		 tagval[out_attr[i].i]);
	    }
	  break;

	case Tag_ARC_CPU_base:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (in_attr[i].i != 0 && out_attr[i].i != in_attr[i].i
		   && ((out_attr[i].i + in_attr[i].i) < 6))
	    {
	      const char *tagval[] = { "Absent", "ARC6xx", "ARC7xx",
					"ARCEM", "ARCHS" };
	      BFD_ASSERT (in_attr[i].i < 5);
	      BFD_ASSERT (out_attr[i].i < 5);
	      /* We cannot mix code for different CPUs.  */
	      _bfd_error_handler
		(_("error: %pB: unable to merge CPU base attributes "
		   "%s with %s"),
		 obfd,
		 tagval[in_attr[i].i],
		 tagval[out_attr[i].i]);
	      result = false;
	      break;
	    }
	  else
	    {
	      /* The CPUs may be different, check if we can still mix
		 the objects against the output choosen CPU.  */
	      unsigned in_feature = 0;
	      unsigned out_feature = 0;
	      char *p1 = in_attr[Tag_ARC_ISA_config].s;
	      char *p2 = out_attr[Tag_ARC_ISA_config].s;
	      unsigned j;
	      unsigned cpu_out;
	      unsigned opcode_map[] = {0, ARC_OPCODE_ARC600, ARC_OPCODE_ARC700,
				       ARC_OPCODE_ARCv2EM, ARC_OPCODE_ARCv2HS};

	      BFD_ASSERT (in_attr[i].i < (sizeof (opcode_map)
					  / sizeof (unsigned)));
	      BFD_ASSERT (out_attr[i].i < (sizeof (opcode_map)
					   / sizeof (unsigned)));
	      cpu_out = opcode_map[out_attr[i].i];

	      in_feature = arc_extract_features (p1);
	      out_feature = arc_extract_features (p2);

	      /* First, check if a feature is compatible with the
		 output object chosen CPU.  */
	      for (j = 0; j < ARRAY_SIZE (bfd_feature_list); j++)
		if (((in_feature | out_feature) & bfd_feature_list[j].feature)
		    && (!(cpu_out & bfd_feature_list[j].cpus)))
		  {
		    _bfd_error_handler
		      (_("error: %pB: unable to merge ISA extension attributes "
			 "%s"),
		       obfd, bfd_feature_list[j].name);
		    result = false;
		    break;
		  }
	      /* Second, if we have compatible features with the
		 chosen CPU, check if they are compatible among
		 them.  */
	      for (j = 0; j < ARRAY_SIZE (bfd_conflict_list); j++)
		if (((in_feature | out_feature) & bfd_conflict_list[j])
		    == bfd_conflict_list[j])
		  {
		    unsigned k;
		    for (k = 0; k < ARRAY_SIZE (bfd_feature_list); k++)
		      {
			if (in_feature &  bfd_feature_list[k].feature
			    & bfd_conflict_list[j])
			  p1 = (char *) bfd_feature_list[k].name;
			if (out_feature &  bfd_feature_list[k].feature
			    & bfd_conflict_list[j])
			  p2 = (char *) bfd_feature_list[k].name;
		      }
		    _bfd_error_handler
		      (_("error: %pB: conflicting ISA extension attributes "
			 "%s with %s"),
		       obfd, p1, p2);
		    result = false;
		    break;
		  }
	      /* Everithing is alright.  */
	      out_feature |= in_feature;
	      p1 = NULL;
	      for (j = 0; j < ARRAY_SIZE (bfd_feature_list); j++)
		if (out_feature & bfd_feature_list[j].feature)
		  p1 = arc_stralloc (p1, bfd_feature_list[j].attr);
	      if (p1)
		out_attr[Tag_ARC_ISA_config].s =
		  _bfd_elf_attr_strdup (obfd, p1);
	    }
	  /* Fall through.  */
	case Tag_ARC_CPU_variation:
	case Tag_ARC_ISA_mpy_option:
	case Tag_ARC_ABI_osver:
	  /* Use the largest value specified.  */
	  if (in_attr[i].i > out_attr[i].i)
	    out_attr[i].i = in_attr[i].i;
	  break;

	  /* The CPU name is given by the vendor, just choose an
	     existing one if missing or different.  There are no fail
	     criteria if they different or both missing.  */
	case Tag_ARC_CPU_name:
	  if (!out_attr[i].s && in_attr[i].s)
	    out_attr[i].s = _bfd_elf_attr_strdup (obfd, in_attr[i].s);
	  break;

	case Tag_ARC_ABI_rf16:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (out_attr[i].i != in_attr[i].i)
	    {
	      /* We cannot mix code with rf16 and without.  */
	      _bfd_error_handler
		(_("error: %pB: cannot mix rf16 with full register set %pB"),
		 obfd, ibfd);
	      result = false;
	    }
	  break;

	case Tag_ARC_ABI_pic:
	  tagname = "PIC";
	  /* fall through */
	case Tag_ARC_ABI_sda:
	  if (!tagname)
	    tagname = "SDA";
	  /* fall through */
	case Tag_ARC_ABI_tls:
	  {
	    const char *tagval[] = { "Absent", "MWDT", "GNU" };

	    if (!tagname)
	      tagname = "TLS";

	    BFD_ASSERT (in_attr[i].i < 3);
	    BFD_ASSERT (out_attr[i].i < 3);
	    if (out_attr[i].i == 0)
	      out_attr[i].i = in_attr[i].i;
	    else if (out_attr[i].i != 0 && in_attr[i].i != 0
		&& out_attr[i].i != in_attr[i].i)
	      {
		_bfd_error_handler
		  (_("error: %pB: conflicting attributes %s: %s with %s"),
		   obfd, tagname,
		   tagval[in_attr[i].i],
		   tagval[out_attr[i].i]);
		result = false;
	      }
	    tagname = NULL;
	    break;
	  }

	case Tag_ARC_ABI_double_size:
	  tagname = "Double size";
	  /* fall through */
	case Tag_ARC_ABI_enumsize:
	  if (!tagname)
	    tagname = "Enum size";
	  /* fall through */
	case Tag_ARC_ABI_exceptions:
	  if (!tagname)
	    tagname = "ABI exceptions";

	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  else if (out_attr[i].i != 0 && in_attr[i].i != 0
	      && out_attr[i].i != in_attr[i].i)
	    {
	      _bfd_error_handler
		(_("error: %pB: conflicting attributes %s"),
		 obfd, tagname);
	      result = false;
	    }
	  break;

	case Tag_ARC_ISA_apex:
	  break; /* Do nothing for APEX attributes.  */

	case Tag_ARC_ISA_config:
	  /* It is handled in Tag_ARC_CPU_base.  */
	  break;

	case Tag_ARC_ATR_version:
	  if (out_attr[i].i == 0)
	    out_attr[i].i = in_attr[i].i;
	  break;

	default:
	  result
	    = result && _bfd_elf_merge_unknown_attribute_low (ibfd, obfd, i);
	}

      /* If out_attr was copied from in_attr then it won't have a type yet.  */
      if (in_attr[i].type && !out_attr[i].type)
	out_attr[i].type = in_attr[i].type;
    }

  /* Merge Tag_compatibility attributes and any common GNU ones.  */
  if (!_bfd_elf_merge_object_attributes (ibfd, info))
    return false;

  /* Check for any attributes not known on ARC.  */
  result &= _bfd_elf_merge_unknown_attribute_list (ibfd, obfd);

  return result;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bool
arc_elf_merge_private_bfd_data (bfd *ibfd, struct bfd_link_info *info)
{
  bfd *obfd = info->output_bfd;
  unsigned short mach_ibfd;
  static unsigned short mach_obfd = EM_NONE;
  flagword out_flags;
  flagword in_flags;
  asection *sec;

   /* Check if we have the same endianess.  */
  if (! _bfd_generic_verify_endian_match (ibfd, info))
    return false;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return true;

  /* Collect ELF flags.  */
  in_flags = elf_elfheader (ibfd)->e_flags & EF_ARC_MACH_MSK;
  out_flags = elf_elfheader (obfd)->e_flags & EF_ARC_MACH_MSK;

  if (!elf_flags_init (obfd)) /* First call, no flags set.  */
    {
      elf_flags_init (obfd) = true;
      out_flags = in_flags;
    }

  if (!arc_elf_merge_attributes (ibfd, info))
    return false;

  /* Check to see if the input BFD actually contains any sections.  Do
     not short-circuit dynamic objects; their section list may be
     emptied by elf_link_add_object_symbols.  */
  if (!(ibfd->flags & DYNAMIC))
    {
      bool null_input_bfd = true;
      bool only_data_sections = true;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  if ((bfd_section_flags (sec)
	       & (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	      == (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	    only_data_sections = false;

	  null_input_bfd = false;
	}

      if (null_input_bfd || only_data_sections)
	return true;
    }

  /* Complain about various flag/architecture mismatches.  */
  mach_ibfd = elf_elfheader (ibfd)->e_machine;
  if (mach_obfd == EM_NONE)
    {
      mach_obfd = mach_ibfd;
    }
  else
    {
      if (mach_ibfd != mach_obfd)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("error: attempting to link %pB "
				"with a binary %pB of different architecture"),
			      ibfd, obfd);
	  return false;
	}
      else if ((in_flags != out_flags)
	       /* If we have object attributes, then we already
		  checked the objects compatibility, skip it.  */
	       && !bfd_elf_get_obj_attr_int (ibfd, OBJ_ATTR_PROC,
					     Tag_ARC_CPU_base))
	{
	  if (in_flags && out_flags)
	    {
	      /* Warn if different flags.  */
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: uses different e_flags (%#x) fields than "
		   "previous modules (%#x)"),
		 ibfd, in_flags, out_flags);
	      return false;
	    }
	  /* MWDT doesnt set the eflags hence make sure we choose the
	     eflags set by gcc.  */
	  in_flags = in_flags > out_flags ? in_flags : out_flags;
	}
      else
	{
	  /* Everything is correct; don't change the output flags.  */
	  in_flags = out_flags;
	}
    }

  /* Update the flags.  */
  elf_elfheader (obfd)->e_flags = in_flags;

  if (bfd_get_mach (obfd) < bfd_get_mach (ibfd))
    {
      return bfd_set_arch_mach (obfd, bfd_arch_arc, bfd_get_mach (ibfd));
    }

  return true;
}

/* Return a best guess for the machine number based on the attributes.  */

static unsigned int
bfd_arc_get_mach_from_attributes (bfd * abfd)
{
  int arch = bfd_elf_get_obj_attr_int (abfd, OBJ_ATTR_PROC, Tag_ARC_CPU_base);
  unsigned e_machine = elf_elfheader (abfd)->e_machine;

  switch (arch)
    {
    case TAG_CPU_ARC6xx:
      return bfd_mach_arc_arc600;
    case TAG_CPU_ARC7xx:
      return bfd_mach_arc_arc700;
    case TAG_CPU_ARCEM:
    case TAG_CPU_ARCHS:
      return bfd_mach_arc_arcv2;
    default:
      break;
    }
  return (e_machine == EM_ARC_COMPACT)
    ? bfd_mach_arc_arc700 : bfd_mach_arc_arcv2;
}

/* Set the right machine number for an ARC ELF file.  */
static bool
arc_elf_object_p (bfd * abfd)
{
  /* Make sure this is initialised, or you'll have the potential of passing
     garbage---or misleading values---into the call to
     bfd_default_set_arch_mach ().  */
  unsigned int	  mach = bfd_mach_arc_arc700;
  unsigned long   arch = elf_elfheader (abfd)->e_flags & EF_ARC_MACH_MSK;
  unsigned	  e_machine = elf_elfheader (abfd)->e_machine;

  if (e_machine == EM_ARC_COMPACT || e_machine == EM_ARC_COMPACT2)
    {
      switch (arch)
	{
	case E_ARC_MACH_ARC600:
	  mach = bfd_mach_arc_arc600;
	  break;
	case E_ARC_MACH_ARC601:
	  mach = bfd_mach_arc_arc601;
	  break;
	case E_ARC_MACH_ARC700:
	  mach = bfd_mach_arc_arc700;
	  break;
	case EF_ARC_CPU_ARCV2HS:
	case EF_ARC_CPU_ARCV2EM:
	  mach = bfd_mach_arc_arcv2;
	  break;
	default:
	  mach = bfd_arc_get_mach_from_attributes (abfd);
	  break;
	}
    }
  else
    {
      if (e_machine == EM_ARC)
	{
	  _bfd_error_handler
	    (_("error: the ARC4 architecture is no longer supported"));
	  return false;
	}
      else
	{
	  _bfd_error_handler
	    (_("warning: unset or old architecture flags; "
	       "use default machine"));
	}
    }

  return bfd_default_set_arch_mach (abfd, bfd_arch_arc, mach);
}

/* The final processing done just before writing out an ARC ELF object file.
   This gets the ARC architecture right based on the machine number.  */

static bool
arc_elf_final_write_processing (bfd *abfd)
{
  unsigned long emf;
  int osver = bfd_elf_get_obj_attr_int (abfd, OBJ_ATTR_PROC,
					Tag_ARC_ABI_osver);
  flagword e_flags = elf_elfheader (abfd)->e_flags & ~EF_ARC_OSABI_MSK;

  switch (bfd_get_mach (abfd))
    {
    case bfd_mach_arc_arcv2:
      emf = EM_ARC_COMPACT2;
      break;
    default:
      emf = EM_ARC_COMPACT;
      break;
    }

  elf_elfheader (abfd)->e_machine = emf;

  /* Record whatever is the current syscall ABI version.  */
  if (osver)
    e_flags |= ((osver & 0x0f) << 8);
  else
    e_flags |= E_ARC_OSABI_V3;

  elf_elfheader (abfd)->e_flags |= e_flags;
  return _bfd_elf_final_write_processing (abfd);
}

#ifdef ARC_ENABLE_DEBUG
#define DEBUG_ARC_RELOC(A) debug_arc_reloc (A)

static void
debug_arc_reloc (struct arc_relocation_data reloc_data)
{
  ARC_DEBUG ("Reloc type=%s, should_relocate = %s\n",
	     reloc_data.howto->name,
	     reloc_data.should_relocate ? "true" : "false");
  ARC_DEBUG ("  offset = 0x%x, addend = 0x%x\n",
	     (unsigned int) reloc_data.reloc_offset,
	     (unsigned int) reloc_data.reloc_addend);
  ARC_DEBUG (" Symbol:\n");
  ARC_DEBUG ("  value = 0x%08x\n",
	     (unsigned int) reloc_data.sym_value);
  if (reloc_data.sym_section != NULL)
    {
      ARC_DEBUG (" Symbol Section:\n");
      ARC_DEBUG ("  section name = %s, output_offset 0x%08x",
		 reloc_data.sym_section->name,
		 (unsigned int) reloc_data.sym_section->output_offset);
      if (reloc_data.sym_section->output_section != NULL)
	ARC_DEBUG (", output_section->vma = 0x%08x",
		   ((unsigned int) reloc_data.sym_section->output_section->vma));
      ARC_DEBUG ("\n");
      if (reloc_data.sym_section->owner
	  && reloc_data.sym_section->owner->filename)
	ARC_DEBUG ("  file: %s\n", reloc_data.sym_section->owner->filename);
    }
  else
    {
      ARC_DEBUG ("  symbol section is NULL\n");
    }

  ARC_DEBUG (" Input_section:\n");
  if (reloc_data.input_section != NULL)
    {
      ARC_DEBUG ("  section name = %s, output_offset 0x%08x, output_section->vma = 0x%08x\n",
		 reloc_data.input_section->name,
		 (unsigned int) reloc_data.input_section->output_offset,
		 (unsigned int) reloc_data.input_section->output_section->vma);
      ARC_DEBUG ("  changed_address = 0x%08x\n",
		 (unsigned int) (reloc_data.input_section->output_section->vma
				 + reloc_data.input_section->output_offset
				 + reloc_data.reloc_offset));
      ARC_DEBUG ("  file: %s\n", reloc_data.input_section->owner->filename);
    }
  else
    {
      ARC_DEBUG ("	input section is NULL\n");
    }
}
#else
#define DEBUG_ARC_RELOC(A)
#endif /* ARC_ENABLE_DEBUG */

static bfd_vma
middle_endian_convert (bfd_vma insn, bool do_it)
{
  if (do_it)
    {
      insn
	= ((insn & 0xffff0000) >> 16)
	  | ((insn & 0xffff) << 16);
    }
  return insn;
}

/* This function is called for relocations that are otherwise marked as NOT
   requiring overflow checks.  In here we perform non-standard checks of
   the relocation value.  */

static inline bfd_reloc_status_type
arc_special_overflow_checks (const struct arc_relocation_data reloc_data,
			     bfd_signed_vma relocation,
			     struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  switch (reloc_data.howto->type)
    {
    case R_ARC_NPS_CMEM16:
      if (((relocation >> 16) & 0xffff) != NPS_CMEM_HIGH_VALUE)
	{
	  if (reloc_data.reloc_addend == 0)
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): CMEM relocation to `%s' is invalid, "
		 "16 MSB should be %#x (value is %#" PRIx64 ")"),
	       reloc_data.input_section->owner,
	       reloc_data.input_section,
	       (uint64_t) reloc_data.reloc_offset,
	       reloc_data.symbol_name,
	       NPS_CMEM_HIGH_VALUE,
	       (uint64_t) relocation);
	  else
	    _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB(%pA+%#" PRIx64 "): CMEM relocation to `%s+%#" PRIx64
		 "' is invalid, 16 MSB should be %#x (value is %#" PRIx64 ")"),
	       reloc_data.input_section->owner,
	       reloc_data.input_section,
	       (uint64_t) reloc_data.reloc_offset,
	       reloc_data.symbol_name,
	       (uint64_t) reloc_data.reloc_addend,
	       NPS_CMEM_HIGH_VALUE,
	       (uint64_t) relocation);
	  return bfd_reloc_overflow;
	}
      break;

    default:
      break;
    }

  return bfd_reloc_ok;
}

#define ME(reloc) (reloc)

#define IS_ME(FORMULA,BFD) ((strstr (FORMULA, "ME") != NULL) \
			    && (!bfd_big_endian (BFD)))

#define S ((bfd_signed_vma) (reloc_data.sym_value			\
	   + (reloc_data.sym_section->output_section != NULL ?		\
	      (reloc_data.sym_section->output_offset			\
	       + reloc_data.sym_section->output_section->vma) : 0)))
#define L ((bfd_signed_vma) (reloc_data.sym_value			\
	   + (reloc_data.sym_section->output_section != NULL ?		\
	      (reloc_data.sym_section->output_offset			\
	      + reloc_data.sym_section->output_section->vma) : 0)))
#define A (reloc_data.reloc_addend)
#define B (0)
#define G (reloc_data.got_offset_value)
#define GOT (reloc_data.got_symbol_vma)
#define GOT_BEGIN (htab->sgot->output_section->vma)

#define MES (0)
	/* P: relative offset to PCL The offset should be to the
	  current location aligned to 32 bits.  */
#define P ((bfd_signed_vma) (						\
	   (								\
	    (reloc_data.input_section->output_section != NULL ?		\
	     reloc_data.input_section->output_section->vma : 0)		\
	    + reloc_data.input_section->output_offset			\
	    + (reloc_data.reloc_offset - (bitsize >= 32 ? 4 : 0)))	\
	   & ~0x3))
#define PDATA ((bfd_signed_vma) ( \
	    (reloc_data.input_section->output_section->vma \
	     + reloc_data.input_section->output_offset \
	     + (reloc_data.reloc_offset))))
#define SECTSTART (bfd_signed_vma) (reloc_data.sym_section->output_section->vma \
				    + reloc_data.sym_section->output_offset)
#define FINAL_SECTSTART \
  (bfd_signed_vma) (reloc_data.sym_section->output_section->vma)
#define JLI (bfd_signed_vma) (reloc_data.sym_section->output_section->vma)
#define _SDA_BASE_ (bfd_signed_vma) (reloc_data.sdata_begin_symbol_vma)
#define TLS_REL (bfd_signed_vma) \
  ((elf_hash_table (info))->tls_sec->output_section->vma)
#define TLS_TBSS (align_power(TCB_SIZE, \
		  reloc_data.sym_section->alignment_power))

#define none (0)

#ifdef ARC_ENABLE_DEBUG
#define PRINT_DEBUG_RELOC_INFO_BEFORE(FORMULA, TYPE)			\
  do									\
    {									\
      asection *sym_section = reloc_data.sym_section;			\
      asection *input_section = reloc_data.input_section;		\
      ARC_DEBUG ("RELOC_TYPE = " TYPE "\n");				\
      ARC_DEBUG ("FORMULA = " FORMULA "\n");				\
      ARC_DEBUG ("S = %#lx\n", S);					\
      ARC_DEBUG ("A = %#lx\n", A);					\
      ARC_DEBUG ("L = %lx\n", L);					\
      if (sym_section->output_section != NULL)				\
	ARC_DEBUG ("symbol_section->vma = %#lx\n",			\
		   sym_section->output_section->vma			\
		   + sym_section->output_offset);			\
      else								\
	ARC_DEBUG ("symbol_section->vma = NULL\n");			\
      if (input_section->output_section != NULL)			\
	ARC_DEBUG ("input_section->vma = %#lx\n",			\
		   input_section->output_section->vma			\
		   + input_section->output_offset);			\
      else								\
	ARC_DEBUG ("input_section->vma = NULL\n");			\
      ARC_DEBUG ("PCL = %#lx\n", P);					\
      ARC_DEBUG ("P = %#lx\n", P);					\
      ARC_DEBUG ("G = %#lx\n", G);					\
      ARC_DEBUG ("SDA_OFFSET = %#lx\n", _SDA_BASE_);			\
      ARC_DEBUG ("SDA_SET = %d\n", reloc_data.sdata_begin_symbol_vma_set); \
      ARC_DEBUG ("GOT_OFFSET = %#lx\n", GOT);				\
      ARC_DEBUG ("relocation = %#08lx\n", relocation);			\
      ARC_DEBUG ("before = %#08x\n", (unsigned) insn);			\
      ARC_DEBUG ("data   = %08x (%u) (%d)\n", (unsigned) relocation,	\
		 (unsigned) relocation, (int) relocation);		\
    }									\
  while (0)

#define PRINT_DEBUG_RELOC_INFO_AFTER				\
  do								\
    {								\
      ARC_DEBUG ("after  = 0x%08x\n", (unsigned int) insn);	\
    }								\
  while (0)

#else

#define PRINT_DEBUG_RELOC_INFO_BEFORE(...)
#define PRINT_DEBUG_RELOC_INFO_AFTER

#endif /* ARC_ENABLE_DEBUG */

#define ARC_RELOC_HOWTO(TYPE, VALUE, SIZE, BITSIZE, RELOC_FUNCTION, OVERFLOW, FORMULA) \
  case R_##TYPE:							\
    {									\
      bfd_signed_vma bitsize ATTRIBUTE_UNUSED = BITSIZE;		\
      relocation = FORMULA  ;						\
      PRINT_DEBUG_RELOC_INFO_BEFORE (#FORMULA, #TYPE);			\
      insn = middle_endian_convert (insn, IS_ME (#FORMULA, abfd));	\
      insn = (* get_replace_function (abfd, TYPE)) (insn, relocation);	\
      insn = middle_endian_convert (insn, IS_ME (#FORMULA, abfd));	\
      PRINT_DEBUG_RELOC_INFO_AFTER;					\
    }									\
    break;

static bfd_reloc_status_type
arc_do_relocation (bfd_byte * contents,
		   struct arc_relocation_data reloc_data,
		   struct bfd_link_info *info)
{
  bfd_signed_vma relocation = 0;
  bfd_vma insn;
  bfd_vma orig_insn ATTRIBUTE_UNUSED;
  bfd * abfd = reloc_data.input_section->owner;
  struct elf_link_hash_table *htab ATTRIBUTE_UNUSED = elf_hash_table (info);
  bfd_reloc_status_type flag;

  if (!reloc_data.should_relocate)
    return bfd_reloc_ok;

  switch (bfd_get_reloc_size (reloc_data.howto))
    {
    case 4:
      insn = arc_bfd_get_32 (abfd,
			     contents + reloc_data.reloc_offset,
			     reloc_data.input_section);
      break;
    case 2:
      insn = arc_bfd_get_16 (abfd,
			     contents + reloc_data.reloc_offset,
			     reloc_data.input_section);
      break;
    case 1:
      insn = arc_bfd_get_8 (abfd,
			    contents + reloc_data.reloc_offset,
			    reloc_data.input_section);
      break;
    default:
      insn = 0;
      BFD_ASSERT (0);
      break;
    }

  orig_insn = insn;

  switch (reloc_data.howto->type)
    {
#include "elf/arc-reloc.def"

    default:
      BFD_ASSERT (0);
      break;
    }

  /* Check for relocation overflow.  */
  if (reloc_data.howto->complain_on_overflow != complain_overflow_dont)
    flag = bfd_check_overflow (reloc_data.howto->complain_on_overflow,
			       reloc_data.howto->bitsize,
			       reloc_data.howto->rightshift,
			       bfd_arch_bits_per_address (abfd),
			       relocation);
  else
    flag = arc_special_overflow_checks (reloc_data, relocation, info);

  if (flag != bfd_reloc_ok)
    {
      ARC_DEBUG ("Relocation overflows !\n");
      DEBUG_ARC_RELOC (reloc_data);
      ARC_DEBUG ("Relocation value = signed -> %d, unsigned -> %u"
		 ", hex -> (0x%08x)\n",
		(int) relocation, (unsigned) relocation, (int) relocation);

      return flag;
    }

  /* Write updated instruction back to memory.  */
  switch (bfd_get_reloc_size (reloc_data.howto))
    {
    case 4:
      arc_bfd_put_32 (abfd, insn,
		      contents + reloc_data.reloc_offset,
		      reloc_data.input_section);
      break;
    case 2:
	arc_bfd_put_16 (abfd, insn,
			contents + reloc_data.reloc_offset,
			reloc_data.input_section);
	break;
    case 1:
      arc_bfd_put_8 (abfd, insn,
		     contents + reloc_data.reloc_offset,
		     reloc_data.input_section);
      break;
    default:
      ARC_DEBUG ("size = %d\n", reloc_data.howto->size);
      BFD_ASSERT (0);
      break;
    }

  return bfd_reloc_ok;
}
#undef S
#undef A
#undef B
#undef G
#undef GOT
#undef L
#undef MES
#undef P
#undef SECTSTAR
#undef SECTSTART
#undef JLI
#undef _SDA_BASE_
#undef none

#undef ARC_RELOC_HOWTO


/* Relocate an arc ELF section.
   Function : elf_arc_relocate_section
   Brief    : Relocate an arc section, by handling all the relocations
	     appearing in that section.
   Args     : output_bfd    : The bfd being written to.
	      info	    : Link information.
	      input_bfd     : The input bfd.
	      input_section : The section being relocated.
	      contents	    : contents of the section being relocated.
	      relocs	    : List of relocations in the section.
	      local_syms    : is a pointer to the swapped in local symbols.
	      local_section : is an array giving the section in the input file
			      corresponding to the st_shndx field of each
			      local symbol.  */
static int
elf_arc_relocate_section (bfd *			  output_bfd,
			  struct bfd_link_info *  info,
			  bfd *			  input_bfd,
			  asection *		  input_section,
			  bfd_byte *		  contents,
			  Elf_Internal_Rela *     relocs,
			  Elf_Internal_Sym *      local_syms,
			  asection **		  local_sections)
{
  Elf_Internal_Shdr *		 symtab_hdr;
  struct elf_link_hash_entry **  sym_hashes;
  Elf_Internal_Rela *		 rel;
  Elf_Internal_Rela *		 wrel;
  Elf_Internal_Rela *		 relend;
  struct elf_link_hash_table *   htab = elf_hash_table (info);

  symtab_hdr = &((elf_tdata (input_bfd))->symtab_hdr);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = wrel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; wrel++, rel++)
    {
      enum elf_arc_reloc_type r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h2;
      const char *msg;
      bool unresolved_reloc = false;

      struct arc_relocation_data reloc_data =
      {
	.reloc_offset = 0,
	.reloc_addend = 0,
	.got_offset_value = 0,
	.sym_value = 0,
	.sym_section = NULL,
	.howto = NULL,
	.input_section = NULL,
	.sdata_begin_symbol_vma = 0,
	.sdata_begin_symbol_vma_set = false,
	.got_symbol_vma = 0,
	.should_relocate = false
      };

      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type >= (int) R_ARC_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      howto = arc_elf_howto (r_type);

      r_symndx = ELF32_R_SYM (rel->r_info);

      /* If we are generating another .o file and the symbol in not
	 local, skip this relocation.  */
      if (bfd_link_relocatable (info))
	{
	  /* This is a relocateable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */

	  /* Checks if this is a local symbol and thus the reloc
	     might (will??) be against a section symbol.  */
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;
	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  sec = local_sections[r_symndx];

		  /* For RELA relocs.  Just adjust the addend
		     value in the relocation entry.  */
		  rel->r_addend += sec->output_offset + sym->st_value;

		  ARC_DEBUG ("local symbols reloc (section=%d %s) seen in %s\n",
			     (int) r_symndx, local_sections[r_symndx]->name,
			     __PRETTY_FUNCTION__);
		}
	    }
	}

      h2 = elf_link_hash_lookup (elf_hash_table (info), "__SDATA_BEGIN__",
				 false, false, true);

      if (!reloc_data.sdata_begin_symbol_vma_set
	  && h2 != NULL && h2->root.type != bfd_link_hash_undefined
	  && h2->root.u.def.section->output_section != NULL)
	/* TODO: Verify this condition.  */
	{
	  reloc_data.sdata_begin_symbol_vma =
	    (h2->root.u.def.value
	     + h2->root.u.def.section->output_section->vma);
	  reloc_data.sdata_begin_symbol_vma_set = true;
	}

      reloc_data.input_section = input_section;
      reloc_data.howto = howto;
      reloc_data.reloc_offset = rel->r_offset;
      reloc_data.reloc_addend = rel->r_addend;

      /* This is a final link.  */
      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info) /* A local symbol.  */
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	}
      else
	{
	  bool warned, ignored;
	  bfd_vma relocation ATTRIBUTE_UNUSED;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  /* TODO: This code is repeated from below.  We should
	     clean it and remove duplications.
	     Sec is used check for discarded sections.
	     Need to redesign code below.  */

	  /* Get the symbol's entry in the symtab.  */
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* If we have encountered a definition for this symbol.  */
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      reloc_data.sym_value = h->root.u.def.value;
	      sec = h->root.u.def.section;
	    }
	}

      /* Clean relocs for symbols in discarded sections.  */
      if (sec != NULL && discarded_section (sec))
	{
	  _bfd_clear_contents (howto, input_bfd, input_section,
			       contents, rel->r_offset);
	  rel->r_info = 0;
	  rel->r_addend = 0;

	  /* For ld -r, remove relocations in debug sections against
	     sections defined in discarded sections.  Not done for
	     eh_frame editing code expects to be present.  */
	   if (bfd_link_relocatable (info)
	       && (input_section->flags & SEC_DEBUGGING))
	     wrel--;

	  continue;
	}

      if (bfd_link_relocatable (info))
	{
	  if (wrel != rel)
	    *wrel = *rel;
	  continue;
	}

      if (r_symndx < symtab_hdr->sh_info) /* A local symbol.  */
	{
	  reloc_data.sym_value = sym->st_value;
	  reloc_data.sym_section = sec;
	  reloc_data.symbol_name =
	    bfd_elf_string_from_elf_section (input_bfd,
					     symtab_hdr->sh_link,
					     sym->st_name);

	  /* Mergeable section handling.  */
	  if ((sec->flags & SEC_MERGE)
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION)
	    {
	      asection *msec;
	      msec = sec;
	      rel->r_addend = _bfd_elf_rel_local_sym (output_bfd, sym,
						      &msec, rel->r_addend);
	      rel->r_addend -= (sec->output_section->vma
				+ sec->output_offset
				+ sym->st_value);
	      rel->r_addend += msec->output_section->vma + msec->output_offset;

	      reloc_data.reloc_addend = rel->r_addend;
	    }

	  BFD_ASSERT (htab->sgot != NULL || !is_reloc_for_GOT (howto));
	  if (htab->sgot != NULL)
	    reloc_data.got_symbol_vma = htab->sgot->output_section->vma
					+ htab->sgot->output_offset;

	  reloc_data.should_relocate = true;
	}
      else /* Global symbol.  */
	{
	  /* FIXME: We should use the RELOC_FOR_GLOBAL_SYMBOL macro
	     (defined in elf-bfd.h) here.  */

	  /* Get the symbol's entry in the symtab.  */
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	  {
	    struct elf_arc_link_hash_entry *ah_old =
	      (struct elf_arc_link_hash_entry *) h;
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	    struct elf_arc_link_hash_entry *ah =
	      (struct elf_arc_link_hash_entry *) h;

	    if (ah->got_ents == 0 && ah_old->got_ents != ah->got_ents)
	      ah->got_ents = ah_old->got_ents;
	  }

	  /* TODO: Need to validate what was the intention.  */
	  /* BFD_ASSERT ((h->dynindx == -1) || (h->forced_local != 0)); */
	  reloc_data.symbol_name = h->root.root.string;

	  /* If we have encountered a definition for this symbol.  */
	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      reloc_data.sym_value = h->root.u.def.value;
	      reloc_data.sym_section = h->root.u.def.section;

	      reloc_data.should_relocate = true;

	      if (is_reloc_for_GOT (howto) && !bfd_link_pic (info))
		{
		  struct elf_arc_link_hash_entry *ah =
		    (struct elf_arc_link_hash_entry *) h;
		  /* TODO: Change it to use arc_do_relocation with
		    ARC_32 reloc.  Try to use ADD_RELA macro.  */
		  bfd_vma relocation =
		    reloc_data.sym_value + reloc_data.reloc_addend
		    + (reloc_data.sym_section->output_section != NULL ?
			(reloc_data.sym_section->output_offset
			 + reloc_data.sym_section->output_section->vma)
		      : 0);

		  BFD_ASSERT (ah->got_ents);
		  bfd_vma got_offset = ah->got_ents->offset;
		  bfd_put_32 (output_bfd, relocation,
			      htab->sgot->contents + got_offset);
		}
	      if (is_reloc_for_PLT (howto) && h->plt.offset != (bfd_vma) -1)
		{
		  /* TODO: This is repeated up here.  */
		  reloc_data.sym_value = h->plt.offset;
		  reloc_data.sym_section = htab->splt;
		}
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    {
	      /* Is weak symbol and has no definition.  */
	      if (is_reloc_for_GOT (howto))
		{
		  reloc_data.sym_value = h->root.u.def.value;
		  reloc_data.sym_section = htab->sgot;
		  reloc_data.should_relocate = true;
		}
	      else if (is_reloc_for_PLT (howto)
		       && h->plt.offset != (bfd_vma) -1)
		{
		  /* TODO: This is repeated up here.  */
		  reloc_data.sym_value = h->plt.offset;
		  reloc_data.sym_section = htab->splt;
		  reloc_data.should_relocate = true;
		}
	      else
		continue;
	    }
	  else
	    {
	      if (is_reloc_for_GOT (howto))
		{
		  reloc_data.sym_value = h->root.u.def.value;
		  reloc_data.sym_section = htab->sgot;

		  reloc_data.should_relocate = true;
		}
	      else if (is_reloc_for_PLT (howto))
		{
		  /* Fail if it is linking for PIE and the symbol is
		     undefined.  */
		  if (bfd_link_executable (info))
		    (*info->callbacks->undefined_symbol)
		      (info, h->root.root.string, input_bfd, input_section,
		       rel->r_offset, true);
		  reloc_data.sym_value = h->plt.offset;
		  reloc_data.sym_section = htab->splt;

		  reloc_data.should_relocate = true;
		}
	      else if (!bfd_link_pic (info) || bfd_link_executable (info))
		(*info->callbacks->undefined_symbol)
		  (info, h->root.root.string, input_bfd, input_section,
		   rel->r_offset, true);
	    }

	  BFD_ASSERT (htab->sgot != NULL || !is_reloc_for_GOT (howto));
	  if (htab->sgot != NULL)
	    reloc_data.got_symbol_vma = htab->sgot->output_section->vma
					+ htab->sgot->output_offset;
	}

      if ((is_reloc_for_GOT (howto)
	   || is_reloc_for_TLS (howto)))
	{
	  reloc_data.should_relocate = true;

	  struct got_entry **list
	    = get_got_entry_list_for_symbol (input_bfd, r_symndx, h);

	  reloc_data.got_offset_value
	    = relocate_fix_got_relocs_for_got_info (list,
						    tls_type_for_reloc (howto),
						    info,
						    output_bfd,
						    r_symndx,
						    local_syms,
						    local_sections,
						    h,
						    &reloc_data);

	  if (h == NULL)
	    {
	      create_got_dynrelocs_for_single_entry (
		  got_entry_for_type (list,
				arc_got_entry_type_for_reloc (howto)),
		  output_bfd, info, NULL);
	    }
	}


#define IS_ARC_PCREL_TYPE(TYPE) \
  (   (TYPE == R_ARC_PC32)      \
   || (TYPE == R_ARC_32_PCREL))

      switch (r_type)
	{
	  case R_ARC_32:
	  case R_ARC_32_ME:
	  case R_ARC_PC32:
	  case R_ARC_32_PCREL:
	    if (bfd_link_pic (info)
		&& (input_section->flags & SEC_ALLOC) != 0
		&& (!IS_ARC_PCREL_TYPE (r_type)
		    || (h != NULL
			&& h->dynindx != -1
			&& !h->def_regular
			&& (!info->symbolic || !h->def_regular))))
	      {
		Elf_Internal_Rela outrel;
		bfd_byte *loc;
		bool skip = false;
		bool relocate = false;
		asection *sreloc = _bfd_elf_get_dynamic_reloc_section
				 (input_bfd, input_section,
				  /*RELA*/ true);

		BFD_ASSERT (sreloc != NULL);

		outrel.r_offset = _bfd_elf_section_offset (output_bfd,
							   info,
							   input_section,
							   rel->r_offset);

		if (outrel.r_offset == (bfd_vma) -1)
		  skip = true;

		outrel.r_addend = rel->r_addend;
		outrel.r_offset += (input_section->output_section->vma
				    + input_section->output_offset);

		if (skip)
		  {
		    memset (&outrel, 0, sizeof outrel);
		    relocate = false;
		  }
		else if (h != NULL
			 && h->dynindx != -1
			 && (IS_ARC_PCREL_TYPE (r_type)
			     || !(bfd_link_executable (info)
				  || SYMBOLIC_BIND (info, h))
			     || ! h->def_regular))
		  {
		    BFD_ASSERT (h != NULL);
		    if ((input_section->flags & SEC_ALLOC) != 0)
		      relocate = false;
		    else
		      relocate = true;

		    BFD_ASSERT (h->dynindx != -1);
		    outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  }
		else
		  {
		    /* Handle local symbols, they either do not have a
		       global hash table entry (h == NULL), or are
		       forced local due to a version script
		       (h->forced_local), or the third condition is
		       legacy, it appears to say something like, for
		       links where we are pre-binding the symbols, or
		       there's not an entry for this symbol in the
		       dynamic symbol table, and it's a regular symbol
		       not defined in a shared object, then treat the
		       symbol as local, resolve it now.  */
		    relocate = true;
		    /* outrel.r_addend = 0; */
		    outrel.r_info = ELF32_R_INFO (0, R_ARC_RELATIVE);
		  }

		BFD_ASSERT (sreloc->contents != 0);

		loc = sreloc->contents;
		loc += sreloc->reloc_count * sizeof (Elf32_External_Rela);
		sreloc->reloc_count += 1;

		bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

		if (!relocate)
		  continue;
	      }
	    break;
	  default:
	    break;
	}

      if (is_reloc_SDA_relative (howto)
	  && !reloc_data.sdata_begin_symbol_vma_set)
	{
	  _bfd_error_handler
	    ("error: linker symbol __SDATA_BEGIN__ not found");
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      DEBUG_ARC_RELOC (reloc_data);

      /* Make sure we have with a dynamic linker.  In case of GOT and PLT
	 the sym_section should point to .got or .plt respectively.  */
      if ((is_reloc_for_GOT (howto) || is_reloc_for_PLT (howto))
	  && reloc_data.sym_section == NULL)
	{
	  _bfd_error_handler
	    (_("GOT and PLT relocations cannot be fixed with a non dynamic linker"));
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}

      msg = NULL;
      switch (arc_do_relocation (contents, reloc_data, info))
	{
	case bfd_reloc_ok:
	  continue; /* The reloc processing loop.  */

	case bfd_reloc_overflow:
	  (*info->callbacks->reloc_overflow)
	    (info, (h ? &h->root : NULL), reloc_data.symbol_name, howto->name, (bfd_vma) 0,
	     input_bfd, input_section, rel->r_offset);
	  break;

	case bfd_reloc_undefined:
	  (*info->callbacks->undefined_symbol)
	    (info, reloc_data.symbol_name, input_bfd, input_section, rel->r_offset, true);
	  break;

	case bfd_reloc_other:
	  /* xgettext:c-format */
	  msg = _("%pB(%pA): warning: unaligned access to symbol '%s' in the small data area");
	  break;

	case bfd_reloc_outofrange:
	  /* xgettext:c-format */
	  msg = _("%pB(%pA): internal error: out of range error");
	  break;

	case bfd_reloc_notsupported:
	  /* xgettext:c-format */
	  msg = _("%pB(%pA): internal error: unsupported relocation error");
	  break;

	case bfd_reloc_dangerous:
	  /* xgettext:c-format */
	  msg = _("%pB(%pA): internal error: dangerous relocation");
	  break;

	default:
	  /* xgettext:c-format */
	  msg = _("%pB(%pA): internal error: unknown error");
	  break;
	}

      if (msg)
	_bfd_error_handler (msg, input_bfd, input_section, reloc_data.symbol_name);
      return false;
    }

  return true;
}

#define elf_arc_hash_table(p) \
  ((is_elf_hash_table ((p)->hash)					\
    && elf_hash_table_id (elf_hash_table (p)) == ARC_ELF_DATA)		\
   ? (struct elf_arc_link_hash_table *) (p)->hash : NULL)

static bool
elf_arc_check_relocs (bfd *			 abfd,
		      struct bfd_link_info *     info,
		      asection *		 sec,
		      const Elf_Internal_Rela *  relocs)
{
  Elf_Internal_Shdr *		symtab_hdr;
  struct elf_link_hash_entry **	sym_hashes;
  const Elf_Internal_Rela *	rel;
  const Elf_Internal_Rela *	rel_end;
  bfd *				dynobj;
  asection *			sreloc = NULL;
  struct elf_link_hash_table *	htab = elf_hash_table (info);

  if (bfd_link_relocatable (info))
    return true;

  if (htab->dynobj == NULL)
    htab->dynobj = abfd;

  dynobj = (elf_hash_table (info))->dynobj;
  symtab_hdr = &((elf_tdata (abfd))->symtab_hdr);
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      enum elf_arc_reloc_type r_type;
      reloc_howto_type *howto;
      unsigned long   r_symndx;
      struct elf_link_hash_entry *h;

      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_type >= (int) R_ARC_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return false;
	}
      howto = arc_elf_howto (r_type);

      /* Load symbol information.  */
      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info) /* Is a local symbol.  */
	h = NULL;
      else /* Global one.  */
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}


      switch (r_type)
	{
	case R_ARC_32:
	case R_ARC_32_ME:
	  /* During shared library creation, these relocs should not
	     appear in a shared library (as memory will be read only
	     and the dynamic linker can not resolve these.  However
	     the error should not occur for e.g. debugging or
	     non-readonly sections.  */
	  if (h != NULL
	      && (bfd_link_dll (info) && !bfd_link_pie (info))
	      && (sec->flags & SEC_ALLOC) != 0
	      && (sec->flags & SEC_READONLY) != 0
	      && ((sec->flags & SEC_CODE) != 0
		  || (sec->flags & SEC_DEBUGGING) != 0))
	    {
	      const char *name;
	      if (h)
		name = h->root.root.string;
	      else
		name = "UNKNOWN";
	      _bfd_error_handler
	      /* xgettext:c-format */
	      (_("%pB: relocation %s against `%s' can not be used"
		 " when making a shared object; recompile with -fPIC"),
		 abfd,
		 arc_elf_howto (r_type)->name,
		 name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }

	    /* In some cases we are not setting the 'non_got_ref'
	       flag, even though the relocations don't require a GOT
	       access.  We should extend the testing in this area to
	       ensure that no significant cases are being missed.  */
	    if (h)
	      h->non_got_ref = 1;
	    /* FALLTHROUGH */
	  case R_ARC_PC32:
	  case R_ARC_32_PCREL:
	    if ((bfd_link_pic (info))
		&& ((r_type != R_ARC_PC32 && r_type != R_ARC_32_PCREL)
		    || (h != NULL
			&& (!info->symbolic || !h->def_regular))))
	      {
		if (sreloc == NULL)
		  {
		    if (info->dynamic
			&& ! htab->dynamic_sections_created
			&& ! _bfd_elf_link_create_dynamic_sections (abfd, info))
		      return false;
		    sreloc = _bfd_elf_make_dynamic_reloc_section (sec, dynobj,
								  2, abfd,
								  /*rela*/
								  true);

		    if (sreloc == NULL)
		      return false;
		  }
		sreloc->size += sizeof (Elf32_External_Rela);

	      }
	  default:
	    break;
	}

      if (is_reloc_for_PLT (howto))
	{
	  if (h == NULL)
	    continue;
	  else
	    if (h->forced_local == 0)
	      h->needs_plt = 1;
	}

      /* Add info to the symbol got_entry_list.  */
      if (is_reloc_for_GOT (howto)
	  || is_reloc_for_TLS (howto))
	{
	  if (bfd_link_dll (info) && !bfd_link_pie (info)
	      && (r_type == R_ARC_TLS_LE_32 || r_type == R_ARC_TLS_LE_S9))
	    {
	      const char *name;
	      if (h)
		name = h->root.root.string;
	      else
		/* bfd_elf_sym_name (abfd, symtab_hdr, isym, NULL);  */
		name = "UNKNOWN";
	      _bfd_error_handler
		/* xgettext:c-format */
		(_("%pB: relocation %s against `%s' can not be used"
		   " when making a shared object; recompile with -fPIC"),
		   abfd,
		   arc_elf_howto (r_type)->name,
		   name);
	      bfd_set_error (bfd_error_bad_value);
	      return false;
	    }
	  if (! _bfd_elf_create_got_section (dynobj, info))
	    return false;

	  arc_fill_got_info_for_reloc (
		  arc_got_entry_type_for_reloc (howto),
		  get_got_entry_list_for_symbol (abfd, r_symndx, h),
		  info,
		  h);
	}
    }

  return true;
}

#define ELF_DYNAMIC_INTERPRETER  "/sbin/ld-uClibc.so"

static const struct plt_version_t *
arc_get_plt_version (struct bfd_link_info *info)
{
  int i;

  for (i = 0; i < 1; i++)
    {
      ARC_DEBUG ("%d: size1 = %d, size2 = %d\n", i,
		 (int) plt_versions[i].entry_size,
		 (int) plt_versions[i].elem_size);
    }

  if (bfd_get_mach (info->output_bfd) == bfd_mach_arc_arcv2)
    {
      if (bfd_link_pic (info))
	return &(plt_versions[ELF_ARCV2_PIC]);
      else
	return &(plt_versions[ELF_ARCV2_ABS]);
    }
  else
    {
      if (bfd_link_pic (info))
	return &(plt_versions[ELF_ARC_PIC]);
      else
	return &(plt_versions[ELF_ARC_ABS]);
    }
}

static bfd_vma
add_symbol_to_plt (struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  bfd_vma ret;

  const struct plt_version_t *plt_data = arc_get_plt_version (info);

  /* If this is the first .plt entry, make room for the special first
     entry.  */
  if (htab->splt->size == 0)
    htab->splt->size += plt_data->entry_size;

  ret = htab->splt->size;

  htab->splt->size += plt_data->elem_size;
  ARC_DEBUG ("PLT_SIZE = %d\n", (int) htab->splt->size);

  htab->sgotplt->size += 4;
  htab->srelplt->size += sizeof (Elf32_External_Rela);

  return ret;
}

#define PLT_DO_RELOCS_FOR_ENTRY(ABFD, DS, RELOCS)	\
  plt_do_relocs_for_symbol (ABFD, DS, RELOCS, 0, 0)

static void
plt_do_relocs_for_symbol (bfd *abfd,
			  struct elf_link_hash_table *htab,
			  const struct plt_reloc *reloc,
			  bfd_vma plt_offset,
			  bfd_vma symbol_got_offset)
{
  while (SYM_ONLY (reloc->symbol) != LAST_RELOC)
    {
      bfd_vma relocation = 0;

      switch (SYM_ONLY (reloc->symbol))
	{
	  case SGOT:
		relocation
		  = htab->sgotplt->output_section->vma
		    + htab->sgotplt->output_offset + symbol_got_offset;
		break;
	}
      relocation += reloc->addend;

      if (IS_RELATIVE (reloc->symbol))
	{
	  bfd_vma reloc_offset = reloc->offset;
	  reloc_offset -= (IS_INSN_32 (reloc->symbol)) ? 4 : 0;
	  reloc_offset -= (IS_INSN_24 (reloc->symbol)) ? 2 : 0;

	  relocation -= htab->splt->output_section->vma
			 + htab->splt->output_offset
			 + plt_offset + reloc_offset;
	}

      /* TODO: being ME is not a property of the relocation but of the
	 section of which is applying the relocation. */
      if (IS_MIDDLE_ENDIAN (reloc->symbol) && !bfd_big_endian (abfd))
	{
	  relocation
	    = ((relocation & 0xffff0000) >> 16)
	      | ((relocation & 0xffff) << 16);
	}

      switch (reloc->size)
	{
	  case 32:
	    bfd_put_32 (htab->splt->output_section->owner,
			relocation,
			htab->splt->contents + plt_offset + reloc->offset);
	    break;
	}

      reloc = &(reloc[1]); /* Jump to next relocation.  */
    }
}

static void
relocate_plt_for_symbol (bfd *output_bfd,
			 struct bfd_link_info *info,
			 struct elf_link_hash_entry *h)
{
  const struct plt_version_t *plt_data = arc_get_plt_version (info);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  bfd_vma plt_index = (h->plt.offset  - plt_data->entry_size)
		      / plt_data->elem_size;
  bfd_vma got_offset = (plt_index + 3) * 4;

  ARC_DEBUG ("arc_info: PLT_OFFSET = %#lx, PLT_ENTRY_VMA = %#lx, \
GOT_ENTRY_OFFSET = %#lx, GOT_ENTRY_VMA = %#lx, for symbol %s\n",
	     (long) h->plt.offset,
	     (long) (htab->splt->output_section->vma
		     + htab->splt->output_offset
		     + h->plt.offset),
	     (long) got_offset,
	     (long) (htab->sgotplt->output_section->vma
		     + htab->sgotplt->output_offset
		     + got_offset),
	     h->root.root.string);

  {
    bfd_vma i = 0;
    uint16_t *ptr = (uint16_t *) plt_data->elem;

    for (i = 0; i < plt_data->elem_size/2; i++)
      {
	uint16_t data = ptr[i];
	bfd_put_16 (output_bfd,
		    (bfd_vma) data,
		    htab->splt->contents + h->plt.offset + (i*2));
      }
  }

  plt_do_relocs_for_symbol (output_bfd, htab,
			    plt_data->elem_relocs,
			    h->plt.offset,
			    got_offset);

  /* Fill in the entry in the global offset table.  */
  bfd_put_32 (output_bfd,
	      (bfd_vma) (htab->splt->output_section->vma
			 + htab->splt->output_offset),
	      htab->sgotplt->contents + got_offset);

  /* TODO: Fill in the entry in the .rela.plt section.  */
  {
    Elf_Internal_Rela rel;
    bfd_byte *loc;

    rel.r_offset = (htab->sgotplt->output_section->vma
		    + htab->sgotplt->output_offset
		    + got_offset);
    rel.r_addend = 0;

    BFD_ASSERT (h->dynindx != -1);
    rel.r_info = ELF32_R_INFO (h->dynindx, R_ARC_JMP_SLOT);

    loc = htab->srelplt->contents;
    loc += plt_index * sizeof (Elf32_External_Rela); /* relA */
    bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
  }
}

static void
relocate_plt_for_entry (bfd *abfd,
			struct bfd_link_info *info)
{
  const struct plt_version_t *plt_data = arc_get_plt_version (info);
  struct elf_link_hash_table *htab = elf_hash_table (info);

  {
    bfd_vma i = 0;
    uint16_t *ptr = (uint16_t *) plt_data->entry;
    for (i = 0; i < plt_data->entry_size/2; i++)
      {
	uint16_t data = ptr[i];
	bfd_put_16 (abfd,
		    (bfd_vma) data,
		    htab->splt->contents + (i*2));
      }
  }
  PLT_DO_RELOCS_FOR_ENTRY (abfd, htab, plt_data->entry_relocs);
}

/* Desc : Adjust a symbol defined by a dynamic object and referenced
   by a regular object.  The current definition is in some section of
   the dynamic object, but we're not including those sections.  We
   have to change the definition to something the rest of the link can
   understand.  */

static bool
elf_arc_adjust_dynamic_symbol (struct bfd_link_info *info,
			      struct elf_link_hash_entry *h)
{
  asection *s;
  bfd *dynobj = (elf_hash_table (info))->dynobj;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (h->type == STT_FUNC
      || h->type == STT_GNU_IFUNC
      || h->needs_plt == 1)
    {
      if (!bfd_link_pic (info) && !h->def_dynamic && !h->ref_dynamic)
	{
	  /* This case can occur if we saw a PLT32 reloc in an input
	     file, but the symbol was never referred to by a dynamic
	     object.  In such a case, we don't actually need to build
	     a procedure linkage table, and we can just do a PC32
	     reloc instead.  */
	  BFD_ASSERT (h->needs_plt);
	  return true;
	}

      /* Make sure this symbol is output as a dynamic symbol.  */
      if (h->dynindx == -1 && !h->forced_local
	  && !bfd_elf_link_record_dynamic_symbol (info, h))
	return false;

      if (bfd_link_pic (info)
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  bfd_vma loc = add_symbol_to_plt (info);

	  if (bfd_link_executable (info) && !h->def_regular)
	    {
	      h->root.u.def.section = htab->splt;
	      h->root.u.def.value = loc;
	    }
	  h->plt.offset = loc;
	}
      else
	{
	  h->plt.offset = (bfd_vma) -1;
	  h->needs_plt = 0;
	}
      return true;
    }

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->is_weakalias)
    {
      struct elf_link_hash_entry *def = weakdef (h);
      BFD_ASSERT (def->root.type == bfd_link_hash_defined);
      h->root.u.def.section = def->root.u.def.section;
      h->root.u.def.value = def->root.u.def.value;
      return true;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (!bfd_link_executable (info))
    return true;

  /* If there are no non-GOT references, we do not need a copy
     relocation.  */
  if (!h->non_got_ref)
    return true;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return true;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

  if (htab == NULL)
    return false;

  /* We must generate a R_ARC_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image.  We need to remember the offset into the
     .rela.bss section we are going to use.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      struct elf_arc_link_hash_table *arc_htab = elf_arc_hash_table (info);

      BFD_ASSERT (arc_htab->elf.srelbss != NULL);
      arc_htab->elf.srelbss->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  /* TODO: Move this also to arc_hash_table.  */
  s = bfd_get_section_by_name (dynobj, ".dynbss");
  BFD_ASSERT (s != NULL);

  return _bfd_elf_adjust_dynamic_copy (info, h, s);
}

/* Function :  elf_arc_finish_dynamic_symbol
   Brief    :  Finish up dynamic symbol handling.  We set the
	     contents of various dynamic sections here.
   Args     :  output_bfd :
	       info	  :
	       h	  :
	       sym	  :
   Returns  : True/False as the return status.  */

static bool
elf_arc_finish_dynamic_symbol (bfd * output_bfd,
			       struct bfd_link_info *info,
			       struct elf_link_hash_entry *h,
			       Elf_Internal_Sym * sym)
{
  if (h->plt.offset != (bfd_vma) -1)
    {
      relocate_plt_for_symbol (output_bfd, info, h);

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  */
	  sym->st_shndx = SHN_UNDEF;
	}
    }


  /* This function traverses list of GOT entries and
     create respective dynamic relocs.  */
  /* TODO: Make function to get list and not access the list directly.  */
  /* TODO: Move function to relocate_section create this relocs eagerly.  */
  struct elf_arc_link_hash_entry *ah =
    (struct elf_arc_link_hash_entry *) h;
  create_got_dynrelocs_for_got_info (&ah->got_ents,
				     output_bfd,
				     info,
				     h);

  if (h->needs_copy)
    {
      struct elf_arc_link_hash_table *arc_htab = elf_arc_hash_table (info);

      if (arc_htab == NULL)
	return false;

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || arc_htab->elf.srelbss == NULL)
	abort ();

      bfd_vma rel_offset = (h->root.u.def.value
			    + h->root.u.def.section->output_section->vma
			    + h->root.u.def.section->output_offset);

      bfd_byte * loc = arc_htab->elf.srelbss->contents
	+ (arc_htab->elf.srelbss->reloc_count * sizeof (Elf32_External_Rela));
      arc_htab->elf.srelbss->reloc_count++;

      Elf_Internal_Rela rel;
      rel.r_addend = 0;
      rel.r_offset = rel_offset;

      BFD_ASSERT (h->dynindx != -1);
      rel.r_info = ELF32_R_INFO (h->dynindx, R_ARC_COPY);

      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "__DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return true;
}

#define GET_SYMBOL_OR_SECTION(TAG, SYMBOL, SECTION)		\
  case TAG:							\
  if (SYMBOL != NULL)						\
    h = elf_link_hash_lookup (elf_hash_table (info),		\
			      SYMBOL, false, false, true);	\
  else if (SECTION != NULL)					\
    s = bfd_get_linker_section (dynobj, SECTION);		\
  break;


struct obfd_info_group {
  bfd *output_bfd;
  struct bfd_link_info *info;
};

static bool
arc_create_forced_local_got_entries_for_tls (struct bfd_hash_entry *bh,
					     void *data)
{
  struct elf_arc_link_hash_entry * h =
    (struct elf_arc_link_hash_entry *) bh;
  struct obfd_info_group *tmp = (struct obfd_info_group *) data;

  if (h->got_ents != NULL)
    {
      BFD_ASSERT (h);

      struct got_entry *list = h->got_ents;

      while (list != NULL)
	{
	  create_got_dynrelocs_for_single_entry (list, tmp->output_bfd,
	    tmp->info,
	    (struct elf_link_hash_entry *) h);
	  list = list->next;
	}
    }

  return true;
}


/* Function :  elf_arc_finish_dynamic_sections
   Brief    :  Finish up the dynamic sections handling.
   Args     :  output_bfd :
	       info	  :
	       h	  :
	       sym	  :
   Returns  : True/False as the return status.  */

static bool
elf_arc_finish_dynamic_sections (bfd * output_bfd,
				 struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);
  bfd *dynobj = (elf_hash_table (info))->dynobj;
  asection *sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (sdyn)
    {
      Elf32_External_Dyn *dyncon, *dynconend;

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend
	= (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn internal_dyn;
	  bool do_it = false;

	  struct elf_link_hash_entry *h = NULL;
	  asection	 *s = NULL;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &internal_dyn);

	  switch (internal_dyn.d_tag)
	    {
	      GET_SYMBOL_OR_SECTION (DT_INIT, info->init_function, NULL)
	      GET_SYMBOL_OR_SECTION (DT_FINI, info->fini_function, NULL)
	      GET_SYMBOL_OR_SECTION (DT_PLTGOT, NULL, ".plt")
	      GET_SYMBOL_OR_SECTION (DT_JMPREL, NULL, ".rela.plt")
	      GET_SYMBOL_OR_SECTION (DT_PLTRELSZ, NULL, ".rela.plt")
	      GET_SYMBOL_OR_SECTION (DT_VERSYM, NULL, ".gnu.version")
	      GET_SYMBOL_OR_SECTION (DT_VERDEF, NULL, ".gnu.version_d")
	      GET_SYMBOL_OR_SECTION (DT_VERNEED, NULL, ".gnu.version_r")
	      default:
		break;
	    }

	  /* In case the dynamic symbols should be updated with a symbol.  */
	  if (h != NULL
	      && (h->root.type == bfd_link_hash_defined
		  || h->root.type == bfd_link_hash_defweak))
	    {
	      asection	     *asec_ptr;

	      internal_dyn.d_un.d_val = h->root.u.def.value;
	      asec_ptr = h->root.u.def.section;
	      if (asec_ptr->output_section != NULL)
		{
		  internal_dyn.d_un.d_val +=
		    (asec_ptr->output_section->vma
		     + asec_ptr->output_offset);
		}
	      else
		{
		  /* The symbol is imported from another shared
		     library and does not apply to this one.  */
		  internal_dyn.d_un.d_val = 0;
		}
	      do_it = true;
	    }
	  else if (s != NULL) /* With a section information.  */
	    {
	      switch (internal_dyn.d_tag)
		{
		  case DT_PLTGOT:
		  case DT_JMPREL:
		  case DT_VERSYM:
		  case DT_VERDEF:
		  case DT_VERNEED:
		    internal_dyn.d_un.d_ptr = (s->output_section->vma
					       + s->output_offset);
		    do_it = true;
		    break;

		  case DT_PLTRELSZ:
		    internal_dyn.d_un.d_val = s->size;
		    do_it = true;
		    break;

		  default:
		    break;
		}
	    }

	  if (do_it)
	    bfd_elf32_swap_dyn_out (output_bfd, &internal_dyn, dyncon);
	}

      if (htab->splt->size > 0)
	{
	  relocate_plt_for_entry (output_bfd, info);
	}

      /* TODO: Validate this.  */
      if (htab->srelplt->output_section != bfd_abs_section_ptr)
	elf_section_data (htab->srelplt->output_section)
	  ->this_hdr.sh_entsize = 12;
    }

  /* Fill in the first three entries in the global offset table.  */
  if (htab->sgot)
    {
      struct elf_link_hash_entry *h;
      h = elf_link_hash_lookup (elf_hash_table (info), "_GLOBAL_OFFSET_TABLE_",
				 false, false, true);

	if (h != NULL && h->root.type != bfd_link_hash_undefined
	    && h->root.u.def.section != NULL)
	{
	  asection *sec = h->root.u.def.section;

	  if (sdyn == NULL)
	    bfd_put_32 (output_bfd, (bfd_vma) 0,
			sec->contents);
	  else
	    bfd_put_32 (output_bfd,
			sdyn->output_section->vma + sdyn->output_offset,
			sec->contents);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sec->contents + 4);
	  bfd_put_32 (output_bfd, (bfd_vma) 0, sec->contents + 8);
	}
    }

  struct obfd_info_group group;
  group.output_bfd = output_bfd;
  group.info = info;
  bfd_hash_traverse (&info->hash->table,
		     arc_create_forced_local_got_entries_for_tls, &group);

  return true;
}

#define ADD_DYNAMIC_SYMBOL(NAME, TAG)					\
  h =  elf_link_hash_lookup (elf_hash_table (info),			\
			     NAME, false, false, false);		\
  if ((h != NULL && (h->ref_regular || h->def_regular)))		\
    if (! _bfd_elf_add_dynamic_entry (info, TAG, 0))			\
      return false;

/* Set the sizes of the dynamic sections.  */
static bool
elf_arc_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bool relocs_exist = false;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  dynobj = htab->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->dynamic_sections_created)
    {
      struct elf_link_hash_entry *h;

      /* Set the contents of the .interp section to the
	 interpreter.  */
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  BFD_ASSERT (s != NULL);
	  s->size = sizeof (ELF_DYNAMIC_INTERPRETER);
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}

      /* Add some entries to the .dynamic section.  We fill in some of
	 the values later, in elf_bfd_final_link, but we must add the
	 entries now so that we know the final size of the .dynamic
	 section.  Checking if the .init section is present.  We also
	 create DT_INIT and DT_FINI entries if the init_str has been
	 changed by the user.  */
      ADD_DYNAMIC_SYMBOL (info->init_function, DT_INIT);
      ADD_DYNAMIC_SYMBOL (info->fini_function, DT_FINI);
    }
  else
    {
      /* We may have created entries in the .rela.got section.
	 However, if we are not creating the dynamic sections, we will
	 not actually use these entries.  Reset the size of .rela.got,
	 which will cause it to get stripped from the output file
	 below.  */
      if (htab->srelgot != NULL)
	htab->srelgot->size = 0;
    }

  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->splt
	  || s == htab->sgot
	  || s == htab->sgotplt
	  || s == htab->sdynbss)
	{
	  /* Strip this section if we don't need it.  */
	}
      else if (startswith (s->name, ".rela"))
	{
	  if (s->size != 0 && s != htab->srelplt)
	    relocs_exist = true;

	  /* We use the reloc_count field as a counter if we need to
	     copy relocs into the output file.  */
	  s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  */
      s->contents = bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return false;
    }

  return _bfd_elf_add_dynamic_tags (output_bfd, info, relocs_exist);
}


/* Classify dynamic relocs such that -z combreloc can reorder and combine
   them.  */
static enum elf_reloc_type_class
elf32_arc_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    const asection *rel_sec ATTRIBUTE_UNUSED,
			    const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_ARC_RELATIVE:
      return reloc_class_relative;
    case R_ARC_JMP_SLOT:
      return reloc_class_plt;
    case R_ARC_COPY:
      return reloc_class_copy;
    /* TODO: Needed in future to support ifunc.  */
    /*
    case R_ARC_IRELATIVE:
      return reloc_class_ifunc;
    */
    default:
      return reloc_class_normal;
    }
}

const struct elf_size_info arc_elf32_size_info =
{
  sizeof (Elf32_External_Ehdr),
  sizeof (Elf32_External_Phdr),
  sizeof (Elf32_External_Shdr),
  sizeof (Elf32_External_Rel),
  sizeof (Elf32_External_Rela),
  sizeof (Elf32_External_Sym),
  sizeof (Elf32_External_Dyn),
  sizeof (Elf_External_Note),
  4,
  1,
  32, 2,
  ELFCLASS32, EV_CURRENT,
  bfd_elf32_write_out_phdrs,
  bfd_elf32_write_shdrs_and_ehdr,
  bfd_elf32_checksum_contents,
  bfd_elf32_write_relocs,
  bfd_elf32_swap_symbol_in,
  bfd_elf32_swap_symbol_out,
  bfd_elf32_slurp_reloc_table,
  bfd_elf32_slurp_symbol_table,
  bfd_elf32_swap_dyn_in,
  bfd_elf32_swap_dyn_out,
  bfd_elf32_swap_reloc_in,
  bfd_elf32_swap_reloc_out,
  bfd_elf32_swap_reloca_in,
  bfd_elf32_swap_reloca_out
};

#define elf_backend_size_info		arc_elf32_size_info

/* GDB expects general purpose registers to be in section .reg.  However Linux
   kernel doesn't create this section and instead writes registers to NOTE
   section.  It is up to the binutils to create a pseudo-section .reg from the
   contents of NOTE.  Also BFD will read pid and signal number from NOTE.  This
   function relies on offsets inside elf_prstatus structure in Linux to be
   stable.  */

static bool
elf32_arc_grok_prstatus (bfd *abfd, Elf_Internal_Note *note)
{
  int offset;
  size_t size;

  switch (note->descsz)
    {
    default:
      return false;

    case 236: /* sizeof (struct elf_prstatus) on Linux/arc.  */
      /* pr_cursig */
      elf_tdata (abfd)->core->signal = bfd_get_16 (abfd, note->descdata + 12);
      /* pr_pid */
      elf_tdata (abfd)->core->lwpid = bfd_get_32 (abfd, note->descdata + 24);
      /* pr_regs */
      offset = 72;
      size = (40 * 4); /* There are 40 registers in user_regs_struct.  */
      break;
    }
  /* Make a ".reg/999" section.  */
  return _bfd_elfcore_make_pseudosection (abfd, ".reg", size,
					  note->descpos + offset);
}

/* Determine whether an object attribute tag takes an integer, a
   string or both.  */

static int
elf32_arc_obj_attrs_arg_type (int tag)
{
  if (tag == Tag_ARC_CPU_name
	   || tag == Tag_ARC_ISA_config
	   || tag == Tag_ARC_ISA_apex)
    return ATTR_TYPE_FLAG_STR_VAL;
  else if (tag < (Tag_ARC_ISA_mpy_option + 1))
    return ATTR_TYPE_FLAG_INT_VAL;
  else
    return (tag & 1) != 0 ? ATTR_TYPE_FLAG_STR_VAL : ATTR_TYPE_FLAG_INT_VAL;
}

/* Attribute numbers >=14 can be safely ignored.  */

static bool
elf32_arc_obj_attrs_handle_unknown (bfd *abfd, int tag)
{
  if ((tag & 127) < (Tag_ARC_ISA_mpy_option + 1))
    {
      _bfd_error_handler
	(_("%pB: unknown mandatory ARC object attribute %d"),
	 abfd, tag);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  else
    {
      _bfd_error_handler
	(_("warning: %pB: unknown ARC object attribute %d"),
	 abfd, tag);
      return true;
    }
}

/* Handle an ARC specific section when reading an object file.  This is
   called when bfd_section_from_shdr finds a section with an unknown
   type.  */

static bool
elf32_arc_section_from_shdr (bfd *abfd,
			     Elf_Internal_Shdr * hdr,
			     const char *name,
			     int shindex)
{
  switch (hdr->sh_type)
    {
    case 0x0c: /* MWDT specific section, don't complain about it.  */
    case SHT_ARC_ATTRIBUTES:
      break;

    default:
      return false;
    }

  if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return false;

  return true;
}

/* Relaxation hook.

   These are the current relaxing opportunities available:

   * R_ARC_GOTPC32 => R_ARC_PCREL.

*/

static bool
arc_elf_relax_section (bfd *abfd, asection *sec,
		       struct bfd_link_info *link_info, bool *again)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;

  /* Assume nothing changes.  */
  *again = false;

  /* We don't have to do anything for a relocatable link, if this
     section does not have relocs, or if this is not a code
     section.  */
  if (bfd_link_relocatable (link_info)
      || sec->reloc_count == 0
      || (sec->flags & SEC_RELOC) == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || (sec->flags & SEC_CODE) == 0)
    return true;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  /* Get a copy of the native relocations.  */
  internal_relocs = _bfd_elf_link_read_relocs (abfd, sec, NULL, NULL,
                                               link_info->keep_memory);
  if (internal_relocs == NULL)
    goto error_return;

  /* Walk through them looking for relaxing opportunities.  */
  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      /* If this isn't something that can be relaxed, then ignore
         this reloc.  */
      if (ELF32_R_TYPE (irel->r_info) != (int) R_ARC_GOTPC32)
        continue;

      /* Get the section contents if we haven't done so already.  */
      if (contents == NULL)
        {
          /* Get cached copy if it exists.  */
          if (elf_section_data (sec)->this_hdr.contents != NULL)
            contents = elf_section_data (sec)->this_hdr.contents;
          /* Go get them off disk.  */
          else if (!bfd_malloc_and_get_section (abfd, sec, &contents))
            goto error_return;
        }

      /* Read this BFD's local symbols if we haven't done so already.  */
      if (isymbuf == NULL && symtab_hdr->sh_info != 0)
        {
          isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
          if (isymbuf == NULL)
            isymbuf = bfd_elf_get_elf_syms (abfd, symtab_hdr,
                                            symtab_hdr->sh_info, 0,
                                            NULL, NULL, NULL);
          if (isymbuf == NULL)
            goto error_return;
        }

      struct elf_link_hash_entry *htop = NULL;

      if (ELF32_R_SYM (irel->r_info) >= symtab_hdr->sh_info)
	{
	  /* An external symbol.  */
	  unsigned int indx = ELF32_R_SYM (irel->r_info) - symtab_hdr->sh_info;
	  htop = elf_sym_hashes (abfd)[indx];
	}

      if (ELF32_R_TYPE (irel->r_info) == (int) R_ARC_GOTPC32
	  && SYMBOL_REFERENCES_LOCAL (link_info, htop))
	{
	  unsigned int code;

	  /* Get the opcode.  */
	  code = bfd_get_32_me (abfd, contents + irel->r_offset - 4);

	  /* Note that we've changed the relocs, section contents, etc.  */
	  elf_section_data (sec)->relocs = internal_relocs;
	  elf_section_data (sec)->this_hdr.contents = contents;
	  symtab_hdr->contents = (unsigned char *) isymbuf;

	  /* Fix the relocation's type.  */
	  irel->r_info = ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_ARC_PC32);

	  /* ld rA,[pcl,symbol@tgot] -> add rA,pcl,symbol@pcl.  */
	  /* 0010 0bbb aa11 0ZZX DBBB 1111 10AA AAAA.
	           111 00    000 0111        xx xxxx*/
	  code &= ~0x27307F80;
	  BFD_ASSERT (code <= 62UL);
	  code |= 0x27007F80;

	  /* Write back the new instruction.  */
	  bfd_put_32_me (abfd, code, contents + irel->r_offset - 4);

	  /* The size isn't changed, don't redo.  */
	  *again = false;
	}
    }

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    {
      if (!link_info->keep_memory)
        free (isymbuf);
      else
       /* Cache the symbols for elf_link_input_bfd.  */
       symtab_hdr->contents = (unsigned char *) isymbuf;
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (!link_info->keep_memory)
        free (contents);
      else
       /* Cache the section contents for elf_link_input_bfd.  */
       elf_section_data (sec)->this_hdr.contents = contents;
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return true;

 error_return:
  if (symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);

  return false;
}

#define TARGET_LITTLE_SYM   arc_elf32_le_vec
#define TARGET_LITTLE_NAME  "elf32-littlearc"
#define TARGET_BIG_SYM	    arc_elf32_be_vec
#define TARGET_BIG_NAME     "elf32-bigarc"
#define ELF_ARCH	    bfd_arch_arc
#define ELF_TARGET_ID	    ARC_ELF_DATA
#define ELF_MACHINE_CODE    EM_ARC_COMPACT
#define ELF_MACHINE_ALT1    EM_ARC_COMPACT2
#define ELF_MAXPAGESIZE     0x2000

#define bfd_elf32_bfd_link_hash_table_create	arc_elf_link_hash_table_create

#define bfd_elf32_bfd_merge_private_bfd_data    arc_elf_merge_private_bfd_data
#define bfd_elf32_bfd_reloc_type_lookup		arc_elf32_bfd_reloc_type_lookup
#define bfd_elf32_bfd_set_private_flags		arc_elf_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data    arc_elf_print_private_bfd_data
#define bfd_elf32_bfd_copy_private_bfd_data     arc_elf_copy_private_bfd_data
#define bfd_elf32_bfd_relax_section		arc_elf_relax_section

#define elf_info_to_howto_rel		     arc_info_to_howto_rel
#define elf_backend_object_p		     arc_elf_object_p
#define elf_backend_final_write_processing   arc_elf_final_write_processing

#define elf_backend_relocate_section	     elf_arc_relocate_section
#define elf_backend_check_relocs	     elf_arc_check_relocs
#define elf_backend_create_dynamic_sections  _bfd_elf_create_dynamic_sections

#define elf_backend_reloc_type_class		elf32_arc_reloc_type_class

#define elf_backend_adjust_dynamic_symbol    elf_arc_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol    elf_arc_finish_dynamic_symbol

#define elf_backend_finish_dynamic_sections  elf_arc_finish_dynamic_sections
#define elf_backend_size_dynamic_sections    elf_arc_size_dynamic_sections

#define elf_backend_can_gc_sections	1
#define elf_backend_want_got_plt	1
#define elf_backend_plt_readonly	1
#define elf_backend_rela_plts_and_copies_p 1
#define elf_backend_want_plt_sym	0
#define elf_backend_got_header_size	12
#define elf_backend_dtrel_excludes_plt	1

#define elf_backend_may_use_rel_p	0
#define elf_backend_may_use_rela_p	1
#define elf_backend_default_use_rela_p	1

#define elf_backend_grok_prstatus elf32_arc_grok_prstatus

#define elf_backend_default_execstack	0

#undef  elf_backend_obj_attrs_vendor
#define elf_backend_obj_attrs_vendor		"ARC"
#undef  elf_backend_obj_attrs_section
#define elf_backend_obj_attrs_section		".ARC.attributes"
#undef  elf_backend_obj_attrs_arg_type
#define elf_backend_obj_attrs_arg_type		elf32_arc_obj_attrs_arg_type
#undef  elf_backend_obj_attrs_section_type
#define elf_backend_obj_attrs_section_type	SHT_ARC_ATTRIBUTES
#define elf_backend_obj_attrs_handle_unknown	elf32_arc_obj_attrs_handle_unknown

#define elf_backend_section_from_shdr		elf32_arc_section_from_shdr

#include "elf32-target.h"
