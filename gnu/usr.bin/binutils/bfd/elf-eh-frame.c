/* .eh_frame section optimization.
   Copyright (C) 2001-2023 Free Software Foundation, Inc.
   Written by Jakub Jelinek <jakub@redhat.com>.

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
#include "dwarf2.h"

#define EH_FRAME_HDR_SIZE 8

struct cie
{
  unsigned int length;
  unsigned int hash;
  unsigned char version;
  unsigned char local_personality;
  char augmentation[20];
  bfd_vma code_align;
  bfd_signed_vma data_align;
  bfd_vma ra_column;
  bfd_vma augmentation_size;
  union {
    struct elf_link_hash_entry *h;
    struct {
      unsigned int bfd_id;
      unsigned int index;
    } sym;
    unsigned int reloc_index;
  } personality;
  struct eh_cie_fde *cie_inf;
  unsigned char per_encoding;
  unsigned char lsda_encoding;
  unsigned char fde_encoding;
  unsigned char initial_insn_length;
  unsigned char can_make_lsda_relative;
  unsigned char initial_instructions[50];
};



/* If *ITER hasn't reached END yet, read the next byte into *RESULT and
   move onto the next byte.  Return true on success.  */

static inline bool
read_byte (bfd_byte **iter, bfd_byte *end, unsigned char *result)
{
  if (*iter >= end)
    return false;
  *result = *((*iter)++);
  return true;
}

/* Move *ITER over LENGTH bytes, or up to END, whichever is closer.
   Return true it was possible to move LENGTH bytes.  */

static inline bool
skip_bytes (bfd_byte **iter, bfd_byte *end, bfd_size_type length)
{
  if ((bfd_size_type) (end - *iter) < length)
    {
      *iter = end;
      return false;
    }
  *iter += length;
  return true;
}

/* Move *ITER over an leb128, stopping at END.  Return true if the end
   of the leb128 was found.  */

static bool
skip_leb128 (bfd_byte **iter, bfd_byte *end)
{
  unsigned char byte;
  do
    if (!read_byte (iter, end, &byte))
      return false;
  while (byte & 0x80);
  return true;
}

/* Like skip_leb128, but treat the leb128 as an unsigned value and
   store it in *VALUE.  */

static bool
read_uleb128 (bfd_byte **iter, bfd_byte *end, bfd_vma *value)
{
  bfd_byte *start, *p;

  start = *iter;
  if (!skip_leb128 (iter, end))
    return false;

  p = *iter;
  *value = *--p;
  while (p > start)
    *value = (*value << 7) | (*--p & 0x7f);

  return true;
}

/* Like read_uleb128, but for signed values.  */

static bool
read_sleb128 (bfd_byte **iter, bfd_byte *end, bfd_signed_vma *value)
{
  bfd_byte *start, *p;

  start = *iter;
  if (!skip_leb128 (iter, end))
    return false;

  p = *iter;
  *value = ((*--p & 0x7f) ^ 0x40) - 0x40;
  while (p > start)
    *value = (*value << 7) | (*--p & 0x7f);

  return true;
}

/* Return 0 if either encoding is variable width, or not yet known to bfd.  */

static
int get_DW_EH_PE_width (int encoding, int ptr_size)
{
  /* DW_EH_PE_ values of 0x60 and 0x70 weren't defined at the time .eh_frame
     was added to bfd.  */
  if ((encoding & 0x60) == 0x60)
    return 0;

  switch (encoding & 7)
    {
    case DW_EH_PE_udata2: return 2;
    case DW_EH_PE_udata4: return 4;
    case DW_EH_PE_udata8: return 8;
    case DW_EH_PE_absptr: return ptr_size;
    default:
      break;
    }

  return 0;
}

#define get_DW_EH_PE_signed(encoding) (((encoding) & DW_EH_PE_signed) != 0)

/* Read a width sized value from memory.  */

static bfd_vma
read_value (bfd *abfd, bfd_byte *buf, int width, int is_signed)
{
  bfd_vma value;

  switch (width)
    {
    case 2:
      if (is_signed)
	value = bfd_get_signed_16 (abfd, buf);
      else
	value = bfd_get_16 (abfd, buf);
      break;
    case 4:
      if (is_signed)
	value = bfd_get_signed_32 (abfd, buf);
      else
	value = bfd_get_32 (abfd, buf);
      break;
    case 8:
      if (is_signed)
	value = bfd_get_signed_64 (abfd, buf);
      else
	value = bfd_get_64 (abfd, buf);
      break;
    default:
      BFD_FAIL ();
      return 0;
    }

  return value;
}

/* Store a width sized value to memory.  */

static void
write_value (bfd *abfd, bfd_byte *buf, bfd_vma value, int width)
{
  switch (width)
    {
    case 2: bfd_put_16 (abfd, value, buf); break;
    case 4: bfd_put_32 (abfd, value, buf); break;
    case 8: bfd_put_64 (abfd, value, buf); break;
    default: BFD_FAIL ();
    }
}

/* Return one if C1 and C2 CIEs can be merged.  */

static int
cie_eq (const void *e1, const void *e2)
{
  const struct cie *c1 = (const struct cie *) e1;
  const struct cie *c2 = (const struct cie *) e2;

  if (c1->hash == c2->hash
      && c1->length == c2->length
      && c1->version == c2->version
      && c1->local_personality == c2->local_personality
      && strcmp (c1->augmentation, c2->augmentation) == 0
      && strcmp (c1->augmentation, "eh") != 0
      && c1->code_align == c2->code_align
      && c1->data_align == c2->data_align
      && c1->ra_column == c2->ra_column
      && c1->augmentation_size == c2->augmentation_size
      && memcmp (&c1->personality, &c2->personality,
		 sizeof (c1->personality)) == 0
      && (c1->cie_inf->u.cie.u.sec->output_section
	  == c2->cie_inf->u.cie.u.sec->output_section)
      && c1->per_encoding == c2->per_encoding
      && c1->lsda_encoding == c2->lsda_encoding
      && c1->fde_encoding == c2->fde_encoding
      && c1->initial_insn_length == c2->initial_insn_length
      && c1->initial_insn_length <= sizeof (c1->initial_instructions)
      && memcmp (c1->initial_instructions,
		 c2->initial_instructions,
		 c1->initial_insn_length) == 0)
    return 1;

  return 0;
}

static hashval_t
cie_hash (const void *e)
{
  const struct cie *c = (const struct cie *) e;
  return c->hash;
}

static hashval_t
cie_compute_hash (struct cie *c)
{
  hashval_t h = 0;
  size_t len;
  h = iterative_hash_object (c->length, h);
  h = iterative_hash_object (c->version, h);
  h = iterative_hash (c->augmentation, strlen (c->augmentation) + 1, h);
  h = iterative_hash_object (c->code_align, h);
  h = iterative_hash_object (c->data_align, h);
  h = iterative_hash_object (c->ra_column, h);
  h = iterative_hash_object (c->augmentation_size, h);
  h = iterative_hash_object (c->personality, h);
  h = iterative_hash_object (c->cie_inf->u.cie.u.sec->output_section, h);
  h = iterative_hash_object (c->per_encoding, h);
  h = iterative_hash_object (c->lsda_encoding, h);
  h = iterative_hash_object (c->fde_encoding, h);
  h = iterative_hash_object (c->initial_insn_length, h);
  len = c->initial_insn_length;
  if (len > sizeof (c->initial_instructions))
    len = sizeof (c->initial_instructions);
  h = iterative_hash (c->initial_instructions, len, h);
  c->hash = h;
  return h;
}

/* Return the number of extra bytes that we'll be inserting into
   ENTRY's augmentation string.  */

static inline unsigned int
extra_augmentation_string_bytes (struct eh_cie_fde *entry)
{
  unsigned int size = 0;
  if (entry->cie)
    {
      if (entry->add_augmentation_size)
	size++;
      if (entry->u.cie.add_fde_encoding)
	size++;
    }
  return size;
}

/* Likewise ENTRY's augmentation data.  */

static inline unsigned int
extra_augmentation_data_bytes (struct eh_cie_fde *entry)
{
  unsigned int size = 0;
  if (entry->add_augmentation_size)
    size++;
  if (entry->cie && entry->u.cie.add_fde_encoding)
    size++;
  return size;
}

/* Return the size that ENTRY will have in the output.  */

static unsigned int
size_of_output_cie_fde (struct eh_cie_fde *entry)
{
  if (entry->removed)
    return 0;
  if (entry->size == 4)
    return 4;
  return (entry->size
	  + extra_augmentation_string_bytes (entry)
	  + extra_augmentation_data_bytes (entry));
}

/* Return the offset of the FDE or CIE after ENT.  */

static unsigned int
next_cie_fde_offset (const struct eh_cie_fde *ent,
		     const struct eh_cie_fde *last,
		     const asection *sec)
{
  while (++ent < last)
    {
      if (!ent->removed)
	return ent->new_offset;
    }
  return sec->size;
}

/* Assume that the bytes between *ITER and END are CFA instructions.
   Try to move *ITER past the first instruction and return true on
   success.  ENCODED_PTR_WIDTH gives the width of pointer entries.  */

static bool
skip_cfa_op (bfd_byte **iter, bfd_byte *end, unsigned int encoded_ptr_width)
{
  bfd_byte op;
  bfd_vma length;

  if (!read_byte (iter, end, &op))
    return false;

  switch (op & 0xc0 ? op & 0xc0 : op)
    {
    case DW_CFA_nop:
    case DW_CFA_advance_loc:
    case DW_CFA_restore:
    case DW_CFA_remember_state:
    case DW_CFA_restore_state:
    case DW_CFA_GNU_window_save:
      /* No arguments.  */
      return true;

    case DW_CFA_offset:
    case DW_CFA_restore_extended:
    case DW_CFA_undefined:
    case DW_CFA_same_value:
    case DW_CFA_def_cfa_register:
    case DW_CFA_def_cfa_offset:
    case DW_CFA_def_cfa_offset_sf:
    case DW_CFA_GNU_args_size:
      /* One leb128 argument.  */
      return skip_leb128 (iter, end);

    case DW_CFA_val_offset:
    case DW_CFA_val_offset_sf:
    case DW_CFA_offset_extended:
    case DW_CFA_register:
    case DW_CFA_def_cfa:
    case DW_CFA_offset_extended_sf:
    case DW_CFA_GNU_negative_offset_extended:
    case DW_CFA_def_cfa_sf:
      /* Two leb128 arguments.  */
      return (skip_leb128 (iter, end)
	      && skip_leb128 (iter, end));

    case DW_CFA_def_cfa_expression:
      /* A variable-length argument.  */
      return (read_uleb128 (iter, end, &length)
	      && skip_bytes (iter, end, length));

    case DW_CFA_expression:
    case DW_CFA_val_expression:
      /* A leb128 followed by a variable-length argument.  */
      return (skip_leb128 (iter, end)
	      && read_uleb128 (iter, end, &length)
	      && skip_bytes (iter, end, length));

    case DW_CFA_set_loc:
      return skip_bytes (iter, end, encoded_ptr_width);

    case DW_CFA_advance_loc1:
      return skip_bytes (iter, end, 1);

    case DW_CFA_advance_loc2:
      return skip_bytes (iter, end, 2);

    case DW_CFA_advance_loc4:
      return skip_bytes (iter, end, 4);

    case DW_CFA_MIPS_advance_loc8:
      return skip_bytes (iter, end, 8);

    default:
      return false;
    }
}

