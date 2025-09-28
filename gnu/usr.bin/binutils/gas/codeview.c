/* codeview.c - CodeView debug support
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

#include "as.h"
#include "codeview.h"
#include "subsegs.h"
#include "filenames.h"
#include "md5.h"

#if defined (TE_PE) && defined (O_secrel)

#define NUM_MD5_BYTES       	16

#define FILE_ENTRY_PADDING	2
#define FILE_ENTRY_LENGTH	(sizeof (struct file_checksum) + NUM_MD5_BYTES \
				 + FILE_ENTRY_PADDING)

struct line
{
  struct line *next;
  unsigned int lineno;
  addressT frag_offset;
};

struct line_file
{
  struct line_file *next;
  unsigned int fileno;
  struct line *lines_head, *lines_tail;
  unsigned int num_lines;
};

struct line_block
{
  struct line_block *next;
  segT seg;
  unsigned int subseg;
  fragS *frag;
  symbolS *sym;
  struct line_file *files_head, *files_tail;
};

struct source_file
{
  struct source_file *next;
  unsigned int num;
  char *filename;
  uint32_t string_pos;
  uint8_t md5[NUM_MD5_BYTES];
};

static struct line_block *blocks_head = NULL, *blocks_tail = NULL;
static struct source_file *files_head = NULL, *files_tail = NULL;
static unsigned int num_source_files = 0;

/* Return the size of the current fragment (taken from dwarf2dbg.c).  */
static offsetT
get_frag_fix (fragS *frag, segT seg)
{
  frchainS *fr;

  if (frag->fr_next)
    return frag->fr_fix;

  for (fr = seg_info (seg)->frchainP; fr; fr = fr->frch_next)
    if (fr->frch_last == frag)
      return (char *) obstack_next_free (&fr->frch_obstack) - frag->fr_literal;

  abort ();
}

/* Emit a .secrel32 relocation.  */
static void
emit_secrel32_reloc (symbolS *sym)
{
  expressionS exp;

  memset (&exp, 0, sizeof (exp));
  exp.X_op = O_secrel;
  exp.X_add_symbol = sym;
  exp.X_add_number = 0;
  emit_expr (&exp, sizeof (uint32_t));
}

/* Emit a .secidx relocation.  */
static void
emit_secidx_reloc (symbolS *sym)
{
  expressionS exp;

  memset (&exp, 0, sizeof (exp));
  exp.X_op = O_secidx;
  exp.X_add_symbol = sym;
  exp.X_add_number = 0;
  emit_expr (&exp, sizeof (uint16_t));
}

/* Write the DEBUG_S_STRINGTABLE subsection.  */
static void
write_string_table (void)
{
  uint32_t len;
  unsigned int padding;
  char *ptr, *start;

  len = 1;

  for (struct source_file *sf = files_head; sf; sf = sf->next)
    {
      len += strlen (sf->filename) + 1;
    }

  if (len % 4)
    padding = 4 - (len % 4);
  else
    padding = 0;

  ptr = frag_more (sizeof (uint32_t) + sizeof (uint32_t) + len + padding);

  bfd_putl32 (DEBUG_S_STRINGTABLE, ptr);
  ptr += sizeof (uint32_t);
  bfd_putl32 (len, ptr);
  ptr += sizeof (uint32_t);

  start = ptr;

  *ptr = 0;
  ptr++;

  for (struct source_file *sf = files_head; sf; sf = sf->next)
    {
      size_t fn_len = strlen (sf->filename);

      sf->string_pos = ptr - start;

      memcpy(ptr, sf->filename, fn_len + 1);
      ptr += fn_len + 1;
    }

  memset (ptr, 0, padding);
}

/* Write the DEBUG_S_FILECHKSMS subsection.  */
static void
write_checksums (void)
{
  uint32_t len;
  char *ptr;

  len = FILE_ENTRY_LENGTH * num_source_files;

  ptr = frag_more (sizeof (uint32_t) + sizeof (uint32_t) + len);

  bfd_putl32 (DEBUG_S_FILECHKSMS, ptr);
  ptr += sizeof (uint32_t);
  bfd_putl32 (len, ptr);
  ptr += sizeof (uint32_t);

  for (struct source_file *sf = files_head; sf; sf = sf->next)
    {
      struct file_checksum fc;

      fc.file_id = sf->string_pos;
      fc.checksum_length = NUM_MD5_BYTES;
      fc.checksum_type = CHKSUM_TYPE_MD5;

      memcpy (ptr, &fc, sizeof (struct file_checksum));
      ptr += sizeof (struct file_checksum);

      memcpy (ptr, sf->md5, NUM_MD5_BYTES);
      ptr += NUM_MD5_BYTES;

      memset (ptr, 0, FILE_ENTRY_PADDING);
      ptr += FILE_ENTRY_PADDING;
    }
}

