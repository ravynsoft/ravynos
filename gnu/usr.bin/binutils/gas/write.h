/* write.h
   Copyright (C) 1987-2023 Free Software Foundation, Inc.

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

#ifndef __write_h__
#define __write_h__

/* This is the name of a fake symbol which will never appear in the
   assembler output.  S_IS_LOCAL detects it because of the \001.  */
#ifndef FAKE_LABEL_NAME
#define FAKE_LABEL_NAME "L0\001"
#endif

/* This is a special character that is used to indicate a fake label.
   It must be present in FAKE_LABEL_NAME, although it does not have to
   be the first character.  It must not be a character that would be
   found in a valid symbol name.

   Also be aware that the function _bfd_elf_is_local_label_name in
   bfd/elf.c has an implicit assumption that FAKE_LABEL_CHAR is '\001'.
   If this is not the case then FAKE_LABEL_NAME must start with ".L" in
   order for the function to continue working.  */
#ifndef FAKE_LABEL_CHAR
#define FAKE_LABEL_CHAR '\001'
#endif

/*
 * FixSs may be built up in any order.
 */

struct fix
{
  /* Next fixS in linked list, or NULL.  */
  struct fix *fx_next;

  /* These small fields are grouped together for compactness of
     this structure, and efficiency of access on some architectures.  */

  /* pc-relative offset adjust (only used by some CPU specific code).
     A 4-bit field would be sufficient for most uses, except for ppc
     which pokes an operand table index here.  Bits may be stolen
     from here should that be necessary, provided PPC_OPINDEX_MAX is
     adjusted suitably.  */
  int fx_pcrel_adjust : 16;

  /* How many bytes are involved? */
  unsigned fx_size : 8;

  /* Is this a pc-relative relocation?  */
  unsigned fx_pcrel : 1;

  /* Has this relocation already been applied?  */
  unsigned fx_done : 1;

  /* Suppress overflow complaints on large addends.  This is used
     in the PowerPC ELF config to allow large addends on the
     BFD_RELOC_{LO16,HI16,HI16_S} relocations.

     @@ Can this be determined from BFD?  */
  unsigned fx_no_overflow : 1;

  /* The value is signed when checking for overflow.  */
  unsigned fx_signed : 1;

  /* Some bits for the CPU specific code.  */
  unsigned fx_tcbit : 1;
  unsigned fx_tcbit2 : 1;

  /* Spare bits.  */
  unsigned fx_unused : 2;

  bfd_reloc_code_real_type fx_r_type;

  /* Which frag does this fix apply to?  */
  fragS *fx_frag;

  /* The location within the frag where the fixup occurs.  */
  unsigned long fx_where;

  /* NULL or Symbol whose value we add in.  */
  symbolS *fx_addsy;

  /* NULL or Symbol whose value we subtract.  */
  symbolS *fx_subsy;

  /* Absolute number we add in.  */
  valueT fx_offset;

  /* The value of dot when the fixup expression was parsed.  */
  addressT fx_dot_value;

  /* The frag fx_dot_value is based on.  */
  fragS *fx_dot_frag;

  /* This field is sort of misnamed.  It appears to be a sort of random
     scratch field, for use by the back ends.  The main gas code doesn't
     do anything but initialize it to zero.  The use of it does need to
     be coordinated between the cpu and format files, though.  E.g., some
     coff targets pass the `addend' field from the cpu file via this
     field.  I don't know why the `fx_offset' field above can't be used
     for that; investigate later and document. KR  */
  valueT fx_addnumber;

  /* The location of the instruction which created the reloc, used
     in error messages.  */
  const char *fx_file;
  unsigned fx_line;

#ifdef USING_CGEN
  struct {
    /* CGEN_INSN entry for this instruction.  */
    const struct cgen_insn *insn;
    /* Target specific data, usually reloc number.  */
    int opinfo;
    /* Which ifield this fixup applies to. */
    struct cgen_maybe_multi_ifield * field;
    /* is this field is the MSB field in a set? */
    int msb_field_p;
  } fx_cgen;
#endif

#ifdef TC_FIX_TYPE
  /* Location where a backend can attach additional data
     needed to perform fixups.  */
  TC_FIX_TYPE tc_fix_data;
#endif
};

typedef struct fix fixS;

struct reloc_list
{
  struct reloc_list *next;
  union
  {
    struct
    {
      symbolS *offset_sym;
      reloc_howto_type *howto;
      symbolS *sym;
      bfd_vma addend;
    } a;
    struct
    {
      asection *sec;
      asymbol *s;
      arelent r;
    } b;
  } u;
  const char *file;
  unsigned int line;
};

extern int finalize_syms;
extern symbolS *abs_section_sym;
extern addressT dot_value;
extern fragS *dot_frag;
extern struct reloc_list* reloc_list;

extern void append (char **, char *, unsigned long);
extern void record_alignment (segT, unsigned);
extern int get_recorded_alignment (segT);
extern void write_object_file (void);
extern long relax_frag (segT, fragS *, long);
extern int relax_segment (struct frag *, segT, int);
extern void number_to_chars_littleendian (char *, valueT, int);
extern void number_to_chars_bigendian (char *, valueT, int);
extern fixS *fix_new (fragS *, unsigned long, unsigned long, symbolS *,
		      offsetT, int, bfd_reloc_code_real_type);
extern fixS *fix_at_start (fragS *, unsigned long, symbolS *,
			   offsetT, int, bfd_reloc_code_real_type);
extern fixS *fix_new_exp (fragS *, unsigned long, unsigned long,
			  expressionS *, int, bfd_reloc_code_real_type);
extern void write_print_statistics (FILE *);
extern void as_bad_subtract (fixS *);

#endif /* __write_h__ */