/* Try to interpret the bytes between BUF and END as CFA instructions.
   If every byte makes sense, return a pointer to the first DW_CFA_nop
   padding byte, or END if there is no padding.  Return null otherwise.
   ENCODED_PTR_WIDTH is as for skip_cfa_op.  */

static bfd_byte *
skip_non_nops (bfd_byte *buf, bfd_byte *end, unsigned int encoded_ptr_width,
	       unsigned int *set_loc_count)
{
  bfd_byte *last;

  last = buf;
  while (buf < end)
    if (*buf == DW_CFA_nop)
      buf++;
    else
      {
	if (*buf == DW_CFA_set_loc)
	  ++*set_loc_count;
	if (!skip_cfa_op (&buf, end, encoded_ptr_width))
	  return 0;
	last = buf;
      }
  return last;
}

/* Convert absolute encoding ENCODING into PC-relative form.
   SIZE is the size of a pointer.  */

static unsigned char
make_pc_relative (unsigned char encoding, unsigned int ptr_size)
{
  if ((encoding & 0x7f) == DW_EH_PE_absptr)
    switch (ptr_size)
      {
      case 2:
	encoding |= DW_EH_PE_sdata2;
	break;
      case 4:
	encoding |= DW_EH_PE_sdata4;
	break;
      case 8:
	encoding |= DW_EH_PE_sdata8;
	break;
      }
  return encoding | DW_EH_PE_pcrel;
}

/*  Examine each .eh_frame_entry section and discard those
    those that are marked SEC_EXCLUDE.  */

static void
bfd_elf_discard_eh_frame_entry (struct eh_frame_hdr_info *hdr_info)
{
  unsigned int i;
  for (i = 0; i < hdr_info->array_count; i++)
    {
      if (hdr_info->u.compact.entries[i]->flags & SEC_EXCLUDE)
	{
	  unsigned int j;
	  for (j = i + 1; j < hdr_info->array_count; j++)
	    hdr_info->u.compact.entries[j-1] = hdr_info->u.compact.entries[j];

	  hdr_info->array_count--;
	  hdr_info->u.compact.entries[hdr_info->array_count] = NULL;
	  i--;
	}
    }
}

/* Add a .eh_frame_entry section.  */

static void
bfd_elf_record_eh_frame_entry (struct eh_frame_hdr_info *hdr_info,
				 asection *sec)
{
  if (hdr_info->array_count == hdr_info->u.compact.allocated_entries)
    {
      if (hdr_info->u.compact.allocated_entries == 0)
	{
	  hdr_info->frame_hdr_is_compact = true;
	  hdr_info->u.compact.allocated_entries = 2;
	  hdr_info->u.compact.entries =
	    bfd_malloc (hdr_info->u.compact.allocated_entries
			* sizeof (hdr_info->u.compact.entries[0]));
	}
      else
	{
	  hdr_info->u.compact.allocated_entries *= 2;
	  hdr_info->u.compact.entries =
	    bfd_realloc (hdr_info->u.compact.entries,
			 hdr_info->u.compact.allocated_entries
			   * sizeof (hdr_info->u.compact.entries[0]));
	}

      BFD_ASSERT (hdr_info->u.compact.entries);
    }

  hdr_info->u.compact.entries[hdr_info->array_count++] = sec;
}

/* Parse a .eh_frame_entry section.  Figure out which text section it
   references.  */

bool
_bfd_elf_parse_eh_frame_entry (struct bfd_link_info *info,
			       asection *sec, struct elf_reloc_cookie *cookie)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  unsigned long r_symndx;
  asection *text_sec;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;

  if (sec->size == 0
      || sec->sec_info_type != SEC_INFO_TYPE_NONE)
    {
      return true;
    }

  if (sec->output_section && bfd_is_abs_section (sec->output_section))
    {
      /* At least one of the sections is being discarded from the
	 link, so we should just ignore them.  */
      return true;
    }

  if (cookie->rel == cookie->relend)
    return false;

  /* The first relocation is the function start.  */
  r_symndx = cookie->rel->r_info >> cookie->r_sym_shift;
  if (r_symndx == STN_UNDEF)
    return false;

  text_sec = _bfd_elf_section_for_symbol (cookie, r_symndx, false);

  if (text_sec == NULL)
    return false;

  elf_section_eh_frame_entry (text_sec) = sec;
  if (text_sec->output_section
      && bfd_is_abs_section (text_sec->output_section))
    sec->flags |= SEC_EXCLUDE;

  sec->sec_info_type = SEC_INFO_TYPE_EH_FRAME_ENTRY;
  elf_section_data (sec)->sec_info = text_sec;
  bfd_elf_record_eh_frame_entry (hdr_info, sec);
  return true;
}

/* Try to parse .eh_frame section SEC, which belongs to ABFD.  Store the
   information in the section's sec_info field on success.  COOKIE
   describes the relocations in SEC.  */