/* Write the DEBUG_S_LINES subsection.  */
static void
write_lines_info (void)
{
  while (blocks_head)
    {
      struct line_block *lb;
      struct line_file *lf;
      uint32_t len;
      uint32_t off;
      char *ptr;

      lb = blocks_head;

      bfd_putl32 (DEBUG_S_LINES, frag_more (sizeof (uint32_t)));

      len = sizeof (struct cv_lines_header);

      for (lf = lb->files_head; lf; lf = lf->next)
	{
	  len += sizeof (struct cv_lines_block);
	  len += sizeof (struct cv_line) * lf->num_lines;
	}

      bfd_putl32 (len, frag_more (sizeof (uint32_t)));

      /* Write the header (struct cv_lines_header).  We can't use a struct
	 for this as we're also emitting relocations.  */

      emit_secrel32_reloc (lb->sym);
      emit_secidx_reloc (lb->sym);

      ptr = frag_more (len - sizeof (uint32_t) - sizeof (uint16_t));

      /* Flags */
      bfd_putl16 (0, ptr);
      ptr += sizeof (uint16_t);

      off = lb->files_head->lines_head->frag_offset;

      /* Length of region */
      bfd_putl32 (get_frag_fix (lb->frag, lb->seg) - off, ptr);
      ptr += sizeof (uint32_t);

      while (lb->files_head)
	{
	  struct cv_lines_block *block = (struct cv_lines_block *) ptr;

	  lf = lb->files_head;

	  bfd_putl32(lf->fileno * FILE_ENTRY_LENGTH, &block->file_id);
	  bfd_putl32(lf->num_lines, &block->num_lines);
	  bfd_putl32(sizeof (struct cv_lines_block)
		     + (sizeof (struct cv_line) * lf->num_lines),
		     &block->length);

	  ptr += sizeof (struct cv_lines_block);

	  while (lf->lines_head)
	    {
	      struct line *l;
	      struct cv_line *l2 = (struct cv_line *) ptr;

	      l = lf->lines_head;

	      /* Only the bottom 24 bits of line_no actually encode the
		 line number.  The top bit is a flag meaning "is
		 a statement".  */

	      bfd_putl32 (l->frag_offset - off, &l2->offset);
	      bfd_putl32 (0x80000000 | (l->lineno & 0xffffff),
			  &l2->line_no);

	      lf->lines_head = l->next;

	      free(l);

	      ptr += sizeof (struct cv_line);
	    }

	  lb->files_head = lf->next;
	  free (lf);
	}

      blocks_head = lb->next;

      free (lb);
    }
}

/* Return the CodeView constant for the selected architecture.  */
static uint16_t
target_processor (void)
{
  switch (stdoutput->arch_info->arch)
    {
    case bfd_arch_i386:
      if (stdoutput->arch_info->mach & bfd_mach_x86_64)
	return CV_CFL_X64;
      else
	return CV_CFL_80386;

    case bfd_arch_aarch64:
      return CV_CFL_ARM64;

    default:
      return 0;
    }
}

/* Write the CodeView symbols, describing the object name and
   assembler version.  */
static void
write_symbols_info (void)
{
  static const char assembler[] = "GNU AS " VERSION;

  char *path = lrealpath (out_file_name);
  char *path2 = remap_debug_filename (path);
  size_t path_len, padding;
  uint32_t len;
  struct OBJNAMESYM objname;
  struct COMPILESYM3 compile3;
  char *ptr;

  free (path);
  path = path2;

  path_len = strlen (path);

  len = sizeof (struct OBJNAMESYM) + path_len + 1;
  len += sizeof (struct COMPILESYM3) + sizeof (assembler);

  if (len % 4)
    padding = 4 - (len % 4);
  else
    padding = 0;

  len += padding;

  ptr = frag_more (sizeof (uint32_t) + sizeof (uint32_t) + len);

  bfd_putl32 (DEBUG_S_SYMBOLS, ptr);
  ptr += sizeof (uint32_t);
  bfd_putl32 (len, ptr);
  ptr += sizeof (uint32_t);

  /* Write S_OBJNAME entry.  */

  bfd_putl16 (sizeof (struct OBJNAMESYM) - sizeof (uint16_t) + path_len + 1,
	      &objname.length);
  bfd_putl16 (S_OBJNAME, &objname.type);
  bfd_putl32 (0, &objname.signature);

  memcpy (ptr, &objname, sizeof (struct OBJNAMESYM));
  ptr += sizeof (struct OBJNAMESYM);
  memcpy (ptr, path, path_len + 1);
  ptr += path_len + 1;

  free (path);

  /* Write S_COMPILE3 entry.  */

  bfd_putl16 (sizeof (struct COMPILESYM3) - sizeof (uint16_t)
	      + sizeof (assembler) + padding, &compile3.length);
  bfd_putl16 (S_COMPILE3, &compile3.type);
  bfd_putl32 (CV_CFL_MASM, &compile3.flags);
  bfd_putl16 (target_processor (), &compile3.machine);
  bfd_putl16 (0, &compile3.frontend_major);
  bfd_putl16 (0, &compile3.frontend_minor);
  bfd_putl16 (0, &compile3.frontend_build);
  bfd_putl16 (0, &compile3.frontend_qfe);
  bfd_putl16 (0, &compile3.backend_major);
  bfd_putl16 (0, &compile3.backend_minor);
  bfd_putl16 (0, &compile3.backend_build);
  bfd_putl16 (0, &compile3.backend_qfe);

  memcpy (ptr, &compile3, sizeof (struct COMPILESYM3));
  ptr += sizeof (struct COMPILESYM3);
  memcpy (ptr, assembler, sizeof (assembler));
  ptr += sizeof (assembler);

  memset (ptr, 0, padding);
}