void
_bfd_elf_parse_eh_frame (bfd *abfd, struct bfd_link_info *info,
			 asection *sec, struct elf_reloc_cookie *cookie)
{
#define REQUIRE(COND)					\
  do							\
    if (!(COND))					\
      goto free_no_table;				\
  while (0)

  bfd_byte *ehbuf = NULL, *buf, *end;
  bfd_byte *last_fde;
  struct eh_cie_fde *this_inf;
  unsigned int hdr_length, hdr_id;
  unsigned int cie_count;
  struct cie *cie, *local_cies = NULL;
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  struct eh_frame_sec_info *sec_info = NULL;
  unsigned int ptr_size;
  unsigned int num_cies;
  unsigned int num_entries;
  elf_gc_mark_hook_fn gc_mark_hook;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;

  if (sec->size == 0
      || (sec->flags & SEC_HAS_CONTENTS) == 0
      || sec->sec_info_type != SEC_INFO_TYPE_NONE)
    {
      /* This file does not contain .eh_frame information.  */
      return;
    }

  if (bfd_is_abs_section (sec->output_section))
    {
      /* At least one of the sections is being discarded from the
	 link, so we should just ignore them.  */
      return;
    }

  /* Read the frame unwind information from abfd.  */

  REQUIRE (bfd_malloc_and_get_section (abfd, sec, &ehbuf));

  /* If .eh_frame section size doesn't fit into int, we cannot handle
     it (it would need to use 64-bit .eh_frame format anyway).  */
  REQUIRE (sec->size == (unsigned int) sec->size);

  ptr_size = (get_elf_backend_data (abfd)
	      ->elf_backend_eh_frame_address_size (abfd, sec));
  REQUIRE (ptr_size != 0);

  /* Go through the section contents and work out how many FDEs and
     CIEs there are.  */
  buf = ehbuf;
  end = ehbuf + sec->size;
  num_cies = 0;
  num_entries = 0;
  while (buf != end)
    {
      num_entries++;

      /* Read the length of the entry.  */
      REQUIRE (skip_bytes (&buf, end, 4));
      hdr_length = bfd_get_32 (abfd, buf - 4);

      /* 64-bit .eh_frame is not supported.  */
      REQUIRE (hdr_length != 0xffffffff);
      if (hdr_length == 0)
	break;

      REQUIRE (skip_bytes (&buf, end, 4));
      hdr_id = bfd_get_32 (abfd, buf - 4);
      if (hdr_id == 0)
	num_cies++;

      REQUIRE (skip_bytes (&buf, end, hdr_length - 4));
    }

  sec_info = (struct eh_frame_sec_info *)
      bfd_zmalloc (sizeof (struct eh_frame_sec_info)
		   + (num_entries - 1) * sizeof (struct eh_cie_fde));
  REQUIRE (sec_info);

  /* We need to have a "struct cie" for each CIE in this section.  */
  if (num_cies)
    {
      local_cies = (struct cie *) bfd_zmalloc (num_cies * sizeof (*local_cies));
      REQUIRE (local_cies);
    }

  /* FIXME: octets_per_byte.  */
#define ENSURE_NO_RELOCS(buf)				\
  while (cookie->rel < cookie->relend			\
	 && (cookie->rel->r_offset			\
	     < (bfd_size_type) ((buf) - ehbuf)))	\
    {							\
      REQUIRE (cookie->rel->r_info == 0);		\
      cookie->rel++;					\
    }

  /* FIXME: octets_per_byte.  */
#define SKIP_RELOCS(buf)				\
  while (cookie->rel < cookie->relend			\
	 && (cookie->rel->r_offset			\
	     < (bfd_size_type) ((buf) - ehbuf)))	\
    cookie->rel++

  /* FIXME: octets_per_byte.  */
#define GET_RELOC(buf)					\
  ((cookie->rel < cookie->relend			\
    && (cookie->rel->r_offset				\
	== (bfd_size_type) ((buf) - ehbuf)))		\
   ? cookie->rel : NULL)

  buf = ehbuf;
  cie_count = 0;
  gc_mark_hook = get_elf_backend_data (abfd)->gc_mark_hook;
  while ((bfd_size_type) (buf - ehbuf) != sec->size)
    {
      char *aug;
      bfd_byte *start, *insns, *insns_end;
      bfd_size_type length;
      unsigned int set_loc_count;

      this_inf = sec_info->entry + sec_info->count;
      last_fde = buf;

      /* Read the length of the entry.  */
      REQUIRE (skip_bytes (&buf, ehbuf + sec->size, 4));
      hdr_length = bfd_get_32 (abfd, buf - 4);

      /* The CIE/FDE must be fully contained in this input section.  */
      REQUIRE ((bfd_size_type) (buf - ehbuf) + hdr_length <= sec->size);
      end = buf + hdr_length;

      this_inf->offset = last_fde - ehbuf;
      this_inf->size = 4 + hdr_length;
      this_inf->reloc_index = cookie->rel - cookie->rels;

      if (hdr_length == 0)
	{
	  /* A zero-length CIE should only be found at the end of
	     the section, but allow multiple terminators.  */
	  while (skip_bytes (&buf, ehbuf + sec->size, 4))
	    REQUIRE (bfd_get_32 (abfd, buf - 4) == 0);
	  REQUIRE ((bfd_size_type) (buf - ehbuf) == sec->size);
	  ENSURE_NO_RELOCS (buf);
	  sec_info->count++;
	  break;
	}

      REQUIRE (skip_bytes (&buf, end, 4));
      hdr_id = bfd_get_32 (abfd, buf - 4);

      if (hdr_id == 0)
	{
	  unsigned int initial_insn_length;

	  /* CIE  */
	  this_inf->cie = 1;

	  /* Point CIE to one of the section-local cie structures.  */
	  cie = local_cies + cie_count++;

	  cie->cie_inf = this_inf;
	  cie->length = hdr_length;
	  start = buf;
	  REQUIRE (read_byte (&buf, end, &cie->version));

	  /* Cannot handle unknown versions.  */
	  REQUIRE (cie->version == 1
		   || cie->version == 3
		   || cie->version == 4);
	  REQUIRE (strlen ((char *) buf) < sizeof (cie->augmentation));

	  strcpy (cie->augmentation, (char *) buf);
	  buf = (bfd_byte *) strchr ((char *) buf, '\0') + 1;
	  this_inf->u.cie.aug_str_len = buf - start - 1;
	  ENSURE_NO_RELOCS (buf);
	  if (buf[0] == 'e' && buf[1] == 'h')
	    {
	      /* GCC < 3.0 .eh_frame CIE */
	      /* We cannot merge "eh" CIEs because __EXCEPTION_TABLE__
		 is private to each CIE, so we don't need it for anything.
		 Just skip it.  */
	      REQUIRE (skip_bytes (&buf, end, ptr_size));
	      SKIP_RELOCS (buf);
	    }
	  if (cie->version >= 4)
	    {
	      REQUIRE (buf + 1 < end);
	      REQUIRE (buf[0] == ptr_size);
	      REQUIRE (buf[1] == 0);
	      buf += 2;
	    }
	  REQUIRE (read_uleb128 (&buf, end, &cie->code_align));
	  REQUIRE (read_sleb128 (&buf, end, &cie->data_align));
	  if (cie->version == 1)
	    {
	      REQUIRE (buf < end);
	      cie->ra_column = *buf++;
	    }
	  else
	    REQUIRE (read_uleb128 (&buf, end, &cie->ra_column));
	  ENSURE_NO_RELOCS (buf);
	  cie->lsda_encoding = DW_EH_PE_omit;
	  cie->fde_encoding = DW_EH_PE_omit;
	  cie->per_encoding = DW_EH_PE_omit;
	  aug = cie->augmentation;
	  if (aug[0] != 'e' || aug[1] != 'h')
	    {
	      if (*aug == 'z')
		{
		  aug++;
		  REQUIRE (read_uleb128 (&buf, end, &cie->augmentation_size));
		  ENSURE_NO_RELOCS (buf);
		}

	      while (*aug != '\0')
		switch (*aug++)
		  {
		  case 'B':
		    break;
		  case 'L':
		    REQUIRE (read_byte (&buf, end, &cie->lsda_encoding));
		    ENSURE_NO_RELOCS (buf);
		    REQUIRE (get_DW_EH_PE_width (cie->lsda_encoding, ptr_size));
		    break;
		  case 'R':
		    REQUIRE (read_byte (&buf, end, &cie->fde_encoding));
		    ENSURE_NO_RELOCS (buf);
		    REQUIRE (get_DW_EH_PE_width (cie->fde_encoding, ptr_size));
		    break;
		  case 'S':
		    break;
		  case 'P':
		    {
		      int per_width;

		      REQUIRE (read_byte (&buf, end, &cie->per_encoding));
		      per_width = get_DW_EH_PE_width (cie->per_encoding,
						      ptr_size);
		      REQUIRE (per_width);
		      if ((cie->per_encoding & 0x70) == DW_EH_PE_aligned)
			{
			  length = -(buf - ehbuf) & (per_width - 1);
			  REQUIRE (skip_bytes (&buf, end, length));
			  if (per_width == 8)
			    this_inf->u.cie.per_encoding_aligned8 = 1;
			}
		      this_inf->u.cie.personality_offset = buf - start;
		      ENSURE_NO_RELOCS (buf);
		      /* Ensure we have a reloc here.  */
		      REQUIRE (GET_RELOC (buf));
		      cie->personality.reloc_index
			= cookie->rel - cookie->rels;
		      /* Cope with MIPS-style composite relocations.  */
		      do
			cookie->rel++;
		      while (GET_RELOC (buf) != NULL);
		      REQUIRE (skip_bytes (&buf, end, per_width));
		    }
		    break;
		  default:
		    /* Unrecognized augmentation. Better bail out.  */
		    goto free_no_table;
		  }
	    }
	  this_inf->u.cie.aug_data_len
	    = buf - start - 1 - this_inf->u.cie.aug_str_len;

	  /* For shared libraries, try to get rid of as many RELATIVE relocs
	     as possible.  */
	  if (bfd_link_pic (info)
	      && (get_elf_backend_data (abfd)
		  ->elf_backend_can_make_relative_eh_frame
		  (abfd, info, sec)))
	    {
	      if ((cie->fde_encoding & 0x70) == DW_EH_PE_absptr)
		this_inf->make_relative = 1;
	      /* If the CIE doesn't already have an 'R' entry, it's fairly
		 easy to add one, provided that there's no aligned data
		 after the augmentation string.  */
	      else if (cie->fde_encoding == DW_EH_PE_omit
		       && (cie->per_encoding & 0x70) != DW_EH_PE_aligned)
		{
		  if (*cie->augmentation == 0)
		    this_inf->add_augmentation_size = 1;
		  this_inf->u.cie.add_fde_encoding = 1;
		  this_inf->make_relative = 1;
		}

	      if ((cie->lsda_encoding & 0x70) == DW_EH_PE_absptr)
		cie->can_make_lsda_relative = 1;
	    }

	  /* If FDE encoding was not specified, it defaults to
	     DW_EH_absptr.  */
	  if (cie->fde_encoding == DW_EH_PE_omit)
	    cie->fde_encoding = DW_EH_PE_absptr;

	  initial_insn_length = end - buf;
	  cie->initial_insn_length = initial_insn_length;
	  memcpy (cie->initial_instructions, buf,
		  initial_insn_length <= sizeof (cie->initial_instructions)
		  ? initial_insn_length : sizeof (cie->initial_instructions));
	  insns = buf;
	  buf += initial_insn_length;
	  ENSURE_NO_RELOCS (buf);

	  if (!bfd_link_relocatable (info))
	    {
	      /* Keep info for merging cies.  */
	      this_inf->u.cie.u.full_cie = cie;
	      this_inf->u.cie.per_encoding_relative
		= (cie->per_encoding & 0x70) == DW_EH_PE_pcrel;
	    }
	}
      else
	{
	  /* Find the corresponding CIE.  */
	  unsigned int cie_offset = this_inf->offset + 4 - hdr_id;
	  for (cie = local_cies; cie < local_cies + cie_count; cie++)
	    if (cie_offset == cie->cie_inf->offset)
	      break;

	  /* Ensure this FDE references one of the CIEs in this input
	     section.  */
	  REQUIRE (cie != local_cies + cie_count);
	  this_inf->u.fde.cie_inf = cie->cie_inf;
	  this_inf->make_relative = cie->cie_inf->make_relative;
	  this_inf->add_augmentation_size
	    = cie->cie_inf->add_augmentation_size;

	  ENSURE_NO_RELOCS (buf);
	  if ((sec->flags & SEC_LINKER_CREATED) == 0 || cookie->rels != NULL)
	    {
	      asection *rsec;

	      REQUIRE (GET_RELOC (buf));

	      /* Chain together the FDEs for each section.  */
	      rsec = _bfd_elf_gc_mark_rsec (info, sec, gc_mark_hook,
					    cookie, NULL);
	      /* RSEC will be NULL if FDE was cleared out as it was belonging to
		 a discarded SHT_GROUP.  */
	      if (rsec)
		{
		  REQUIRE (rsec->owner == abfd);
		  this_inf->u.fde.next_for_section = elf_fde_list (rsec);
		  elf_fde_list (rsec) = this_inf;
		}
	    }

	  /* Skip the initial location and address range.  */
	  start = buf;
	  length = get_DW_EH_PE_width (cie->fde_encoding, ptr_size);
	  REQUIRE (skip_bytes (&buf, end, 2 * length));

	  SKIP_RELOCS (buf - length);
	  if (!GET_RELOC (buf - length)
	      && read_value (abfd, buf - length, length, false) == 0)
	    {
	      (*info->callbacks->minfo)
		/* xgettext:c-format */
		(_("discarding zero address range FDE in %pB(%pA).\n"),
		 abfd, sec);
	      this_inf->u.fde.cie_inf = NULL;
	    }

	  /* Skip the augmentation size, if present.  */
	  if (cie->augmentation[0] == 'z')
	    REQUIRE (read_uleb128 (&buf, end, &length));
	  else
	    length = 0;

	  /* Of the supported augmentation characters above, only 'L'
	     adds augmentation data to the FDE.  This code would need to
	     be adjusted if any future augmentations do the same thing.  */
	  if (cie->lsda_encoding != DW_EH_PE_omit)
	    {
	      SKIP_RELOCS (buf);
	      if (cie->can_make_lsda_relative && GET_RELOC (buf))
		cie->cie_inf->u.cie.make_lsda_relative = 1;
	      this_inf->lsda_offset = buf - start;
	      /* If there's no 'z' augmentation, we don't know where the
		 CFA insns begin.  Assume no padding.  */
	      if (cie->augmentation[0] != 'z')
		length = end - buf;
	    }

	  /* Skip over the augmentation data.  */
	  REQUIRE (skip_bytes (&buf, end, length));
	  insns = buf;

	  buf = last_fde + 4 + hdr_length;

	  /* For NULL RSEC (cleared FDE belonging to a discarded section)
	     the relocations are commonly cleared.  We do not sanity check if
	     all these relocations are cleared as (1) relocations to
	     .gcc_except_table will remain uncleared (they will get dropped
	     with the drop of this unused FDE) and (2) BFD already safely drops
	     relocations of any type to .eh_frame by
	     elf_section_ignore_discarded_relocs.
	     TODO: The .gcc_except_table entries should be also filtered as
	     .eh_frame entries; or GCC could rather use COMDAT for them.  */
	  SKIP_RELOCS (buf);
	}

      /* Try to interpret the CFA instructions and find the first
	 padding nop.  Shrink this_inf's size so that it doesn't
	 include the padding.  */
      length = get_DW_EH_PE_width (cie->fde_encoding, ptr_size);
      set_loc_count = 0;
      insns_end = skip_non_nops (insns, end, length, &set_loc_count);
      /* If we don't understand the CFA instructions, we can't know
	 what needs to be adjusted there.  */
      if (insns_end == NULL
	  /* For the time being we don't support DW_CFA_set_loc in
	     CIE instructions.  */
	  || (set_loc_count && this_inf->cie))
	goto free_no_table;
      this_inf->size -= end - insns_end;
      if (insns_end != end && this_inf->cie)
	{
	  cie->initial_insn_length -= end - insns_end;
	  cie->length -= end - insns_end;
	}
      if (set_loc_count
	  && ((cie->fde_encoding & 0x70) == DW_EH_PE_pcrel
	      || this_inf->make_relative))
	{
	  unsigned int cnt;
	  bfd_byte *p;

	  this_inf->set_loc = (unsigned int *)
	      bfd_malloc ((set_loc_count + 1) * sizeof (unsigned int));
	  REQUIRE (this_inf->set_loc);
	  this_inf->set_loc[0] = set_loc_count;
	  p = insns;
	  cnt = 0;
	  while (p < end)
	    {
	      if (*p == DW_CFA_set_loc)
		this_inf->set_loc[++cnt] = p + 1 - start;
	      REQUIRE (skip_cfa_op (&p, end, length));
	    }
	}

      this_inf->removed = 1;
      this_inf->fde_encoding = cie->fde_encoding;
      this_inf->lsda_encoding = cie->lsda_encoding;
      sec_info->count++;
    }
  BFD_ASSERT (sec_info->count == num_entries);
  BFD_ASSERT (cie_count == num_cies);

  elf_section_data (sec)->sec_info = sec_info;
  sec->sec_info_type = SEC_INFO_TYPE_EH_FRAME;
  if (!bfd_link_relocatable (info))
    {
      /* Keep info for merging cies.  */
      sec_info->cies = local_cies;
      local_cies = NULL;
    }
  goto success;

 free_no_table:
  _bfd_error_handler
    /* xgettext:c-format */
    (_("error in %pB(%pA); no .eh_frame_hdr table will be created"),
     abfd, sec);
  hdr_info->u.dwarf.table = false;
  free (sec_info);
 success:
  free (ehbuf);
  free (local_cies);
#undef REQUIRE
}

/* Order eh_frame_hdr entries by the VMA of their text section.  */

static int
cmp_eh_frame_hdr (const void *a, const void *b)
{
  bfd_vma text_a;
  bfd_vma text_b;
  asection *sec;

  sec = *(asection *const *)a;
  sec = (asection *) elf_section_data (sec)->sec_info;
  text_a = sec->output_section->vma + sec->output_offset;
  sec = *(asection *const *)b;
  sec = (asection *) elf_section_data (sec)->sec_info;
  text_b = sec->output_section->vma + sec->output_offset;

  if (text_a < text_b)
    return -1;
  return text_a > text_b;

}

/* Add space for a CANTUNWIND terminator to SEC if the text sections
   referenced by it and NEXT are not contiguous, or NEXT is NULL.  */

static void
add_eh_frame_hdr_terminator (asection *sec,
			     asection *next)
{
  bfd_vma end;
  bfd_vma next_start;
  asection *text_sec;

  if (next)
    {
      /* See if there is a gap (presumably a text section without unwind info)
	 between these two entries.  */
      text_sec = (asection *) elf_section_data (sec)->sec_info;
      end = text_sec->output_section->vma + text_sec->output_offset
	    + text_sec->size;
      text_sec = (asection *) elf_section_data (next)->sec_info;
      next_start = text_sec->output_section->vma + text_sec->output_offset;
      if (end == next_start)
	return;
    }

  /* Add space for a CANTUNWIND terminator.  */
  if (!sec->rawsize)
    sec->rawsize = sec->size;

  bfd_set_section_size (sec, sec->size + 8);
}

/* Finish a pass over all .eh_frame_entry sections.  */

bool
_bfd_elf_end_eh_frame_parsing (struct bfd_link_info *info)
{
  struct eh_frame_hdr_info *hdr_info;
  unsigned int i;

  hdr_info = &elf_hash_table (info)->eh_info;

  if (info->eh_frame_hdr_type != COMPACT_EH_HDR
      || hdr_info->array_count == 0)
    return false;

  bfd_elf_discard_eh_frame_entry (hdr_info);

  qsort (hdr_info->u.compact.entries, hdr_info->array_count,
	 sizeof (asection *), cmp_eh_frame_hdr);

  for (i = 0; i < hdr_info->array_count - 1; i++)
    {
      add_eh_frame_hdr_terminator (hdr_info->u.compact.entries[i],
				   hdr_info->u.compact.entries[i + 1]);
    }

  /* Add a CANTUNWIND terminator after the last entry.  */
  add_eh_frame_hdr_terminator (hdr_info->u.compact.entries[i], NULL);
  return true;
}

/* Mark all relocations against CIE or FDE ENT, which occurs in
   .eh_frame section SEC.  COOKIE describes the relocations in SEC;
   its "rel" field can be changed freely.  */

static bool
mark_entry (struct bfd_link_info *info, asection *sec,
	    struct eh_cie_fde *ent, elf_gc_mark_hook_fn gc_mark_hook,
	    struct elf_reloc_cookie *cookie)
{
  /* FIXME: octets_per_byte.  */
  for (cookie->rel = cookie->rels + ent->reloc_index;
       cookie->rel < cookie->relend
	 && cookie->rel->r_offset < ent->offset + ent->size;
       cookie->rel++)
    if (!_bfd_elf_gc_mark_reloc (info, sec, gc_mark_hook, cookie))
      return false;

  return true;
}

/* Mark all the relocations against FDEs that relate to code in input
   section SEC.  The FDEs belong to .eh_frame section EH_FRAME, whose
   relocations are described by COOKIE.  */

bool
_bfd_elf_gc_mark_fdes (struct bfd_link_info *info, asection *sec,
		       asection *eh_frame, elf_gc_mark_hook_fn gc_mark_hook,
		       struct elf_reloc_cookie *cookie)
{
  struct eh_cie_fde *fde, *cie;

  for (fde = elf_fde_list (sec); fde; fde = fde->u.fde.next_for_section)
    {
      if (!mark_entry (info, eh_frame, fde, gc_mark_hook, cookie))
	return false;

      /* At this stage, all cie_inf fields point to local CIEs, so we
	 can use the same cookie to refer to them.  */
      cie = fde->u.fde.cie_inf;
      if (cie != NULL && !cie->u.cie.gc_mark)
	{
	  cie->u.cie.gc_mark = 1;
	  if (!mark_entry (info, eh_frame, cie, gc_mark_hook, cookie))
	    return false;
	}
    }
  return true;
}

/* Input section SEC of ABFD is an .eh_frame section that contains the
   CIE described by CIE_INF.  Return a version of CIE_INF that is going
   to be kept in the output, adding CIE_INF to the output if necessary.

   HDR_INFO is the .eh_frame_hdr information and COOKIE describes the
   relocations in REL.  */

static struct eh_cie_fde *
find_merged_cie (bfd *abfd, struct bfd_link_info *info, asection *sec,
		 struct eh_frame_hdr_info *hdr_info,
		 struct elf_reloc_cookie *cookie,
		 struct eh_cie_fde *cie_inf)
{
  unsigned long r_symndx;
  struct cie *cie, *new_cie;
  Elf_Internal_Rela *rel;
  void **loc;

  /* Use CIE_INF if we have already decided to keep it.  */
  if (!cie_inf->removed)
    return cie_inf;

  /* If we have merged CIE_INF with another CIE, use that CIE instead.  */
  if (cie_inf->u.cie.merged)
    return cie_inf->u.cie.u.merged_with;

  cie = cie_inf->u.cie.u.full_cie;

  /* Assume we will need to keep CIE_INF.  */
  cie_inf->removed = 0;
  cie_inf->u.cie.u.sec = sec;

  /* If we are not merging CIEs, use CIE_INF.  */
  if (cie == NULL)
    return cie_inf;

  if (cie->per_encoding != DW_EH_PE_omit)
    {
      bool per_binds_local;

      /* Work out the address of personality routine, or at least
	 enough info that we could calculate the address had we made a
	 final section layout.  The symbol on the reloc is enough,
	 either the hash for a global, or (bfd id, index) pair for a
	 local.  The assumption here is that no one uses addends on
	 the reloc.  */
      rel = cookie->rels + cie->personality.reloc_index;
      memset (&cie->personality, 0, sizeof (cie->personality));
#ifdef BFD64
      if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64)
	r_symndx = ELF64_R_SYM (rel->r_info);
      else
#endif
	r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx >= cookie->locsymcount
	  || ELF_ST_BIND (cookie->locsyms[r_symndx].st_info) != STB_LOCAL)
	{
	  struct elf_link_hash_entry *h;

	  r_symndx -= cookie->extsymoff;
	  h = cookie->sym_hashes[r_symndx];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  cie->personality.h = h;
	  per_binds_local = SYMBOL_REFERENCES_LOCAL (info, h);
	}
      else
	{
	  Elf_Internal_Sym *sym;
	  asection *sym_sec;

	  sym = &cookie->locsyms[r_symndx];
	  sym_sec = bfd_section_from_elf_index (abfd, sym->st_shndx);
	  if (sym_sec == NULL)
	    return cie_inf;

	  if (sym_sec->kept_section != NULL)
	    sym_sec = sym_sec->kept_section;
	  if (sym_sec->output_section == NULL)
	    return cie_inf;

	  cie->local_personality = 1;
	  cie->personality.sym.bfd_id = abfd->id;
	  cie->personality.sym.index = r_symndx;
	  per_binds_local = true;
	}

      if (per_binds_local
	  && bfd_link_pic (info)
	  && (cie->per_encoding & 0x70) == DW_EH_PE_absptr
	  && (get_elf_backend_data (abfd)
	      ->elf_backend_can_make_relative_eh_frame (abfd, info, sec)))
	{
	  cie_inf->u.cie.make_per_encoding_relative = 1;
	  cie_inf->u.cie.per_encoding_relative = 1;
	}
    }

  /* See if we can merge this CIE with an earlier one.  */
  cie_compute_hash (cie);
  if (hdr_info->u.dwarf.cies == NULL)
    {
      hdr_info->u.dwarf.cies = htab_try_create (1, cie_hash, cie_eq, free);
      if (hdr_info->u.dwarf.cies == NULL)
	return cie_inf;
    }
  loc = htab_find_slot_with_hash (hdr_info->u.dwarf.cies, cie,
				  cie->hash, INSERT);
  if (loc == NULL)
    return cie_inf;

  new_cie = (struct cie *) *loc;
  if (new_cie == NULL)
    {
      /* Keep CIE_INF and record it in the hash table.  */
      new_cie = (struct cie *) malloc (sizeof (struct cie));
      if (new_cie == NULL)
	return cie_inf;

      memcpy (new_cie, cie, sizeof (struct cie));
      *loc = new_cie;
    }
  else
    {
      /* Merge CIE_INF with NEW_CIE->CIE_INF.  */
      cie_inf->removed = 1;
      cie_inf->u.cie.merged = 1;
      cie_inf->u.cie.u.merged_with = new_cie->cie_inf;
      if (cie_inf->u.cie.make_lsda_relative)
	new_cie->cie_inf->u.cie.make_lsda_relative = 1;
    }
  return new_cie->cie_inf;
}