/* Processing of the file has finished, emit the .debug$S section.  */
void
codeview_finish (void)
{
  segT seg;

  if (!blocks_head)
    return;

  seg = subseg_new (".debug$S", 0);

  bfd_set_section_flags (seg, SEC_READONLY | SEC_NEVER_LOAD);

  bfd_putl32 (CV_SIGNATURE_C13, frag_more (sizeof (uint32_t)));

  write_string_table ();
  write_checksums ();
  write_lines_info ();
  write_symbols_info ();
}

/* Assign a new index number for the given file, or return the existing
   one if already assigned.  */
static unsigned int
get_fileno (const char *file)
{
  struct source_file *sf;
  char *path = lrealpath (file);
  char *path2 = remap_debug_filename (path);
  size_t path_len;
  FILE *f;

  free (path);
  path = path2;

  path_len = strlen (path);

  for (sf = files_head; sf; sf = sf->next)
    {
      if (path_len == strlen (sf->filename)
	  && !filename_ncmp (sf->filename, path, path_len))
	{
	  free (path);
	  return sf->num;
	}
    }

  sf = xmalloc (sizeof (struct source_file));

  sf->next = NULL;
  sf->num = num_source_files;
  sf->filename = path;

  f = fopen (file, "r");
  if (!f)
    as_fatal (_("could not open %s for reading"), file);

  if (md5_stream (f, sf->md5))
    {
      fclose(f);
      as_fatal (_("md5_stream failed"));
    }

  fclose(f);

  if (!files_head)
    files_head = sf;
  else
    files_tail->next = sf;

  files_tail = sf;

  num_source_files++;

  return num_source_files - 1;
}

/* Called for each new line in asm file.  */
void
codeview_generate_asm_lineno (void)
{
  const char *file;
  unsigned int filenr;
  unsigned int lineno;
  struct line *l;
  symbolS *sym = NULL;
  struct line_block *lb;
  struct line_file *lf;

  file = as_where (&lineno);

  filenr = get_fileno (file);

  if (!blocks_tail || blocks_tail->frag != frag_now)
    {
      static int label_num = 0;
      char name[32];

      sprintf (name, ".Loc.%u", label_num);
      label_num++;
      sym = symbol_new (name, now_seg, frag_now, frag_now_fix ());

      lb = xmalloc (sizeof (struct line_block));
      lb->next = NULL;
      lb->seg = now_seg;
      lb->subseg = now_subseg;
      lb->frag = frag_now;
      lb->sym = sym;
      lb->files_head = lb->files_tail = NULL;

      if (!blocks_head)
	blocks_head = lb;
      else
	blocks_tail->next = lb;

      blocks_tail = lb;
    }
  else
    {
      lb = blocks_tail;
    }

  if (!lb->files_tail || lb->files_tail->fileno != filenr)
    {
      lf = xmalloc (sizeof (struct line_file));
      lf->next = NULL;
      lf->fileno = filenr;
      lf->lines_head = lf->lines_tail = NULL;
      lf->num_lines = 0;

      if (!lb->files_head)
	lb->files_head = lf;
      else
	lb->files_tail->next = lf;

      lb->files_tail = lf;
    }
  else
    {
      lf = lb->files_tail;
    }

  l = xmalloc (sizeof (struct line));
  l->next = NULL;
  l->lineno = lineno;
  l->frag_offset = frag_now_fix ();

  if (!lf->lines_head)
    lf->lines_head = l;
  else
    lf->lines_tail->next = l;

  lf->lines_tail = l;
  lf->num_lines++;
}

#else

void
codeview_finish (void)
{
}

void
codeview_generate_asm_lineno (void)
{
}

#endif /* TE_PE && O_secrel */