/* For a given OFFSET in SEC, return the delta to the new location
   after .eh_frame editing.  */

static bfd_signed_vma
offset_adjust (bfd_vma offset, const asection *sec)
{
  struct eh_frame_sec_info *sec_info
    = (struct eh_frame_sec_info *) elf_section_data (sec)->sec_info;
  unsigned int lo, hi, mid;
  struct eh_cie_fde *ent = NULL;
  bfd_signed_vma delta;

  lo = 0;
  hi = sec_info->count;
  if (hi == 0)
    return 0;

  while (lo < hi)
    {
      mid = (lo + hi) / 2;
      ent = &sec_info->entry[mid];
      if (offset < ent->offset)
	hi = mid;
      else if (mid + 1 >= hi)
	break;
      else if (offset >= ent[1].offset)
	lo = mid + 1;
      else
	break;
    }

  if (!ent->removed)
    delta = (bfd_vma) ent->new_offset - (bfd_vma) ent->offset;
  else if (ent->cie && ent->u.cie.merged)
    {
      struct eh_cie_fde *cie = ent->u.cie.u.merged_with;
      delta = ((bfd_vma) cie->new_offset + cie->u.cie.u.sec->output_offset
	       - (bfd_vma) ent->offset - sec->output_offset);
    }
  else
    {
      /* Is putting the symbol on the next entry best for a deleted
	 CIE/FDE?  */
      struct eh_cie_fde *last = sec_info->entry + sec_info->count;
      delta = ((bfd_vma) next_cie_fde_offset (ent, last, sec)
	       - (bfd_vma) ent->offset);
      return delta;
    }

  /* Account for editing within this CIE/FDE.  */
  offset -= ent->offset;
  if (ent->cie)
    {
      unsigned int extra
	= ent->add_augmentation_size + ent->u.cie.add_fde_encoding;
      if (extra == 0
	  || offset <= 9u + ent->u.cie.aug_str_len)
	return delta;
      delta += extra;
      if (offset <= 9u + ent->u.cie.aug_str_len + ent->u.cie.aug_data_len)
	return delta;
      delta += extra;
    }
  else
    {
      unsigned int ptr_size, width, extra = ent->add_augmentation_size;
      if (offset <= 12 || extra == 0)
	return delta;
      ptr_size = (get_elf_backend_data (sec->owner)
		  ->elf_backend_eh_frame_address_size (sec->owner, sec));
      width = get_DW_EH_PE_width (ent->fde_encoding, ptr_size);
      if (offset <= 8 + 2 * width)
	return delta;
      delta += extra;
    }

  return delta;
}

/* Adjust a global symbol defined in .eh_frame, so that it stays
   relative to its original CIE/FDE.  It is assumed that a symbol
   defined at the beginning of a CIE/FDE belongs to that CIE/FDE
   rather than marking the end of the previous CIE/FDE.  This matters
   when a CIE is merged with a previous CIE, since the symbol is
   moved to the merged CIE.  */

bool
_bfd_elf_adjust_eh_frame_global_symbol (struct elf_link_hash_entry *h,
					void *arg ATTRIBUTE_UNUSED)
{
  asection *sym_sec;
  bfd_signed_vma delta;

  if (h->root.type != bfd_link_hash_defined
      && h->root.type != bfd_link_hash_defweak)
    return true;

  sym_sec = h->root.u.def.section;
  if (sym_sec->sec_info_type != SEC_INFO_TYPE_EH_FRAME
      || elf_section_data (sym_sec)->sec_info == NULL)
    return true;

  delta = offset_adjust (h->root.u.def.value, sym_sec);
  h->root.u.def.value += delta;

  return true;
}

/* The same for all local symbols defined in .eh_frame.  Returns true
   if any symbol was changed.  */

static int
adjust_eh_frame_local_symbols (const asection *sec,
			       struct elf_reloc_cookie *cookie)
{
  int adjusted = 0;

  if (cookie->locsymcount > 1)
    {
      unsigned int shndx = elf_section_data (sec)->this_idx;
      Elf_Internal_Sym *end_sym = cookie->locsyms + cookie->locsymcount;
      Elf_Internal_Sym *sym;

      for (sym = cookie->locsyms + 1; sym < end_sym; ++sym)
	if (sym->st_info <= ELF_ST_INFO (STB_LOCAL, STT_OBJECT)
	    && sym->st_shndx == shndx)
	  {
	    bfd_signed_vma delta = offset_adjust (sym->st_value, sec);

	    if (delta != 0)
	      {
		adjusted = 1;
		sym->st_value += delta;
	      }
	  }
    }
  return adjusted;
}

/* This function is called for each input file before the .eh_frame
   section is relocated.  It discards duplicate CIEs and FDEs for discarded
   functions.  The function returns TRUE iff any entries have been
   deleted.  */

bool
_bfd_elf_discard_section_eh_frame
   (bfd *abfd, struct bfd_link_info *info, asection *sec,
    bool (*reloc_symbol_deleted_p) (bfd_vma, void *),
    struct elf_reloc_cookie *cookie)
{
  struct eh_cie_fde *ent;
  struct eh_frame_sec_info *sec_info;
  struct eh_frame_hdr_info *hdr_info;
  unsigned int ptr_size, offset, eh_alignment;
  int changed;

  if (sec->sec_info_type != SEC_INFO_TYPE_EH_FRAME)
    return false;

  sec_info = (struct eh_frame_sec_info *) elf_section_data (sec)->sec_info;
  if (sec_info == NULL)
    return false;

  ptr_size = (get_elf_backend_data (sec->owner)
	      ->elf_backend_eh_frame_address_size (sec->owner, sec));

  hdr_info = &elf_hash_table (info)->eh_info;
  for (ent = sec_info->entry; ent < sec_info->entry + sec_info->count; ++ent)
    if (ent->size == 4)
      /* There should only be one zero terminator, on the last input
	 file supplying .eh_frame (crtend.o).  Remove any others.  */
      ent->removed = sec->map_head.s != NULL;
    else if (!ent->cie && ent->u.fde.cie_inf != NULL)
      {
	bool keep;
	if ((sec->flags & SEC_LINKER_CREATED) != 0 && cookie->rels == NULL)
	  {
	    unsigned int width
	      = get_DW_EH_PE_width (ent->fde_encoding, ptr_size);
	    bfd_vma value
	      = read_value (abfd, sec->contents + ent->offset + 8 + width,
			    width, get_DW_EH_PE_signed (ent->fde_encoding));
	    keep = value != 0;
	  }
	else
	  {
	    cookie->rel = cookie->rels + ent->reloc_index;
	    /* FIXME: octets_per_byte.  */
	    BFD_ASSERT (cookie->rel < cookie->relend
			&& cookie->rel->r_offset == ent->offset + 8);
	    keep = !(*reloc_symbol_deleted_p) (ent->offset + 8, cookie);
	  }
	if (keep)
	  {
	    if (bfd_link_pic (info)
		&& (((ent->fde_encoding & 0x70) == DW_EH_PE_absptr
		     && ent->make_relative == 0)
		    || (ent->fde_encoding & 0x70) == DW_EH_PE_aligned))
	      {
		static int num_warnings_issued = 0;

		/* If a shared library uses absolute pointers
		   which we cannot turn into PC relative,
		   don't create the binary search table,
		   since it is affected by runtime relocations.  */
		hdr_info->u.dwarf.table = false;
		/* Only warn if --eh-frame-hdr was specified.  */
		if (info->eh_frame_hdr_type != 0)
		  {
		    if (num_warnings_issued < 10)
		      {
			_bfd_error_handler
			  /* xgettext:c-format */
			  (_("FDE encoding in %pB(%pA) prevents .eh_frame_hdr"
			     " table being created"), abfd, sec);
			num_warnings_issued ++;
		      }
		    else if (num_warnings_issued == 10)
		      {
			_bfd_error_handler
			  (_("further warnings about FDE encoding preventing .eh_frame_hdr generation dropped"));
			num_warnings_issued ++;
		      }
		  }
	      }
	    ent->removed = 0;
	    hdr_info->u.dwarf.fde_count++;
	    ent->u.fde.cie_inf = find_merged_cie (abfd, info, sec, hdr_info,
						  cookie, ent->u.fde.cie_inf);
	  }
      }

  free (sec_info->cies);
  sec_info->cies = NULL;

  /* It may be that some .eh_frame input section has greater alignment
     than other .eh_frame sections.  In that case we run the risk of
     padding with zeros before that section, which would be seen as a
     zero terminator.  Alignment padding must be added *inside* the
     last FDE instead.  For other FDEs we align according to their
     encoding, in order to align FDE address range entries naturally.  */
  offset = 0;
  changed = 0;
  for (ent = sec_info->entry; ent < sec_info->entry + sec_info->count; ++ent)
    if (!ent->removed)
      {
	eh_alignment = 4;
	if (ent->size == 4)
	  ;
	else if (ent->cie)
	  {
	    if (ent->u.cie.per_encoding_aligned8)
	      eh_alignment = 8;
	  }
	else
	  {
	    eh_alignment = get_DW_EH_PE_width (ent->fde_encoding, ptr_size);
	    if (eh_alignment < 4)
	      eh_alignment = 4;
	  }
	offset = (offset + eh_alignment - 1) & -eh_alignment;
	ent->new_offset = offset;
	if (ent->new_offset != ent->offset)
	  changed = 1;
	offset += size_of_output_cie_fde (ent);
      }

  eh_alignment = 4;
  offset = (offset + eh_alignment - 1) & -eh_alignment;
  sec->rawsize = sec->size;
  sec->size = offset;
  if (sec->size != sec->rawsize)
    changed = 1;

  if (changed && adjust_eh_frame_local_symbols (sec, cookie))
    {
      Elf_Internal_Shdr *symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
      symtab_hdr->contents = (unsigned char *) cookie->locsyms;
    }
  return changed;
}

/* This function is called for .eh_frame_hdr section after
   _bfd_elf_discard_section_eh_frame has been called on all .eh_frame
   input sections.  It finalizes the size of .eh_frame_hdr section.  */

bool
_bfd_elf_discard_section_eh_frame_hdr (struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  asection *sec;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;

  if (!hdr_info->frame_hdr_is_compact && hdr_info->u.dwarf.cies != NULL)
    {
      htab_delete (hdr_info->u.dwarf.cies);
      hdr_info->u.dwarf.cies = NULL;
    }

  sec = hdr_info->hdr_sec;
  if (sec == NULL)
    return false;

  if (info->eh_frame_hdr_type == COMPACT_EH_HDR)
    {
      /* For compact frames we only add the header.  The actual table comes
	 from the .eh_frame_entry sections.  */
      sec->size = 8;
    }
  else
    {
      sec->size = EH_FRAME_HDR_SIZE;
      if (hdr_info->u.dwarf.table)
	sec->size += 4 + hdr_info->u.dwarf.fde_count * 8;
    }

  return true;
}

/* Return true if there is at least one non-empty .eh_frame section in
   input files.  Can only be called after ld has mapped input to
   output sections, and before sections are stripped.  */

bool
_bfd_elf_eh_frame_present (struct bfd_link_info *info)
{
  asection *eh = bfd_get_section_by_name (info->output_bfd, ".eh_frame");

  if (eh == NULL)
    return false;

  /* Count only sections which have at least a single CIE or FDE.
     There cannot be any CIE or FDE <= 8 bytes.  */
  for (eh = eh->map_head.s; eh != NULL; eh = eh->map_head.s)
    if (eh->size > 8)
      return true;

  return false;
}

/* Return true if there is at least one .eh_frame_entry section in
   input files.  */

bool
_bfd_elf_eh_frame_entry_present (struct bfd_link_info *info)
{
  asection *o;
  bfd *abfd;

  for (abfd = info->input_bfds; abfd != NULL; abfd = abfd->link.next)
    {
      for (o = abfd->sections; o; o = o->next)
	{
	  const char *name = bfd_section_name (o);

	  if (strcmp (name, ".eh_frame_entry")
	      && !bfd_is_abs_section (o->output_section))
	    return true;
	}
    }
  return false;
}

/* This function is called from size_dynamic_sections.
   It needs to decide whether .eh_frame_hdr should be output or not,
   because when the dynamic symbol table has been sized it is too late
   to strip sections.  */

bool
_bfd_elf_maybe_strip_eh_frame_hdr (struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  struct bfd_link_hash_entry *bh = NULL;
  struct elf_link_hash_entry *h;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;
  if (hdr_info->hdr_sec == NULL)
    return true;

  if (bfd_is_abs_section (hdr_info->hdr_sec->output_section)
      || info->eh_frame_hdr_type == 0
      || (info->eh_frame_hdr_type == DWARF2_EH_HDR
	  && !_bfd_elf_eh_frame_present (info))
      || (info->eh_frame_hdr_type == COMPACT_EH_HDR
	  && !_bfd_elf_eh_frame_entry_present (info)))
    {
      hdr_info->hdr_sec->flags |= SEC_EXCLUDE;
      hdr_info->hdr_sec = NULL;
      return true;
    }

  /* Add a hidden symbol so that systems without access to PHDRs can
     find the table.  */
  if (! (_bfd_generic_link_add_one_symbol
	 (info, info->output_bfd, "__GNU_EH_FRAME_HDR", BSF_LOCAL,
	  hdr_info->hdr_sec, 0, NULL, false, false, &bh)))
    return false;

  h = (struct elf_link_hash_entry *) bh;
  h->def_regular = 1;
  h->other = STV_HIDDEN;
  get_elf_backend_data
    (info->output_bfd)->elf_backend_hide_symbol (info, h, true);

  if (!hdr_info->frame_hdr_is_compact)
    hdr_info->u.dwarf.table = true;
  return true;
}

/* Adjust an address in the .eh_frame section.  Given OFFSET within
   SEC, this returns the new offset in the adjusted .eh_frame section,
   or -1 if the address refers to a CIE/FDE which has been removed
   or to offset with dynamic relocation which is no longer needed.  */

bfd_vma
_bfd_elf_eh_frame_section_offset (bfd *output_bfd ATTRIBUTE_UNUSED,
				  struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  asection *sec,
				  bfd_vma offset)
{
  struct eh_frame_sec_info *sec_info;
  unsigned int lo, hi, mid;

  if (sec->sec_info_type != SEC_INFO_TYPE_EH_FRAME)
    return offset;
  sec_info = (struct eh_frame_sec_info *) elf_section_data (sec)->sec_info;

  if (offset >= sec->rawsize)
    return offset - sec->rawsize + sec->size;

  lo = 0;
  hi = sec_info->count;
  mid = 0;
  while (lo < hi)
    {
      mid = (lo + hi) / 2;
      if (offset < sec_info->entry[mid].offset)
	hi = mid;
      else if (offset
	       >= sec_info->entry[mid].offset + sec_info->entry[mid].size)
	lo = mid + 1;
      else
	break;
    }

  BFD_ASSERT (lo < hi);

  /* FDE or CIE was removed.  */
  if (sec_info->entry[mid].removed)
    return (bfd_vma) -1;

  /* If converting personality pointers to DW_EH_PE_pcrel, there will be
     no need for run-time relocation against the personality field.  */
  if (sec_info->entry[mid].cie
      && sec_info->entry[mid].u.cie.make_per_encoding_relative
      && offset == (sec_info->entry[mid].offset + 8
		    + sec_info->entry[mid].u.cie.personality_offset))
    return (bfd_vma) -2;

  /* If converting to DW_EH_PE_pcrel, there will be no need for run-time
     relocation against FDE's initial_location field.  */
  if (!sec_info->entry[mid].cie
      && sec_info->entry[mid].make_relative
      && offset == sec_info->entry[mid].offset + 8)
    return (bfd_vma) -2;

  /* If converting LSDA pointers to DW_EH_PE_pcrel, there will be no need
     for run-time relocation against LSDA field.  */
  if (!sec_info->entry[mid].cie
      && sec_info->entry[mid].u.fde.cie_inf->u.cie.make_lsda_relative
      && offset == (sec_info->entry[mid].offset + 8
		    + sec_info->entry[mid].lsda_offset))
    return (bfd_vma) -2;

  /* If converting to DW_EH_PE_pcrel, there will be no need for run-time
     relocation against DW_CFA_set_loc's arguments.  */
  if (sec_info->entry[mid].set_loc
      && sec_info->entry[mid].make_relative
      && (offset >= sec_info->entry[mid].offset + 8
		    + sec_info->entry[mid].set_loc[1]))
    {
      unsigned int cnt;

      for (cnt = 1; cnt <= sec_info->entry[mid].set_loc[0]; cnt++)
	if (offset == sec_info->entry[mid].offset + 8
		      + sec_info->entry[mid].set_loc[cnt])
	  return (bfd_vma) -2;
    }

  /* Any new augmentation bytes go before the first relocation.  */
  return (offset + sec_info->entry[mid].new_offset
	  - sec_info->entry[mid].offset
	  + extra_augmentation_string_bytes (sec_info->entry + mid)
	  + extra_augmentation_data_bytes (sec_info->entry + mid));
}

/* Write out .eh_frame_entry section.  Add CANTUNWIND terminator if needed.
   Also check that the contents look sane.  */

bool
_bfd_elf_write_section_eh_frame_entry (bfd *abfd, struct bfd_link_info *info,
				       asection *sec, bfd_byte *contents)
{
  const struct elf_backend_data *bed;
  bfd_byte cantunwind[8];
  bfd_vma addr;
  bfd_vma last_addr;
  bfd_vma offset;
  asection *text_sec = (asection *) elf_section_data (sec)->sec_info;

  if (!sec->rawsize)
    sec->rawsize = sec->size;

  BFD_ASSERT (sec->sec_info_type == SEC_INFO_TYPE_EH_FRAME_ENTRY);

  /* Check to make sure that the text section corresponding to this eh_frame_entry
     section has not been excluded.  In particular, mips16 stub entries will be
     excluded outside of the normal process.  */
  if (sec->flags & SEC_EXCLUDE
      || text_sec->flags & SEC_EXCLUDE)
    return true;

  if (!bfd_set_section_contents (abfd, sec->output_section, contents,
				 sec->output_offset, sec->rawsize))
      return false;

  last_addr = bfd_get_signed_32 (abfd, contents);
  /* Check that all the entries are in order.  */
  for (offset = 8; offset < sec->rawsize; offset += 8)
    {
      addr = bfd_get_signed_32 (abfd, contents + offset) + offset;
      if (addr <= last_addr)
	{
	  /* xgettext:c-format */
	  _bfd_error_handler (_("%pB: %pA not in order"), sec->owner, sec);
	  return false;
	}

      last_addr = addr;
    }

  addr = text_sec->output_section->vma + text_sec->output_offset
	 + text_sec->size;
  addr &= ~1;
  addr -= (sec->output_section->vma + sec->output_offset + sec->rawsize);
  if (addr & 1)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: %pA invalid input section size"),
			  sec->owner, sec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }
  if (last_addr >= addr + sec->rawsize)
    {
      /* xgettext:c-format */
      _bfd_error_handler (_("%pB: %pA points past end of text section"),
			  sec->owner, sec);
      bfd_set_error (bfd_error_bad_value);
      return false;
    }

  if (sec->size == sec->rawsize)
    return true;

  bed = get_elf_backend_data (abfd);
  BFD_ASSERT (sec->size == sec->rawsize + 8);
  BFD_ASSERT ((addr & 1) == 0);
  BFD_ASSERT (bed->cant_unwind_opcode);

  bfd_put_32 (abfd, addr, cantunwind);
  bfd_put_32 (abfd, (*bed->cant_unwind_opcode) (info), cantunwind + 4);
  return bfd_set_section_contents (abfd, sec->output_section, cantunwind,
				   sec->output_offset + sec->rawsize, 8);
}

/* Write out .eh_frame section.  This is called with the relocated
   contents.  */

bool
_bfd_elf_write_section_eh_frame (bfd *abfd,
				 struct bfd_link_info *info,
				 asection *sec,
				 bfd_byte *contents)
{
  struct eh_frame_sec_info *sec_info;
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  unsigned int ptr_size;
  struct eh_cie_fde *ent, *last_ent;

  if (sec->sec_info_type != SEC_INFO_TYPE_EH_FRAME)
    /* FIXME: octets_per_byte.  */
    return bfd_set_section_contents (abfd, sec->output_section, contents,
				     sec->output_offset, sec->size);

  ptr_size = (get_elf_backend_data (abfd)
	      ->elf_backend_eh_frame_address_size (abfd, sec));
  BFD_ASSERT (ptr_size != 0);

  sec_info = (struct eh_frame_sec_info *) elf_section_data (sec)->sec_info;
  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;

  if (hdr_info->u.dwarf.table && hdr_info->u.dwarf.array == NULL)
    {
      hdr_info->frame_hdr_is_compact = false;
      hdr_info->u.dwarf.array = (struct eh_frame_array_ent *)
	bfd_malloc (hdr_info->u.dwarf.fde_count
		    * sizeof (*hdr_info->u.dwarf.array));
    }
  if (hdr_info->u.dwarf.array == NULL)
    hdr_info = NULL;

  /* The new offsets can be bigger or smaller than the original offsets.
     We therefore need to make two passes over the section: one backward
     pass to move entries up and one forward pass to move entries down.
     The two passes won't interfere with each other because entries are
     not reordered  */
  for (ent = sec_info->entry + sec_info->count; ent-- != sec_info->entry;)
    if (!ent->removed && ent->new_offset > ent->offset)
      memmove (contents + ent->new_offset, contents + ent->offset, ent->size);

  for (ent = sec_info->entry; ent < sec_info->entry + sec_info->count; ++ent)
    if (!ent->removed && ent->new_offset < ent->offset)
      memmove (contents + ent->new_offset, contents + ent->offset, ent->size);

  last_ent = sec_info->entry + sec_info->count;
  for (ent = sec_info->entry; ent < last_ent; ++ent)
    {
      unsigned char *buf, *end;
      unsigned int new_size;

      if (ent->removed)
	continue;

      if (ent->size == 4)
	{
	  /* Any terminating FDE must be at the end of the section.  */
	  BFD_ASSERT (ent == last_ent - 1);
	  continue;
	}

      buf = contents + ent->new_offset;
      end = buf + ent->size;
      new_size = next_cie_fde_offset (ent, last_ent, sec) - ent->new_offset;

      /* Update the size.  It may be shrinked.  */
      bfd_put_32 (abfd, new_size - 4, buf);

      /* Filling the extra bytes with DW_CFA_nops.  */
      if (new_size != ent->size)
	memset (end, 0, new_size - ent->size);

      if (ent->cie)
	{
	  /* CIE */
	  if (ent->make_relative
	      || ent->u.cie.make_lsda_relative
	      || ent->u.cie.per_encoding_relative)
	    {
	      char *aug;
	      unsigned int version, action, extra_string, extra_data;
	      unsigned int per_width, per_encoding;

	      /* Need to find 'R' or 'L' augmentation's argument and modify
		 DW_EH_PE_* value.  */
	      action = ((ent->make_relative ? 1 : 0)
			| (ent->u.cie.make_lsda_relative ? 2 : 0)
			| (ent->u.cie.per_encoding_relative ? 4 : 0));
	      extra_string = extra_augmentation_string_bytes (ent);
	      extra_data = extra_augmentation_data_bytes (ent);

	      /* Skip length, id.  */
	      buf += 8;
	      version = *buf++;
	      aug = (char *) buf;
	      buf += strlen (aug) + 1;
	      skip_leb128 (&buf, end);
	      skip_leb128 (&buf, end);
	      if (version == 1)
		skip_bytes (&buf, end, 1);
	      else
		skip_leb128 (&buf, end);
	      if (*aug == 'z')
		{
		  /* The uleb128 will always be a single byte for the kind
		     of augmentation strings that we're prepared to handle.  */
		  *buf++ += extra_data;
		  aug++;
		}

	      /* Make room for the new augmentation string and data bytes.  */
	      memmove (buf + extra_string + extra_data, buf, end - buf);
	      memmove (aug + extra_string, aug, buf - (bfd_byte *) aug);
	      buf += extra_string;
	      end += extra_string + extra_data;

	      if (ent->add_augmentation_size)
		{
		  *aug++ = 'z';
		  *buf++ = extra_data - 1;
		}
	      if (ent->u.cie.add_fde_encoding)
		{
		  BFD_ASSERT (action & 1);
		  *aug++ = 'R';
		  *buf++ = make_pc_relative (DW_EH_PE_absptr, ptr_size);
		  action &= ~1;
		}

	      while (action)
		switch (*aug++)
		  {
		  case 'L':
		    if (action & 2)
		      {
			BFD_ASSERT (*buf == ent->lsda_encoding);
			*buf = make_pc_relative (*buf, ptr_size);
			action &= ~2;
		      }
		    buf++;
		    break;
		  case 'P':
		    if (ent->u.cie.make_per_encoding_relative)
		      *buf = make_pc_relative (*buf, ptr_size);
		    per_encoding = *buf++;
		    per_width = get_DW_EH_PE_width (per_encoding, ptr_size);
		    BFD_ASSERT (per_width != 0);
		    BFD_ASSERT (((per_encoding & 0x70) == DW_EH_PE_pcrel)
				== ent->u.cie.per_encoding_relative);
		    if ((per_encoding & 0x70) == DW_EH_PE_aligned)
		      buf = (contents
			     + ((buf - contents + per_width - 1)
				& ~((bfd_size_type) per_width - 1)));
		    if (action & 4)
		      {
			bfd_vma val;

			val = read_value (abfd, buf, per_width,
					  get_DW_EH_PE_signed (per_encoding));
			if (ent->u.cie.make_per_encoding_relative)
			  val -= (sec->output_section->vma
				  + sec->output_offset
				  + (buf - contents));
			else
			  {
			    val += (bfd_vma) ent->offset - ent->new_offset;
			    val -= extra_string + extra_data;
			  }
			write_value (abfd, buf, val, per_width);
			action &= ~4;
		      }
		    buf += per_width;
		    break;
		  case 'R':
		    if (action & 1)
		      {
			BFD_ASSERT (*buf == ent->fde_encoding);
			*buf = make_pc_relative (*buf, ptr_size);
			action &= ~1;
		      }
		    buf++;
		    break;
		  case 'S':
		    break;
		  default:
		    BFD_FAIL ();
		  }
	    }
	}
      else
	{
	  /* FDE */
	  bfd_vma value, address;
	  unsigned int width;
	  bfd_byte *start;
	  struct eh_cie_fde *cie;

	  /* Skip length.  */
	  cie = ent->u.fde.cie_inf;
	  buf += 4;
	  value = ((ent->new_offset + sec->output_offset + 4)
		   - (cie->new_offset + cie->u.cie.u.sec->output_offset));
	  bfd_put_32 (abfd, value, buf);
	  if (bfd_link_relocatable (info))
	    continue;
	  buf += 4;
	  width = get_DW_EH_PE_width (ent->fde_encoding, ptr_size);
	  value = read_value (abfd, buf, width,
			      get_DW_EH_PE_signed (ent->fde_encoding));
	  address = value;
	  if (value)
	    {
	      switch (ent->fde_encoding & 0x70)
		{
		case DW_EH_PE_textrel:
		  BFD_ASSERT (hdr_info == NULL);
		  break;
		case DW_EH_PE_datarel:
		  {
		    switch (abfd->arch_info->arch)
		      {
		      case bfd_arch_ia64:
			BFD_ASSERT (elf_gp (abfd) != 0);
			address += elf_gp (abfd);
			break;
		      default:
			_bfd_error_handler
			  (_("DW_EH_PE_datarel unspecified"
			     " for this architecture"));
			/* Fall thru */
		      case bfd_arch_frv:
		      case bfd_arch_i386:
		      case bfd_arch_nios2:
			BFD_ASSERT (htab->hgot != NULL
				    && ((htab->hgot->root.type
					 == bfd_link_hash_defined)
					|| (htab->hgot->root.type
					    == bfd_link_hash_defweak)));
			address
			  += (htab->hgot->root.u.def.value
			      + htab->hgot->root.u.def.section->output_offset
			      + (htab->hgot->root.u.def.section->output_section
				 ->vma));
			break;
		      }
		  }
		  break;
		case DW_EH_PE_pcrel:
		  value += (bfd_vma) ent->offset - ent->new_offset;
		  address += (sec->output_section->vma
			      + sec->output_offset
			      + ent->offset + 8);
		  break;
		}
	      if (ent->make_relative)
		value -= (sec->output_section->vma
			  + sec->output_offset
			  + ent->new_offset + 8);
	      write_value (abfd, buf, value, width);
	    }

	  start = buf;

	  if (hdr_info)
	    {
	      /* The address calculation may overflow, giving us a
		 value greater than 4G on a 32-bit target when
		 dwarf_vma is 64-bit.  */
	      if (sizeof (address) > 4 && ptr_size == 4)
		address &= 0xffffffff;
	      hdr_info->u.dwarf.array[hdr_info->array_count].initial_loc
		= address;
	      hdr_info->u.dwarf.array[hdr_info->array_count].range
		= read_value (abfd, buf + width, width, false);
	      hdr_info->u.dwarf.array[hdr_info->array_count++].fde
		= (sec->output_section->vma
		   + sec->output_offset
		   + ent->new_offset);
	    }

	  if ((ent->lsda_encoding & 0x70) == DW_EH_PE_pcrel
	      || cie->u.cie.make_lsda_relative)
	    {
	      buf += ent->lsda_offset;
	      width = get_DW_EH_PE_width (ent->lsda_encoding, ptr_size);
	      value = read_value (abfd, buf, width,
				  get_DW_EH_PE_signed (ent->lsda_encoding));
	      if (value)
		{
		  if ((ent->lsda_encoding & 0x70) == DW_EH_PE_pcrel)
		    value += (bfd_vma) ent->offset - ent->new_offset;
		  else if (cie->u.cie.make_lsda_relative)
		    value -= (sec->output_section->vma
			      + sec->output_offset
			      + ent->new_offset + 8 + ent->lsda_offset);
		  write_value (abfd, buf, value, width);
		}
	    }
	  else if (ent->add_augmentation_size)
	    {
	      /* Skip the PC and length and insert a zero byte for the
		 augmentation size.  */
	      buf += width * 2;
	      memmove (buf + 1, buf, end - buf);
	      *buf = 0;
	    }

	  if (ent->set_loc)
	    {
	      /* Adjust DW_CFA_set_loc.  */
	      unsigned int cnt;
	      bfd_vma new_offset;

	      width = get_DW_EH_PE_width (ent->fde_encoding, ptr_size);
	      new_offset = ent->new_offset + 8
			   + extra_augmentation_string_bytes (ent)
			   + extra_augmentation_data_bytes (ent);

	      for (cnt = 1; cnt <= ent->set_loc[0]; cnt++)
		{
		  buf = start + ent->set_loc[cnt];

		  value = read_value (abfd, buf, width,
				      get_DW_EH_PE_signed (ent->fde_encoding));
		  if (!value)
		    continue;

		  if ((ent->fde_encoding & 0x70) == DW_EH_PE_pcrel)
		    value += (bfd_vma) ent->offset + 8 - new_offset;
		  if (ent->make_relative)
		    value -= (sec->output_section->vma
			      + sec->output_offset
			      + new_offset + ent->set_loc[cnt]);
		  write_value (abfd, buf, value, width);
		}
	    }
	}
    }

  /* FIXME: octets_per_byte.  */
  return bfd_set_section_contents (abfd, sec->output_section,
				   contents, (file_ptr) sec->output_offset,
				   sec->size);
}

/* Helper function used to sort .eh_frame_hdr search table by increasing
   VMA of FDE initial location.  */

static int
vma_compare (const void *a, const void *b)
{
  const struct eh_frame_array_ent *p = (const struct eh_frame_array_ent *) a;
  const struct eh_frame_array_ent *q = (const struct eh_frame_array_ent *) b;
  if (p->initial_loc > q->initial_loc)
    return 1;
  if (p->initial_loc < q->initial_loc)
    return -1;
  if (p->range > q->range)
    return 1;
  if (p->range < q->range)
    return -1;
  return 0;
}

/* Reorder .eh_frame_entry sections to match the associated text sections.
   This routine is called during the final linking step, just before writing
   the contents.  At this stage, sections in the eh_frame_hdr_info are already
   sorted in order of increasing text section address and so we simply need
   to make the .eh_frame_entrys follow that same order.  Note that it is
   invalid for a linker script to try to force a particular order of
   .eh_frame_entry sections.  */

bool
_bfd_elf_fixup_eh_frame_hdr (struct bfd_link_info *info)
{
  asection *sec = NULL;
  asection *osec;
  struct eh_frame_hdr_info *hdr_info;
  unsigned int i;
  bfd_vma offset;
  struct bfd_link_order *p;

  hdr_info = &elf_hash_table (info)->eh_info;

  if (hdr_info->hdr_sec == NULL
      || info->eh_frame_hdr_type != COMPACT_EH_HDR
      || hdr_info->array_count == 0)
    return true;

  /* Change section output offsets to be in text section order.  */
  offset = 8;
  osec = hdr_info->u.compact.entries[0]->output_section;
  for (i = 0; i < hdr_info->array_count; i++)
    {
      sec = hdr_info->u.compact.entries[i];
      if (sec->output_section != osec)
	{
	  _bfd_error_handler
	    (_("invalid output section for .eh_frame_entry: %pA"),
	     sec->output_section);
	  return false;
	}
      sec->output_offset = offset;
      offset += sec->size;
    }


  /* Fix the link_order to match.  */
  for (p = sec->output_section->map_head.link_order; p != NULL; p = p->next)
    {
      if (p->type != bfd_indirect_link_order)
	abort();

      p->offset = p->u.indirect.section->output_offset;
      if (p->next != NULL)
	i--;
    }

  if (i != 0)
    {
      _bfd_error_handler
	(_("invalid contents in %pA section"), osec);
      return false;
    }

  return true;
}

/* The .eh_frame_hdr format for Compact EH frames:
   ubyte version		(2)
   ubyte eh_ref_enc		(DW_EH_PE_* encoding of typinfo references)
   uint32_t count		(Number of entries in table)
   [array from .eh_frame_entry sections]  */

static bool
write_compact_eh_frame_hdr (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  asection *sec;
  const struct elf_backend_data *bed;
  bfd_vma count;
  bfd_byte contents[8];
  unsigned int i;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;
  sec = hdr_info->hdr_sec;

  if (sec->size != 8)
    abort();

  for (i = 0; i < sizeof (contents); i++)
    contents[i] = 0;

  contents[0] = COMPACT_EH_HDR;
  bed = get_elf_backend_data (abfd);

  BFD_ASSERT (bed->compact_eh_encoding);
  contents[1] = (*bed->compact_eh_encoding) (info);

  count = (sec->output_section->size - 8) / 8;
  bfd_put_32 (abfd, count, contents + 4);
  return bfd_set_section_contents (abfd, sec->output_section, contents,
				   (file_ptr) sec->output_offset, sec->size);
}

/* The .eh_frame_hdr format for DWARF frames:

   ubyte version		(currently 1)
   ubyte eh_frame_ptr_enc	(DW_EH_PE_* encoding of pointer to start of
				 .eh_frame section)
   ubyte fde_count_enc		(DW_EH_PE_* encoding of total FDE count
				 number (or DW_EH_PE_omit if there is no
				 binary search table computed))
   ubyte table_enc		(DW_EH_PE_* encoding of binary search table,
				 or DW_EH_PE_omit if not present.
				 DW_EH_PE_datarel is using address of
				 .eh_frame_hdr section start as base)
   [encoded] eh_frame_ptr	(pointer to start of .eh_frame section)
   optionally followed by:
   [encoded] fde_count		(total number of FDEs in .eh_frame section)
   fde_count x [encoded] initial_loc, fde
				(array of encoded pairs containing
				 FDE initial_location field and FDE address,
				 sorted by increasing initial_loc).  */

static bool
write_dwarf_eh_frame_hdr (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  asection *sec;
  bool retval = true;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;
  sec = hdr_info->hdr_sec;
  bfd_byte *contents;
  asection *eh_frame_sec;
  bfd_size_type size;
  bfd_vma encoded_eh_frame;

  size = EH_FRAME_HDR_SIZE;
  if (hdr_info->u.dwarf.array
      && hdr_info->array_count == hdr_info->u.dwarf.fde_count)
    size += 4 + hdr_info->u.dwarf.fde_count * 8;
  contents = (bfd_byte *) bfd_malloc (size);
  if (contents == NULL)
    return false;

  eh_frame_sec = bfd_get_section_by_name (abfd, ".eh_frame");
  if (eh_frame_sec == NULL)
    {
      free (contents);
      return false;
    }

  memset (contents, 0, EH_FRAME_HDR_SIZE);
  /* Version.  */
  contents[0] = 1;
  /* .eh_frame offset.  */
  contents[1] = get_elf_backend_data (abfd)->elf_backend_encode_eh_address
    (abfd, info, eh_frame_sec, 0, sec, 4, &encoded_eh_frame);

  if (hdr_info->u.dwarf.array
      && hdr_info->array_count == hdr_info->u.dwarf.fde_count)
    {
      /* FDE count encoding.  */
      contents[2] = DW_EH_PE_udata4;
      /* Search table encoding.  */
      contents[3] = DW_EH_PE_datarel | DW_EH_PE_sdata4;
    }
  else
    {
      contents[2] = DW_EH_PE_omit;
      contents[3] = DW_EH_PE_omit;
    }
  bfd_put_32 (abfd, encoded_eh_frame, contents + 4);

  if (contents[2] != DW_EH_PE_omit)
    {
      unsigned int i;
      bool overlap, overflow;

      bfd_put_32 (abfd, hdr_info->u.dwarf.fde_count,
		  contents + EH_FRAME_HDR_SIZE);
      qsort (hdr_info->u.dwarf.array, hdr_info->u.dwarf.fde_count,
	     sizeof (*hdr_info->u.dwarf.array), vma_compare);
      overlap = false;
      overflow = false;
      for (i = 0; i < hdr_info->u.dwarf.fde_count; i++)
	{
	  bfd_vma val;

	  val = hdr_info->u.dwarf.array[i].initial_loc
	    - sec->output_section->vma;
	  val = ((val & 0xffffffff) ^ 0x80000000) - 0x80000000;
	  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64
	      && (hdr_info->u.dwarf.array[i].initial_loc
		  != sec->output_section->vma + val))
	    overflow = true;
	  bfd_put_32 (abfd, val, contents + EH_FRAME_HDR_SIZE + i * 8 + 4);
	  val = hdr_info->u.dwarf.array[i].fde - sec->output_section->vma;
	  val = ((val & 0xffffffff) ^ 0x80000000) - 0x80000000;
	  if (elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64
	      && (hdr_info->u.dwarf.array[i].fde
		  != sec->output_section->vma + val))
	    overflow = true;
	  bfd_put_32 (abfd, val, contents + EH_FRAME_HDR_SIZE + i * 8 + 8);
	  if (i != 0
	      && (hdr_info->u.dwarf.array[i].initial_loc
		  < (hdr_info->u.dwarf.array[i - 1].initial_loc
		     + hdr_info->u.dwarf.array[i - 1].range)))
	    overlap = true;
	}
      if (overflow)
	_bfd_error_handler (_(".eh_frame_hdr entry overflow"));
      if (overlap)
	_bfd_error_handler (_(".eh_frame_hdr refers to overlapping FDEs"));
      if (overflow || overlap)
	{
	  bfd_set_error (bfd_error_bad_value);
	  retval = false;
	}
    }

  /* FIXME: octets_per_byte.  */
  if (!bfd_set_section_contents (abfd, sec->output_section, contents,
				 (file_ptr) sec->output_offset,
				 sec->size))
    retval = false;
  free (contents);

  free (hdr_info->u.dwarf.array);
  return retval;
}

/* Write out .eh_frame_hdr section.  This must be called after
   _bfd_elf_write_section_eh_frame has been called on all input
   .eh_frame sections.  */

bool
_bfd_elf_write_section_eh_frame_hdr (bfd *abfd, struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab;
  struct eh_frame_hdr_info *hdr_info;
  asection *sec;

  htab = elf_hash_table (info);
  hdr_info = &htab->eh_info;
  sec = hdr_info->hdr_sec;

  if (info->eh_frame_hdr_type == 0 || sec == NULL)
    return true;

  if (info->eh_frame_hdr_type == COMPACT_EH_HDR)
    return write_compact_eh_frame_hdr (abfd, info);
  else
    return write_dwarf_eh_frame_hdr (abfd, info);
}

/* Return the width of FDE addresses.  This is the default implementation.  */

unsigned int
_bfd_elf_eh_frame_address_size (bfd *abfd, const asection *sec ATTRIBUTE_UNUSED)
{
  return elf_elfheader (abfd)->e_ident[EI_CLASS] == ELFCLASS64 ? 8 : 4;
}

/* Decide whether we can use a PC-relative encoding within the given
   EH frame section.  This is the default implementation.  */

bool
_bfd_elf_can_make_relative (bfd *input_bfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    asection *eh_frame_section ATTRIBUTE_UNUSED)
{
  return true;
}

/* Select an encoding for the given address.  Preference is given to
   PC-relative addressing modes.  */

bfd_byte
_bfd_elf_encode_eh_address (bfd *abfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info ATTRIBUTE_UNUSED,
			    asection *osec, bfd_vma offset,
			    asection *loc_sec, bfd_vma loc_offset,
			    bfd_vma *encoded)
{
  *encoded = osec->vma + offset -
    (loc_sec->output_section->vma + loc_sec->output_offset + loc_offset);
  return DW_EH_PE_pcrel | DW_EH_PE_sdata4;
}
