/* BFD support for handling relocation entries.
   Copyright (C) 1990-2023 Free Software Foundation, Inc.
   Written by Cygnus Support.

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

/*
SECTION
	Relocations

	BFD maintains relocations in much the same way it maintains
	symbols: they are left alone until required, then read in
	en-masse and translated into an internal form.  A common
	routine <<bfd_perform_relocation>> acts upon the
	canonical form to do the fixup.

	Relocations are maintained on a per section basis,
	while symbols are maintained on a per BFD basis.

	All that a back end has to do to fit the BFD interface is to create
	a <<struct reloc_cache_entry>> for each relocation
	in a particular section, and fill in the right bits of the structures.

@menu
@* typedef arelent::
@* howto manager::
@end menu

*/

/* DO compile in the reloc_code name table from libbfd.h.  */
#define _BFD_MAKE_TABLE_bfd_reloc_code_real

#include "sysdep.h"
#include "bfd.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "bfdver.h"

/*
DOCDD
INODE
	typedef arelent, howto manager, Relocations, Relocations

SUBSECTION
	typedef arelent

	This is the structure of a relocation entry:

EXTERNAL
.typedef enum bfd_reloc_status
.{
.  {* No errors detected.  Note - the value 2 is used so that it
.     will not be mistaken for the boolean TRUE or FALSE values.  *}
.  bfd_reloc_ok = 2,
.
.  {* The relocation was performed, but there was an overflow.  *}
.  bfd_reloc_overflow,
.
.  {* The address to relocate was not within the section supplied.  *}
.  bfd_reloc_outofrange,
.
.  {* Used by special functions.  *}
.  bfd_reloc_continue,
.
.  {* Unsupported relocation size requested.  *}
.  bfd_reloc_notsupported,
.
.  {* Target specific meaning.  *}
.  bfd_reloc_other,
.
.  {* The symbol to relocate against was undefined.  *}
.  bfd_reloc_undefined,
.
.  {* The relocation was performed, but may not be ok.  If this type is
.     returned, the error_message argument to bfd_perform_relocation
.     will be set.  *}
.  bfd_reloc_dangerous
. }
. bfd_reloc_status_type;
.
.typedef const struct reloc_howto_struct reloc_howto_type;
.

CODE_FRAGMENT
.struct reloc_cache_entry
.{
.  {* A pointer into the canonical table of pointers.  *}
.  struct bfd_symbol **sym_ptr_ptr;
.
.  {* offset in section.  *}
.  bfd_size_type address;
.
.  {* addend for relocation value.  *}
.  bfd_vma addend;
.
.  {* Pointer to how to perform the required relocation.  *}
.  reloc_howto_type *howto;
.
.};
.
*/

/*
DESCRIPTION

	Here is a description of each of the fields within an <<arelent>>:

	o <<sym_ptr_ptr>>

	The symbol table pointer points to a pointer to the symbol
	associated with the relocation request.  It is the pointer
	into the table returned by the back end's
	<<canonicalize_symtab>> action. @xref{Symbols}. The symbol is
	referenced through a pointer to a pointer so that tools like
	the linker can fix up all the symbols of the same name by
	modifying only one pointer. The relocation routine looks in
	the symbol and uses the base of the section the symbol is
	attached to and the value of the symbol as the initial
	relocation offset. If the symbol pointer is zero, then the
	section provided is looked up.

	o <<address>>

	The <<address>> field gives the offset in bytes from the base of
	the section data which owns the relocation record to the first
	byte of relocatable information. The actual data relocated
	will be relative to this point; for example, a relocation
	type which modifies the bottom two bytes of a four byte word
	would not touch the first byte pointed to in a big endian
	world.

	o <<addend>>

	The <<addend>> is a value provided by the back end to be added (!)
	to the relocation offset. Its interpretation is dependent upon
	the howto. For example, on the 68k the code:

|        char foo[];
|        main()
|                {
|                return foo[0x12345678];
|                }

	Could be compiled into:

|        linkw fp,#-4
|        moveb @@#12345678,d0
|        extbl d0
|        unlk fp
|        rts

	This could create a reloc pointing to <<foo>>, but leave the
	offset in the data, something like:

|RELOCATION RECORDS FOR [.text]:
|offset   type      value
|00000006 32        _foo
|
|00000000 4e56 fffc          ; linkw fp,#-4
|00000004 1039 1234 5678     ; moveb @@#12345678,d0
|0000000a 49c0               ; extbl d0
|0000000c 4e5e               ; unlk fp
|0000000e 4e75               ; rts

	Using coff and an 88k, some instructions don't have enough
	space in them to represent the full address range, and
	pointers have to be loaded in two parts. So you'd get something like:

|        or.u     r13,r0,hi16(_foo+0x12345678)
|        ld.b     r2,r13,lo16(_foo+0x12345678)
|        jmp      r1

	This should create two relocs, both pointing to <<_foo>>, and with
	0x12340000 in their addend field. The data would consist of:

|RELOCATION RECORDS FOR [.text]:
|offset   type      value
|00000002 HVRT16    _foo+0x12340000
|00000006 LVRT16    _foo+0x12340000
|
|00000000 5da05678           ; or.u r13,r0,0x5678
|00000004 1c4d5678           ; ld.b r2,r13,0x5678
|00000008 f400c001           ; jmp r1

	The relocation routine digs out the value from the data, adds
	it to the addend to get the original offset, and then adds the
	value of <<_foo>>. Note that all 32 bits have to be kept around
	somewhere, to cope with carry from bit 15 to bit 16.

	One further example is the sparc and the a.out format. The
	sparc has a similar problem to the 88k, in that some
	instructions don't have room for an entire offset, but on the
	sparc the parts are created in odd sized lumps. The designers of
	the a.out format chose to not use the data within the section
	for storing part of the offset; all the offset is kept within
	the reloc. Anything in the data should be ignored.

|        save %sp,-112,%sp
|        sethi %hi(_foo+0x12345678),%g2
|        ldsb [%g2+%lo(_foo+0x12345678)],%i0
|        ret
|        restore

	Both relocs contain a pointer to <<foo>>, and the offsets
	contain junk.

|RELOCATION RECORDS FOR [.text]:
|offset   type      value
|00000004 HI22      _foo+0x12345678
|00000008 LO10      _foo+0x12345678
|
|00000000 9de3bf90     ; save %sp,-112,%sp
|00000004 05000000     ; sethi %hi(_foo+0),%g2
|00000008 f048a000     ; ldsb [%g2+%lo(_foo+0)],%i0
|0000000c 81c7e008     ; ret
|00000010 81e80000     ; restore

	o <<howto>>

	The <<howto>> field can be imagined as a
	relocation instruction. It is a pointer to a structure which
	contains information on what to do with all of the other
	information in the reloc record and data section. A back end
	would normally have a relocation instruction set and turn
	relocations into pointers to the correct structure on input -
	but it would be possible to create each howto field on demand.

*/

/*
SUBSUBSECTION
	<<enum complain_overflow>>

	Indicates what sort of overflow checking should be done when
	performing a relocation.

CODE_FRAGMENT
.enum complain_overflow
.{
.  {* Do not complain on overflow.  *}
.  complain_overflow_dont,
.
.  {* Complain if the value overflows when considered as a signed
.     number one bit larger than the field.  ie. A bitfield of N bits
.     is allowed to represent -2**n to 2**n-1.  *}
.  complain_overflow_bitfield,
.
.  {* Complain if the value overflows when considered as a signed
.     number.  *}
.  complain_overflow_signed,
.
.  {* Complain if the value overflows when considered as an
.     unsigned number.  *}
.  complain_overflow_unsigned
.};
.
*/

/*
SUBSUBSECTION
	<<reloc_howto_type>>

	The <<reloc_howto_type>> is a structure which contains all the
	information that libbfd needs to know to tie up a back end's data.

CODE_FRAGMENT
.struct reloc_howto_struct
.{
.  {* The type field has mainly a documentary use - the back end can
.     do what it wants with it, though normally the back end's idea of
.     an external reloc number is stored in this field.  *}
.  unsigned int type;
.
.  {* The size of the item to be relocated in bytes.  *}
.  unsigned int size:4;
.
.  {* The number of bits in the field to be relocated.  This is used
.     when doing overflow checking.  *}
.  unsigned int bitsize:7;
.
.  {* The value the final relocation is shifted right by.  This drops
.     unwanted data from the relocation.  *}
.  unsigned int rightshift:6;
.
.  {* The bit position of the reloc value in the destination.
.     The relocated value is left shifted by this amount.  *}
.  unsigned int bitpos:6;
.
.  {* What type of overflow error should be checked for when
.     relocating.  *}
.  ENUM_BITFIELD (complain_overflow) complain_on_overflow:2;
.
.  {* The relocation value should be negated before applying.  *}
.  unsigned int negate:1;
.
.  {* The relocation is relative to the item being relocated.  *}
.  unsigned int pc_relative:1;
.
.  {* Some formats record a relocation addend in the section contents
.     rather than with the relocation.  For ELF formats this is the
.     distinction between USE_REL and USE_RELA (though the code checks
.     for USE_REL == 1/0).  The value of this field is TRUE if the
.     addend is recorded with the section contents; when performing a
.     partial link (ld -r) the section contents (the data) will be
.     modified.  The value of this field is FALSE if addends are
.     recorded with the relocation (in arelent.addend); when performing
.     a partial link the relocation will be modified.
.     All relocations for all ELF USE_RELA targets should set this field
.     to FALSE (values of TRUE should be looked on with suspicion).
.     However, the converse is not true: not all relocations of all ELF
.     USE_REL targets set this field to TRUE.  Why this is so is peculiar
.     to each particular target.  For relocs that aren't used in partial
.     links (e.g. GOT stuff) it doesn't matter what this is set to.  *}
.  unsigned int partial_inplace:1;
.
.  {* When some formats create PC relative instructions, they leave
.     the value of the pc of the place being relocated in the offset
.     slot of the instruction, so that a PC relative relocation can
.     be made just by adding in an ordinary offset (e.g., sun3 a.out).
.     Some formats leave the displacement part of an instruction
.     empty (e.g., ELF); this flag signals the fact.  *}
.  unsigned int pcrel_offset:1;
.
.  {* Whether bfd_install_relocation should just install the addend,
.     or should follow the practice of some older object formats and
.     install a value including the symbol.  *}
.  unsigned int install_addend:1;
.
.  {* src_mask selects the part of the instruction (or data) to be used
.     in the relocation sum.  If the target relocations don't have an
.     addend in the reloc, eg. ELF USE_REL, src_mask will normally equal
.     dst_mask to extract the addend from the section contents.  If
.     relocations do have an addend in the reloc, eg. ELF USE_RELA, this
.     field should normally be zero.  Non-zero values for ELF USE_RELA
.     targets should be viewed with suspicion as normally the value in
.     the dst_mask part of the section contents should be ignored.  *}
.  bfd_vma src_mask;
.
.  {* dst_mask selects which parts of the instruction (or data) are
.     replaced with a relocated value.  *}
.  bfd_vma dst_mask;
.
.  {* If this field is non null, then the supplied function is
.     called rather than the normal function.  This allows really
.     strange relocation methods to be accommodated.  *}
.  bfd_reloc_status_type (*special_function)
.    (bfd *, arelent *, struct bfd_symbol *, void *, asection *,
.     bfd *, char **);
.
.  {* The textual name of the relocation type.  *}
.  const char *name;
.};
.
*/

/*
FUNCTION
	The HOWTO Macro

DESCRIPTION
	The HOWTO macro fills in a reloc_howto_type (a typedef for
	const struct reloc_howto_struct).

.#define HOWTO_INSTALL_ADDEND 0
.#define HOWTO_RSIZE(sz) ((sz) < 0 ? -(sz) : (sz))
.#define HOWTO(type, right, size, bits, pcrel, left, ovf, func, name,	\
.              inplace, src_mask, dst_mask, pcrel_off)			\
.  { (unsigned) type, HOWTO_RSIZE (size), bits, right, left, ovf,	\
.    size < 0, pcrel, inplace, pcrel_off, HOWTO_INSTALL_ADDEND,		\
.    src_mask, dst_mask, func, name }

DESCRIPTION
	This is used to fill in an empty howto entry in an array.

.#define EMPTY_HOWTO(C) \
.  HOWTO ((C), 0, 1, 0, false, 0, complain_overflow_dont, NULL, \
.	  NULL, false, 0, 0, false)
.
.static inline unsigned int
.bfd_get_reloc_size (reloc_howto_type *howto)
.{
.  return howto->size;
.}
.
*/

/*
DEFINITION
	arelent_chain

DESCRIPTION
	How relocs are tied together in an <<asection>>:

.typedef struct relent_chain
.{
.  arelent relent;
.  struct relent_chain *next;
.}
.arelent_chain;
.
*/

/* N_ONES produces N one bits, without undefined behaviour for N
   between zero and the number of bits in a bfd_vma.  */
#define N_ONES(n) ((n) == 0 ? 0 : ((bfd_vma) 1 << ((n) - 1) << 1) - 1)

/*
FUNCTION
	bfd_check_overflow

SYNOPSIS
	bfd_reloc_status_type bfd_check_overflow
	  (enum complain_overflow how,
	   unsigned int bitsize,
	   unsigned int rightshift,
	   unsigned int addrsize,
	   bfd_vma relocation);

DESCRIPTION
	Perform overflow checking on @var{relocation} which has
	@var{bitsize} significant bits and will be shifted right by
	@var{rightshift} bits, on a machine with addresses containing
	@var{addrsize} significant bits.  The result is either of
	@code{bfd_reloc_ok} or @code{bfd_reloc_overflow}.

*/

bfd_reloc_status_type
bfd_check_overflow (enum complain_overflow how,
		    unsigned int bitsize,
		    unsigned int rightshift,
		    unsigned int addrsize,
		    bfd_vma relocation)
{
  bfd_vma fieldmask, addrmask, signmask, ss, a;
  bfd_reloc_status_type flag = bfd_reloc_ok;

  if (bitsize == 0)
    return flag;

  /* Note: BITSIZE should always be <= ADDRSIZE, but in case it's not,
     we'll be permissive: extra bits in the field mask will
     automatically extend the address mask for purposes of the
     overflow check.  */
  fieldmask = N_ONES (bitsize);
  signmask = ~fieldmask;
  addrmask = N_ONES (addrsize) | (fieldmask << rightshift);
  a = (relocation & addrmask) >> rightshift;

  switch (how)
    {
    case complain_overflow_dont:
      break;

    case complain_overflow_signed:
      /* If any sign bits are set, all sign bits must be set.  That
	 is, A must be a valid negative address after shifting.  */
      signmask = ~ (fieldmask >> 1);
      /* Fall thru */

    case complain_overflow_bitfield:
      /* Bitfields are sometimes signed, sometimes unsigned.  We
	 explicitly allow an address wrap too, which means a bitfield
	 of n bits is allowed to store -2**n to 2**n-1.  Thus overflow
	 if the value has some, but not all, bits set outside the
	 field.  */
      ss = a & signmask;
      if (ss != 0 && ss != ((addrmask >> rightshift) & signmask))
	flag = bfd_reloc_overflow;
      break;

    case complain_overflow_unsigned:
      /* We have an overflow if the address does not fit in the field.  */
      if ((a & signmask) != 0)
	flag = bfd_reloc_overflow;
      break;

    default:
      abort ();
    }

  return flag;
}

/*
FUNCTION
	bfd_reloc_offset_in_range

SYNOPSIS
	bool bfd_reloc_offset_in_range
	  (reloc_howto_type *howto,
	   bfd *abfd,
	   asection *section,
	   bfd_size_type offset);

DESCRIPTION
	Returns TRUE if the reloc described by @var{HOWTO} can be
	applied at @var{OFFSET} octets in @var{SECTION}.

*/

/* HOWTO describes a relocation, at offset OCTET.  Return whether the
   relocation field is within SECTION of ABFD.  */

bool
bfd_reloc_offset_in_range (reloc_howto_type *howto,
			   bfd *abfd,
			   asection *section,
			   bfd_size_type octet)
{
  bfd_size_type octet_end = bfd_get_section_limit_octets (abfd, section);
  bfd_size_type reloc_size = bfd_get_reloc_size (howto);

  /* The reloc field must be contained entirely within the section.
     Allow zero length fields (marker relocs or NONE relocs where no
     relocation will be performed) at the end of the section.  */
  return octet <= octet_end && reloc_size <= octet_end - octet;
}

/* Read and return the section contents at DATA converted to a host
   integer (bfd_vma).  The number of bytes read is given by the HOWTO.  */

static bfd_vma
read_reloc (bfd *abfd, bfd_byte *data, reloc_howto_type *howto)
{
  switch (bfd_get_reloc_size (howto))
    {
    case 0:
      break;

    case 1:
      return bfd_get_8 (abfd, data);

    case 2:
      return bfd_get_16 (abfd, data);

    case 3:
      return bfd_get_24 (abfd, data);

    case 4:
      return bfd_get_32 (abfd, data);

#ifdef BFD64
    case 8:
      return bfd_get_64 (abfd, data);
#endif

    default:
      abort ();
    }
  return 0;
}

/* Convert VAL to target format and write to DATA.  The number of
   bytes written is given by the HOWTO.  */

static void
write_reloc (bfd *abfd, bfd_vma val, bfd_byte *data, reloc_howto_type *howto)
{
  switch (bfd_get_reloc_size (howto))
    {
    case 0:
      break;

    case 1:
      bfd_put_8 (abfd, val, data);
      break;

    case 2:
      bfd_put_16 (abfd, val, data);
      break;

    case 3:
      bfd_put_24 (abfd, val, data);
      break;

    case 4:
      bfd_put_32 (abfd, val, data);
      break;

#ifdef BFD64
    case 8:
      bfd_put_64 (abfd, val, data);
      break;
#endif

    default:
      abort ();
    }
}

/* Apply RELOCATION value to target bytes at DATA, according to
   HOWTO.  */

static void
apply_reloc (bfd *abfd, bfd_byte *data, reloc_howto_type *howto,
	     bfd_vma relocation)
{
  bfd_vma val = read_reloc (abfd, data, howto);

  if (howto->negate)
    relocation = -relocation;

  val = ((val & ~howto->dst_mask)
	 | (((val & howto->src_mask) + relocation) & howto->dst_mask));

  write_reloc (abfd, val, data, howto);
}

/*
FUNCTION
	bfd_perform_relocation

SYNOPSIS
	bfd_reloc_status_type bfd_perform_relocation
	  (bfd *abfd,
	   arelent *reloc_entry,
	   void *data,
	   asection *input_section,
	   bfd *output_bfd,
	   char **error_message);

DESCRIPTION
	If @var{output_bfd} is supplied to this function, the
	generated image will be relocatable; the relocations are
	copied to the output file after they have been changed to
	reflect the new state of the world. There are two ways of
	reflecting the results of partial linkage in an output file:
	by modifying the output data in place, and by modifying the
	relocation record.  Some native formats (e.g., basic a.out and
	basic coff) have no way of specifying an addend in the
	relocation type, so the addend has to go in the output data.
	This is no big deal since in these formats the output data
	slot will always be big enough for the addend. Complex reloc
	types with addends were invented to solve just this problem.
	The @var{error_message} argument is set to an error message if
	this return @code{bfd_reloc_dangerous}.

*/

bfd_reloc_status_type
bfd_perform_relocation (bfd *abfd,
			arelent *reloc_entry,
			void *data,
			asection *input_section,
			bfd *output_bfd,
			char **error_message)
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type octets;
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;
  asymbol *symbol;

  symbol = *(reloc_entry->sym_ptr_ptr);

  /* If we are not producing relocatable output, return an error if
     the symbol is not defined.  An undefined weak symbol is
     considered to have a value of zero (SVR4 ABI, p. 4-27).  */
  if (bfd_is_und_section (symbol->section)
      && (symbol->flags & BSF_WEAK) == 0
      && output_bfd == NULL)
    flag = bfd_reloc_undefined;

  /* If there is a function supplied to handle this relocation type,
     call it.  It'll return `bfd_reloc_continue' if further processing
     can be done.  */
  if (howto && howto->special_function)
    {
      bfd_reloc_status_type cont;

      /* Note - we do not call bfd_reloc_offset_in_range here as the
	 reloc_entry->address field might actually be valid for the
	 backend concerned.  It is up to the special_function itself
	 to call bfd_reloc_offset_in_range if needed.  */
      cont = howto->special_function (abfd, reloc_entry, symbol, data,
				      input_section, output_bfd,
				      error_message);
      if (cont != bfd_reloc_continue)
	return cont;
    }

  if (bfd_is_abs_section (symbol->section)
      && output_bfd != NULL)
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  /* PR 17512: file: 0f67f69d.  */
  if (howto == NULL)
    return bfd_reloc_undefined;

  /* Is the address of the relocation really within the section?  */
  octets = reloc_entry->address * bfd_octets_per_byte (abfd, input_section);
  if (!bfd_reloc_offset_in_range (howto, abfd, input_section, octets))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targeted at and the
     initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if ((output_bfd && ! howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  output_base += symbol->section->output_offset;

  /* If symbol addresses are in octets, convert to bytes.  */
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
      && (symbol->section->flags & SEC_ELF_OCTETS))
    output_base *= bfd_octets_per_byte (abfd, input_section);

  relocation += output_base;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */

  if (howto->pc_relative)
    {
      /* This is a PC relative relocation.  We want to set RELOCATION
	 to the distance between the address of the symbol and the
	 location.  RELOCATION is already the address of the symbol.

	 We start by subtracting the address of the section containing
	 the location.

	 If pcrel_offset is set, we must further subtract the position
	 of the location within the section.  Some targets arrange for
	 the addend to be the negative of the position of the location
	 within the section; for example, i386-aout does this.  For
	 i386-aout, pcrel_offset is FALSE.  Some other targets do not
	 include the position of the location; for example, ELF.
	 For those targets, pcrel_offset is TRUE.

	 If we are producing relocatable output, then we must ensure
	 that this reloc will be correctly computed when the final
	 relocation is done.  If pcrel_offset is FALSE we want to wind
	 up with the negative of the location within the section,
	 which means we must adjust the existing addend by the change
	 in the location within the section.  If pcrel_offset is TRUE
	 we do not want to adjust the existing addend at all.

	 FIXME: This seems logical to me, but for the case of
	 producing relocatable output it is not what the code
	 actually does.  I don't want to change it, because it seems
	 far too likely that something will break.  */

      relocation -=
	input_section->output_section->vma + input_section->output_offset;

      if (howto->pcrel_offset)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != NULL)
    {
      if (! howto->partial_inplace)
	{
	  /* This is a partial relocation, and we want to apply the relocation
	     to the reloc entry rather than the raw data. Modify the reloc
	     inplace to reflect what we now know.  */
	  reloc_entry->addend = relocation;
	  reloc_entry->address += input_section->output_offset;
	  return flag;
	}
      else
	{
	  /* This is a partial relocation, but inplace, so modify the
	     reloc record a bit.

	     If we've relocated with a symbol with a section, change
	     into a ref to the section belonging to the symbol.  */

	  reloc_entry->address += input_section->output_offset;

	  /* WTF?? */
	  if (abfd->xvec->flavour == bfd_target_coff_flavour)
	    {
	      /* For m68k-coff, the addend was being subtracted twice during
		 relocation with -r.  Removing the line below this comment
		 fixes that problem; see PR 2953.

However, Ian wrote the following, regarding removing the line below,
which explains why it is still enabled:  --djm

If you put a patch like that into BFD you need to check all the COFF
linkers.  I am fairly certain that patch will break coff-i386 (e.g.,
SCO); see coff_i386_reloc in coff-i386.c where I worked around the
problem in a different way.  There may very well be a reason that the
code works as it does.

Hmmm.  The first obvious point is that bfd_perform_relocation should
not have any tests that depend upon the flavour.  It's seem like
entirely the wrong place for such a thing.  The second obvious point
is that the current code ignores the reloc addend when producing
relocatable output for COFF.  That's peculiar.  In fact, I really
have no idea what the point of the line you want to remove is.

A typical COFF reloc subtracts the old value of the symbol and adds in
the new value to the location in the object file (if it's a pc
relative reloc it adds the difference between the symbol value and the
location).  When relocating we need to preserve that property.

BFD handles this by setting the addend to the negative of the old
value of the symbol.  Unfortunately it handles common symbols in a
non-standard way (it doesn't subtract the old value) but that's a
different story (we can't change it without losing backward
compatibility with old object files) (coff-i386 does subtract the old
value, to be compatible with existing coff-i386 targets, like SCO).

So everything works fine when not producing relocatable output.  When
we are producing relocatable output, logically we should do exactly
what we do when not producing relocatable output.  Therefore, your
patch is correct.  In fact, it should probably always just set
reloc_entry->addend to 0 for all cases, since it is, in fact, going to
add the value into the object file.  This won't hurt the COFF code,
which doesn't use the addend; I'm not sure what it will do to other
formats (the thing to check for would be whether any formats both use
the addend and set partial_inplace).

When I wanted to make coff-i386 produce relocatable output, I ran
into the problem that you are running into: I wanted to remove that
line.  Rather than risk it, I made the coff-i386 relocs use a special
function; it's coff_i386_reloc in coff-i386.c.  The function
specifically adds the addend field into the object file, knowing that
bfd_perform_relocation is not going to.  If you remove that line, then
coff-i386.c will wind up adding the addend field in twice.  It's
trivial to fix; it just needs to be done.

The problem with removing the line is just that it may break some
working code.  With BFD it's hard to be sure of anything.  The right
way to deal with this is simply to build and test at least all the
supported COFF targets.  It should be straightforward if time and disk
space consuming.  For each target:
    1) build the linker
    2) generate some executable, and link it using -r (I would
       probably use paranoia.o and link against newlib/libc.a, which
       for all the supported targets would be available in
       /usr/cygnus/progressive/H-host/target/lib/libc.a).
    3) make the change to reloc.c
    4) rebuild the linker
    5) repeat step 2
    6) if the resulting object files are the same, you have at least
       made it no worse
    7) if they are different you have to figure out which version is
       right
*/
	      relocation -= reloc_entry->addend;
	      reloc_entry->addend = 0;
	    }
	  else
	    {
	      reloc_entry->addend = relocation;
	    }
	}
    }

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont
      && flag == bfd_reloc_ok)
    flag = bfd_check_overflow (howto->complain_on_overflow,
			       howto->bitsize,
			       howto->rightshift,
			       bfd_arch_bits_per_address (abfd),
			       relocation);

  /* Either we are relocating all the way, or we don't want to apply
     the relocation to the reloc entry (probably because there isn't
     any room in the output format to describe addends to relocs).  */

  /* The cast to bfd_vma avoids a bug in the Alpha OSF/1 C compiler
     (OSF version 1.3, compiler version 3.11).  It miscompiles the
     following program:

     struct str
     {
       unsigned int i0;
     } s = { 0 };

     int
     main ()
     {
       unsigned long x;

       x = 0x100000000;
       x <<= (unsigned long) s.i0;
       if (x == 0)
	 printf ("failed\n");
       else
	 printf ("succeeded (%lx)\n", x);
     }
     */

  relocation >>= (bfd_vma) howto->rightshift;

  /* Shift everything up to where it's going to be used.  */
  relocation <<= (bfd_vma) howto->bitpos;

  /* Wait for the day when all have the mask in them.  */

  /* What we do:
     i instruction to be left alone
     o offset within instruction
     r relocation offset to apply
     S src mask
     D dst mask
     N ~dst mask
     A part 1
     B part 2
     R result

     Do this:
     ((	 i i i i i o o o o o  from bfd_get<size>
     and	   S S S S S) to get the size offset we want
     +	 r r r r r r r r r r) to get the final value to place
     and	   D D D D D  to chop to right size
     -----------------------
     =		   A A A A A
     And this:
     (	 i i i i i o o o o o  from bfd_get<size>
     and N N N N N	    ) get instruction
     -----------------------
     =	 B B B B B

     And then:
     (	 B B B B B
     or		   A A A A A)
     -----------------------
     =	 R R R R R R R R R R  put into bfd_put<size>
     */

  data = (bfd_byte *) data + octets;
  apply_reloc (abfd, data, howto, relocation);
  return flag;
}

/*
FUNCTION
	bfd_install_relocation

SYNOPSIS
	bfd_reloc_status_type bfd_install_relocation
	  (bfd *abfd,
	   arelent *reloc_entry,
	   void *data, bfd_vma data_start,
	   asection *input_section,
	   char **error_message);

DESCRIPTION
	This looks remarkably like <<bfd_perform_relocation>>, except it
	does not expect that the section contents have been filled in.
	I.e., it's suitable for use when creating, rather than applying
	a relocation.

	For now, this function should be considered reserved for the
	assembler.
*/

bfd_reloc_status_type
bfd_install_relocation (bfd *abfd,
			arelent *reloc_entry,
			void *data_start,
			bfd_vma data_start_offset,
			asection *input_section,
			char **error_message)
{
  bfd_vma relocation;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_size_type octets;
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;
  asymbol *symbol;
  bfd_byte *data;

  symbol = *(reloc_entry->sym_ptr_ptr);

  /* If there is a function supplied to handle this relocation type,
     call it.  It'll return `bfd_reloc_continue' if further processing
     can be done.  */
  if (howto && howto->special_function)
    {
      bfd_reloc_status_type cont;

      /* Note - we do not call bfd_reloc_offset_in_range here as the
	 reloc_entry->address field might actually be valid for the
	 backend concerned.  It is up to the special_function itself
	 to call bfd_reloc_offset_in_range if needed.  */
      cont = howto->special_function (abfd, reloc_entry, symbol,
				      /* XXX - Non-portable! */
				      ((bfd_byte *) data_start
				       - data_start_offset),
				      input_section, abfd, error_message);
      if (cont != bfd_reloc_continue)
	return cont;
    }

  if (howto->install_addend)
    relocation = reloc_entry->addend;
  else
    {
      if (bfd_is_abs_section (symbol->section))
	return bfd_reloc_ok;

      /* Work out which section the relocation is targeted at and the
	 initial relocation command value.  */

      /* Get symbol value.  (Common symbols are special.)  */
      if (bfd_is_com_section (symbol->section))
	relocation = 0;
      else
	relocation = symbol->value;

      reloc_target_output_section = symbol->section;

      /* Convert input-section-relative symbol value to absolute.  */
      if (! howto->partial_inplace)
	output_base = 0;
      else
	output_base = reloc_target_output_section->vma;

      /* If symbol addresses are in octets, convert to bytes.  */
      if (bfd_get_flavour (abfd) == bfd_target_elf_flavour
	  && (symbol->section->flags & SEC_ELF_OCTETS))
	output_base *= bfd_octets_per_byte (abfd, input_section);

      relocation += output_base;

      /* Add in supplied addend.  */
      relocation += reloc_entry->addend;

      /* Here the variable relocation holds the final address of the
	 symbol we are relocating against, plus any addend.  */

      if (howto->pc_relative)
	{
	  relocation -= input_section->vma;

	  if (howto->pcrel_offset && howto->partial_inplace)
	    relocation -= reloc_entry->address;
	}
    }

  if (!howto->partial_inplace)
    {
      reloc_entry->addend = relocation;
      return flag;
    }

  if (!howto->install_addend
      && abfd->xvec->flavour == bfd_target_coff_flavour)
    {
      /* This is just weird.  We're subtracting out the original
	 addend, so that for COFF the addend is ignored???  */
      relocation -= reloc_entry->addend;
      /* FIXME: There should be no target specific code here...  */
      if (strcmp (abfd->xvec->name, "coff-z8k") != 0)
	reloc_entry->addend = 0;
    }
  else
    reloc_entry->addend = relocation;

  /* Is the address of the relocation really within the section?  */
  octets = reloc_entry->address * bfd_octets_per_byte (abfd, input_section);
  if (!bfd_reloc_offset_in_range (howto, abfd, input_section, octets))
    return bfd_reloc_outofrange;

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.  */
  if (howto->complain_on_overflow != complain_overflow_dont)
    flag = bfd_check_overflow (howto->complain_on_overflow,
			       howto->bitsize,
			       howto->rightshift,
			       bfd_arch_bits_per_address (abfd),
			       relocation);

  relocation >>= (bfd_vma) howto->rightshift;

  /* Shift everything up to where it's going to be used.  */
  relocation <<= (bfd_vma) howto->bitpos;

  data = (bfd_byte *) data_start + (octets - data_start_offset);
  apply_reloc (abfd, data, howto, relocation);
  return flag;
}

/* This relocation routine is used by some of the backend linkers.
   They do not construct asymbol or arelent structures, so there is no
   reason for them to use bfd_perform_relocation.  Also,
   bfd_perform_relocation is so hacked up it is easier to write a new
   function than to try to deal with it.

   This routine does a final relocation.  Whether it is useful for a
   relocatable link depends upon how the object format defines
   relocations.

   FIXME: This routine ignores any special_function in the HOWTO,
   since the existing special_function values have been written for
   bfd_perform_relocation.

   HOWTO is the reloc howto information.
   INPUT_BFD is the BFD which the reloc applies to.
   INPUT_SECTION is the section which the reloc applies to.
   CONTENTS is the contents of the section.
   ADDRESS is the address of the reloc within INPUT_SECTION.
   VALUE is the value of the symbol the reloc refers to.
   ADDEND is the addend of the reloc.  */

bfd_reloc_status_type
_bfd_final_link_relocate (reloc_howto_type *howto,
			  bfd *input_bfd,
			  asection *input_section,
			  bfd_byte *contents,
			  bfd_vma address,
			  bfd_vma value,
			  bfd_vma addend)
{
  bfd_vma relocation;
  bfd_size_type octets = (address
			  * bfd_octets_per_byte (input_bfd, input_section));

  /* Sanity check the address.  */
  if (!bfd_reloc_offset_in_range (howto, input_bfd, input_section, octets))
    return bfd_reloc_outofrange;

  /* This function assumes that we are dealing with a basic relocation
     against a symbol.  We want to compute the value of the symbol to
     relocate to.  This is just VALUE, the value of the symbol, plus
     ADDEND, any addend associated with the reloc.  */
  relocation = value + addend;

  /* If the relocation is PC relative, we want to set RELOCATION to
     the distance between the symbol (currently in RELOCATION) and the
     location we are relocating.  Some targets (e.g., i386-aout)
     arrange for the contents of the section to be the negative of the
     offset of the location within the section; for such targets
     pcrel_offset is FALSE.  Other targets (e.g., ELF) simply leave
     the contents of the section as zero; for such targets
     pcrel_offset is TRUE.  If pcrel_offset is FALSE we do not need to
     subtract out the offset of the location within the section (which
     is just ADDRESS).  */
  if (howto->pc_relative)
    {
      relocation -= (input_section->output_section->vma
		     + input_section->output_offset);
      if (howto->pcrel_offset)
	relocation -= address;
    }

  return _bfd_relocate_contents (howto, input_bfd, relocation,
				 contents + octets);
}

/* Relocate a given location using a given value and howto.  */

bfd_reloc_status_type
_bfd_relocate_contents (reloc_howto_type *howto,
			bfd *input_bfd,
			bfd_vma relocation,
			bfd_byte *location)
{
  bfd_vma x;
  bfd_reloc_status_type flag;
  unsigned int rightshift = howto->rightshift;
  unsigned int bitpos = howto->bitpos;

  if (howto->negate)
    relocation = -relocation;

  /* Get the value we are going to relocate.  */
  x = read_reloc (input_bfd, location, howto);

  /* Check for overflow.  FIXME: We may drop bits during the addition
     which we don't check for.  We must either check at every single
     operation, which would be tedious, or we must do the computations
     in a type larger than bfd_vma, which would be inefficient.  */
  flag = bfd_reloc_ok;
  if (howto->complain_on_overflow != complain_overflow_dont)
    {
      bfd_vma addrmask, fieldmask, signmask, ss;
      bfd_vma a, b, sum;

      /* Get the values to be added together.  For signed and unsigned
	 relocations, we assume that all values should be truncated to
	 the size of an address.  For bitfields, all the bits matter.
	 See also bfd_check_overflow.  */
      fieldmask = N_ONES (howto->bitsize);
      signmask = ~fieldmask;
      addrmask = (N_ONES (bfd_arch_bits_per_address (input_bfd))
		  | (fieldmask << rightshift));
      a = (relocation & addrmask) >> rightshift;
      b = (x & howto->src_mask & addrmask) >> bitpos;
      addrmask >>= rightshift;

      switch (howto->complain_on_overflow)
	{
	case complain_overflow_signed:
	  /* If any sign bits are set, all sign bits must be set.
	     That is, A must be a valid negative address after
	     shifting.  */
	  signmask = ~(fieldmask >> 1);
	  /* Fall thru */

	case complain_overflow_bitfield:
	  /* Much like the signed check, but for a field one bit
	     wider.  We allow a bitfield to represent numbers in the
	     range -2**n to 2**n-1, where n is the number of bits in the
	     field.  Note that when bfd_vma is 32 bits, a 32-bit reloc
	     can't overflow, which is exactly what we want.  */
	  ss = a & signmask;
	  if (ss != 0 && ss != (addrmask & signmask))
	    flag = bfd_reloc_overflow;

	  /* We only need this next bit of code if the sign bit of B
	     is below the sign bit of A.  This would only happen if
	     SRC_MASK had fewer bits than BITSIZE.  Note that if
	     SRC_MASK has more bits than BITSIZE, we can get into
	     trouble; we would need to verify that B is in range, as
	     we do for A above.  */
	  ss = ((~howto->src_mask) >> 1) & howto->src_mask;
	  ss >>= bitpos;

	  /* Set all the bits above the sign bit.  */
	  b = (b ^ ss) - ss;

	  /* Now we can do the addition.  */
	  sum = a + b;

	  /* See if the result has the correct sign.  Bits above the
	     sign bit are junk now; ignore them.  If the sum is
	     positive, make sure we did not have all negative inputs;
	     if the sum is negative, make sure we did not have all
	     positive inputs.  The test below looks only at the sign
	     bits, and it really just
		 SIGN (A) == SIGN (B) && SIGN (A) != SIGN (SUM)

	     We mask with addrmask here to explicitly allow an address
	     wrap-around.  The Linux kernel relies on it, and it is
	     the only way to write assembler code which can run when
	     loaded at a location 0x80000000 away from the location at
	     which it is linked.  */
	  if (((~(a ^ b)) & (a ^ sum)) & signmask & addrmask)
	    flag = bfd_reloc_overflow;
	  break;

	case complain_overflow_unsigned:
	  /* Checking for an unsigned overflow is relatively easy:
	     trim the addresses and add, and trim the result as well.
	     Overflow is normally indicated when the result does not
	     fit in the field.  However, we also need to consider the
	     case when, e.g., fieldmask is 0x7fffffff or smaller, an
	     input is 0x80000000, and bfd_vma is only 32 bits; then we
	     will get sum == 0, but there is an overflow, since the
	     inputs did not fit in the field.  Instead of doing a
	     separate test, we can check for this by or-ing in the
	     operands when testing for the sum overflowing its final
	     field.  */
	  sum = (a + b) & addrmask;
	  if ((a | b | sum) & signmask)
	    flag = bfd_reloc_overflow;
	  break;

	default:
	  abort ();
	}
    }

  /* Put RELOCATION in the right bits.  */
  relocation >>= (bfd_vma) rightshift;
  relocation <<= (bfd_vma) bitpos;

  /* Add RELOCATION to the right bits of X.  */
  x = ((x & ~howto->dst_mask)
       | (((x & howto->src_mask) + relocation) & howto->dst_mask));

  /* Put the relocated value back in the object file.  */
  write_reloc (input_bfd, x, location, howto);
  return flag;
}

/* Clear a given location using a given howto, by applying a fixed relocation
   value and discarding any in-place addend.  This is used for fixed-up
   relocations against discarded symbols, to make ignorable debug or unwind
   information more obvious.  */

bfd_reloc_status_type
_bfd_clear_contents (reloc_howto_type *howto,
		     bfd *input_bfd,
		     asection *input_section,
		     bfd_byte *buf,
		     bfd_vma off)
{
  bfd_vma x;
  bfd_byte *location;

  if (!bfd_reloc_offset_in_range (howto, input_bfd, input_section, off))
    return bfd_reloc_outofrange;

  /* Get the value we are going to relocate.  */
  location = buf + off;
  x = read_reloc (input_bfd, location, howto);

  /* Zero out the unwanted bits of X.  */
  x &= ~howto->dst_mask;

  /* For a range list, use 1 instead of 0 as placeholder.  0
     would terminate the list, hiding any later entries.  */
  if (strcmp (bfd_section_name (input_section), ".debug_ranges") == 0
      && (howto->dst_mask & 1) != 0)
    x |= 1;

  /* Put the relocated value back in the object file.  */
  write_reloc (input_bfd, x, location, howto);
  return bfd_reloc_ok;
}

/*
DOCDD
INODE
	howto manager,  , typedef arelent, Relocations

SUBSECTION
	The howto manager

	When an application wants to create a relocation, but doesn't
	know what the target machine might call it, it can find out by
	using this bit of code.

*/

/*
DEFINITION
	bfd_reloc_code_real_type

DESCRIPTION
	The insides of a reloc code.  The idea is that, eventually, there
	will be one enumerator for every type of relocation we ever do.
	Pass one of these values to <<bfd_reloc_type_lookup>>, and it'll
	return a howto pointer.

	This does mean that the application must determine the correct
	enumerator value; you can't get a howto pointer from a random set
	of attributes.

SENUM
   bfd_reloc_code_real

ENUM
  BFD_RELOC_64
ENUMX
  BFD_RELOC_32
ENUMX
  BFD_RELOC_26
ENUMX
  BFD_RELOC_24
ENUMX
  BFD_RELOC_16
ENUMX
  BFD_RELOC_14
ENUMX
  BFD_RELOC_8
ENUMDOC
  Basic absolute relocations of N bits.

ENUM
  BFD_RELOC_64_PCREL
ENUMX
  BFD_RELOC_32_PCREL
ENUMX
  BFD_RELOC_24_PCREL
ENUMX
  BFD_RELOC_16_PCREL
ENUMX
  BFD_RELOC_12_PCREL
ENUMX
  BFD_RELOC_8_PCREL
ENUMDOC
  PC-relative relocations.  Sometimes these are relative to the address
of the relocation itself; sometimes they are relative to the start of
the section containing the relocation.  It depends on the specific target.

ENUM
  BFD_RELOC_32_SECREL
ENUMX
  BFD_RELOC_16_SECIDX
ENUMDOC
  Section relative relocations.  Some targets need this for DWARF2.

ENUM
  BFD_RELOC_32_GOT_PCREL
ENUMX
  BFD_RELOC_16_GOT_PCREL
ENUMX
  BFD_RELOC_8_GOT_PCREL
ENUMX
  BFD_RELOC_32_GOTOFF
ENUMX
  BFD_RELOC_16_GOTOFF
ENUMX
  BFD_RELOC_LO16_GOTOFF
ENUMX
  BFD_RELOC_HI16_GOTOFF
ENUMX
  BFD_RELOC_HI16_S_GOTOFF
ENUMX
  BFD_RELOC_8_GOTOFF
ENUMX
  BFD_RELOC_64_PLT_PCREL
ENUMX
  BFD_RELOC_32_PLT_PCREL
ENUMX
  BFD_RELOC_24_PLT_PCREL
ENUMX
  BFD_RELOC_16_PLT_PCREL
ENUMX
  BFD_RELOC_8_PLT_PCREL
ENUMX
  BFD_RELOC_64_PLTOFF
ENUMX
  BFD_RELOC_32_PLTOFF
ENUMX
  BFD_RELOC_16_PLTOFF
ENUMX
  BFD_RELOC_LO16_PLTOFF
ENUMX
  BFD_RELOC_HI16_PLTOFF
ENUMX
  BFD_RELOC_HI16_S_PLTOFF
ENUMX
  BFD_RELOC_8_PLTOFF
ENUMDOC
  For ELF.

ENUM
  BFD_RELOC_SIZE32
ENUMX
  BFD_RELOC_SIZE64
ENUMDOC
  Size relocations.

ENUM
  BFD_RELOC_68K_GLOB_DAT
ENUMX
  BFD_RELOC_68K_JMP_SLOT
ENUMX
  BFD_RELOC_68K_RELATIVE
ENUMX
  BFD_RELOC_68K_TLS_GD32
ENUMX
  BFD_RELOC_68K_TLS_GD16
ENUMX
  BFD_RELOC_68K_TLS_GD8
ENUMX
  BFD_RELOC_68K_TLS_LDM32
ENUMX
  BFD_RELOC_68K_TLS_LDM16
ENUMX
  BFD_RELOC_68K_TLS_LDM8
ENUMX
  BFD_RELOC_68K_TLS_LDO32
ENUMX
  BFD_RELOC_68K_TLS_LDO16
ENUMX
  BFD_RELOC_68K_TLS_LDO8
ENUMX
  BFD_RELOC_68K_TLS_IE32
ENUMX
  BFD_RELOC_68K_TLS_IE16
ENUMX
  BFD_RELOC_68K_TLS_IE8
ENUMX
  BFD_RELOC_68K_TLS_LE32
ENUMX
  BFD_RELOC_68K_TLS_LE16
ENUMX
  BFD_RELOC_68K_TLS_LE8
ENUMDOC
  Relocations used by 68K ELF.

ENUM
  BFD_RELOC_32_BASEREL
ENUMX
  BFD_RELOC_16_BASEREL
ENUMX
  BFD_RELOC_LO16_BASEREL
ENUMX
  BFD_RELOC_HI16_BASEREL
ENUMX
  BFD_RELOC_HI16_S_BASEREL
ENUMX
  BFD_RELOC_8_BASEREL
ENUMX
  BFD_RELOC_RVA
ENUMDOC
  Linkage-table relative.

ENUM
  BFD_RELOC_8_FFnn
ENUMDOC
  Absolute 8-bit relocation, but used to form an address like 0xFFnn.

ENUM
  BFD_RELOC_32_PCREL_S2
ENUMX
  BFD_RELOC_16_PCREL_S2
ENUMX
  BFD_RELOC_23_PCREL_S2
ENUMDOC
  These PC-relative relocations are stored as word displacements --
i.e., byte displacements shifted right two bits.  The 30-bit word
displacement (<<32_PCREL_S2>> -- 32 bits, shifted 2) is used on the
SPARC.  (SPARC tools generally refer to this as <<WDISP30>>.)  The
signed 16-bit displacement is used on the MIPS, and the 23-bit
displacement is used on the Alpha.

ENUM
  BFD_RELOC_HI22
ENUMX
  BFD_RELOC_LO10
ENUMDOC
  High 22 bits and low 10 bits of 32-bit value, placed into lower bits of
the target word.  These are used on the SPARC.

ENUM
  BFD_RELOC_GPREL16
ENUMX
  BFD_RELOC_GPREL32
ENUMDOC
  For systems that allocate a Global Pointer register, these are
displacements off that register.  These relocation types are
handled specially, because the value the register will have is
decided relatively late.

ENUM
  BFD_RELOC_NONE
ENUMX
  BFD_RELOC_SPARC_WDISP22
ENUMX
  BFD_RELOC_SPARC22
ENUMX
  BFD_RELOC_SPARC13
ENUMX
  BFD_RELOC_SPARC_GOT10
ENUMX
  BFD_RELOC_SPARC_GOT13
ENUMX
  BFD_RELOC_SPARC_GOT22
ENUMX
  BFD_RELOC_SPARC_PC10
ENUMX
  BFD_RELOC_SPARC_PC22
ENUMX
  BFD_RELOC_SPARC_WPLT30
ENUMX
  BFD_RELOC_SPARC_COPY
ENUMX
  BFD_RELOC_SPARC_GLOB_DAT
ENUMX
  BFD_RELOC_SPARC_JMP_SLOT
ENUMX
  BFD_RELOC_SPARC_RELATIVE
ENUMX
  BFD_RELOC_SPARC_UA16
ENUMX
  BFD_RELOC_SPARC_UA32
ENUMX
  BFD_RELOC_SPARC_UA64
ENUMX
  BFD_RELOC_SPARC_GOTDATA_HIX22
ENUMX
  BFD_RELOC_SPARC_GOTDATA_LOX10
ENUMX
  BFD_RELOC_SPARC_GOTDATA_OP_HIX22
ENUMX
  BFD_RELOC_SPARC_GOTDATA_OP_LOX10
ENUMX
  BFD_RELOC_SPARC_GOTDATA_OP
ENUMX
  BFD_RELOC_SPARC_JMP_IREL
ENUMX
  BFD_RELOC_SPARC_IRELATIVE
ENUMDOC
  SPARC ELF relocations.  There is probably some overlap with other
  relocation types already defined.

ENUM
  BFD_RELOC_SPARC_BASE13
ENUMX
  BFD_RELOC_SPARC_BASE22
ENUMDOC
  I think these are specific to SPARC a.out (e.g., Sun 4).

ENUMEQ
  BFD_RELOC_SPARC_64
  BFD_RELOC_64
ENUMX
  BFD_RELOC_SPARC_10
ENUMX
  BFD_RELOC_SPARC_11
ENUMX
  BFD_RELOC_SPARC_OLO10
ENUMX
  BFD_RELOC_SPARC_HH22
ENUMX
  BFD_RELOC_SPARC_HM10
ENUMX
  BFD_RELOC_SPARC_LM22
ENUMX
  BFD_RELOC_SPARC_PC_HH22
ENUMX
  BFD_RELOC_SPARC_PC_HM10
ENUMX
  BFD_RELOC_SPARC_PC_LM22
ENUMX
  BFD_RELOC_SPARC_WDISP16
ENUMX
  BFD_RELOC_SPARC_WDISP19
ENUMX
  BFD_RELOC_SPARC_7
ENUMX
  BFD_RELOC_SPARC_6
ENUMX
  BFD_RELOC_SPARC_5
ENUMEQX
  BFD_RELOC_SPARC_DISP64
  BFD_RELOC_64_PCREL
ENUMX
  BFD_RELOC_SPARC_PLT32
ENUMX
  BFD_RELOC_SPARC_PLT64
ENUMX
  BFD_RELOC_SPARC_HIX22
ENUMX
  BFD_RELOC_SPARC_LOX10
ENUMX
  BFD_RELOC_SPARC_H44
ENUMX
  BFD_RELOC_SPARC_M44
ENUMX
  BFD_RELOC_SPARC_L44
ENUMX
  BFD_RELOC_SPARC_REGISTER
ENUMX
  BFD_RELOC_SPARC_H34
ENUMX
  BFD_RELOC_SPARC_SIZE32
ENUMX
  BFD_RELOC_SPARC_SIZE64
ENUMX
  BFD_RELOC_SPARC_WDISP10
ENUMDOC
  SPARC64 relocations

ENUM
  BFD_RELOC_SPARC_REV32
ENUMDOC
  SPARC little endian relocation
ENUM
  BFD_RELOC_SPARC_TLS_GD_HI22
ENUMX
  BFD_RELOC_SPARC_TLS_GD_LO10
ENUMX
  BFD_RELOC_SPARC_TLS_GD_ADD
ENUMX
  BFD_RELOC_SPARC_TLS_GD_CALL
ENUMX
  BFD_RELOC_SPARC_TLS_LDM_HI22
ENUMX
  BFD_RELOC_SPARC_TLS_LDM_LO10
ENUMX
  BFD_RELOC_SPARC_TLS_LDM_ADD
ENUMX
  BFD_RELOC_SPARC_TLS_LDM_CALL
ENUMX
  BFD_RELOC_SPARC_TLS_LDO_HIX22
ENUMX
  BFD_RELOC_SPARC_TLS_LDO_LOX10
ENUMX
  BFD_RELOC_SPARC_TLS_LDO_ADD
ENUMX
  BFD_RELOC_SPARC_TLS_IE_HI22
ENUMX
  BFD_RELOC_SPARC_TLS_IE_LO10
ENUMX
  BFD_RELOC_SPARC_TLS_IE_LD
ENUMX
  BFD_RELOC_SPARC_TLS_IE_LDX
ENUMX
  BFD_RELOC_SPARC_TLS_IE_ADD
ENUMX
  BFD_RELOC_SPARC_TLS_LE_HIX22
ENUMX
  BFD_RELOC_SPARC_TLS_LE_LOX10
ENUMX
  BFD_RELOC_SPARC_TLS_DTPMOD32
ENUMX
  BFD_RELOC_SPARC_TLS_DTPMOD64
ENUMX
  BFD_RELOC_SPARC_TLS_DTPOFF32
ENUMX
  BFD_RELOC_SPARC_TLS_DTPOFF64
ENUMX
  BFD_RELOC_SPARC_TLS_TPOFF32
ENUMX
  BFD_RELOC_SPARC_TLS_TPOFF64
ENUMDOC
  SPARC TLS relocations

ENUM
  BFD_RELOC_SPU_IMM7
ENUMX
  BFD_RELOC_SPU_IMM8
ENUMX
  BFD_RELOC_SPU_IMM10
ENUMX
  BFD_RELOC_SPU_IMM10W
ENUMX
  BFD_RELOC_SPU_IMM16
ENUMX
  BFD_RELOC_SPU_IMM16W
ENUMX
  BFD_RELOC_SPU_IMM18
ENUMX
  BFD_RELOC_SPU_PCREL9a
ENUMX
  BFD_RELOC_SPU_PCREL9b
ENUMX
  BFD_RELOC_SPU_PCREL16
ENUMX
  BFD_RELOC_SPU_LO16
ENUMX
  BFD_RELOC_SPU_HI16
ENUMX
  BFD_RELOC_SPU_PPU32
ENUMX
  BFD_RELOC_SPU_PPU64
ENUMX
  BFD_RELOC_SPU_ADD_PIC
ENUMDOC
  SPU Relocations.

ENUM
  BFD_RELOC_ALPHA_GPDISP_HI16
ENUMDOC
  Alpha ECOFF and ELF relocations.  Some of these treat the symbol or
     "addend" in some special way.
  For GPDISP_HI16 ("gpdisp") relocations, the symbol is ignored when
     writing; when reading, it will be the absolute section symbol.  The
     addend is the displacement in bytes of the "lda" instruction from
     the "ldah" instruction (which is at the address of this reloc).
ENUM
  BFD_RELOC_ALPHA_GPDISP_LO16
ENUMDOC
  For GPDISP_LO16 ("ignore") relocations, the symbol is handled as
     with GPDISP_HI16 relocs.  The addend is ignored when writing the
     relocations out, and is filled in with the file's GP value on
     reading, for convenience.

ENUM
  BFD_RELOC_ALPHA_GPDISP
ENUMDOC
  The ELF GPDISP relocation is exactly the same as the GPDISP_HI16
     relocation except that there is no accompanying GPDISP_LO16
     relocation.

ENUM
  BFD_RELOC_ALPHA_LITERAL
ENUMX
  BFD_RELOC_ALPHA_ELF_LITERAL
ENUMX
  BFD_RELOC_ALPHA_LITUSE
ENUMDOC
  The Alpha LITERAL/LITUSE relocs are produced by a symbol reference;
     the assembler turns it into a LDQ instruction to load the address of
     the symbol, and then fills in a register in the real instruction.

     The LITERAL reloc, at the LDQ instruction, refers to the .lita
     section symbol.  The addend is ignored when writing, but is filled
     in with the file's GP value on reading, for convenience, as with the
     GPDISP_LO16 reloc.

     The ELF_LITERAL reloc is somewhere between 16_GOTOFF and GPDISP_LO16.
     It should refer to the symbol to be referenced, as with 16_GOTOFF,
     but it generates output not based on the position within the .got
     section, but relative to the GP value chosen for the file during the
     final link stage.

     The LITUSE reloc, on the instruction using the loaded address, gives
     information to the linker that it might be able to use to optimize
     away some literal section references.  The symbol is ignored (read
     as the absolute section symbol), and the "addend" indicates the type
     of instruction using the register:
	      1 - "memory" fmt insn
	      2 - byte-manipulation (byte offset reg)
	      3 - jsr (target of branch)

ENUM
  BFD_RELOC_ALPHA_HINT
ENUMDOC
  The HINT relocation indicates a value that should be filled into the
     "hint" field of a jmp/jsr/ret instruction, for possible branch-
     prediction logic which may be provided on some processors.

ENUM
  BFD_RELOC_ALPHA_LINKAGE
ENUMDOC
  The LINKAGE relocation outputs a linkage pair in the object file,
     which is filled by the linker.

ENUM
  BFD_RELOC_ALPHA_CODEADDR
ENUMDOC
  The CODEADDR relocation outputs a STO_CA in the object file,
     which is filled by the linker.

ENUM
  BFD_RELOC_ALPHA_GPREL_HI16
ENUMX
  BFD_RELOC_ALPHA_GPREL_LO16
ENUMDOC
  The GPREL_HI/LO relocations together form a 32-bit offset from the
     GP register.

ENUM
  BFD_RELOC_ALPHA_BRSGP
ENUMDOC
  Like BFD_RELOC_23_PCREL_S2, except that the source and target must
  share a common GP, and the target address is adjusted for
  STO_ALPHA_STD_GPLOAD.

ENUM
  BFD_RELOC_ALPHA_NOP
ENUMDOC
  The NOP relocation outputs a NOP if the longword displacement
     between two procedure entry points is < 2^21.

ENUM
  BFD_RELOC_ALPHA_BSR
ENUMDOC
  The BSR relocation outputs a BSR if the longword displacement
     between two procedure entry points is < 2^21.

ENUM
  BFD_RELOC_ALPHA_LDA
ENUMDOC
  The LDA relocation outputs a LDA if the longword displacement
     between two procedure entry points is < 2^16.

ENUM
  BFD_RELOC_ALPHA_BOH
ENUMDOC
  The BOH relocation outputs a BSR if the longword displacement
     between two procedure entry points is < 2^21, or else a hint.

ENUM
  BFD_RELOC_ALPHA_TLSGD
ENUMX
  BFD_RELOC_ALPHA_TLSLDM
ENUMX
  BFD_RELOC_ALPHA_DTPMOD64
ENUMX
  BFD_RELOC_ALPHA_GOTDTPREL16
ENUMX
  BFD_RELOC_ALPHA_DTPREL64
ENUMX
  BFD_RELOC_ALPHA_DTPREL_HI16
ENUMX
  BFD_RELOC_ALPHA_DTPREL_LO16
ENUMX
  BFD_RELOC_ALPHA_DTPREL16
ENUMX
  BFD_RELOC_ALPHA_GOTTPREL16
ENUMX
  BFD_RELOC_ALPHA_TPREL64
ENUMX
  BFD_RELOC_ALPHA_TPREL_HI16
ENUMX
  BFD_RELOC_ALPHA_TPREL_LO16
ENUMX
  BFD_RELOC_ALPHA_TPREL16
ENUMDOC
  Alpha thread-local storage relocations.

ENUM
  BFD_RELOC_MIPS_JMP
ENUMX
  BFD_RELOC_MICROMIPS_JMP
ENUMDOC
  The MIPS jump instruction.

ENUM
  BFD_RELOC_MIPS16_JMP
ENUMDOC
  The MIPS16 jump instruction.

ENUM
  BFD_RELOC_MIPS16_GPREL
ENUMDOC
  MIPS16 GP relative reloc.

ENUM
  BFD_RELOC_HI16
ENUMDOC
  High 16 bits of 32-bit value; simple reloc.

ENUM
  BFD_RELOC_HI16_S
ENUMDOC
  High 16 bits of 32-bit value but the low 16 bits will be sign
     extended and added to form the final result.  If the low 16
     bits form a negative number, we need to add one to the high value
     to compensate for the borrow when the low bits are added.

ENUM
  BFD_RELOC_LO16
ENUMDOC
  Low 16 bits.

ENUM
  BFD_RELOC_HI16_PCREL
ENUMDOC
  High 16 bits of 32-bit pc-relative value
ENUM
  BFD_RELOC_HI16_S_PCREL
ENUMDOC
  High 16 bits of 32-bit pc-relative value, adjusted
ENUM
  BFD_RELOC_LO16_PCREL
ENUMDOC
  Low 16 bits of pc-relative value

ENUM
  BFD_RELOC_MIPS16_GOT16
ENUMX
  BFD_RELOC_MIPS16_CALL16
ENUMDOC
  Equivalent of BFD_RELOC_MIPS_*, but with the MIPS16 layout of
     16-bit immediate fields
ENUM
  BFD_RELOC_MIPS16_HI16
ENUMDOC
  MIPS16 high 16 bits of 32-bit value.
ENUM
  BFD_RELOC_MIPS16_HI16_S
ENUMDOC
  MIPS16 high 16 bits of 32-bit value but the low 16 bits will be sign
     extended and added to form the final result.  If the low 16
     bits form a negative number, we need to add one to the high value
     to compensate for the borrow when the low bits are added.
ENUM
  BFD_RELOC_MIPS16_LO16
ENUMDOC
  MIPS16 low 16 bits.

ENUM
  BFD_RELOC_MIPS16_TLS_GD
ENUMX
  BFD_RELOC_MIPS16_TLS_LDM
ENUMX
  BFD_RELOC_MIPS16_TLS_DTPREL_HI16
ENUMX
  BFD_RELOC_MIPS16_TLS_DTPREL_LO16
ENUMX
  BFD_RELOC_MIPS16_TLS_GOTTPREL
ENUMX
  BFD_RELOC_MIPS16_TLS_TPREL_HI16
ENUMX
  BFD_RELOC_MIPS16_TLS_TPREL_LO16
ENUMDOC
  MIPS16 TLS relocations

ENUM
  BFD_RELOC_MIPS_LITERAL
ENUMX
  BFD_RELOC_MICROMIPS_LITERAL
ENUMDOC
  Relocation against a MIPS literal section.

ENUM
  BFD_RELOC_MICROMIPS_7_PCREL_S1
ENUMX
  BFD_RELOC_MICROMIPS_10_PCREL_S1
ENUMX
  BFD_RELOC_MICROMIPS_16_PCREL_S1
ENUMDOC
  microMIPS PC-relative relocations.

ENUM
  BFD_RELOC_MIPS16_16_PCREL_S1
ENUMDOC
  MIPS16 PC-relative relocation.

ENUM
  BFD_RELOC_MIPS_21_PCREL_S2
ENUMX
  BFD_RELOC_MIPS_26_PCREL_S2
ENUMX
  BFD_RELOC_MIPS_18_PCREL_S3
ENUMX
  BFD_RELOC_MIPS_19_PCREL_S2
ENUMDOC
  MIPS PC-relative relocations.

ENUM
  BFD_RELOC_MICROMIPS_GPREL16
ENUMX
  BFD_RELOC_MICROMIPS_HI16
ENUMX
  BFD_RELOC_MICROMIPS_HI16_S
ENUMX
  BFD_RELOC_MICROMIPS_LO16
ENUMDOC
  microMIPS versions of generic BFD relocs.

ENUM
  BFD_RELOC_MIPS_GOT16
ENUMX
  BFD_RELOC_MICROMIPS_GOT16
ENUMX
  BFD_RELOC_MIPS_CALL16
ENUMX
  BFD_RELOC_MICROMIPS_CALL16
ENUMX
  BFD_RELOC_MIPS_GOT_HI16
ENUMX
  BFD_RELOC_MICROMIPS_GOT_HI16
ENUMX
  BFD_RELOC_MIPS_GOT_LO16
ENUMX
  BFD_RELOC_MICROMIPS_GOT_LO16
ENUMX
  BFD_RELOC_MIPS_CALL_HI16
ENUMX
  BFD_RELOC_MICROMIPS_CALL_HI16
ENUMX
  BFD_RELOC_MIPS_CALL_LO16
ENUMX
  BFD_RELOC_MICROMIPS_CALL_LO16
ENUMX
  BFD_RELOC_MIPS_SUB
ENUMX
  BFD_RELOC_MICROMIPS_SUB
ENUMX
  BFD_RELOC_MIPS_GOT_PAGE
ENUMX
  BFD_RELOC_MICROMIPS_GOT_PAGE
ENUMX
  BFD_RELOC_MIPS_GOT_OFST
ENUMX
  BFD_RELOC_MICROMIPS_GOT_OFST
ENUMX
  BFD_RELOC_MIPS_GOT_DISP
ENUMX
  BFD_RELOC_MICROMIPS_GOT_DISP
ENUMX
  BFD_RELOC_MIPS_SHIFT5
ENUMX
  BFD_RELOC_MIPS_SHIFT6
ENUMX
  BFD_RELOC_MIPS_INSERT_A
ENUMX
  BFD_RELOC_MIPS_INSERT_B
ENUMX
  BFD_RELOC_MIPS_DELETE
ENUMX
  BFD_RELOC_MIPS_HIGHEST
ENUMX
  BFD_RELOC_MICROMIPS_HIGHEST
ENUMX
  BFD_RELOC_MIPS_HIGHER
ENUMX
  BFD_RELOC_MICROMIPS_HIGHER
ENUMX
  BFD_RELOC_MIPS_SCN_DISP
ENUMX
  BFD_RELOC_MICROMIPS_SCN_DISP
ENUMX
  BFD_RELOC_MIPS_16
ENUMX
  BFD_RELOC_MIPS_RELGOT
ENUMX
  BFD_RELOC_MIPS_JALR
ENUMX
  BFD_RELOC_MICROMIPS_JALR
ENUMX
  BFD_RELOC_MIPS_TLS_DTPMOD32
ENUMX
  BFD_RELOC_MIPS_TLS_DTPREL32
ENUMX
  BFD_RELOC_MIPS_TLS_DTPMOD64
ENUMX
  BFD_RELOC_MIPS_TLS_DTPREL64
ENUMX
  BFD_RELOC_MIPS_TLS_GD
ENUMX
  BFD_RELOC_MICROMIPS_TLS_GD
ENUMX
  BFD_RELOC_MIPS_TLS_LDM
ENUMX
  BFD_RELOC_MICROMIPS_TLS_LDM
ENUMX
  BFD_RELOC_MIPS_TLS_DTPREL_HI16
ENUMX
  BFD_RELOC_MICROMIPS_TLS_DTPREL_HI16
ENUMX
  BFD_RELOC_MIPS_TLS_DTPREL_LO16
ENUMX
  BFD_RELOC_MICROMIPS_TLS_DTPREL_LO16
ENUMX
  BFD_RELOC_MIPS_TLS_GOTTPREL
ENUMX
  BFD_RELOC_MICROMIPS_TLS_GOTTPREL
ENUMX
  BFD_RELOC_MIPS_TLS_TPREL32
ENUMX
  BFD_RELOC_MIPS_TLS_TPREL64
ENUMX
  BFD_RELOC_MIPS_TLS_TPREL_HI16
ENUMX
  BFD_RELOC_MICROMIPS_TLS_TPREL_HI16
ENUMX
  BFD_RELOC_MIPS_TLS_TPREL_LO16
ENUMX
  BFD_RELOC_MICROMIPS_TLS_TPREL_LO16
ENUMX
  BFD_RELOC_MIPS_EH
ENUMDOC
  MIPS ELF relocations.
COMMENT

ENUM
  BFD_RELOC_MIPS_COPY
ENUMX
  BFD_RELOC_MIPS_JUMP_SLOT
ENUMDOC
  MIPS ELF relocations (VxWorks and PLT extensions).
COMMENT

ENUM
  BFD_RELOC_MOXIE_10_PCREL
ENUMDOC
  Moxie ELF relocations.
COMMENT

ENUM
  BFD_RELOC_FT32_10
ENUMX
  BFD_RELOC_FT32_20
ENUMX
  BFD_RELOC_FT32_17
ENUMX
  BFD_RELOC_FT32_18
ENUMX
  BFD_RELOC_FT32_RELAX
ENUMX
  BFD_RELOC_FT32_SC0
ENUMX
  BFD_RELOC_FT32_SC1
ENUMX
  BFD_RELOC_FT32_15
ENUMX
  BFD_RELOC_FT32_DIFF32
ENUMDOC
  FT32 ELF relocations.
COMMENT

ENUM
  BFD_RELOC_FRV_LABEL16
ENUMX
  BFD_RELOC_FRV_LABEL24
ENUMX
  BFD_RELOC_FRV_LO16
ENUMX
  BFD_RELOC_FRV_HI16
ENUMX
  BFD_RELOC_FRV_GPREL12
ENUMX
  BFD_RELOC_FRV_GPRELU12
ENUMX
  BFD_RELOC_FRV_GPREL32
ENUMX
  BFD_RELOC_FRV_GPRELHI
ENUMX
  BFD_RELOC_FRV_GPRELLO
ENUMX
  BFD_RELOC_FRV_GOT12
ENUMX
  BFD_RELOC_FRV_GOTHI
ENUMX
  BFD_RELOC_FRV_GOTLO
ENUMX
  BFD_RELOC_FRV_FUNCDESC
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOT12
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOTHI
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOTLO
ENUMX
  BFD_RELOC_FRV_FUNCDESC_VALUE
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOTOFF12
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOTOFFHI
ENUMX
  BFD_RELOC_FRV_FUNCDESC_GOTOFFLO
ENUMX
  BFD_RELOC_FRV_GOTOFF12
ENUMX
  BFD_RELOC_FRV_GOTOFFHI
ENUMX
  BFD_RELOC_FRV_GOTOFFLO
ENUMX
  BFD_RELOC_FRV_GETTLSOFF
ENUMX
  BFD_RELOC_FRV_TLSDESC_VALUE
ENUMX
  BFD_RELOC_FRV_GOTTLSDESC12
ENUMX
  BFD_RELOC_FRV_GOTTLSDESCHI
ENUMX
  BFD_RELOC_FRV_GOTTLSDESCLO
ENUMX
  BFD_RELOC_FRV_TLSMOFF12
ENUMX
  BFD_RELOC_FRV_TLSMOFFHI
ENUMX
  BFD_RELOC_FRV_TLSMOFFLO
ENUMX
  BFD_RELOC_FRV_GOTTLSOFF12
ENUMX
  BFD_RELOC_FRV_GOTTLSOFFHI
ENUMX
  BFD_RELOC_FRV_GOTTLSOFFLO
ENUMX
  BFD_RELOC_FRV_TLSOFF
ENUMX
  BFD_RELOC_FRV_TLSDESC_RELAX
ENUMX
  BFD_RELOC_FRV_GETTLSOFF_RELAX
ENUMX
  BFD_RELOC_FRV_TLSOFF_RELAX
ENUMX
  BFD_RELOC_FRV_TLSMOFF
ENUMDOC
  Fujitsu Frv Relocations.
COMMENT

ENUM
  BFD_RELOC_MN10300_GOTOFF24
ENUMDOC
  This is a 24bit GOT-relative reloc for the mn10300.
ENUM
  BFD_RELOC_MN10300_GOT32
ENUMDOC
  This is a 32bit GOT-relative reloc for the mn10300, offset by two bytes
  in the instruction.
ENUM
  BFD_RELOC_MN10300_GOT24
ENUMDOC
  This is a 24bit GOT-relative reloc for the mn10300, offset by two bytes
  in the instruction.
ENUM
  BFD_RELOC_MN10300_GOT16
ENUMDOC
  This is a 16bit GOT-relative reloc for the mn10300, offset by two bytes
  in the instruction.
ENUM
  BFD_RELOC_MN10300_COPY
ENUMDOC
  Copy symbol at runtime.
ENUM
  BFD_RELOC_MN10300_GLOB_DAT
ENUMDOC
  Create GOT entry.
ENUM
  BFD_RELOC_MN10300_JMP_SLOT
ENUMDOC
  Create PLT entry.
ENUM
  BFD_RELOC_MN10300_RELATIVE
ENUMDOC
  Adjust by program base.
ENUM
  BFD_RELOC_MN10300_SYM_DIFF
ENUMDOC
  Together with another reloc targeted at the same location,
  allows for a value that is the difference of two symbols
  in the same section.
ENUM
  BFD_RELOC_MN10300_ALIGN
ENUMDOC
  The addend of this reloc is an alignment power that must
  be honoured at the offset's location, regardless of linker
  relaxation.
ENUM
  BFD_RELOC_MN10300_TLS_GD
ENUMX
  BFD_RELOC_MN10300_TLS_LD
ENUMX
  BFD_RELOC_MN10300_TLS_LDO
ENUMX
  BFD_RELOC_MN10300_TLS_GOTIE
ENUMX
  BFD_RELOC_MN10300_TLS_IE
ENUMX
  BFD_RELOC_MN10300_TLS_LE
ENUMX
  BFD_RELOC_MN10300_TLS_DTPMOD
ENUMX
  BFD_RELOC_MN10300_TLS_DTPOFF
ENUMX
  BFD_RELOC_MN10300_TLS_TPOFF
ENUMDOC
  Various TLS-related relocations.
ENUM
  BFD_RELOC_MN10300_32_PCREL
ENUMDOC
  This is a 32bit pcrel reloc for the mn10300, offset by two bytes in the
  instruction.
ENUM
  BFD_RELOC_MN10300_16_PCREL
ENUMDOC
  This is a 16bit pcrel reloc for the mn10300, offset by two bytes in the
  instruction.
COMMENT

ENUM
  BFD_RELOC_386_GOT32
ENUMX
  BFD_RELOC_386_PLT32
ENUMX
  BFD_RELOC_386_COPY
ENUMX
  BFD_RELOC_386_GLOB_DAT
ENUMX
  BFD_RELOC_386_JUMP_SLOT
ENUMX
  BFD_RELOC_386_RELATIVE
ENUMX
  BFD_RELOC_386_GOTOFF
ENUMX
  BFD_RELOC_386_GOTPC
ENUMX
  BFD_RELOC_386_TLS_TPOFF
ENUMX
  BFD_RELOC_386_TLS_IE
ENUMX
  BFD_RELOC_386_TLS_GOTIE
ENUMX
  BFD_RELOC_386_TLS_LE
ENUMX
  BFD_RELOC_386_TLS_GD
ENUMX
  BFD_RELOC_386_TLS_LDM
ENUMX
  BFD_RELOC_386_TLS_LDO_32
ENUMX
  BFD_RELOC_386_TLS_IE_32
ENUMX
  BFD_RELOC_386_TLS_LE_32
ENUMX
  BFD_RELOC_386_TLS_DTPMOD32
ENUMX
  BFD_RELOC_386_TLS_DTPOFF32
ENUMX
  BFD_RELOC_386_TLS_TPOFF32
ENUMX
  BFD_RELOC_386_TLS_GOTDESC
ENUMX
  BFD_RELOC_386_TLS_DESC_CALL
ENUMX
  BFD_RELOC_386_TLS_DESC
ENUMX
  BFD_RELOC_386_IRELATIVE
ENUMX
  BFD_RELOC_386_GOT32X
ENUMDOC
  i386/elf relocations

ENUM
  BFD_RELOC_X86_64_GOT32
ENUMX
  BFD_RELOC_X86_64_PLT32
ENUMX
  BFD_RELOC_X86_64_COPY
ENUMX
  BFD_RELOC_X86_64_GLOB_DAT
ENUMX
  BFD_RELOC_X86_64_JUMP_SLOT
ENUMX
  BFD_RELOC_X86_64_RELATIVE
ENUMX
  BFD_RELOC_X86_64_GOTPCREL
ENUMX
  BFD_RELOC_X86_64_32S
ENUMX
  BFD_RELOC_X86_64_DTPMOD64
ENUMX
  BFD_RELOC_X86_64_DTPOFF64
ENUMX
  BFD_RELOC_X86_64_TPOFF64
ENUMX
  BFD_RELOC_X86_64_TLSGD
ENUMX
  BFD_RELOC_X86_64_TLSLD
ENUMX
  BFD_RELOC_X86_64_DTPOFF32
ENUMX
  BFD_RELOC_X86_64_GOTTPOFF
ENUMX
  BFD_RELOC_X86_64_TPOFF32
ENUMX
  BFD_RELOC_X86_64_GOTOFF64
ENUMX
  BFD_RELOC_X86_64_GOTPC32
ENUMX
  BFD_RELOC_X86_64_GOT64
ENUMX
  BFD_RELOC_X86_64_GOTPCREL64
ENUMX
  BFD_RELOC_X86_64_GOTPC64
ENUMX
  BFD_RELOC_X86_64_GOTPLT64
ENUMX
  BFD_RELOC_X86_64_PLTOFF64
ENUMX
  BFD_RELOC_X86_64_GOTPC32_TLSDESC
ENUMX
  BFD_RELOC_X86_64_TLSDESC_CALL
ENUMX
  BFD_RELOC_X86_64_TLSDESC
ENUMX
  BFD_RELOC_X86_64_IRELATIVE
ENUMX
  BFD_RELOC_X86_64_PC32_BND
ENUMX
  BFD_RELOC_X86_64_PLT32_BND
ENUMX
  BFD_RELOC_X86_64_GOTPCRELX
ENUMX
  BFD_RELOC_X86_64_REX_GOTPCRELX
ENUMDOC
  x86-64/elf relocations

ENUM
  BFD_RELOC_NS32K_IMM_8
ENUMX
  BFD_RELOC_NS32K_IMM_16
ENUMX
  BFD_RELOC_NS32K_IMM_32
ENUMX
  BFD_RELOC_NS32K_IMM_8_PCREL
ENUMX
  BFD_RELOC_NS32K_IMM_16_PCREL
ENUMX
  BFD_RELOC_NS32K_IMM_32_PCREL
ENUMX
  BFD_RELOC_NS32K_DISP_8
ENUMX
  BFD_RELOC_NS32K_DISP_16
ENUMX
  BFD_RELOC_NS32K_DISP_32
ENUMX
  BFD_RELOC_NS32K_DISP_8_PCREL
ENUMX
  BFD_RELOC_NS32K_DISP_16_PCREL
ENUMX
  BFD_RELOC_NS32K_DISP_32_PCREL
ENUMDOC
  ns32k relocations

ENUM
  BFD_RELOC_PDP11_DISP_8_PCREL
ENUMX
  BFD_RELOC_PDP11_DISP_6_PCREL
ENUMDOC
  PDP11 relocations

ENUM
  BFD_RELOC_PJ_CODE_HI16
ENUMX
  BFD_RELOC_PJ_CODE_LO16
ENUMX
  BFD_RELOC_PJ_CODE_DIR16
ENUMX
  BFD_RELOC_PJ_CODE_DIR32
ENUMX
  BFD_RELOC_PJ_CODE_REL16
ENUMX
  BFD_RELOC_PJ_CODE_REL32
ENUMDOC
  Picojava relocs.  Not all of these appear in object files.

ENUM
  BFD_RELOC_PPC_B26
ENUMX
  BFD_RELOC_PPC_BA26
ENUMX
  BFD_RELOC_PPC_TOC16
ENUMX
  BFD_RELOC_PPC_TOC16_LO
ENUMX
  BFD_RELOC_PPC_TOC16_HI
ENUMX
  BFD_RELOC_PPC_B16
ENUMX
  BFD_RELOC_PPC_B16_BRTAKEN
ENUMX
  BFD_RELOC_PPC_B16_BRNTAKEN
ENUMX
  BFD_RELOC_PPC_BA16
ENUMX
  BFD_RELOC_PPC_BA16_BRTAKEN
ENUMX
  BFD_RELOC_PPC_BA16_BRNTAKEN
ENUMX
  BFD_RELOC_PPC_COPY
ENUMX
  BFD_RELOC_PPC_GLOB_DAT
ENUMX
  BFD_RELOC_PPC_JMP_SLOT
ENUMX
  BFD_RELOC_PPC_RELATIVE
ENUMX
  BFD_RELOC_PPC_LOCAL24PC
ENUMX
  BFD_RELOC_PPC_EMB_NADDR32
ENUMX
  BFD_RELOC_PPC_EMB_NADDR16
ENUMX
  BFD_RELOC_PPC_EMB_NADDR16_LO
ENUMX
  BFD_RELOC_PPC_EMB_NADDR16_HI
ENUMX
  BFD_RELOC_PPC_EMB_NADDR16_HA
ENUMX
  BFD_RELOC_PPC_EMB_SDAI16
ENUMX
  BFD_RELOC_PPC_EMB_SDA2I16
ENUMX
  BFD_RELOC_PPC_EMB_SDA2REL
ENUMX
  BFD_RELOC_PPC_EMB_SDA21
ENUMX
  BFD_RELOC_PPC_EMB_MRKREF
ENUMX
  BFD_RELOC_PPC_EMB_RELSEC16
ENUMX
  BFD_RELOC_PPC_EMB_RELST_LO
ENUMX
  BFD_RELOC_PPC_EMB_RELST_HI
ENUMX
  BFD_RELOC_PPC_EMB_RELST_HA
ENUMX
  BFD_RELOC_PPC_EMB_BIT_FLD
ENUMX
  BFD_RELOC_PPC_EMB_RELSDA
ENUMX
  BFD_RELOC_PPC_VLE_REL8
ENUMX
  BFD_RELOC_PPC_VLE_REL15
ENUMX
  BFD_RELOC_PPC_VLE_REL24
ENUMX
  BFD_RELOC_PPC_VLE_LO16A
ENUMX
  BFD_RELOC_PPC_VLE_LO16D
ENUMX
  BFD_RELOC_PPC_VLE_HI16A
ENUMX
  BFD_RELOC_PPC_VLE_HI16D
ENUMX
  BFD_RELOC_PPC_VLE_HA16A
ENUMX
  BFD_RELOC_PPC_VLE_HA16D
ENUMX
  BFD_RELOC_PPC_VLE_SDA21
ENUMX
  BFD_RELOC_PPC_VLE_SDA21_LO
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_LO16A
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_LO16D
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_HI16A
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_HI16D
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_HA16A
ENUMX
  BFD_RELOC_PPC_VLE_SDAREL_HA16D
ENUMX
  BFD_RELOC_PPC_16DX_HA
ENUMX
  BFD_RELOC_PPC_REL16DX_HA
ENUMX
  BFD_RELOC_PPC_NEG
ENUMX
  BFD_RELOC_PPC64_HIGHER
ENUMX
  BFD_RELOC_PPC64_HIGHER_S
ENUMX
  BFD_RELOC_PPC64_HIGHEST
ENUMX
  BFD_RELOC_PPC64_HIGHEST_S
ENUMX
  BFD_RELOC_PPC64_TOC16_LO
ENUMX
  BFD_RELOC_PPC64_TOC16_HI
ENUMX
  BFD_RELOC_PPC64_TOC16_HA
ENUMX
  BFD_RELOC_PPC64_TOC
ENUMX
  BFD_RELOC_PPC64_PLTGOT16
ENUMX
  BFD_RELOC_PPC64_PLTGOT16_LO
ENUMX
  BFD_RELOC_PPC64_PLTGOT16_HI
ENUMX
  BFD_RELOC_PPC64_PLTGOT16_HA
ENUMX
  BFD_RELOC_PPC64_ADDR16_DS
ENUMX
  BFD_RELOC_PPC64_ADDR16_LO_DS
ENUMX
  BFD_RELOC_PPC64_GOT16_DS
ENUMX
  BFD_RELOC_PPC64_GOT16_LO_DS
ENUMX
  BFD_RELOC_PPC64_PLT16_LO_DS
ENUMX
  BFD_RELOC_PPC64_SECTOFF_DS
ENUMX
  BFD_RELOC_PPC64_SECTOFF_LO_DS
ENUMX
  BFD_RELOC_PPC64_TOC16_DS
ENUMX
  BFD_RELOC_PPC64_TOC16_LO_DS
ENUMX
  BFD_RELOC_PPC64_PLTGOT16_DS
ENUMX
  BFD_RELOC_PPC64_PLTGOT16_LO_DS
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGH
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGHA
ENUMX
  BFD_RELOC_PPC64_REL16_HIGH
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHA
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHER
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHERA
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHEST
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHESTA
ENUMX
  BFD_RELOC_PPC64_ADDR64_LOCAL
ENUMX
  BFD_RELOC_PPC64_ENTRY
ENUMX
  BFD_RELOC_PPC64_REL24_NOTOC
ENUMX
  BFD_RELOC_PPC64_REL24_P9NOTOC
ENUMX
  BFD_RELOC_PPC64_D34
ENUMX
  BFD_RELOC_PPC64_D34_LO
ENUMX
  BFD_RELOC_PPC64_D34_HI30
ENUMX
  BFD_RELOC_PPC64_D34_HA30
ENUMX
  BFD_RELOC_PPC64_PCREL34
ENUMX
  BFD_RELOC_PPC64_GOT_PCREL34
ENUMX
  BFD_RELOC_PPC64_PLT_PCREL34
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGHER34
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGHERA34
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGHEST34
ENUMX
  BFD_RELOC_PPC64_ADDR16_HIGHESTA34
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHER34
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHERA34
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHEST34
ENUMX
  BFD_RELOC_PPC64_REL16_HIGHESTA34
ENUMX
  BFD_RELOC_PPC64_D28
ENUMX
  BFD_RELOC_PPC64_PCREL28
ENUMDOC
  Power(rs6000) and PowerPC relocations.

ENUM
  BFD_RELOC_PPC_TLS
ENUMX
  BFD_RELOC_PPC_TLSGD
ENUMX
  BFD_RELOC_PPC_TLSLD
ENUMX
  BFD_RELOC_PPC_TLSLE
ENUMX
  BFD_RELOC_PPC_TLSIE
ENUMX
  BFD_RELOC_PPC_TLSM
ENUMX
  BFD_RELOC_PPC_TLSML
ENUMX
  BFD_RELOC_PPC_DTPMOD
ENUMX
  BFD_RELOC_PPC_TPREL16
ENUMX
  BFD_RELOC_PPC_TPREL16_LO
ENUMX
  BFD_RELOC_PPC_TPREL16_HI
ENUMX
  BFD_RELOC_PPC_TPREL16_HA
ENUMX
  BFD_RELOC_PPC_TPREL
ENUMX
  BFD_RELOC_PPC_DTPREL16
ENUMX
  BFD_RELOC_PPC_DTPREL16_LO
ENUMX
  BFD_RELOC_PPC_DTPREL16_HI
ENUMX
  BFD_RELOC_PPC_DTPREL16_HA
ENUMX
  BFD_RELOC_PPC_DTPREL
ENUMX
  BFD_RELOC_PPC_GOT_TLSGD16
ENUMX
  BFD_RELOC_PPC_GOT_TLSGD16_LO
ENUMX
  BFD_RELOC_PPC_GOT_TLSGD16_HI
ENUMX
  BFD_RELOC_PPC_GOT_TLSGD16_HA
ENUMX
  BFD_RELOC_PPC_GOT_TLSLD16
ENUMX
  BFD_RELOC_PPC_GOT_TLSLD16_LO
ENUMX
  BFD_RELOC_PPC_GOT_TLSLD16_HI
ENUMX
  BFD_RELOC_PPC_GOT_TLSLD16_HA
ENUMX
  BFD_RELOC_PPC_GOT_TPREL16
ENUMX
  BFD_RELOC_PPC_GOT_TPREL16_LO
ENUMX
  BFD_RELOC_PPC_GOT_TPREL16_HI
ENUMX
  BFD_RELOC_PPC_GOT_TPREL16_HA
ENUMX
  BFD_RELOC_PPC_GOT_DTPREL16
ENUMX
  BFD_RELOC_PPC_GOT_DTPREL16_LO
ENUMX
  BFD_RELOC_PPC_GOT_DTPREL16_HI
ENUMX
  BFD_RELOC_PPC_GOT_DTPREL16_HA
ENUMX
  BFD_RELOC_PPC64_TLSGD
ENUMX
  BFD_RELOC_PPC64_TLSLD
ENUMX
  BFD_RELOC_PPC64_TLSLE
ENUMX
  BFD_RELOC_PPC64_TLSIE
ENUMX
  BFD_RELOC_PPC64_TLSM
ENUMX
  BFD_RELOC_PPC64_TLSML
ENUMX
  BFD_RELOC_PPC64_TPREL16_DS
ENUMX
  BFD_RELOC_PPC64_TPREL16_LO_DS
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGH
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGHA
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGHER
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGHERA
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGHEST
ENUMX
  BFD_RELOC_PPC64_TPREL16_HIGHESTA
ENUMX
  BFD_RELOC_PPC64_DTPREL16_DS
ENUMX
  BFD_RELOC_PPC64_DTPREL16_LO_DS
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGH
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGHA
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGHER
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGHERA
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGHEST
ENUMX
  BFD_RELOC_PPC64_DTPREL16_HIGHESTA
ENUMX
  BFD_RELOC_PPC64_TPREL34
ENUMX
  BFD_RELOC_PPC64_DTPREL34
ENUMX
  BFD_RELOC_PPC64_GOT_TLSGD_PCREL34
ENUMX
  BFD_RELOC_PPC64_GOT_TLSLD_PCREL34
ENUMX
  BFD_RELOC_PPC64_GOT_TPREL_PCREL34
ENUMX
  BFD_RELOC_PPC64_GOT_DTPREL_PCREL34
ENUMX
  BFD_RELOC_PPC64_TLS_PCREL
ENUMDOC
  PowerPC and PowerPC64 thread-local storage relocations.

ENUM
  BFD_RELOC_I370_D12
ENUMDOC
  IBM 370/390 relocations

ENUM
  BFD_RELOC_CTOR
ENUMDOC
  The type of reloc used to build a constructor table - at the moment
  probably a 32 bit wide absolute relocation, but the target can choose.
  It generally does map to one of the other relocation types.

ENUM
  BFD_RELOC_ARM_PCREL_BRANCH
ENUMDOC
  ARM 26 bit pc-relative branch.  The lowest two bits must be zero and are
  not stored in the instruction.
ENUM
  BFD_RELOC_ARM_PCREL_BLX
ENUMDOC
  ARM 26 bit pc-relative branch.  The lowest bit must be zero and is
  not stored in the instruction.  The 2nd lowest bit comes from a 1 bit
  field in the instruction.
ENUM
  BFD_RELOC_THUMB_PCREL_BLX
ENUMDOC
  Thumb 22 bit pc-relative branch.  The lowest bit must be zero and is
  not stored in the instruction.  The 2nd lowest bit comes from a 1 bit
  field in the instruction.
ENUM
  BFD_RELOC_ARM_PCREL_CALL
ENUMDOC
  ARM 26-bit pc-relative branch for an unconditional BL or BLX instruction.
ENUM
  BFD_RELOC_ARM_PCREL_JUMP
ENUMDOC
  ARM 26-bit pc-relative branch for B or conditional BL instruction.

ENUM
  BFD_RELOC_THUMB_PCREL_BRANCH5
ENUMDOC
  ARM 5-bit pc-relative branch for Branch Future instructions.

ENUM
  BFD_RELOC_THUMB_PCREL_BFCSEL
ENUMDOC
  ARM 6-bit pc-relative branch for BFCSEL instruction.

ENUM
  BFD_RELOC_ARM_THUMB_BF17
ENUMDOC
  ARM 17-bit pc-relative branch for Branch Future instructions.

ENUM
  BFD_RELOC_ARM_THUMB_BF13
ENUMDOC
  ARM 13-bit pc-relative branch for BFCSEL instruction.

ENUM
  BFD_RELOC_ARM_THUMB_BF19
ENUMDOC
  ARM 19-bit pc-relative branch for Branch Future Link instruction.

ENUM
  BFD_RELOC_ARM_THUMB_LOOP12
ENUMDOC
  ARM 12-bit pc-relative branch for Low Overhead Loop instructions.

ENUM
  BFD_RELOC_THUMB_PCREL_BRANCH7
ENUMX
  BFD_RELOC_THUMB_PCREL_BRANCH9
ENUMX
  BFD_RELOC_THUMB_PCREL_BRANCH12
ENUMX
  BFD_RELOC_THUMB_PCREL_BRANCH20
ENUMX
  BFD_RELOC_THUMB_PCREL_BRANCH23
ENUMX
  BFD_RELOC_THUMB_PCREL_BRANCH25
ENUMDOC
  Thumb 7-, 9-, 12-, 20-, 23-, and 25-bit pc-relative branches.
  The lowest bit must be zero and is not stored in the instruction.
  Note that the corresponding ELF R_ARM_THM_JUMPnn constant has an
  "nn" one smaller in all cases.  Note further that BRANCH23
  corresponds to R_ARM_THM_CALL.

ENUM
  BFD_RELOC_ARM_OFFSET_IMM
ENUMDOC
  12-bit immediate offset, used in ARM-format ldr and str instructions.

ENUM
  BFD_RELOC_ARM_THUMB_OFFSET
ENUMDOC
  5-bit immediate offset, used in Thumb-format ldr and str instructions.

ENUM
  BFD_RELOC_ARM_TARGET1
ENUMDOC
  Pc-relative or absolute relocation depending on target.  Used for
  entries in .init_array sections.
ENUM
  BFD_RELOC_ARM_ROSEGREL32
ENUMDOC
  Read-only segment base relative address.
ENUM
  BFD_RELOC_ARM_SBREL32
ENUMDOC
  Data segment base relative address.
ENUM
  BFD_RELOC_ARM_TARGET2
ENUMDOC
  This reloc is used for references to RTTI data from exception handling
  tables.  The actual definition depends on the target.  It may be a
  pc-relative or some form of GOT-indirect relocation.
ENUM
  BFD_RELOC_ARM_PREL31
ENUMDOC
  31-bit PC relative address.
ENUM
  BFD_RELOC_ARM_MOVW
ENUMX
  BFD_RELOC_ARM_MOVT
ENUMX
  BFD_RELOC_ARM_MOVW_PCREL
ENUMX
  BFD_RELOC_ARM_MOVT_PCREL
ENUMX
  BFD_RELOC_ARM_THUMB_MOVW
ENUMX
  BFD_RELOC_ARM_THUMB_MOVT
ENUMX
  BFD_RELOC_ARM_THUMB_MOVW_PCREL
ENUMX
  BFD_RELOC_ARM_THUMB_MOVT_PCREL
ENUMDOC
  Low and High halfword relocations for MOVW and MOVT instructions.

ENUM
  BFD_RELOC_ARM_GOTFUNCDESC
ENUMX
  BFD_RELOC_ARM_GOTOFFFUNCDESC
ENUMX
  BFD_RELOC_ARM_FUNCDESC
ENUMX
  BFD_RELOC_ARM_FUNCDESC_VALUE
ENUMX
  BFD_RELOC_ARM_TLS_GD32_FDPIC
ENUMX
  BFD_RELOC_ARM_TLS_LDM32_FDPIC
ENUMX
  BFD_RELOC_ARM_TLS_IE32_FDPIC
ENUMDOC
  ARM FDPIC specific relocations.

ENUM
  BFD_RELOC_ARM_JUMP_SLOT
ENUMX
  BFD_RELOC_ARM_GLOB_DAT
ENUMX
  BFD_RELOC_ARM_GOT32
ENUMX
  BFD_RELOC_ARM_PLT32
ENUMX
  BFD_RELOC_ARM_RELATIVE
ENUMX
  BFD_RELOC_ARM_GOTOFF
ENUMX
  BFD_RELOC_ARM_GOTPC
ENUMX
  BFD_RELOC_ARM_GOT_PREL
ENUMDOC
  Relocations for setting up GOTs and PLTs for shared libraries.

ENUM
  BFD_RELOC_ARM_TLS_GD32
ENUMX
  BFD_RELOC_ARM_TLS_LDO32
ENUMX
  BFD_RELOC_ARM_TLS_LDM32
ENUMX
  BFD_RELOC_ARM_TLS_DTPOFF32
ENUMX
  BFD_RELOC_ARM_TLS_DTPMOD32
ENUMX
  BFD_RELOC_ARM_TLS_TPOFF32
ENUMX
  BFD_RELOC_ARM_TLS_IE32
ENUMX
  BFD_RELOC_ARM_TLS_LE32
ENUMX
  BFD_RELOC_ARM_TLS_GOTDESC
ENUMX
  BFD_RELOC_ARM_TLS_CALL
ENUMX
  BFD_RELOC_ARM_THM_TLS_CALL
ENUMX
  BFD_RELOC_ARM_TLS_DESCSEQ
ENUMX
  BFD_RELOC_ARM_THM_TLS_DESCSEQ
ENUMX
  BFD_RELOC_ARM_TLS_DESC
ENUMDOC
  ARM thread-local storage relocations.

ENUM
  BFD_RELOC_ARM_ALU_PC_G0_NC
ENUMX
  BFD_RELOC_ARM_ALU_PC_G0
ENUMX
  BFD_RELOC_ARM_ALU_PC_G1_NC
ENUMX
  BFD_RELOC_ARM_ALU_PC_G1
ENUMX
  BFD_RELOC_ARM_ALU_PC_G2
ENUMX
  BFD_RELOC_ARM_LDR_PC_G0
ENUMX
  BFD_RELOC_ARM_LDR_PC_G1
ENUMX
  BFD_RELOC_ARM_LDR_PC_G2
ENUMX
  BFD_RELOC_ARM_LDRS_PC_G0
ENUMX
  BFD_RELOC_ARM_LDRS_PC_G1
ENUMX
  BFD_RELOC_ARM_LDRS_PC_G2
ENUMX
  BFD_RELOC_ARM_LDC_PC_G0
ENUMX
  BFD_RELOC_ARM_LDC_PC_G1
ENUMX
  BFD_RELOC_ARM_LDC_PC_G2
ENUMX
  BFD_RELOC_ARM_ALU_SB_G0_NC
ENUMX
  BFD_RELOC_ARM_ALU_SB_G0
ENUMX
  BFD_RELOC_ARM_ALU_SB_G1_NC
ENUMX
  BFD_RELOC_ARM_ALU_SB_G1
ENUMX
  BFD_RELOC_ARM_ALU_SB_G2
ENUMX
  BFD_RELOC_ARM_LDR_SB_G0
ENUMX
  BFD_RELOC_ARM_LDR_SB_G1
ENUMX
  BFD_RELOC_ARM_LDR_SB_G2
ENUMX
  BFD_RELOC_ARM_LDRS_SB_G0
ENUMX
  BFD_RELOC_ARM_LDRS_SB_G1
ENUMX
  BFD_RELOC_ARM_LDRS_SB_G2
ENUMX
  BFD_RELOC_ARM_LDC_SB_G0
ENUMX
  BFD_RELOC_ARM_LDC_SB_G1
ENUMX
  BFD_RELOC_ARM_LDC_SB_G2
ENUMDOC
  ARM group relocations.

ENUM
  BFD_RELOC_ARM_V4BX
ENUMDOC
  Annotation of BX instructions.

ENUM
  BFD_RELOC_ARM_IRELATIVE
ENUMDOC
  ARM support for STT_GNU_IFUNC.

ENUM
  BFD_RELOC_ARM_THUMB_ALU_ABS_G0_NC
ENUMX
  BFD_RELOC_ARM_THUMB_ALU_ABS_G1_NC
ENUMX
  BFD_RELOC_ARM_THUMB_ALU_ABS_G2_NC
ENUMX
  BFD_RELOC_ARM_THUMB_ALU_ABS_G3_NC
ENUMDOC
  Thumb1 relocations to support execute-only code.

ENUM
  BFD_RELOC_ARM_IMMEDIATE
ENUMX
  BFD_RELOC_ARM_ADRL_IMMEDIATE
ENUMX
  BFD_RELOC_ARM_T32_IMMEDIATE
ENUMX
  BFD_RELOC_ARM_T32_ADD_IMM
ENUMX
  BFD_RELOC_ARM_T32_IMM12
ENUMX
  BFD_RELOC_ARM_T32_ADD_PC12
ENUMX
  BFD_RELOC_ARM_SHIFT_IMM
ENUMX
  BFD_RELOC_ARM_SMC
ENUMX
  BFD_RELOC_ARM_HVC
ENUMX
  BFD_RELOC_ARM_SWI
ENUMX
  BFD_RELOC_ARM_MULTI
ENUMX
  BFD_RELOC_ARM_CP_OFF_IMM
ENUMX
  BFD_RELOC_ARM_CP_OFF_IMM_S2
ENUMX
  BFD_RELOC_ARM_T32_CP_OFF_IMM
ENUMX
  BFD_RELOC_ARM_T32_CP_OFF_IMM_S2
ENUMX
  BFD_RELOC_ARM_T32_VLDR_VSTR_OFF_IMM
ENUMX
  BFD_RELOC_ARM_ADR_IMM
ENUMX
  BFD_RELOC_ARM_LDR_IMM
ENUMX
  BFD_RELOC_ARM_LITERAL
ENUMX
  BFD_RELOC_ARM_IN_POOL
ENUMX
  BFD_RELOC_ARM_OFFSET_IMM8
ENUMX
  BFD_RELOC_ARM_T32_OFFSET_U8
ENUMX
  BFD_RELOC_ARM_T32_OFFSET_IMM
ENUMX
  BFD_RELOC_ARM_HWLITERAL
ENUMX
  BFD_RELOC_ARM_THUMB_ADD
ENUMX
  BFD_RELOC_ARM_THUMB_IMM
ENUMX
  BFD_RELOC_ARM_THUMB_SHIFT
ENUMDOC
  These relocs are only used within the ARM assembler.  They are not
  (at present) written to any object files.

ENUM
  BFD_RELOC_SH_PCDISP8BY2
ENUMX
  BFD_RELOC_SH_PCDISP12BY2
ENUMX
  BFD_RELOC_SH_IMM3
ENUMX
  BFD_RELOC_SH_IMM3U
ENUMX
  BFD_RELOC_SH_DISP12
ENUMX
  BFD_RELOC_SH_DISP12BY2
ENUMX
  BFD_RELOC_SH_DISP12BY4
ENUMX
  BFD_RELOC_SH_DISP12BY8
ENUMX
  BFD_RELOC_SH_DISP20
ENUMX
  BFD_RELOC_SH_DISP20BY8
ENUMX
  BFD_RELOC_SH_IMM4
ENUMX
  BFD_RELOC_SH_IMM4BY2
ENUMX
  BFD_RELOC_SH_IMM4BY4
ENUMX
  BFD_RELOC_SH_IMM8
ENUMX
  BFD_RELOC_SH_IMM8BY2
ENUMX
  BFD_RELOC_SH_IMM8BY4
ENUMX
  BFD_RELOC_SH_PCRELIMM8BY2
ENUMX
  BFD_RELOC_SH_PCRELIMM8BY4
ENUMX
  BFD_RELOC_SH_SWITCH16
ENUMX
  BFD_RELOC_SH_SWITCH32
ENUMX
  BFD_RELOC_SH_USES
ENUMX
  BFD_RELOC_SH_COUNT
ENUMX
  BFD_RELOC_SH_ALIGN
ENUMX
  BFD_RELOC_SH_CODE
ENUMX
  BFD_RELOC_SH_DATA
ENUMX
  BFD_RELOC_SH_LABEL
ENUMX
  BFD_RELOC_SH_LOOP_START
ENUMX
  BFD_RELOC_SH_LOOP_END
ENUMX
  BFD_RELOC_SH_COPY
ENUMX
  BFD_RELOC_SH_GLOB_DAT
ENUMX
  BFD_RELOC_SH_JMP_SLOT
ENUMX
  BFD_RELOC_SH_RELATIVE
ENUMX
  BFD_RELOC_SH_GOTPC
ENUMX
  BFD_RELOC_SH_GOT_LOW16
ENUMX
  BFD_RELOC_SH_GOT_MEDLOW16
ENUMX
  BFD_RELOC_SH_GOT_MEDHI16
ENUMX
  BFD_RELOC_SH_GOT_HI16
ENUMX
  BFD_RELOC_SH_GOTPLT_LOW16
ENUMX
  BFD_RELOC_SH_GOTPLT_MEDLOW16
ENUMX
  BFD_RELOC_SH_GOTPLT_MEDHI16
ENUMX
  BFD_RELOC_SH_GOTPLT_HI16
ENUMX
  BFD_RELOC_SH_PLT_LOW16
ENUMX
  BFD_RELOC_SH_PLT_MEDLOW16
ENUMX
  BFD_RELOC_SH_PLT_MEDHI16
ENUMX
  BFD_RELOC_SH_PLT_HI16
ENUMX
  BFD_RELOC_SH_GOTOFF_LOW16
ENUMX
  BFD_RELOC_SH_GOTOFF_MEDLOW16
ENUMX
  BFD_RELOC_SH_GOTOFF_MEDHI16
ENUMX
  BFD_RELOC_SH_GOTOFF_HI16
ENUMX
  BFD_RELOC_SH_GOTPC_LOW16
ENUMX
  BFD_RELOC_SH_GOTPC_MEDLOW16
ENUMX
  BFD_RELOC_SH_GOTPC_MEDHI16
ENUMX
  BFD_RELOC_SH_GOTPC_HI16
ENUMX
  BFD_RELOC_SH_COPY64
ENUMX
  BFD_RELOC_SH_GLOB_DAT64
ENUMX
  BFD_RELOC_SH_JMP_SLOT64
ENUMX
  BFD_RELOC_SH_RELATIVE64
ENUMX
  BFD_RELOC_SH_GOT10BY4
ENUMX
  BFD_RELOC_SH_GOT10BY8
ENUMX
  BFD_RELOC_SH_GOTPLT10BY4
ENUMX
  BFD_RELOC_SH_GOTPLT10BY8
ENUMX
  BFD_RELOC_SH_GOTPLT32
ENUMX
  BFD_RELOC_SH_SHMEDIA_CODE
ENUMX
  BFD_RELOC_SH_IMMU5
ENUMX
  BFD_RELOC_SH_IMMS6
ENUMX
  BFD_RELOC_SH_IMMS6BY32
ENUMX
  BFD_RELOC_SH_IMMU6
ENUMX
  BFD_RELOC_SH_IMMS10
ENUMX
  BFD_RELOC_SH_IMMS10BY2
ENUMX
  BFD_RELOC_SH_IMMS10BY4
ENUMX
  BFD_RELOC_SH_IMMS10BY8
ENUMX
  BFD_RELOC_SH_IMMS16
ENUMX
  BFD_RELOC_SH_IMMU16
ENUMX
  BFD_RELOC_SH_IMM_LOW16
ENUMX
  BFD_RELOC_SH_IMM_LOW16_PCREL
ENUMX
  BFD_RELOC_SH_IMM_MEDLOW16
ENUMX
  BFD_RELOC_SH_IMM_MEDLOW16_PCREL
ENUMX
  BFD_RELOC_SH_IMM_MEDHI16
ENUMX
  BFD_RELOC_SH_IMM_MEDHI16_PCREL
ENUMX
  BFD_RELOC_SH_IMM_HI16
ENUMX
  BFD_RELOC_SH_IMM_HI16_PCREL
ENUMX
  BFD_RELOC_SH_PT_16
ENUMX
  BFD_RELOC_SH_TLS_GD_32
ENUMX
  BFD_RELOC_SH_TLS_LD_32
ENUMX
  BFD_RELOC_SH_TLS_LDO_32
ENUMX
  BFD_RELOC_SH_TLS_IE_32
ENUMX
  BFD_RELOC_SH_TLS_LE_32
ENUMX
  BFD_RELOC_SH_TLS_DTPMOD32
ENUMX
  BFD_RELOC_SH_TLS_DTPOFF32
ENUMX
  BFD_RELOC_SH_TLS_TPOFF32
ENUMX
  BFD_RELOC_SH_GOT20
ENUMX
  BFD_RELOC_SH_GOTOFF20
ENUMX
  BFD_RELOC_SH_GOTFUNCDESC
ENUMX
  BFD_RELOC_SH_GOTFUNCDESC20
ENUMX
  BFD_RELOC_SH_GOTOFFFUNCDESC
ENUMX
  BFD_RELOC_SH_GOTOFFFUNCDESC20
ENUMX
  BFD_RELOC_SH_FUNCDESC
ENUMDOC
  Renesas / SuperH SH relocs.  Not all of these appear in object files.

ENUM
  BFD_RELOC_ARC_NONE
ENUMX
  BFD_RELOC_ARC_8
ENUMX
  BFD_RELOC_ARC_16
ENUMX
  BFD_RELOC_ARC_24
ENUMX
  BFD_RELOC_ARC_32
ENUMX
  BFD_RELOC_ARC_N8
ENUMX
  BFD_RELOC_ARC_N16
ENUMX
  BFD_RELOC_ARC_N24
ENUMX
  BFD_RELOC_ARC_N32
ENUMX
  BFD_RELOC_ARC_SDA
ENUMX
  BFD_RELOC_ARC_SECTOFF
ENUMX
  BFD_RELOC_ARC_S21H_PCREL
ENUMX
  BFD_RELOC_ARC_S21W_PCREL
ENUMX
  BFD_RELOC_ARC_S25H_PCREL
ENUMX
  BFD_RELOC_ARC_S25W_PCREL
ENUMX
  BFD_RELOC_ARC_SDA32
ENUMX
  BFD_RELOC_ARC_SDA_LDST
ENUMX
  BFD_RELOC_ARC_SDA_LDST1
ENUMX
  BFD_RELOC_ARC_SDA_LDST2
ENUMX
  BFD_RELOC_ARC_SDA16_LD
ENUMX
  BFD_RELOC_ARC_SDA16_LD1
ENUMX
  BFD_RELOC_ARC_SDA16_LD2
ENUMX
  BFD_RELOC_ARC_S13_PCREL
ENUMX
  BFD_RELOC_ARC_W
ENUMX
  BFD_RELOC_ARC_32_ME
ENUMX
  BFD_RELOC_ARC_32_ME_S
ENUMX
  BFD_RELOC_ARC_N32_ME
ENUMX
  BFD_RELOC_ARC_SECTOFF_ME
ENUMX
  BFD_RELOC_ARC_SDA32_ME
ENUMX
  BFD_RELOC_ARC_W_ME
ENUMX
  BFD_RELOC_AC_SECTOFF_U8
ENUMX
  BFD_RELOC_AC_SECTOFF_U8_1
ENUMX
  BFD_RELOC_AC_SECTOFF_U8_2
ENUMX
  BFD_RELOC_AC_SECTOFF_S9
ENUMX
  BFD_RELOC_AC_SECTOFF_S9_1
ENUMX
  BFD_RELOC_AC_SECTOFF_S9_2
ENUMX
  BFD_RELOC_ARC_SECTOFF_ME_1
ENUMX
  BFD_RELOC_ARC_SECTOFF_ME_2
ENUMX
  BFD_RELOC_ARC_SECTOFF_1
ENUMX
  BFD_RELOC_ARC_SECTOFF_2
ENUMX
  BFD_RELOC_ARC_SDA_12
ENUMX
  BFD_RELOC_ARC_SDA16_ST2
ENUMX
  BFD_RELOC_ARC_32_PCREL
ENUMX
  BFD_RELOC_ARC_PC32
ENUMX
  BFD_RELOC_ARC_GOT32
ENUMX
  BFD_RELOC_ARC_GOTPC32
ENUMX
  BFD_RELOC_ARC_PLT32
ENUMX
  BFD_RELOC_ARC_COPY
ENUMX
  BFD_RELOC_ARC_GLOB_DAT
ENUMX
  BFD_RELOC_ARC_JMP_SLOT
ENUMX
  BFD_RELOC_ARC_RELATIVE
ENUMX
  BFD_RELOC_ARC_GOTOFF
ENUMX
  BFD_RELOC_ARC_GOTPC
ENUMX
  BFD_RELOC_ARC_S21W_PCREL_PLT
ENUMX
  BFD_RELOC_ARC_S25H_PCREL_PLT
ENUMX
  BFD_RELOC_ARC_TLS_DTPMOD
ENUMX
  BFD_RELOC_ARC_TLS_TPOFF
ENUMX
  BFD_RELOC_ARC_TLS_GD_GOT
ENUMX
  BFD_RELOC_ARC_TLS_GD_LD
ENUMX
  BFD_RELOC_ARC_TLS_GD_CALL
ENUMX
  BFD_RELOC_ARC_TLS_IE_GOT
ENUMX
  BFD_RELOC_ARC_TLS_DTPOFF
ENUMX
  BFD_RELOC_ARC_TLS_DTPOFF_S9
ENUMX
  BFD_RELOC_ARC_TLS_LE_S9
ENUMX
  BFD_RELOC_ARC_TLS_LE_32
ENUMX
  BFD_RELOC_ARC_S25W_PCREL_PLT
ENUMX
  BFD_RELOC_ARC_S21H_PCREL_PLT
ENUMX
  BFD_RELOC_ARC_NPS_CMEM16
ENUMX
  BFD_RELOC_ARC_JLI_SECTOFF
ENUMDOC
  ARC relocs.

ENUM
  BFD_RELOC_BFIN_16_IMM
ENUMDOC
  ADI Blackfin 16 bit immediate absolute reloc.
ENUM
  BFD_RELOC_BFIN_16_HIGH
ENUMDOC
  ADI Blackfin 16 bit immediate absolute reloc higher 16 bits.
ENUM
  BFD_RELOC_BFIN_4_PCREL
ENUMDOC
  ADI Blackfin 'a' part of LSETUP.
ENUM
  BFD_RELOC_BFIN_5_PCREL
ENUMDOC
  ADI Blackfin.
ENUM
  BFD_RELOC_BFIN_16_LOW
ENUMDOC
  ADI Blackfin 16 bit immediate absolute reloc lower 16 bits.
ENUM
  BFD_RELOC_BFIN_10_PCREL
ENUMDOC
  ADI Blackfin.
ENUM
  BFD_RELOC_BFIN_11_PCREL
ENUMDOC
  ADI Blackfin 'b' part of LSETUP.
ENUM
  BFD_RELOC_BFIN_12_PCREL_JUMP
ENUMDOC
  ADI Blackfin.
ENUM
  BFD_RELOC_BFIN_12_PCREL_JUMP_S
ENUMDOC
  ADI Blackfin Short jump, pcrel.
ENUM
  BFD_RELOC_BFIN_24_PCREL_CALL_X
ENUMDOC
  ADI Blackfin Call.x not implemented.
ENUM
  BFD_RELOC_BFIN_24_PCREL_JUMP_L
ENUMDOC
  ADI Blackfin Long Jump pcrel.
ENUM
  BFD_RELOC_BFIN_GOT17M4
ENUMX
  BFD_RELOC_BFIN_GOTHI
ENUMX
  BFD_RELOC_BFIN_GOTLO
ENUMX
  BFD_RELOC_BFIN_FUNCDESC
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOT17M4
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOTHI
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOTLO
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_VALUE
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOTOFF17M4
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOTOFFHI
ENUMX
  BFD_RELOC_BFIN_FUNCDESC_GOTOFFLO
ENUMX
  BFD_RELOC_BFIN_GOTOFF17M4
ENUMX
  BFD_RELOC_BFIN_GOTOFFHI
ENUMX
  BFD_RELOC_BFIN_GOTOFFLO
ENUMDOC
  ADI Blackfin FD-PIC relocations.
ENUM
  BFD_RELOC_BFIN_GOT
ENUMDOC
  ADI Blackfin GOT relocation.
ENUM
  BFD_RELOC_BFIN_PLTPC
ENUMDOC
  ADI Blackfin PLTPC relocation.
ENUM
  BFD_ARELOC_BFIN_PUSH
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_CONST
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_ADD
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_SUB
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_MULT
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_DIV
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_MOD
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_LSHIFT
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_RSHIFT
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_AND
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_OR
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_XOR
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_LAND
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_LOR
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_LEN
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_NEG
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_COMP
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_PAGE
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_HWPAGE
ENUMDOC
  ADI Blackfin arithmetic relocation.
ENUM
  BFD_ARELOC_BFIN_ADDR
ENUMDOC
  ADI Blackfin arithmetic relocation.

ENUM
  BFD_RELOC_D10V_10_PCREL_R
ENUMDOC
  Mitsubishi D10V relocs.
  This is a 10-bit reloc with the right 2 bits
  assumed to be 0.
ENUM
  BFD_RELOC_D10V_10_PCREL_L
ENUMDOC
  Mitsubishi D10V relocs.
  This is a 10-bit reloc with the right 2 bits
  assumed to be 0.  This is the same as the previous reloc
  except it is in the left container, i.e.,
  shifted left 15 bits.
ENUM
  BFD_RELOC_D10V_18
ENUMDOC
  This is an 18-bit reloc with the right 2 bits
  assumed to be 0.
ENUM
  BFD_RELOC_D10V_18_PCREL
ENUMDOC
  This is an 18-bit reloc with the right 2 bits
  assumed to be 0.

ENUM
  BFD_RELOC_D30V_6
ENUMDOC
  Mitsubishi D30V relocs.
  This is a 6-bit absolute reloc.
ENUM
  BFD_RELOC_D30V_9_PCREL
ENUMDOC
  This is a 6-bit pc-relative reloc with
  the right 3 bits assumed to be 0.
ENUM
  BFD_RELOC_D30V_9_PCREL_R
ENUMDOC
  This is a 6-bit pc-relative reloc with
  the right 3 bits assumed to be 0. Same
  as the previous reloc but on the right side
  of the container.
ENUM
  BFD_RELOC_D30V_15
ENUMDOC
  This is a 12-bit absolute reloc with the
  right 3 bitsassumed to be 0.
ENUM
  BFD_RELOC_D30V_15_PCREL
ENUMDOC
  This is a 12-bit pc-relative reloc with
  the right 3 bits assumed to be 0.
ENUM
  BFD_RELOC_D30V_15_PCREL_R
ENUMDOC
  This is a 12-bit pc-relative reloc with
  the right 3 bits assumed to be 0. Same
  as the previous reloc but on the right side
  of the container.
ENUM
  BFD_RELOC_D30V_21
ENUMDOC
  This is an 18-bit absolute reloc with
  the right 3 bits assumed to be 0.
ENUM
  BFD_RELOC_D30V_21_PCREL
ENUMDOC
  This is an 18-bit pc-relative reloc with
  the right 3 bits assumed to be 0.
ENUM
  BFD_RELOC_D30V_21_PCREL_R
ENUMDOC
  This is an 18-bit pc-relative reloc with
  the right 3 bits assumed to be 0. Same
  as the previous reloc but on the right side
  of the container.
ENUM
  BFD_RELOC_D30V_32
ENUMDOC
  This is a 32-bit absolute reloc.
ENUM
  BFD_RELOC_D30V_32_PCREL
ENUMDOC
  This is a 32-bit pc-relative reloc.

ENUM
  BFD_RELOC_DLX_HI16_S
ENUMDOC
  DLX relocs
ENUM
  BFD_RELOC_DLX_LO16
ENUMDOC
  DLX relocs
ENUM
  BFD_RELOC_DLX_JMP26
ENUMDOC
  DLX relocs

ENUM
  BFD_RELOC_M32C_HI8
ENUMX
  BFD_RELOC_M32C_RL_JUMP
ENUMX
  BFD_RELOC_M32C_RL_1ADDR
ENUMX
  BFD_RELOC_M32C_RL_2ADDR
ENUMDOC
  Renesas M16C/M32C Relocations.

ENUM
  BFD_RELOC_M32R_24
ENUMDOC
  Renesas M32R (formerly Mitsubishi M32R) relocs.
  This is a 24 bit absolute address.
ENUM
  BFD_RELOC_M32R_10_PCREL
ENUMDOC
  This is a 10-bit pc-relative reloc with the right 2 bits assumed to be 0.
ENUM
  BFD_RELOC_M32R_18_PCREL
ENUMDOC
  This is an 18-bit reloc with the right 2 bits assumed to be 0.
ENUM
  BFD_RELOC_M32R_26_PCREL
ENUMDOC
  This is a 26-bit reloc with the right 2 bits assumed to be 0.
ENUM
  BFD_RELOC_M32R_HI16_ULO
ENUMDOC
  This is a 16-bit reloc containing the high 16 bits of an address
  used when the lower 16 bits are treated as unsigned.
ENUM
  BFD_RELOC_M32R_HI16_SLO
ENUMDOC
  This is a 16-bit reloc containing the high 16 bits of an address
  used when the lower 16 bits are treated as signed.
ENUM
  BFD_RELOC_M32R_LO16
ENUMDOC
  This is a 16-bit reloc containing the lower 16 bits of an address.
ENUM
  BFD_RELOC_M32R_SDA16
ENUMDOC
  This is a 16-bit reloc containing the small data area offset for use in
  add3, load, and store instructions.
ENUM
  BFD_RELOC_M32R_GOT24
ENUMX
  BFD_RELOC_M32R_26_PLTREL
ENUMX
  BFD_RELOC_M32R_COPY
ENUMX
  BFD_RELOC_M32R_GLOB_DAT
ENUMX
  BFD_RELOC_M32R_JMP_SLOT
ENUMX
  BFD_RELOC_M32R_RELATIVE
ENUMX
  BFD_RELOC_M32R_GOTOFF
ENUMX
  BFD_RELOC_M32R_GOTOFF_HI_ULO
ENUMX
  BFD_RELOC_M32R_GOTOFF_HI_SLO
ENUMX
  BFD_RELOC_M32R_GOTOFF_LO
ENUMX
  BFD_RELOC_M32R_GOTPC24
ENUMX
  BFD_RELOC_M32R_GOT16_HI_ULO
ENUMX
  BFD_RELOC_M32R_GOT16_HI_SLO
ENUMX
  BFD_RELOC_M32R_GOT16_LO
ENUMX
  BFD_RELOC_M32R_GOTPC_HI_ULO
ENUMX
  BFD_RELOC_M32R_GOTPC_HI_SLO
ENUMX
  BFD_RELOC_M32R_GOTPC_LO
ENUMDOC
  For PIC.


ENUM
  BFD_RELOC_NDS32_20
ENUMDOC
  NDS32 relocs.
  This is a 20 bit absolute address.
ENUM
  BFD_RELOC_NDS32_9_PCREL
ENUMDOC
  This is a 9-bit pc-relative reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_WORD_9_PCREL
ENUMDOC
  This is a 9-bit pc-relative reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_15_PCREL
ENUMDOC
  This is an 15-bit reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_17_PCREL
ENUMDOC
  This is an 17-bit reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_25_PCREL
ENUMDOC
  This is a 25-bit reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_HI20
ENUMDOC
  This is a 20-bit reloc containing the high 20 bits of an address
  used with the lower 12 bits
ENUM
  BFD_RELOC_NDS32_LO12S3
ENUMDOC
  This is a 12-bit reloc containing the lower 12 bits of an address
  then shift right by 3. This is used with ldi,sdi...
ENUM
  BFD_RELOC_NDS32_LO12S2
ENUMDOC
  This is a 12-bit reloc containing the lower 12 bits of an address
  then shift left by 2. This is used with lwi,swi...
ENUM
  BFD_RELOC_NDS32_LO12S1
ENUMDOC
  This is a 12-bit reloc containing the lower 12 bits of an address
  then shift left by 1. This is used with lhi,shi...
ENUM
  BFD_RELOC_NDS32_LO12S0
ENUMDOC
  This is a 12-bit reloc containing the lower 12 bits of an address
  then shift left by 0. This is used with lbisbi...
ENUM
  BFD_RELOC_NDS32_LO12S0_ORI
ENUMDOC
  This is a 12-bit reloc containing the lower 12 bits of an address
  then shift left by 0. This is only used with branch relaxations
ENUM
  BFD_RELOC_NDS32_SDA15S3
ENUMDOC
  This is a 15-bit reloc containing the small data area 18-bit signed offset
  and shift left by 3 for use in ldi, sdi...
ENUM
  BFD_RELOC_NDS32_SDA15S2
ENUMDOC
  This is a 15-bit reloc containing the small data area 17-bit signed offset
  and shift left by 2 for use in lwi, swi...
ENUM
  BFD_RELOC_NDS32_SDA15S1
ENUMDOC
  This is a 15-bit reloc containing the small data area 16-bit signed offset
  and shift left by 1 for use in lhi, shi...
ENUM
  BFD_RELOC_NDS32_SDA15S0
ENUMDOC
  This is a 15-bit reloc containing the small data area 15-bit signed offset
  and shift left by 0 for use in lbi, sbi...
ENUM
  BFD_RELOC_NDS32_SDA16S3
ENUMDOC
  This is a 16-bit reloc containing the small data area 16-bit signed offset
  and shift left by 3
ENUM
  BFD_RELOC_NDS32_SDA17S2
ENUMDOC
  This is a 17-bit reloc containing the small data area 17-bit signed offset
  and shift left by 2 for use in lwi.gp, swi.gp...
ENUM
  BFD_RELOC_NDS32_SDA18S1
ENUMDOC
  This is a 18-bit reloc containing the small data area 18-bit signed offset
  and shift left by 1 for use in lhi.gp, shi.gp...
ENUM
  BFD_RELOC_NDS32_SDA19S0
ENUMDOC
  This is a 19-bit reloc containing the small data area 19-bit signed offset
  and shift left by 0 for use in lbi.gp, sbi.gp...
ENUM
  BFD_RELOC_NDS32_GOT20
ENUMX
  BFD_RELOC_NDS32_9_PLTREL
ENUMX
  BFD_RELOC_NDS32_25_PLTREL
ENUMX
  BFD_RELOC_NDS32_COPY
ENUMX
  BFD_RELOC_NDS32_GLOB_DAT
ENUMX
  BFD_RELOC_NDS32_JMP_SLOT
ENUMX
  BFD_RELOC_NDS32_RELATIVE
ENUMX
  BFD_RELOC_NDS32_GOTOFF
ENUMX
  BFD_RELOC_NDS32_GOTOFF_HI20
ENUMX
  BFD_RELOC_NDS32_GOTOFF_LO12
ENUMX
  BFD_RELOC_NDS32_GOTPC20
ENUMX
  BFD_RELOC_NDS32_GOT_HI20
ENUMX
  BFD_RELOC_NDS32_GOT_LO12
ENUMX
  BFD_RELOC_NDS32_GOTPC_HI20
ENUMX
  BFD_RELOC_NDS32_GOTPC_LO12
ENUMDOC
  for PIC
ENUM
  BFD_RELOC_NDS32_INSN16
ENUMX
  BFD_RELOC_NDS32_LABEL
ENUMX
  BFD_RELOC_NDS32_LONGCALL1
ENUMX
  BFD_RELOC_NDS32_LONGCALL2
ENUMX
  BFD_RELOC_NDS32_LONGCALL3
ENUMX
  BFD_RELOC_NDS32_LONGJUMP1
ENUMX
  BFD_RELOC_NDS32_LONGJUMP2
ENUMX
  BFD_RELOC_NDS32_LONGJUMP3
ENUMX
  BFD_RELOC_NDS32_LOADSTORE
ENUMX
  BFD_RELOC_NDS32_9_FIXED
ENUMX
  BFD_RELOC_NDS32_15_FIXED
ENUMX
  BFD_RELOC_NDS32_17_FIXED
ENUMX
  BFD_RELOC_NDS32_25_FIXED
ENUMX
  BFD_RELOC_NDS32_LONGCALL4
ENUMX
  BFD_RELOC_NDS32_LONGCALL5
ENUMX
  BFD_RELOC_NDS32_LONGCALL6
ENUMX
  BFD_RELOC_NDS32_LONGJUMP4
ENUMX
  BFD_RELOC_NDS32_LONGJUMP5
ENUMX
  BFD_RELOC_NDS32_LONGJUMP6
ENUMX
  BFD_RELOC_NDS32_LONGJUMP7
ENUMDOC
  for relax
ENUM
  BFD_RELOC_NDS32_PLTREL_HI20
ENUMX
  BFD_RELOC_NDS32_PLTREL_LO12
ENUMX
  BFD_RELOC_NDS32_PLT_GOTREL_HI20
ENUMX
  BFD_RELOC_NDS32_PLT_GOTREL_LO12
ENUMDOC
  for PIC
ENUM
  BFD_RELOC_NDS32_SDA12S2_DP
ENUMX
  BFD_RELOC_NDS32_SDA12S2_SP
ENUMX
  BFD_RELOC_NDS32_LO12S2_DP
ENUMX
  BFD_RELOC_NDS32_LO12S2_SP
ENUMDOC
  for floating point
ENUM
  BFD_RELOC_NDS32_DWARF2_OP1
ENUMX
  BFD_RELOC_NDS32_DWARF2_OP2
ENUMX
  BFD_RELOC_NDS32_DWARF2_LEB
ENUMDOC
  for dwarf2 debug_line.
ENUM
  BFD_RELOC_NDS32_UPDATE_TA
ENUMDOC
  for eliminate 16-bit instructions
ENUM
  BFD_RELOC_NDS32_PLT_GOTREL_LO20
ENUMX
  BFD_RELOC_NDS32_PLT_GOTREL_LO15
ENUMX
  BFD_RELOC_NDS32_PLT_GOTREL_LO19
ENUMX
  BFD_RELOC_NDS32_GOT_LO15
ENUMX
  BFD_RELOC_NDS32_GOT_LO19
ENUMX
  BFD_RELOC_NDS32_GOTOFF_LO15
ENUMX
  BFD_RELOC_NDS32_GOTOFF_LO19
ENUMX
  BFD_RELOC_NDS32_GOT15S2
ENUMX
  BFD_RELOC_NDS32_GOT17S2
ENUMDOC
  for PIC object relaxation
ENUM
  BFD_RELOC_NDS32_5
ENUMDOC
  NDS32 relocs.
  This is a 5 bit absolute address.
ENUM
  BFD_RELOC_NDS32_10_UPCREL
ENUMDOC
  This is a 10-bit unsigned pc-relative reloc with the right 1 bit assumed to be 0.
ENUM
  BFD_RELOC_NDS32_SDA_FP7U2_RELA
ENUMDOC
  If fp were omitted, fp can used as another gp.
ENUM
  BFD_RELOC_NDS32_RELAX_ENTRY
ENUMX
  BFD_RELOC_NDS32_GOT_SUFF
ENUMX
  BFD_RELOC_NDS32_GOTOFF_SUFF
ENUMX
  BFD_RELOC_NDS32_PLT_GOT_SUFF
ENUMX
  BFD_RELOC_NDS32_MULCALL_SUFF
ENUMX
  BFD_RELOC_NDS32_PTR
ENUMX
  BFD_RELOC_NDS32_PTR_COUNT
ENUMX
  BFD_RELOC_NDS32_PTR_RESOLVED
ENUMX
  BFD_RELOC_NDS32_PLTBLOCK
ENUMX
  BFD_RELOC_NDS32_RELAX_REGION_BEGIN
ENUMX
  BFD_RELOC_NDS32_RELAX_REGION_END
ENUMX
  BFD_RELOC_NDS32_MINUEND
ENUMX
  BFD_RELOC_NDS32_SUBTRAHEND
ENUMX
  BFD_RELOC_NDS32_DIFF8
ENUMX
  BFD_RELOC_NDS32_DIFF16
ENUMX
  BFD_RELOC_NDS32_DIFF32
ENUMX
  BFD_RELOC_NDS32_DIFF_ULEB128
ENUMX
  BFD_RELOC_NDS32_EMPTY
ENUMDOC
  relaxation relative relocation types
ENUM
  BFD_RELOC_NDS32_25_ABS
ENUMDOC
  This is a 25 bit absolute address.
ENUM
  BFD_RELOC_NDS32_DATA
ENUMX
  BFD_RELOC_NDS32_TRAN
ENUMX
  BFD_RELOC_NDS32_17IFC_PCREL
ENUMX
  BFD_RELOC_NDS32_10IFCU_PCREL
ENUMDOC
  For ex9 and ifc using.
ENUM
  BFD_RELOC_NDS32_TPOFF
ENUMX
  BFD_RELOC_NDS32_GOTTPOFF
ENUMX
  BFD_RELOC_NDS32_TLS_LE_HI20
ENUMX
  BFD_RELOC_NDS32_TLS_LE_LO12
ENUMX
  BFD_RELOC_NDS32_TLS_LE_20
ENUMX
  BFD_RELOC_NDS32_TLS_LE_15S0
ENUMX
  BFD_RELOC_NDS32_TLS_LE_15S1
ENUMX
  BFD_RELOC_NDS32_TLS_LE_15S2
ENUMX
  BFD_RELOC_NDS32_TLS_LE_ADD
ENUMX
  BFD_RELOC_NDS32_TLS_LE_LS
ENUMX
  BFD_RELOC_NDS32_TLS_IE_HI20
ENUMX
  BFD_RELOC_NDS32_TLS_IE_LO12
ENUMX
  BFD_RELOC_NDS32_TLS_IE_LO12S2
ENUMX
  BFD_RELOC_NDS32_TLS_IEGP_HI20
ENUMX
  BFD_RELOC_NDS32_TLS_IEGP_LO12
ENUMX
  BFD_RELOC_NDS32_TLS_IEGP_LO12S2
ENUMX
  BFD_RELOC_NDS32_TLS_IEGP_LW
ENUMX
  BFD_RELOC_NDS32_TLS_DESC
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_HI20
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_LO12
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_20
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_SDA17S2
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_ADD
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_FUNC
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_CALL
ENUMX
  BFD_RELOC_NDS32_TLS_DESC_MEM
ENUMX
  BFD_RELOC_NDS32_REMOVE
ENUMX
  BFD_RELOC_NDS32_GROUP
ENUMDOC
  For TLS.
ENUM
  BFD_RELOC_NDS32_LSI
ENUMDOC
  For floating load store relaxation.


ENUM
  BFD_RELOC_V850_9_PCREL
ENUMDOC
  This is a 9-bit reloc
ENUM
  BFD_RELOC_V850_22_PCREL
ENUMDOC
  This is a 22-bit reloc

ENUM
  BFD_RELOC_V850_SDA_16_16_OFFSET
ENUMDOC
  This is a 16 bit offset from the short data area pointer.
ENUM
  BFD_RELOC_V850_SDA_15_16_OFFSET
ENUMDOC
  This is a 16 bit offset (of which only 15 bits are used) from the
  short data area pointer.
ENUM
  BFD_RELOC_V850_ZDA_16_16_OFFSET
ENUMDOC
  This is a 16 bit offset from the zero data area pointer.
ENUM
  BFD_RELOC_V850_ZDA_15_16_OFFSET
ENUMDOC
  This is a 16 bit offset (of which only 15 bits are used) from the
  zero data area pointer.
ENUM
  BFD_RELOC_V850_TDA_6_8_OFFSET
ENUMDOC
  This is an 8 bit offset (of which only 6 bits are used) from the
  tiny data area pointer.
ENUM
  BFD_RELOC_V850_TDA_7_8_OFFSET
ENUMDOC
  This is an 8bit offset (of which only 7 bits are used) from the tiny
  data area pointer.
ENUM
  BFD_RELOC_V850_TDA_7_7_OFFSET
ENUMDOC
  This is a 7 bit offset from the tiny data area pointer.
ENUM
  BFD_RELOC_V850_TDA_16_16_OFFSET
ENUMDOC
  This is a 16 bit offset from the tiny data area pointer.
COMMENT
ENUM
  BFD_RELOC_V850_TDA_4_5_OFFSET
ENUMDOC
  This is a 5 bit offset (of which only 4 bits are used) from the tiny
  data area pointer.
ENUM
  BFD_RELOC_V850_TDA_4_4_OFFSET
ENUMDOC
  This is a 4 bit offset from the tiny data area pointer.
ENUM
  BFD_RELOC_V850_SDA_16_16_SPLIT_OFFSET
ENUMDOC
  This is a 16 bit offset from the short data area pointer, with the
  bits placed non-contiguously in the instruction.
ENUM
  BFD_RELOC_V850_ZDA_16_16_SPLIT_OFFSET
ENUMDOC
  This is a 16 bit offset from the zero data area pointer, with the
  bits placed non-contiguously in the instruction.
ENUM
  BFD_RELOC_V850_CALLT_6_7_OFFSET
ENUMDOC
  This is a 6 bit offset from the call table base pointer.
ENUM
  BFD_RELOC_V850_CALLT_16_16_OFFSET
ENUMDOC
  This is a 16 bit offset from the call table base pointer.
ENUM
  BFD_RELOC_V850_LONGCALL
ENUMDOC
  Used for relaxing indirect function calls.
ENUM
  BFD_RELOC_V850_LONGJUMP
ENUMDOC
  Used for relaxing indirect jumps.
ENUM
  BFD_RELOC_V850_ALIGN
ENUMDOC
  Used to maintain alignment whilst relaxing.
ENUM
  BFD_RELOC_V850_LO16_SPLIT_OFFSET
ENUMDOC
  This is a variation of BFD_RELOC_LO16 that can be used in v850e ld.bu
  instructions.
ENUM
  BFD_RELOC_V850_16_PCREL
ENUMDOC
  This is a 16-bit reloc.
ENUM
  BFD_RELOC_V850_17_PCREL
ENUMDOC
  This is a 17-bit reloc.
ENUM
  BFD_RELOC_V850_23
ENUMDOC
  This is a 23-bit reloc.
ENUM
  BFD_RELOC_V850_32_PCREL
ENUMDOC
  This is a 32-bit reloc.
ENUM
  BFD_RELOC_V850_32_ABS
ENUMDOC
  This is a 32-bit reloc.
ENUM
  BFD_RELOC_V850_16_SPLIT_OFFSET
ENUMDOC
  This is a 16-bit reloc.
ENUM
  BFD_RELOC_V850_16_S1
ENUMDOC
  This is a 16-bit reloc.
ENUM
  BFD_RELOC_V850_LO16_S1
ENUMDOC
  Low 16 bits. 16 bit shifted by 1.
ENUM
  BFD_RELOC_V850_CALLT_15_16_OFFSET
ENUMDOC
  This is a 16 bit offset from the call table base pointer.
ENUM
  BFD_RELOC_V850_32_GOTPCREL
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_16_GOT
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_32_GOT
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_22_PLT_PCREL
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_32_PLT_PCREL
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_COPY
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_GLOB_DAT
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_JMP_SLOT
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_RELATIVE
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_16_GOTOFF
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_32_GOTOFF
ENUMDOC
  DSO relocations.
ENUM
  BFD_RELOC_V850_CODE
ENUMDOC
  start code.
ENUM
  BFD_RELOC_V850_DATA
ENUMDOC
  start data in text.

ENUM
  BFD_RELOC_TIC30_LDP
ENUMDOC
  This is a 8bit DP reloc for the tms320c30, where the most
  significant 8 bits of a 24 bit word are placed into the least
  significant 8 bits of the opcode.

ENUM
  BFD_RELOC_TIC54X_PARTLS7
ENUMDOC
  This is a 7bit reloc for the tms320c54x, where the least
  significant 7 bits of a 16 bit word are placed into the least
  significant 7 bits of the opcode.

ENUM
  BFD_RELOC_TIC54X_PARTMS9
ENUMDOC
  This is a 9bit DP reloc for the tms320c54x, where the most
  significant 9 bits of a 16 bit word are placed into the least
  significant 9 bits of the opcode.

ENUM
  BFD_RELOC_TIC54X_23
ENUMDOC
  This is an extended address 23-bit reloc for the tms320c54x.

ENUM
  BFD_RELOC_TIC54X_16_OF_23
ENUMDOC
  This is a 16-bit reloc for the tms320c54x, where the least
  significant 16 bits of a 23-bit extended address are placed into
  the opcode.

ENUM
  BFD_RELOC_TIC54X_MS7_OF_23
ENUMDOC
  This is a reloc for the tms320c54x, where the most
  significant 7 bits of a 23-bit extended address are placed into
  the opcode.

ENUM
  BFD_RELOC_C6000_PCR_S21
ENUMX
  BFD_RELOC_C6000_PCR_S12
ENUMX
  BFD_RELOC_C6000_PCR_S10
ENUMX
  BFD_RELOC_C6000_PCR_S7
ENUMX
  BFD_RELOC_C6000_ABS_S16
ENUMX
  BFD_RELOC_C6000_ABS_L16
ENUMX
  BFD_RELOC_C6000_ABS_H16
ENUMX
  BFD_RELOC_C6000_SBR_U15_B
ENUMX
  BFD_RELOC_C6000_SBR_U15_H
ENUMX
  BFD_RELOC_C6000_SBR_U15_W
ENUMX
  BFD_RELOC_C6000_SBR_S16
ENUMX
  BFD_RELOC_C6000_SBR_L16_B
ENUMX
  BFD_RELOC_C6000_SBR_L16_H
ENUMX
  BFD_RELOC_C6000_SBR_L16_W
ENUMX
  BFD_RELOC_C6000_SBR_H16_B
ENUMX
  BFD_RELOC_C6000_SBR_H16_H
ENUMX
  BFD_RELOC_C6000_SBR_H16_W
ENUMX
  BFD_RELOC_C6000_SBR_GOT_U15_W
ENUMX
  BFD_RELOC_C6000_SBR_GOT_L16_W
ENUMX
  BFD_RELOC_C6000_SBR_GOT_H16_W
ENUMX
  BFD_RELOC_C6000_DSBT_INDEX
ENUMX
  BFD_RELOC_C6000_PREL31
ENUMX
  BFD_RELOC_C6000_COPY
ENUMX
  BFD_RELOC_C6000_JUMP_SLOT
ENUMX
  BFD_RELOC_C6000_EHTYPE
ENUMX
  BFD_RELOC_C6000_PCR_H16
ENUMX
  BFD_RELOC_C6000_PCR_L16
ENUMX
  BFD_RELOC_C6000_ALIGN
ENUMX
  BFD_RELOC_C6000_FPHEAD
ENUMX
  BFD_RELOC_C6000_NOCMP
ENUMDOC
  TMS320C6000 relocations.

ENUM
  BFD_RELOC_FR30_48
ENUMDOC
  This is a 48 bit reloc for the FR30 that stores 32 bits.
ENUM
  BFD_RELOC_FR30_20
ENUMDOC
  This is a 32 bit reloc for the FR30 that stores 20 bits split up into
  two sections.
ENUM
  BFD_RELOC_FR30_6_IN_4
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores a 6 bit word offset in
  4 bits.
ENUM
  BFD_RELOC_FR30_8_IN_8
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores an 8 bit byte offset
  into 8 bits.
ENUM
  BFD_RELOC_FR30_9_IN_8
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores a 9 bit short offset
  into 8 bits.
ENUM
  BFD_RELOC_FR30_10_IN_8
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores a 10 bit word offset
  into 8 bits.
ENUM
  BFD_RELOC_FR30_9_PCREL
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores a 9 bit pc relative
  short offset into 8 bits.
ENUM
  BFD_RELOC_FR30_12_PCREL
ENUMDOC
  This is a 16 bit reloc for the FR30 that stores a 12 bit pc relative
  short offset into 11 bits.

ENUM
  BFD_RELOC_MCORE_PCREL_IMM8BY4
ENUMX
  BFD_RELOC_MCORE_PCREL_IMM11BY2
ENUMX
  BFD_RELOC_MCORE_PCREL_IMM4BY2
ENUMX
  BFD_RELOC_MCORE_PCREL_32
ENUMX
  BFD_RELOC_MCORE_PCREL_JSR_IMM11BY2
ENUMX
  BFD_RELOC_MCORE_RVA
ENUMDOC
  Motorola Mcore relocations.

ENUM
  BFD_RELOC_MEP_8
ENUMX
  BFD_RELOC_MEP_16
ENUMX
  BFD_RELOC_MEP_32
ENUMX
  BFD_RELOC_MEP_PCREL8A2
ENUMX
  BFD_RELOC_MEP_PCREL12A2
ENUMX
  BFD_RELOC_MEP_PCREL17A2
ENUMX
  BFD_RELOC_MEP_PCREL24A2
ENUMX
  BFD_RELOC_MEP_PCABS24A2
ENUMX
  BFD_RELOC_MEP_LOW16
ENUMX
  BFD_RELOC_MEP_HI16U
ENUMX
  BFD_RELOC_MEP_HI16S
ENUMX
  BFD_RELOC_MEP_GPREL
ENUMX
  BFD_RELOC_MEP_TPREL
ENUMX
  BFD_RELOC_MEP_TPREL7
ENUMX
  BFD_RELOC_MEP_TPREL7A2
ENUMX
  BFD_RELOC_MEP_TPREL7A4
ENUMX
  BFD_RELOC_MEP_UIMM24
ENUMX
  BFD_RELOC_MEP_ADDR24A4
ENUMX
  BFD_RELOC_MEP_GNU_VTINHERIT
ENUMX
  BFD_RELOC_MEP_GNU_VTENTRY
ENUMDOC
  Toshiba Media Processor Relocations.
COMMENT

ENUM
  BFD_RELOC_METAG_HIADDR16
ENUMX
  BFD_RELOC_METAG_LOADDR16
ENUMX
  BFD_RELOC_METAG_RELBRANCH
ENUMX
  BFD_RELOC_METAG_GETSETOFF
ENUMX
  BFD_RELOC_METAG_HIOG
ENUMX
  BFD_RELOC_METAG_LOOG
ENUMX
  BFD_RELOC_METAG_REL8
ENUMX
  BFD_RELOC_METAG_REL16
ENUMX
  BFD_RELOC_METAG_HI16_GOTOFF
ENUMX
  BFD_RELOC_METAG_LO16_GOTOFF
ENUMX
  BFD_RELOC_METAG_GETSET_GOTOFF
ENUMX
  BFD_RELOC_METAG_GETSET_GOT
ENUMX
  BFD_RELOC_METAG_HI16_GOTPC
ENUMX
  BFD_RELOC_METAG_LO16_GOTPC
ENUMX
  BFD_RELOC_METAG_HI16_PLT
ENUMX
  BFD_RELOC_METAG_LO16_PLT
ENUMX
  BFD_RELOC_METAG_RELBRANCH_PLT
ENUMX
  BFD_RELOC_METAG_GOTOFF
ENUMX
  BFD_RELOC_METAG_PLT
ENUMX
  BFD_RELOC_METAG_COPY
ENUMX
  BFD_RELOC_METAG_JMP_SLOT
ENUMX
  BFD_RELOC_METAG_RELATIVE
ENUMX
  BFD_RELOC_METAG_GLOB_DAT
ENUMX
  BFD_RELOC_METAG_TLS_GD
ENUMX
  BFD_RELOC_METAG_TLS_LDM
ENUMX
  BFD_RELOC_METAG_TLS_LDO_HI16
ENUMX
  BFD_RELOC_METAG_TLS_LDO_LO16
ENUMX
  BFD_RELOC_METAG_TLS_LDO
ENUMX
  BFD_RELOC_METAG_TLS_IE
ENUMX
  BFD_RELOC_METAG_TLS_IENONPIC
ENUMX
  BFD_RELOC_METAG_TLS_IENONPIC_HI16
ENUMX
  BFD_RELOC_METAG_TLS_IENONPIC_LO16
ENUMX
  BFD_RELOC_METAG_TLS_TPOFF
ENUMX
  BFD_RELOC_METAG_TLS_DTPMOD
ENUMX
  BFD_RELOC_METAG_TLS_DTPOFF
ENUMX
  BFD_RELOC_METAG_TLS_LE
ENUMX
  BFD_RELOC_METAG_TLS_LE_HI16
ENUMX
  BFD_RELOC_METAG_TLS_LE_LO16
ENUMDOC
  Imagination Technologies Meta relocations.

ENUM
  BFD_RELOC_MMIX_GETA
ENUMX
  BFD_RELOC_MMIX_GETA_1
ENUMX
  BFD_RELOC_MMIX_GETA_2
ENUMX
  BFD_RELOC_MMIX_GETA_3
ENUMDOC
  These are relocations for the GETA instruction.
ENUM
  BFD_RELOC_MMIX_CBRANCH
ENUMX
  BFD_RELOC_MMIX_CBRANCH_J
ENUMX
  BFD_RELOC_MMIX_CBRANCH_1
ENUMX
  BFD_RELOC_MMIX_CBRANCH_2
ENUMX
  BFD_RELOC_MMIX_CBRANCH_3
ENUMDOC
  These are relocations for a conditional branch instruction.
ENUM
  BFD_RELOC_MMIX_PUSHJ
ENUMX
  BFD_RELOC_MMIX_PUSHJ_1
ENUMX
  BFD_RELOC_MMIX_PUSHJ_2
ENUMX
  BFD_RELOC_MMIX_PUSHJ_3
ENUMX
  BFD_RELOC_MMIX_PUSHJ_STUBBABLE
ENUMDOC
  These are relocations for the PUSHJ instruction.
ENUM
  BFD_RELOC_MMIX_JMP
ENUMX
  BFD_RELOC_MMIX_JMP_1
ENUMX
  BFD_RELOC_MMIX_JMP_2
ENUMX
  BFD_RELOC_MMIX_JMP_3
ENUMDOC
  These are relocations for the JMP instruction.
ENUM
  BFD_RELOC_MMIX_ADDR19
ENUMDOC
  This is a relocation for a relative address as in a GETA instruction or
  a branch.
ENUM
  BFD_RELOC_MMIX_ADDR27
ENUMDOC
  This is a relocation for a relative address as in a JMP instruction.
ENUM
  BFD_RELOC_MMIX_REG_OR_BYTE
ENUMDOC
  This is a relocation for an instruction field that may be a general
  register or a value 0..255.
ENUM
  BFD_RELOC_MMIX_REG
ENUMDOC
  This is a relocation for an instruction field that may be a general
  register.
ENUM
  BFD_RELOC_MMIX_BASE_PLUS_OFFSET
ENUMDOC
  This is a relocation for two instruction fields holding a register and
  an offset, the equivalent of the relocation.
ENUM
  BFD_RELOC_MMIX_LOCAL
ENUMDOC
  This relocation is an assertion that the expression is not allocated as
  a global register.  It does not modify contents.

ENUM
  BFD_RELOC_AVR_7_PCREL
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit pc relative
  short offset into 7 bits.
ENUM
  BFD_RELOC_AVR_13_PCREL
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 13 bit pc relative
  short offset into 12 bits.
ENUM
  BFD_RELOC_AVR_16_PM
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 17 bit value (usually
  program memory address) into 16 bits.
ENUM
  BFD_RELOC_AVR_LO8_LDI
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (usually
  data memory address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_HI8_LDI
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (high 8 bit
  of data memory address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_HH8_LDI
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
  of program memory address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_MS8_LDI
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
  of 32 bit value) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_LO8_LDI_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (usually data memory address) into 8 bit immediate value of SUBI insn.
ENUM
  BFD_RELOC_AVR_HI8_LDI_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (high 8 bit of data memory address) into 8 bit immediate value of
  SUBI insn.
ENUM
  BFD_RELOC_AVR_HH8_LDI_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (most high 8 bit of program memory address) into 8 bit immediate value
  of LDI or SUBI insn.
ENUM
  BFD_RELOC_AVR_MS8_LDI_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value (msb
  of 32 bit value) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_LO8_LDI_PM
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (usually
  command address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_LO8_LDI_GS
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value
  (command address) into 8 bit immediate value of LDI insn. If the address
  is beyond the 128k boundary, the linker inserts a jump stub for this reloc
  in the lower 128k.
ENUM
  BFD_RELOC_AVR_HI8_LDI_PM
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (high 8 bit
  of command address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_HI8_LDI_GS
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (high 8 bit
  of command address) into 8 bit immediate value of LDI insn.  If the address
  is beyond the 128k boundary, the linker inserts a jump stub for this reloc
  below 128k.
ENUM
  BFD_RELOC_AVR_HH8_LDI_PM
ENUMDOC
  This is a 16 bit reloc for the AVR that stores 8 bit value (most high 8 bit
  of command address) into 8 bit immediate value of LDI insn.
ENUM
  BFD_RELOC_AVR_LO8_LDI_PM_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (usually command address) into 8 bit immediate value of SUBI insn.
ENUM
  BFD_RELOC_AVR_HI8_LDI_PM_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (high 8 bit of 16 bit command address) into 8 bit immediate value
  of SUBI insn.
ENUM
  BFD_RELOC_AVR_HH8_LDI_PM_NEG
ENUMDOC
  This is a 16 bit reloc for the AVR that stores negated 8 bit value
  (high 6 bit of 22 bit command address) into 8 bit immediate
  value of SUBI insn.
ENUM
  BFD_RELOC_AVR_CALL
ENUMDOC
  This is a 32 bit reloc for the AVR that stores 23 bit value
  into 22 bits.
ENUM
  BFD_RELOC_AVR_LDI
ENUMDOC
  This is a 16 bit reloc for the AVR that stores all needed bits
  for absolute addressing with ldi with overflow check to linktime
ENUM
  BFD_RELOC_AVR_6
ENUMDOC
  This is a 6 bit reloc for the AVR that stores offset for ldd/std
  instructions
ENUM
  BFD_RELOC_AVR_6_ADIW
ENUMDOC
  This is a 6 bit reloc for the AVR that stores offset for adiw/sbiw
  instructions
ENUM
  BFD_RELOC_AVR_8_LO
ENUMDOC
  This is a 8 bit reloc for the AVR that stores bits 0..7 of a symbol
  in .byte lo8(symbol)
ENUM
  BFD_RELOC_AVR_8_HI
ENUMDOC
  This is a 8 bit reloc for the AVR that stores bits 8..15 of a symbol
  in .byte hi8(symbol)
ENUM
  BFD_RELOC_AVR_8_HLO
ENUMDOC
  This is a 8 bit reloc for the AVR that stores bits 16..23 of a symbol
  in .byte hlo8(symbol)
ENUM
  BFD_RELOC_AVR_DIFF8
ENUMX
  BFD_RELOC_AVR_DIFF16
ENUMX
  BFD_RELOC_AVR_DIFF32
ENUMDOC
  AVR relocations to mark the difference of two local symbols.
  These are only needed to support linker relaxation and can be ignored
  when not relaxing.  The field is set to the value of the difference
  assuming no relaxation.  The relocation encodes the position of the
  second symbol so the linker can determine whether to adjust the field
  value.
ENUM
  BFD_RELOC_AVR_LDS_STS_16
ENUMDOC
  This is a 7 bit reloc for the AVR that stores SRAM address for 16bit
  lds and sts instructions supported only tiny core.
ENUM
  BFD_RELOC_AVR_PORT6
ENUMDOC
  This is a 6 bit reloc for the AVR that stores an I/O register
  number for the IN and OUT instructions
ENUM
  BFD_RELOC_AVR_PORT5
ENUMDOC
  This is a 5 bit reloc for the AVR that stores an I/O register
  number for the SBIC, SBIS, SBI and CBI instructions

ENUM
  BFD_RELOC_RISCV_HI20
ENUMX
  BFD_RELOC_RISCV_PCREL_HI20
ENUMX
  BFD_RELOC_RISCV_PCREL_LO12_I
ENUMX
  BFD_RELOC_RISCV_PCREL_LO12_S
ENUMX
  BFD_RELOC_RISCV_LO12_I
ENUMX
  BFD_RELOC_RISCV_LO12_S
ENUMX
  BFD_RELOC_RISCV_GPREL12_I
ENUMX
  BFD_RELOC_RISCV_GPREL12_S
ENUMX
  BFD_RELOC_RISCV_TPREL_HI20
ENUMX
  BFD_RELOC_RISCV_TPREL_LO12_I
ENUMX
  BFD_RELOC_RISCV_TPREL_LO12_S
ENUMX
  BFD_RELOC_RISCV_TPREL_ADD
ENUMX
  BFD_RELOC_RISCV_CALL
ENUMX
  BFD_RELOC_RISCV_CALL_PLT
ENUMX
  BFD_RELOC_RISCV_ADD8
ENUMX
  BFD_RELOC_RISCV_ADD16
ENUMX
  BFD_RELOC_RISCV_ADD32
ENUMX
  BFD_RELOC_RISCV_ADD64
ENUMX
  BFD_RELOC_RISCV_SUB8
ENUMX
  BFD_RELOC_RISCV_SUB16
ENUMX
  BFD_RELOC_RISCV_SUB32
ENUMX
  BFD_RELOC_RISCV_SUB64
ENUMX
  BFD_RELOC_RISCV_GOT_HI20
ENUMX
  BFD_RELOC_RISCV_TLS_GOT_HI20
ENUMX
  BFD_RELOC_RISCV_TLS_GD_HI20
ENUMX
  BFD_RELOC_RISCV_JMP
ENUMX
  BFD_RELOC_RISCV_TLS_DTPMOD32
ENUMX
  BFD_RELOC_RISCV_TLS_DTPREL32
ENUMX
  BFD_RELOC_RISCV_TLS_DTPMOD64
ENUMX
  BFD_RELOC_RISCV_TLS_DTPREL64
ENUMX
  BFD_RELOC_RISCV_TLS_TPREL32
ENUMX
  BFD_RELOC_RISCV_TLS_TPREL64
ENUMX
  BFD_RELOC_RISCV_ALIGN
ENUMX
  BFD_RELOC_RISCV_RVC_BRANCH
ENUMX
  BFD_RELOC_RISCV_RVC_JUMP
ENUMX
  BFD_RELOC_RISCV_RVC_LUI
ENUMX
  BFD_RELOC_RISCV_GPREL_I
ENUMX
  BFD_RELOC_RISCV_GPREL_S
ENUMX
  BFD_RELOC_RISCV_TPREL_I
ENUMX
  BFD_RELOC_RISCV_TPREL_S
ENUMX
  BFD_RELOC_RISCV_RELAX
ENUMX
  BFD_RELOC_RISCV_CFA
ENUMX
  BFD_RELOC_RISCV_SUB6
ENUMX
  BFD_RELOC_RISCV_SET6
ENUMX
  BFD_RELOC_RISCV_SET8
ENUMX
  BFD_RELOC_RISCV_SET16
ENUMX
  BFD_RELOC_RISCV_SET32
ENUMX
  BFD_RELOC_RISCV_32_PCREL
ENUMX
  BFD_RELOC_RISCV_SET_ULEB128
ENUMX
  BFD_RELOC_RISCV_SUB_ULEB128
ENUMDOC
  RISC-V relocations.

ENUM
  BFD_RELOC_RL78_NEG8
ENUMX
  BFD_RELOC_RL78_NEG16
ENUMX
  BFD_RELOC_RL78_NEG24
ENUMX
  BFD_RELOC_RL78_NEG32
ENUMX
  BFD_RELOC_RL78_16_OP
ENUMX
  BFD_RELOC_RL78_24_OP
ENUMX
  BFD_RELOC_RL78_32_OP
ENUMX
  BFD_RELOC_RL78_8U
ENUMX
  BFD_RELOC_RL78_16U
ENUMX
  BFD_RELOC_RL78_24U
ENUMX
  BFD_RELOC_RL78_DIR3U_PCREL
ENUMX
  BFD_RELOC_RL78_DIFF
ENUMX
  BFD_RELOC_RL78_GPRELB
ENUMX
  BFD_RELOC_RL78_GPRELW
ENUMX
  BFD_RELOC_RL78_GPRELL
ENUMX
  BFD_RELOC_RL78_SYM
ENUMX
  BFD_RELOC_RL78_OP_SUBTRACT
ENUMX
  BFD_RELOC_RL78_OP_NEG
ENUMX
  BFD_RELOC_RL78_OP_AND
ENUMX
  BFD_RELOC_RL78_OP_SHRA
ENUMX
  BFD_RELOC_RL78_ABS8
ENUMX
  BFD_RELOC_RL78_ABS16
ENUMX
  BFD_RELOC_RL78_ABS16_REV
ENUMX
  BFD_RELOC_RL78_ABS32
ENUMX
  BFD_RELOC_RL78_ABS32_REV
ENUMX
  BFD_RELOC_RL78_ABS16U
ENUMX
  BFD_RELOC_RL78_ABS16UW
ENUMX
  BFD_RELOC_RL78_ABS16UL
ENUMX
  BFD_RELOC_RL78_RELAX
ENUMX
  BFD_RELOC_RL78_HI16
ENUMX
  BFD_RELOC_RL78_HI8
ENUMX
  BFD_RELOC_RL78_LO16
ENUMX
  BFD_RELOC_RL78_CODE
ENUMX
  BFD_RELOC_RL78_SADDR
ENUMDOC
  Renesas RL78 Relocations.

ENUM
  BFD_RELOC_RX_NEG8
ENUMX
  BFD_RELOC_RX_NEG16
ENUMX
  BFD_RELOC_RX_NEG24
ENUMX
  BFD_RELOC_RX_NEG32
ENUMX
  BFD_RELOC_RX_16_OP
ENUMX
  BFD_RELOC_RX_24_OP
ENUMX
  BFD_RELOC_RX_32_OP
ENUMX
  BFD_RELOC_RX_8U
ENUMX
  BFD_RELOC_RX_16U
ENUMX
  BFD_RELOC_RX_24U
ENUMX
  BFD_RELOC_RX_DIR3U_PCREL
ENUMX
  BFD_RELOC_RX_DIFF
ENUMX
  BFD_RELOC_RX_GPRELB
ENUMX
  BFD_RELOC_RX_GPRELW
ENUMX
  BFD_RELOC_RX_GPRELL
ENUMX
  BFD_RELOC_RX_SYM
ENUMX
  BFD_RELOC_RX_OP_SUBTRACT
ENUMX
  BFD_RELOC_RX_OP_NEG
ENUMX
  BFD_RELOC_RX_ABS8
ENUMX
  BFD_RELOC_RX_ABS16
ENUMX
  BFD_RELOC_RX_ABS16_REV
ENUMX
  BFD_RELOC_RX_ABS32
ENUMX
  BFD_RELOC_RX_ABS32_REV
ENUMX
  BFD_RELOC_RX_ABS16U
ENUMX
  BFD_RELOC_RX_ABS16UW
ENUMX
  BFD_RELOC_RX_ABS16UL
ENUMX
  BFD_RELOC_RX_RELAX
ENUMDOC
  Renesas RX Relocations.

ENUM
  BFD_RELOC_390_12
ENUMDOC
   Direct 12 bit.
ENUM
  BFD_RELOC_390_GOT12
ENUMDOC
  12 bit GOT offset.
ENUM
  BFD_RELOC_390_PLT32
ENUMDOC
  32 bit PC relative PLT address.
ENUM
  BFD_RELOC_390_COPY
ENUMDOC
  Copy symbol at runtime.
ENUM
  BFD_RELOC_390_GLOB_DAT
ENUMDOC
  Create GOT entry.
ENUM
  BFD_RELOC_390_JMP_SLOT
ENUMDOC
  Create PLT entry.
ENUM
  BFD_RELOC_390_RELATIVE
ENUMDOC
  Adjust by program base.
ENUM
  BFD_RELOC_390_GOTPC
ENUMDOC
  32 bit PC relative offset to GOT.
ENUM
  BFD_RELOC_390_GOT16
ENUMDOC
  16 bit GOT offset.
ENUM
  BFD_RELOC_390_PC12DBL
ENUMDOC
  PC relative 12 bit shifted by 1.
ENUM
  BFD_RELOC_390_PLT12DBL
ENUMDOC
  12 bit PC rel. PLT shifted by 1.
ENUM
  BFD_RELOC_390_PC16DBL
ENUMDOC
  PC relative 16 bit shifted by 1.
ENUM
  BFD_RELOC_390_PLT16DBL
ENUMDOC
  16 bit PC rel. PLT shifted by 1.
ENUM
  BFD_RELOC_390_PC24DBL
ENUMDOC
  PC relative 24 bit shifted by 1.
ENUM
  BFD_RELOC_390_PLT24DBL
ENUMDOC
  24 bit PC rel. PLT shifted by 1.
ENUM
  BFD_RELOC_390_PC32DBL
ENUMDOC
  PC relative 32 bit shifted by 1.
ENUM
  BFD_RELOC_390_PLT32DBL
ENUMDOC
  32 bit PC rel. PLT shifted by 1.
ENUM
  BFD_RELOC_390_GOTPCDBL
ENUMDOC
  32 bit PC rel. GOT shifted by 1.
ENUM
  BFD_RELOC_390_GOT64
ENUMDOC
  64 bit GOT offset.
ENUM
  BFD_RELOC_390_PLT64
ENUMDOC
  64 bit PC relative PLT address.
ENUM
  BFD_RELOC_390_GOTENT
ENUMDOC
  32 bit rel. offset to GOT entry.
ENUM
  BFD_RELOC_390_GOTOFF64
ENUMDOC
  64 bit offset to GOT.
ENUM
  BFD_RELOC_390_GOTPLT12
ENUMDOC
  12-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_390_GOTPLT16
ENUMDOC
  16-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_390_GOTPLT32
ENUMDOC
  32-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_390_GOTPLT64
ENUMDOC
  64-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_390_GOTPLTENT
ENUMDOC
  32-bit rel. offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_390_PLTOFF16
ENUMDOC
  16-bit rel. offset from the GOT to a PLT entry.
ENUM
  BFD_RELOC_390_PLTOFF32
ENUMDOC
  32-bit rel. offset from the GOT to a PLT entry.
ENUM
  BFD_RELOC_390_PLTOFF64
ENUMDOC
  64-bit rel. offset from the GOT to a PLT entry.

ENUM
  BFD_RELOC_390_TLS_LOAD
ENUMX
  BFD_RELOC_390_TLS_GDCALL
ENUMX
  BFD_RELOC_390_TLS_LDCALL
ENUMX
  BFD_RELOC_390_TLS_GD32
ENUMX
  BFD_RELOC_390_TLS_GD64
ENUMX
  BFD_RELOC_390_TLS_GOTIE12
ENUMX
  BFD_RELOC_390_TLS_GOTIE32
ENUMX
  BFD_RELOC_390_TLS_GOTIE64
ENUMX
  BFD_RELOC_390_TLS_LDM32
ENUMX
  BFD_RELOC_390_TLS_LDM64
ENUMX
  BFD_RELOC_390_TLS_IE32
ENUMX
  BFD_RELOC_390_TLS_IE64
ENUMX
  BFD_RELOC_390_TLS_IEENT
ENUMX
  BFD_RELOC_390_TLS_LE32
ENUMX
  BFD_RELOC_390_TLS_LE64
ENUMX
  BFD_RELOC_390_TLS_LDO32
ENUMX
  BFD_RELOC_390_TLS_LDO64
ENUMX
  BFD_RELOC_390_TLS_DTPMOD
ENUMX
  BFD_RELOC_390_TLS_DTPOFF
ENUMX
  BFD_RELOC_390_TLS_TPOFF
ENUMDOC
  s390 tls relocations.

ENUM
  BFD_RELOC_390_20
ENUMX
  BFD_RELOC_390_GOT20
ENUMX
  BFD_RELOC_390_GOTPLT20
ENUMX
  BFD_RELOC_390_TLS_GOTIE20
ENUMDOC
  Long displacement extension.

ENUM
  BFD_RELOC_390_IRELATIVE
ENUMDOC
  STT_GNU_IFUNC relocation.

ENUM
  BFD_RELOC_SCORE_GPREL15
ENUMDOC
  Score relocations
  Low 16 bit for load/store
ENUM
  BFD_RELOC_SCORE_DUMMY2
ENUMX
  BFD_RELOC_SCORE_JMP
ENUMDOC
  This is a 24-bit reloc with the right 1 bit assumed to be 0
ENUM
  BFD_RELOC_SCORE_BRANCH
ENUMDOC
  This is a 19-bit reloc with the right 1 bit assumed to be 0
ENUM
  BFD_RELOC_SCORE_IMM30
ENUMDOC
  This is a 32-bit reloc for 48-bit instructions.
ENUM
  BFD_RELOC_SCORE_IMM32
ENUMDOC
  This is a 32-bit reloc for 48-bit instructions.
ENUM
  BFD_RELOC_SCORE16_JMP
ENUMDOC
  This is a 11-bit reloc with the right 1 bit assumed to be 0
ENUM
  BFD_RELOC_SCORE16_BRANCH
ENUMDOC
  This is a 8-bit reloc with the right 1 bit assumed to be 0
ENUM
  BFD_RELOC_SCORE_BCMP
ENUMDOC
   This is a 9-bit reloc with the right 1 bit assumed to be 0
ENUM
  BFD_RELOC_SCORE_GOT15
ENUMX
  BFD_RELOC_SCORE_GOT_LO16
ENUMX
  BFD_RELOC_SCORE_CALL15
ENUMX
  BFD_RELOC_SCORE_DUMMY_HI16
ENUMDOC
  Undocumented Score relocs

ENUM
  BFD_RELOC_IP2K_FR9
ENUMDOC
  Scenix IP2K - 9-bit register number / data address
ENUM
  BFD_RELOC_IP2K_BANK
ENUMDOC
  Scenix IP2K - 4-bit register/data bank number
ENUM
  BFD_RELOC_IP2K_ADDR16CJP
ENUMDOC
  Scenix IP2K - low 13 bits of instruction word address
ENUM
  BFD_RELOC_IP2K_PAGE3
ENUMDOC
  Scenix IP2K - high 3 bits of instruction word address
ENUM
  BFD_RELOC_IP2K_LO8DATA
ENUMX
  BFD_RELOC_IP2K_HI8DATA
ENUMX
  BFD_RELOC_IP2K_EX8DATA
ENUMDOC
  Scenix IP2K - ext/low/high 8 bits of data address
ENUM
  BFD_RELOC_IP2K_LO8INSN
ENUMX
  BFD_RELOC_IP2K_HI8INSN
ENUMDOC
  Scenix IP2K - low/high 8 bits of instruction word address
ENUM
  BFD_RELOC_IP2K_PC_SKIP
ENUMDOC
  Scenix IP2K - even/odd PC modifier to modify snb pcl.0
ENUM
  BFD_RELOC_IP2K_TEXT
ENUMDOC
  Scenix IP2K - 16 bit word address in text section.
ENUM
  BFD_RELOC_IP2K_FR_OFFSET
ENUMDOC
  Scenix IP2K - 7-bit sp or dp offset
ENUM
  BFD_RELOC_VPE4KMATH_DATA
ENUMX
  BFD_RELOC_VPE4KMATH_INSN
ENUMDOC
  Scenix VPE4K coprocessor - data/insn-space addressing

ENUM
  BFD_RELOC_VTABLE_INHERIT
ENUMX
  BFD_RELOC_VTABLE_ENTRY
ENUMDOC
  These two relocations are used by the linker to determine which of
  the entries in a C++ virtual function table are actually used.  When
  the --gc-sections option is given, the linker will zero out the entries
  that are not used, so that the code for those functions need not be
  included in the output.

  VTABLE_INHERIT is a zero-space relocation used to describe to the
  linker the inheritance tree of a C++ virtual function table.  The
  relocation's symbol should be the parent class' vtable, and the
  relocation should be located at the child vtable.

  VTABLE_ENTRY is a zero-space relocation that describes the use of a
  virtual function table entry.  The reloc's symbol should refer to the
  table of the class mentioned in the code.  Off of that base, an offset
  describes the entry that is being used.  For Rela hosts, this offset
  is stored in the reloc's addend.  For Rel hosts, we are forced to put
  this offset in the reloc's section offset.

ENUM
  BFD_RELOC_IA64_IMM14
ENUMX
  BFD_RELOC_IA64_IMM22
ENUMX
  BFD_RELOC_IA64_IMM64
ENUMX
  BFD_RELOC_IA64_DIR32MSB
ENUMX
  BFD_RELOC_IA64_DIR32LSB
ENUMX
  BFD_RELOC_IA64_DIR64MSB
ENUMX
  BFD_RELOC_IA64_DIR64LSB
ENUMX
  BFD_RELOC_IA64_GPREL22
ENUMX
  BFD_RELOC_IA64_GPREL64I
ENUMX
  BFD_RELOC_IA64_GPREL32MSB
ENUMX
  BFD_RELOC_IA64_GPREL32LSB
ENUMX
  BFD_RELOC_IA64_GPREL64MSB
ENUMX
  BFD_RELOC_IA64_GPREL64LSB
ENUMX
  BFD_RELOC_IA64_LTOFF22
ENUMX
  BFD_RELOC_IA64_LTOFF64I
ENUMX
  BFD_RELOC_IA64_PLTOFF22
ENUMX
  BFD_RELOC_IA64_PLTOFF64I
ENUMX
  BFD_RELOC_IA64_PLTOFF64MSB
ENUMX
  BFD_RELOC_IA64_PLTOFF64LSB
ENUMX
  BFD_RELOC_IA64_FPTR64I
ENUMX
  BFD_RELOC_IA64_FPTR32MSB
ENUMX
  BFD_RELOC_IA64_FPTR32LSB
ENUMX
  BFD_RELOC_IA64_FPTR64MSB
ENUMX
  BFD_RELOC_IA64_FPTR64LSB
ENUMX
  BFD_RELOC_IA64_PCREL21B
ENUMX
  BFD_RELOC_IA64_PCREL21BI
ENUMX
  BFD_RELOC_IA64_PCREL21M
ENUMX
  BFD_RELOC_IA64_PCREL21F
ENUMX
  BFD_RELOC_IA64_PCREL22
ENUMX
  BFD_RELOC_IA64_PCREL60B
ENUMX
  BFD_RELOC_IA64_PCREL64I
ENUMX
  BFD_RELOC_IA64_PCREL32MSB
ENUMX
  BFD_RELOC_IA64_PCREL32LSB
ENUMX
  BFD_RELOC_IA64_PCREL64MSB
ENUMX
  BFD_RELOC_IA64_PCREL64LSB
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR22
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR64I
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR32MSB
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR32LSB
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR64MSB
ENUMX
  BFD_RELOC_IA64_LTOFF_FPTR64LSB
ENUMX
  BFD_RELOC_IA64_SEGREL32MSB
ENUMX
  BFD_RELOC_IA64_SEGREL32LSB
ENUMX
  BFD_RELOC_IA64_SEGREL64MSB
ENUMX
  BFD_RELOC_IA64_SEGREL64LSB
ENUMX
  BFD_RELOC_IA64_SECREL32MSB
ENUMX
  BFD_RELOC_IA64_SECREL32LSB
ENUMX
  BFD_RELOC_IA64_SECREL64MSB
ENUMX
  BFD_RELOC_IA64_SECREL64LSB
ENUMX
  BFD_RELOC_IA64_REL32MSB
ENUMX
  BFD_RELOC_IA64_REL32LSB
ENUMX
  BFD_RELOC_IA64_REL64MSB
ENUMX
  BFD_RELOC_IA64_REL64LSB
ENUMX
  BFD_RELOC_IA64_LTV32MSB
ENUMX
  BFD_RELOC_IA64_LTV32LSB
ENUMX
  BFD_RELOC_IA64_LTV64MSB
ENUMX
  BFD_RELOC_IA64_LTV64LSB
ENUMX
  BFD_RELOC_IA64_IPLTMSB
ENUMX
  BFD_RELOC_IA64_IPLTLSB
ENUMX
  BFD_RELOC_IA64_COPY
ENUMX
  BFD_RELOC_IA64_LTOFF22X
ENUMX
  BFD_RELOC_IA64_LDXMOV
ENUMX
  BFD_RELOC_IA64_TPREL14
ENUMX
  BFD_RELOC_IA64_TPREL22
ENUMX
  BFD_RELOC_IA64_TPREL64I
ENUMX
  BFD_RELOC_IA64_TPREL64MSB
ENUMX
  BFD_RELOC_IA64_TPREL64LSB
ENUMX
  BFD_RELOC_IA64_LTOFF_TPREL22
ENUMX
  BFD_RELOC_IA64_DTPMOD64MSB
ENUMX
  BFD_RELOC_IA64_DTPMOD64LSB
ENUMX
  BFD_RELOC_IA64_LTOFF_DTPMOD22
ENUMX
  BFD_RELOC_IA64_DTPREL14
ENUMX
  BFD_RELOC_IA64_DTPREL22
ENUMX
  BFD_RELOC_IA64_DTPREL64I
ENUMX
  BFD_RELOC_IA64_DTPREL32MSB
ENUMX
  BFD_RELOC_IA64_DTPREL32LSB
ENUMX
  BFD_RELOC_IA64_DTPREL64MSB
ENUMX
  BFD_RELOC_IA64_DTPREL64LSB
ENUMX
  BFD_RELOC_IA64_LTOFF_DTPREL22
ENUMDOC
  Intel IA64 Relocations.

ENUM
  BFD_RELOC_M68HC11_HI8
ENUMDOC
  Motorola 68HC11 reloc.
  This is the 8 bit high part of an absolute address.
ENUM
  BFD_RELOC_M68HC11_LO8
ENUMDOC
  Motorola 68HC11 reloc.
  This is the 8 bit low part of an absolute address.
ENUM
  BFD_RELOC_M68HC11_3B
ENUMDOC
  Motorola 68HC11 reloc.
  This is the 3 bit of a value.
ENUM
  BFD_RELOC_M68HC11_RL_JUMP
ENUMDOC
  Motorola 68HC11 reloc.
  This reloc marks the beginning of a jump/call instruction.
  It is used for linker relaxation to correctly identify beginning
  of instruction and change some branches to use PC-relative
  addressing mode.
ENUM
  BFD_RELOC_M68HC11_RL_GROUP
ENUMDOC
  Motorola 68HC11 reloc.
  This reloc marks a group of several instructions that gcc generates
  and for which the linker relaxation pass can modify and/or remove
  some of them.
ENUM
  BFD_RELOC_M68HC11_LO16
ENUMDOC
  Motorola 68HC11 reloc.
  This is the 16-bit lower part of an address.  It is used for 'call'
  instruction to specify the symbol address without any special
  transformation (due to memory bank window).
ENUM
  BFD_RELOC_M68HC11_PAGE
ENUMDOC
  Motorola 68HC11 reloc.
  This is a 8-bit reloc that specifies the page number of an address.
  It is used by 'call' instruction to specify the page number of
  the symbol.
ENUM
  BFD_RELOC_M68HC11_24
ENUMDOC
  Motorola 68HC11 reloc.
  This is a 24-bit reloc that represents the address with a 16-bit
  value and a 8-bit page number.  The symbol address is transformed
  to follow the 16K memory bank of 68HC12 (seen as mapped in the window).
ENUM
  BFD_RELOC_M68HC12_5B
ENUMDOC
  Motorola 68HC12 reloc.
  This is the 5 bits of a value.
ENUM
  BFD_RELOC_XGATE_RL_JUMP
ENUMDOC
  Freescale XGATE reloc.
  This reloc marks the beginning of a bra/jal instruction.
ENUM
  BFD_RELOC_XGATE_RL_GROUP
ENUMDOC
  Freescale XGATE reloc.
  This reloc marks a group of several instructions that gcc generates
  and for which the linker relaxation pass can modify and/or remove
  some of them.
ENUM
  BFD_RELOC_XGATE_LO16
ENUMDOC
  Freescale XGATE reloc.
  This is the 16-bit lower part of an address.  It is used for the '16-bit'
  instructions.
ENUM
  BFD_RELOC_XGATE_GPAGE
ENUMDOC
  Freescale XGATE reloc.
ENUM
  BFD_RELOC_XGATE_24
ENUMDOC
  Freescale XGATE reloc.
ENUM
  BFD_RELOC_XGATE_PCREL_9
ENUMDOC
  Freescale XGATE reloc.
  This is a 9-bit pc-relative reloc.
ENUM
  BFD_RELOC_XGATE_PCREL_10
ENUMDOC
  Freescale XGATE reloc.
  This is a 10-bit pc-relative reloc.
ENUM
  BFD_RELOC_XGATE_IMM8_LO
ENUMDOC
  Freescale XGATE reloc.
  This is the 16-bit lower part of an address.  It is used for the '16-bit'
  instructions.
ENUM
  BFD_RELOC_XGATE_IMM8_HI
ENUMDOC
  Freescale XGATE reloc.
  This is the 16-bit higher part of an address.  It is used for the '16-bit'
  instructions.
ENUM
  BFD_RELOC_XGATE_IMM3
ENUMDOC
  Freescale XGATE reloc.
  This is a 3-bit pc-relative reloc.
ENUM
  BFD_RELOC_XGATE_IMM4
ENUMDOC
  Freescale XGATE reloc.
  This is a 4-bit pc-relative reloc.
ENUM
  BFD_RELOC_XGATE_IMM5
ENUMDOC
  Freescale XGATE reloc.
  This is a 5-bit pc-relative reloc.
ENUM
  BFD_RELOC_M68HC12_9B
ENUMDOC
  Motorola 68HC12 reloc.
  This is the 9 bits of a value.
ENUM
  BFD_RELOC_M68HC12_16B
ENUMDOC
  Motorola 68HC12 reloc.
  This is the 16 bits of a value.
ENUM
  BFD_RELOC_M68HC12_9_PCREL
ENUMDOC
  Motorola 68HC12/XGATE reloc.
  This is a PCREL9 branch.
ENUM
  BFD_RELOC_M68HC12_10_PCREL
ENUMDOC
  Motorola 68HC12/XGATE reloc.
  This is a PCREL10 branch.
ENUM
  BFD_RELOC_M68HC12_LO8XG
ENUMDOC
  Motorola 68HC12/XGATE reloc.
  This is the 8 bit low part of an absolute address and immediately precedes
  a matching HI8XG part.
ENUM
  BFD_RELOC_M68HC12_HI8XG
ENUMDOC
  Motorola 68HC12/XGATE reloc.
  This is the 8 bit high part of an absolute address and immediately follows
  a matching LO8XG part.
ENUM
  BFD_RELOC_S12Z_15_PCREL
ENUMDOC
  Freescale S12Z reloc.
  This is a 15 bit relative address.  If the most significant bits are all zero
  then it may be truncated to 8 bits.

ENUM
  BFD_RELOC_CR16_NUM8
ENUMX
  BFD_RELOC_CR16_NUM16
ENUMX
  BFD_RELOC_CR16_NUM32
ENUMX
  BFD_RELOC_CR16_NUM32a
ENUMX
  BFD_RELOC_CR16_REGREL0
ENUMX
  BFD_RELOC_CR16_REGREL4
ENUMX
  BFD_RELOC_CR16_REGREL4a
ENUMX
  BFD_RELOC_CR16_REGREL14
ENUMX
  BFD_RELOC_CR16_REGREL14a
ENUMX
  BFD_RELOC_CR16_REGREL16
ENUMX
  BFD_RELOC_CR16_REGREL20
ENUMX
  BFD_RELOC_CR16_REGREL20a
ENUMX
  BFD_RELOC_CR16_ABS20
ENUMX
  BFD_RELOC_CR16_ABS24
ENUMX
  BFD_RELOC_CR16_IMM4
ENUMX
  BFD_RELOC_CR16_IMM8
ENUMX
  BFD_RELOC_CR16_IMM16
ENUMX
  BFD_RELOC_CR16_IMM20
ENUMX
  BFD_RELOC_CR16_IMM24
ENUMX
  BFD_RELOC_CR16_IMM32
ENUMX
  BFD_RELOC_CR16_IMM32a
ENUMX
  BFD_RELOC_CR16_DISP4
ENUMX
  BFD_RELOC_CR16_DISP8
ENUMX
  BFD_RELOC_CR16_DISP16
ENUMX
  BFD_RELOC_CR16_DISP20
ENUMX
  BFD_RELOC_CR16_DISP24
ENUMX
  BFD_RELOC_CR16_DISP24a
ENUMX
  BFD_RELOC_CR16_SWITCH8
ENUMX
  BFD_RELOC_CR16_SWITCH16
ENUMX
  BFD_RELOC_CR16_SWITCH32
ENUMX
  BFD_RELOC_CR16_GOT_REGREL20
ENUMX
  BFD_RELOC_CR16_GOTC_REGREL20
ENUMX
  BFD_RELOC_CR16_GLOB_DAT
ENUMDOC
  NS CR16 Relocations.

ENUM
  BFD_RELOC_CRX_REL4
ENUMX
  BFD_RELOC_CRX_REL8
ENUMX
  BFD_RELOC_CRX_REL8_CMP
ENUMX
  BFD_RELOC_CRX_REL16
ENUMX
  BFD_RELOC_CRX_REL24
ENUMX
  BFD_RELOC_CRX_REL32
ENUMX
  BFD_RELOC_CRX_REGREL12
ENUMX
  BFD_RELOC_CRX_REGREL22
ENUMX
  BFD_RELOC_CRX_REGREL28
ENUMX
  BFD_RELOC_CRX_REGREL32
ENUMX
  BFD_RELOC_CRX_ABS16
ENUMX
  BFD_RELOC_CRX_ABS32
ENUMX
  BFD_RELOC_CRX_NUM8
ENUMX
  BFD_RELOC_CRX_NUM16
ENUMX
  BFD_RELOC_CRX_NUM32
ENUMX
  BFD_RELOC_CRX_IMM16
ENUMX
  BFD_RELOC_CRX_IMM32
ENUMX
  BFD_RELOC_CRX_SWITCH8
ENUMX
  BFD_RELOC_CRX_SWITCH16
ENUMX
  BFD_RELOC_CRX_SWITCH32
ENUMDOC
  NS CRX Relocations.

ENUM
  BFD_RELOC_CRIS_BDISP8
ENUMX
  BFD_RELOC_CRIS_UNSIGNED_5
ENUMX
  BFD_RELOC_CRIS_SIGNED_6
ENUMX
  BFD_RELOC_CRIS_UNSIGNED_6
ENUMX
  BFD_RELOC_CRIS_SIGNED_8
ENUMX
  BFD_RELOC_CRIS_UNSIGNED_8
ENUMX
  BFD_RELOC_CRIS_SIGNED_16
ENUMX
  BFD_RELOC_CRIS_UNSIGNED_16
ENUMX
  BFD_RELOC_CRIS_LAPCQ_OFFSET
ENUMX
  BFD_RELOC_CRIS_UNSIGNED_4
ENUMDOC
  These relocs are only used within the CRIS assembler.  They are not
  (at present) written to any object files.
ENUM
  BFD_RELOC_CRIS_COPY
ENUMX
  BFD_RELOC_CRIS_GLOB_DAT
ENUMX
  BFD_RELOC_CRIS_JUMP_SLOT
ENUMX
  BFD_RELOC_CRIS_RELATIVE
ENUMDOC
  Relocs used in ELF shared libraries for CRIS.
ENUM
  BFD_RELOC_CRIS_32_GOT
ENUMDOC
  32-bit offset to symbol-entry within GOT.
ENUM
  BFD_RELOC_CRIS_16_GOT
ENUMDOC
  16-bit offset to symbol-entry within GOT.
ENUM
  BFD_RELOC_CRIS_32_GOTPLT
ENUMDOC
  32-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_CRIS_16_GOTPLT
ENUMDOC
  16-bit offset to symbol-entry within GOT, with PLT handling.
ENUM
  BFD_RELOC_CRIS_32_GOTREL
ENUMDOC
  32-bit offset to symbol, relative to GOT.
ENUM
  BFD_RELOC_CRIS_32_PLT_GOTREL
ENUMDOC
  32-bit offset to symbol with PLT entry, relative to GOT.
ENUM
  BFD_RELOC_CRIS_32_PLT_PCREL
ENUMDOC
  32-bit offset to symbol with PLT entry, relative to this relocation.

ENUM
  BFD_RELOC_CRIS_32_GOT_GD
ENUMX
  BFD_RELOC_CRIS_16_GOT_GD
ENUMX
  BFD_RELOC_CRIS_32_GD
ENUMX
  BFD_RELOC_CRIS_DTP
ENUMX
  BFD_RELOC_CRIS_32_DTPREL
ENUMX
  BFD_RELOC_CRIS_16_DTPREL
ENUMX
  BFD_RELOC_CRIS_32_GOT_TPREL
ENUMX
  BFD_RELOC_CRIS_16_GOT_TPREL
ENUMX
  BFD_RELOC_CRIS_32_TPREL
ENUMX
  BFD_RELOC_CRIS_16_TPREL
ENUMX
  BFD_RELOC_CRIS_DTPMOD
ENUMX
  BFD_RELOC_CRIS_32_IE
ENUMDOC
  Relocs used in TLS code for CRIS.

ENUM
  BFD_RELOC_OR1K_REL_26
ENUMX
  BFD_RELOC_OR1K_SLO16
ENUMX
  BFD_RELOC_OR1K_PCREL_PG21
ENUMX
  BFD_RELOC_OR1K_LO13
ENUMX
  BFD_RELOC_OR1K_SLO13
ENUMX
  BFD_RELOC_OR1K_GOTPC_HI16
ENUMX
  BFD_RELOC_OR1K_GOTPC_LO16
ENUMX
  BFD_RELOC_OR1K_GOT_AHI16
ENUMX
  BFD_RELOC_OR1K_GOT16
ENUMX
  BFD_RELOC_OR1K_GOT_PG21
ENUMX
  BFD_RELOC_OR1K_GOT_LO13
ENUMX
  BFD_RELOC_OR1K_PLT26
ENUMX
  BFD_RELOC_OR1K_PLTA26
ENUMX
  BFD_RELOC_OR1K_GOTOFF_SLO16
ENUMX
  BFD_RELOC_OR1K_COPY
ENUMX
  BFD_RELOC_OR1K_GLOB_DAT
ENUMX
  BFD_RELOC_OR1K_JMP_SLOT
ENUMX
  BFD_RELOC_OR1K_RELATIVE
ENUMX
  BFD_RELOC_OR1K_TLS_GD_HI16
ENUMX
  BFD_RELOC_OR1K_TLS_GD_LO16
ENUMX
  BFD_RELOC_OR1K_TLS_GD_PG21
ENUMX
  BFD_RELOC_OR1K_TLS_GD_LO13
ENUMX
  BFD_RELOC_OR1K_TLS_LDM_HI16
ENUMX
  BFD_RELOC_OR1K_TLS_LDM_LO16
ENUMX
  BFD_RELOC_OR1K_TLS_LDM_PG21
ENUMX
  BFD_RELOC_OR1K_TLS_LDM_LO13
ENUMX
  BFD_RELOC_OR1K_TLS_LDO_HI16
ENUMX
  BFD_RELOC_OR1K_TLS_LDO_LO16
ENUMX
  BFD_RELOC_OR1K_TLS_IE_HI16
ENUMX
  BFD_RELOC_OR1K_TLS_IE_AHI16
ENUMX
  BFD_RELOC_OR1K_TLS_IE_LO16
ENUMX
  BFD_RELOC_OR1K_TLS_IE_PG21
ENUMX
  BFD_RELOC_OR1K_TLS_IE_LO13
ENUMX
  BFD_RELOC_OR1K_TLS_LE_HI16
ENUMX
  BFD_RELOC_OR1K_TLS_LE_AHI16
ENUMX
  BFD_RELOC_OR1K_TLS_LE_LO16
ENUMX
  BFD_RELOC_OR1K_TLS_LE_SLO16
ENUMX
  BFD_RELOC_OR1K_TLS_TPOFF
ENUMX
  BFD_RELOC_OR1K_TLS_DTPOFF
ENUMX
  BFD_RELOC_OR1K_TLS_DTPMOD
ENUMDOC
  OpenRISC 1000 Relocations.

ENUM
  BFD_RELOC_H8_DIR16A8
ENUMX
  BFD_RELOC_H8_DIR16R8
ENUMX
  BFD_RELOC_H8_DIR24A8
ENUMX
  BFD_RELOC_H8_DIR24R8
ENUMX
  BFD_RELOC_H8_DIR32A16
ENUMX
  BFD_RELOC_H8_DISP32A16
ENUMDOC
  H8 elf Relocations.

ENUM
  BFD_RELOC_XSTORMY16_REL_12
ENUMX
  BFD_RELOC_XSTORMY16_12
ENUMX
  BFD_RELOC_XSTORMY16_24
ENUMX
  BFD_RELOC_XSTORMY16_FPTR16
ENUMDOC
  Sony Xstormy16 Relocations.

ENUM
  BFD_RELOC_RELC
ENUMDOC
  Self-describing complex relocations.
COMMENT

ENUM
  BFD_RELOC_VAX_GLOB_DAT
ENUMX
  BFD_RELOC_VAX_JMP_SLOT
ENUMX
  BFD_RELOC_VAX_RELATIVE
ENUMDOC
  Relocations used by VAX ELF.

ENUM
  BFD_RELOC_MT_PC16
ENUMDOC
  Morpho MT - 16 bit immediate relocation.
ENUM
  BFD_RELOC_MT_HI16
ENUMDOC
  Morpho MT - Hi 16 bits of an address.
ENUM
  BFD_RELOC_MT_LO16
ENUMDOC
  Morpho MT - Low 16 bits of an address.
ENUM
  BFD_RELOC_MT_GNU_VTINHERIT
ENUMDOC
  Morpho MT - Used to tell the linker which vtable entries are used.
ENUM
  BFD_RELOC_MT_GNU_VTENTRY
ENUMDOC
  Morpho MT - Used to tell the linker which vtable entries are used.
ENUM
  BFD_RELOC_MT_PCINSN8
ENUMDOC
  Morpho MT - 8 bit immediate relocation.

ENUM
  BFD_RELOC_MSP430_10_PCREL
ENUMX
  BFD_RELOC_MSP430_16_PCREL
ENUMX
  BFD_RELOC_MSP430_16
ENUMX
  BFD_RELOC_MSP430_16_PCREL_BYTE
ENUMX
  BFD_RELOC_MSP430_16_BYTE
ENUMX
  BFD_RELOC_MSP430_2X_PCREL
ENUMX
  BFD_RELOC_MSP430_RL_PCREL
ENUMX
  BFD_RELOC_MSP430_ABS8
ENUMX
  BFD_RELOC_MSP430X_PCR20_EXT_SRC
ENUMX
  BFD_RELOC_MSP430X_PCR20_EXT_DST
ENUMX
  BFD_RELOC_MSP430X_PCR20_EXT_ODST
ENUMX
  BFD_RELOC_MSP430X_ABS20_EXT_SRC
ENUMX
  BFD_RELOC_MSP430X_ABS20_EXT_DST
ENUMX
  BFD_RELOC_MSP430X_ABS20_EXT_ODST
ENUMX
  BFD_RELOC_MSP430X_ABS20_ADR_SRC
ENUMX
  BFD_RELOC_MSP430X_ABS20_ADR_DST
ENUMX
  BFD_RELOC_MSP430X_PCR16
ENUMX
  BFD_RELOC_MSP430X_PCR20_CALL
ENUMX
  BFD_RELOC_MSP430X_ABS16
ENUMX
  BFD_RELOC_MSP430_ABS_HI16
ENUMX
  BFD_RELOC_MSP430_PREL31
ENUMX
  BFD_RELOC_MSP430_SYM_DIFF
ENUMX
  BFD_RELOC_MSP430_SET_ULEB128
ENUMX
  BFD_RELOC_MSP430_SUB_ULEB128

ENUMDOC
  msp430 specific relocation codes

ENUM
  BFD_RELOC_NIOS2_S16
ENUMX
  BFD_RELOC_NIOS2_U16
ENUMX
  BFD_RELOC_NIOS2_CALL26
ENUMX
  BFD_RELOC_NIOS2_IMM5
ENUMX
  BFD_RELOC_NIOS2_CACHE_OPX
ENUMX
  BFD_RELOC_NIOS2_IMM6
ENUMX
  BFD_RELOC_NIOS2_IMM8
ENUMX
  BFD_RELOC_NIOS2_HI16
ENUMX
  BFD_RELOC_NIOS2_LO16
ENUMX
  BFD_RELOC_NIOS2_HIADJ16
ENUMX
  BFD_RELOC_NIOS2_GPREL
ENUMX
  BFD_RELOC_NIOS2_UJMP
ENUMX
  BFD_RELOC_NIOS2_CJMP
ENUMX
  BFD_RELOC_NIOS2_CALLR
ENUMX
  BFD_RELOC_NIOS2_ALIGN
ENUMX
  BFD_RELOC_NIOS2_GOT16
ENUMX
  BFD_RELOC_NIOS2_CALL16
ENUMX
  BFD_RELOC_NIOS2_GOTOFF_LO
ENUMX
  BFD_RELOC_NIOS2_GOTOFF_HA
ENUMX
  BFD_RELOC_NIOS2_PCREL_LO
ENUMX
  BFD_RELOC_NIOS2_PCREL_HA
ENUMX
  BFD_RELOC_NIOS2_TLS_GD16
ENUMX
  BFD_RELOC_NIOS2_TLS_LDM16
ENUMX
  BFD_RELOC_NIOS2_TLS_LDO16
ENUMX
  BFD_RELOC_NIOS2_TLS_IE16
ENUMX
  BFD_RELOC_NIOS2_TLS_LE16
ENUMX
  BFD_RELOC_NIOS2_TLS_DTPMOD
ENUMX
  BFD_RELOC_NIOS2_TLS_DTPREL
ENUMX
  BFD_RELOC_NIOS2_TLS_TPREL
ENUMX
  BFD_RELOC_NIOS2_COPY
ENUMX
  BFD_RELOC_NIOS2_GLOB_DAT
ENUMX
  BFD_RELOC_NIOS2_JUMP_SLOT
ENUMX
  BFD_RELOC_NIOS2_RELATIVE
ENUMX
  BFD_RELOC_NIOS2_GOTOFF
ENUMX
  BFD_RELOC_NIOS2_CALL26_NOAT
ENUMX
  BFD_RELOC_NIOS2_GOT_LO
ENUMX
  BFD_RELOC_NIOS2_GOT_HA
ENUMX
  BFD_RELOC_NIOS2_CALL_LO
ENUMX
  BFD_RELOC_NIOS2_CALL_HA
ENUMX
  BFD_RELOC_NIOS2_R2_S12
ENUMX
  BFD_RELOC_NIOS2_R2_I10_1_PCREL
ENUMX
  BFD_RELOC_NIOS2_R2_T1I7_1_PCREL
ENUMX
  BFD_RELOC_NIOS2_R2_T1I7_2
ENUMX
  BFD_RELOC_NIOS2_R2_T2I4
ENUMX
  BFD_RELOC_NIOS2_R2_T2I4_1
ENUMX
  BFD_RELOC_NIOS2_R2_T2I4_2
ENUMX
  BFD_RELOC_NIOS2_R2_X1I7_2
ENUMX
  BFD_RELOC_NIOS2_R2_X2L5
ENUMX
  BFD_RELOC_NIOS2_R2_F1I5_2
ENUMX
  BFD_RELOC_NIOS2_R2_L5I4X1
ENUMX
  BFD_RELOC_NIOS2_R2_T1X1I6
ENUMX
  BFD_RELOC_NIOS2_R2_T1X1I6_2
ENUMDOC
  Relocations used by the Altera Nios II core.

ENUM
  BFD_RELOC_PRU_U16
ENUMDOC
  PRU LDI 16-bit unsigned data-memory relocation.
ENUM
  BFD_RELOC_PRU_U16_PMEMIMM
ENUMDOC
  PRU LDI 16-bit unsigned instruction-memory relocation.
ENUM
  BFD_RELOC_PRU_LDI32
ENUMDOC
  PRU relocation for two consecutive LDI load instructions that load a
  32 bit value into a register. If the higher bits are all zero, then
  the second instruction may be relaxed.
ENUM
  BFD_RELOC_PRU_S10_PCREL
ENUMDOC
  PRU QBBx 10-bit signed PC-relative relocation.
ENUM
  BFD_RELOC_PRU_U8_PCREL
ENUMDOC
  PRU 8-bit unsigned relocation used for the LOOP instruction.
ENUM
  BFD_RELOC_PRU_32_PMEM
ENUMX
  BFD_RELOC_PRU_16_PMEM
ENUMDOC
  PRU Program Memory relocations.  Used to convert from byte addressing to
  32-bit word addressing.
ENUM
  BFD_RELOC_PRU_GNU_DIFF8
ENUMX
  BFD_RELOC_PRU_GNU_DIFF16
ENUMX
  BFD_RELOC_PRU_GNU_DIFF32
ENUMX
  BFD_RELOC_PRU_GNU_DIFF16_PMEM
ENUMX
  BFD_RELOC_PRU_GNU_DIFF32_PMEM
ENUMDOC
  PRU relocations to mark the difference of two local symbols.
  These are only needed to support linker relaxation and can be ignored
  when not relaxing.  The field is set to the value of the difference
  assuming no relaxation.  The relocation encodes the position of the
  second symbol so the linker can determine whether to adjust the field
  value. The PMEM variants encode the word difference, instead of byte
  difference between symbols.

ENUM
  BFD_RELOC_IQ2000_OFFSET_16
ENUMX
  BFD_RELOC_IQ2000_OFFSET_21
ENUMX
  BFD_RELOC_IQ2000_UHI16
ENUMDOC
  IQ2000 Relocations.

ENUM
  BFD_RELOC_XTENSA_RTLD
ENUMDOC
  Special Xtensa relocation used only by PLT entries in ELF shared
  objects to indicate that the runtime linker should set the value
  to one of its own internal functions or data structures.
ENUM
  BFD_RELOC_XTENSA_GLOB_DAT
ENUMX
  BFD_RELOC_XTENSA_JMP_SLOT
ENUMX
  BFD_RELOC_XTENSA_RELATIVE
ENUMDOC
  Xtensa relocations for ELF shared objects.
ENUM
  BFD_RELOC_XTENSA_PLT
ENUMDOC
  Xtensa relocation used in ELF object files for symbols that may require
  PLT entries.  Otherwise, this is just a generic 32-bit relocation.
ENUM
  BFD_RELOC_XTENSA_DIFF8
ENUMX
  BFD_RELOC_XTENSA_DIFF16
ENUMX
  BFD_RELOC_XTENSA_DIFF32
ENUMDOC
  Xtensa relocations for backward compatibility.  These have been replaced
  by BFD_RELOC_XTENSA_PDIFF and BFD_RELOC_XTENSA_NDIFF.
  Xtensa relocations to mark the difference of two local symbols.
  These are only needed to support linker relaxation and can be ignored
  when not relaxing.  The field is set to the value of the difference
  assuming no relaxation.  The relocation encodes the position of the
  first symbol so the linker can determine whether to adjust the field
  value.
ENUM
  BFD_RELOC_XTENSA_SLOT0_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT1_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT2_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT3_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT4_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT5_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT6_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT7_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT8_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT9_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT10_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT11_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT12_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT13_OP
ENUMX
  BFD_RELOC_XTENSA_SLOT14_OP
ENUMDOC
  Generic Xtensa relocations for instruction operands.  Only the slot
  number is encoded in the relocation.  The relocation applies to the
  last PC-relative immediate operand, or if there are no PC-relative
  immediates, to the last immediate operand.
ENUM
  BFD_RELOC_XTENSA_SLOT0_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT1_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT2_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT3_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT4_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT5_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT6_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT7_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT8_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT9_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT10_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT11_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT12_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT13_ALT
ENUMX
  BFD_RELOC_XTENSA_SLOT14_ALT
ENUMDOC
  Alternate Xtensa relocations.  Only the slot is encoded in the
  relocation.  The meaning of these relocations is opcode-specific.
ENUM
  BFD_RELOC_XTENSA_OP0
ENUMX
  BFD_RELOC_XTENSA_OP1
ENUMX
  BFD_RELOC_XTENSA_OP2
ENUMDOC
  Xtensa relocations for backward compatibility.  These have all been
  replaced by BFD_RELOC_XTENSA_SLOT0_OP.
ENUM
  BFD_RELOC_XTENSA_ASM_EXPAND
ENUMDOC
  Xtensa relocation to mark that the assembler expanded the
  instructions from an original target.  The expansion size is
  encoded in the reloc size.
ENUM
  BFD_RELOC_XTENSA_ASM_SIMPLIFY
ENUMDOC
  Xtensa relocation to mark that the linker should simplify
  assembler-expanded instructions.  This is commonly used
  internally by the linker after analysis of a
  BFD_RELOC_XTENSA_ASM_EXPAND.
ENUM
  BFD_RELOC_XTENSA_TLSDESC_FN
ENUMX
  BFD_RELOC_XTENSA_TLSDESC_ARG
ENUMX
  BFD_RELOC_XTENSA_TLS_DTPOFF
ENUMX
  BFD_RELOC_XTENSA_TLS_TPOFF
ENUMX
  BFD_RELOC_XTENSA_TLS_FUNC
ENUMX
  BFD_RELOC_XTENSA_TLS_ARG
ENUMX
  BFD_RELOC_XTENSA_TLS_CALL
ENUMDOC
  Xtensa TLS relocations.
ENUM
  BFD_RELOC_XTENSA_PDIFF8
ENUMX
  BFD_RELOC_XTENSA_PDIFF16
ENUMX
  BFD_RELOC_XTENSA_PDIFF32
ENUMX
  BFD_RELOC_XTENSA_NDIFF8
ENUMX
  BFD_RELOC_XTENSA_NDIFF16
ENUMX
  BFD_RELOC_XTENSA_NDIFF32
ENUMDOC
  Xtensa relocations to mark the difference of two local symbols.
  These are only needed to support linker relaxation and can be ignored
  when not relaxing.  The field is set to the value of the difference
  assuming no relaxation.  The relocation encodes the position of the
  subtracted symbol so the linker can determine whether to adjust the field
  value.  PDIFF relocations are used for positive differences, NDIFF
  relocations are used for negative differences.  The difference value
  is treated as unsigned with these relocation types, giving full
  8/16 value ranges.

ENUM
  BFD_RELOC_Z80_DISP8
ENUMDOC
  8 bit signed offset in (ix+d) or (iy+d).
ENUM
  BFD_RELOC_Z80_BYTE0
ENUMDOC
  First 8 bits of multibyte (32, 24 or 16 bit) value.
ENUM
  BFD_RELOC_Z80_BYTE1
ENUMDOC
  Second 8 bits of multibyte (32, 24 or 16 bit) value.
ENUM
  BFD_RELOC_Z80_BYTE2
ENUMDOC
  Third 8 bits of multibyte (32 or 24 bit) value.
ENUM
  BFD_RELOC_Z80_BYTE3
ENUMDOC
  Fourth 8 bits of multibyte (32 bit) value.
ENUM
  BFD_RELOC_Z80_WORD0
ENUMDOC
  Lowest 16 bits of multibyte (32 or 24 bit) value.
ENUM
  BFD_RELOC_Z80_WORD1
ENUMDOC
  Highest 16 bits of multibyte (32 or 24 bit) value.
ENUM
  BFD_RELOC_Z80_16_BE
ENUMDOC
  Like BFD_RELOC_16 but big-endian.

ENUM
  BFD_RELOC_Z8K_DISP7
ENUMDOC
  DJNZ offset.
ENUM
  BFD_RELOC_Z8K_CALLR
ENUMDOC
  CALR offset.
ENUM
  BFD_RELOC_Z8K_IMM4L
ENUMDOC
  4 bit value.

ENUM
   BFD_RELOC_LM32_CALL
ENUMX
   BFD_RELOC_LM32_BRANCH
ENUMX
   BFD_RELOC_LM32_16_GOT
ENUMX
   BFD_RELOC_LM32_GOTOFF_HI16
ENUMX
   BFD_RELOC_LM32_GOTOFF_LO16
ENUMX
   BFD_RELOC_LM32_COPY
ENUMX
   BFD_RELOC_LM32_GLOB_DAT
ENUMX
   BFD_RELOC_LM32_JMP_SLOT
ENUMX
   BFD_RELOC_LM32_RELATIVE
ENUMDOC
 Lattice Mico32 relocations.

ENUM
  BFD_RELOC_MACH_O_SECTDIFF
ENUMDOC
  Difference between two section addreses.  Must be followed by a
  BFD_RELOC_MACH_O_PAIR.
ENUM
  BFD_RELOC_MACH_O_LOCAL_SECTDIFF
ENUMDOC
  Like BFD_RELOC_MACH_O_SECTDIFF but with a local symbol.
ENUM
  BFD_RELOC_MACH_O_PAIR
ENUMDOC
  Pair of relocation.  Contains the first symbol.
ENUM
  BFD_RELOC_MACH_O_SUBTRACTOR32
ENUMDOC
  Symbol will be substracted.  Must be followed by a BFD_RELOC_32.
ENUM
  BFD_RELOC_MACH_O_SUBTRACTOR64
ENUMDOC
  Symbol will be substracted.  Must be followed by a BFD_RELOC_64.

ENUM
  BFD_RELOC_MACH_O_X86_64_BRANCH32
ENUMX
  BFD_RELOC_MACH_O_X86_64_BRANCH8
ENUMDOC
  PCREL relocations.  They are marked as branch to create PLT entry if
  required.
ENUM
  BFD_RELOC_MACH_O_X86_64_GOT
ENUMDOC
  Used when referencing a GOT entry.
ENUM
  BFD_RELOC_MACH_O_X86_64_GOT_LOAD
ENUMDOC
  Used when loading a GOT entry with movq.  It is specially marked so that
  the linker could optimize the movq to a leaq if possible.
ENUM
  BFD_RELOC_MACH_O_X86_64_PCREL32_1
ENUMDOC
  Same as BFD_RELOC_32_PCREL but with an implicit -1 addend.
ENUM
  BFD_RELOC_MACH_O_X86_64_PCREL32_2
ENUMDOC
  Same as BFD_RELOC_32_PCREL but with an implicit -2 addend.
ENUM
  BFD_RELOC_MACH_O_X86_64_PCREL32_4
ENUMDOC
  Same as BFD_RELOC_32_PCREL but with an implicit -4 addend.
ENUM
  BFD_RELOC_MACH_O_X86_64_TLV
ENUMDOC
  Used when referencing a TLV entry.


ENUM
  BFD_RELOC_MACH_O_ARM64_ADDEND
ENUMDOC
  Addend for PAGE or PAGEOFF.
ENUM
  BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGE21
ENUMDOC
  Relative offset to page of GOT slot.
ENUM
  BFD_RELOC_MACH_O_ARM64_GOT_LOAD_PAGEOFF12
ENUMDOC
  Relative offset within page of GOT slot.
ENUM
  BFD_RELOC_MACH_O_ARM64_POINTER_TO_GOT
ENUMDOC
  Address of a GOT entry.

ENUM
  BFD_RELOC_MICROBLAZE_32_LO
ENUMDOC
  This is a 32 bit reloc for the microblaze that stores the
  low 16 bits of a value
ENUM
  BFD_RELOC_MICROBLAZE_32_LO_PCREL
ENUMDOC
  This is a 32 bit pc-relative reloc for the microblaze that
  stores the low 16 bits of a value
ENUM
  BFD_RELOC_MICROBLAZE_32_ROSDA
ENUMDOC
  This is a 32 bit reloc for the microblaze that stores a
  value relative to the read-only small data area anchor
ENUM
  BFD_RELOC_MICROBLAZE_32_RWSDA
ENUMDOC
  This is a 32 bit reloc for the microblaze that stores a
  value relative to the read-write small data area anchor
ENUM
  BFD_RELOC_MICROBLAZE_32_SYM_OP_SYM
ENUMDOC
  This is a 32 bit reloc for the microblaze to handle
  expressions of the form "Symbol Op Symbol"
ENUM
  BFD_RELOC_MICROBLAZE_64_NONE
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit pc relative
  value in two words (with an imm instruction).  No relocation is
  done here - only used for relaxing
ENUM
  BFD_RELOC_MICROBLAZE_64_GOTPC
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit pc relative
  value in two words (with an imm instruction).  The relocation is
  PC-relative GOT offset
ENUM
  BFD_RELOC_MICROBLAZE_64_GOT
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit pc relative
  value in two words (with an imm instruction).  The relocation is
  GOT offset
ENUM
  BFD_RELOC_MICROBLAZE_64_PLT
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit pc relative
  value in two words (with an imm instruction).  The relocation is
  PC-relative offset into PLT
ENUM
  BFD_RELOC_MICROBLAZE_64_GOTOFF
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit GOT relative
  value in two words (with an imm instruction).  The relocation is
  relative offset from _GLOBAL_OFFSET_TABLE_
ENUM
  BFD_RELOC_MICROBLAZE_32_GOTOFF
ENUMDOC
  This is a 32 bit reloc that stores the 32 bit GOT relative
  value in a word.  The relocation is relative offset from
  _GLOBAL_OFFSET_TABLE_
ENUM
  BFD_RELOC_MICROBLAZE_COPY
ENUMDOC
  This is used to tell the dynamic linker to copy the value out of
  the dynamic object into the runtime process image.
ENUM
  BFD_RELOC_MICROBLAZE_64_TLS
ENUMDOC
  Unused Reloc
ENUM
  BFD_RELOC_MICROBLAZE_64_TLSGD
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit GOT relative value
  of the GOT TLS GD info entry in two words (with an imm instruction). The
  relocation is GOT offset.
ENUM
  BFD_RELOC_MICROBLAZE_64_TLSLD
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit GOT relative value
  of the GOT TLS LD info entry in two words (with an imm instruction). The
  relocation is GOT offset.
ENUM
  BFD_RELOC_MICROBLAZE_32_TLSDTPMOD
ENUMDOC
  This is a 32 bit reloc that stores the Module ID to GOT(n).
ENUM
  BFD_RELOC_MICROBLAZE_32_TLSDTPREL
ENUMDOC
  This is a 32 bit reloc that stores TLS offset to GOT(n+1).
ENUM
  BFD_RELOC_MICROBLAZE_64_TLSDTPREL
ENUMDOC
  This is a 32 bit reloc for storing TLS offset to two words (uses imm
  instruction)
ENUM
  BFD_RELOC_MICROBLAZE_64_TLSGOTTPREL
ENUMDOC
  This is a 64 bit reloc that stores 32-bit thread pointer relative offset
  to two words (uses imm instruction).
ENUM
  BFD_RELOC_MICROBLAZE_64_TLSTPREL
ENUMDOC
  This is a 64 bit reloc that stores 32-bit thread pointer relative offset
  to two words (uses imm instruction).
ENUM
  BFD_RELOC_MICROBLAZE_64_TEXTPCREL
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit pc relative
  value in two words (with an imm instruction).  The relocation is
  PC-relative offset from start of TEXT.
ENUM
  BFD_RELOC_MICROBLAZE_64_TEXTREL
ENUMDOC
  This is a 64 bit reloc that stores the 32 bit offset
  value in two words (with an imm instruction).  The relocation is
  relative offset from start of TEXT.

ENUM
  BFD_RELOC_AARCH64_RELOC_START
ENUMDOC
  AArch64 pseudo relocation code to mark the start of the AArch64
  relocation enumerators.  N.B. the order of the enumerators is
  important as several tables in the AArch64 bfd backend are indexed
  by these enumerators; make sure they are all synced.
ENUM
  BFD_RELOC_AARCH64_NULL
ENUMDOC
  Deprecated AArch64 null relocation code.
ENUM
  BFD_RELOC_AARCH64_NONE
ENUMDOC
  AArch64 null relocation code.
ENUM
  BFD_RELOC_AARCH64_64
ENUMX
  BFD_RELOC_AARCH64_32
ENUMX
  BFD_RELOC_AARCH64_16
ENUMDOC
  Basic absolute relocations of N bits.  These are equivalent to
BFD_RELOC_N and they were added to assist the indexing of the howto
table.
ENUM
  BFD_RELOC_AARCH64_64_PCREL
ENUMX
  BFD_RELOC_AARCH64_32_PCREL
ENUMX
  BFD_RELOC_AARCH64_16_PCREL
ENUMDOC
  PC-relative relocations.  These are equivalent to BFD_RELOC_N_PCREL
and they were added to assist the indexing of the howto table.
ENUM
  BFD_RELOC_AARCH64_MOVW_G0
ENUMDOC
  AArch64 MOV[NZK] instruction with most significant bits 0 to 15
  of an unsigned address/value.
ENUM
  BFD_RELOC_AARCH64_MOVW_G0_NC
ENUMDOC
  AArch64 MOV[NZK] instruction with less significant bits 0 to 15 of
  an address/value.  No overflow checking.
ENUM
  BFD_RELOC_AARCH64_MOVW_G1
ENUMDOC
  AArch64 MOV[NZK] instruction with most significant bits 16 to 31
  of an unsigned address/value.
ENUM
  BFD_RELOC_AARCH64_MOVW_G1_NC
ENUMDOC
  AArch64 MOV[NZK] instruction with less significant bits 16 to 31
  of an address/value.  No overflow checking.
ENUM
  BFD_RELOC_AARCH64_MOVW_G2
ENUMDOC
  AArch64 MOV[NZK] instruction with most significant bits 32 to 47
  of an unsigned address/value.
ENUM
  BFD_RELOC_AARCH64_MOVW_G2_NC
ENUMDOC
  AArch64 MOV[NZK] instruction with less significant bits 32 to 47
  of an address/value.  No overflow checking.
ENUM
  BFD_RELOC_AARCH64_MOVW_G3
ENUMDOC
  AArch64 MOV[NZK] instruction with most signficant bits 48 to 64
  of a signed or unsigned address/value.
ENUM
  BFD_RELOC_AARCH64_MOVW_G0_S
ENUMDOC
  AArch64 MOV[NZ] instruction with most significant bits 0 to 15
  of a signed value.  Changes instruction to MOVZ or MOVN depending on the
  value's sign.
ENUM
  BFD_RELOC_AARCH64_MOVW_G1_S
ENUMDOC
  AArch64 MOV[NZ] instruction with most significant bits 16 to 31
  of a signed value.  Changes instruction to MOVZ or MOVN depending on the
  value's sign.
ENUM
  BFD_RELOC_AARCH64_MOVW_G2_S
ENUMDOC
  AArch64 MOV[NZ] instruction with most significant bits 32 to 47
  of a signed value.  Changes instruction to MOVZ or MOVN depending on the
  value's sign.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G0
ENUMDOC
  AArch64 MOV[NZ] instruction with most significant bits 0 to 15
  of a signed value.  Changes instruction to MOVZ or MOVN depending on the
  value's sign.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G0_NC
ENUMDOC
  AArch64 MOV[NZ] instruction with most significant bits 0 to 15
  of a signed value.  Changes instruction to MOVZ or MOVN depending on the
  value's sign.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G1
ENUMDOC
  AArch64 MOVK instruction with most significant bits 16 to 31
  of a signed value.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G1_NC
ENUMDOC
  AArch64 MOVK instruction with most significant bits 16 to 31
  of a signed value.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G2
ENUMDOC
  AArch64 MOVK instruction with most significant bits 32 to 47
  of a signed value.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G2_NC
ENUMDOC
  AArch64 MOVK instruction with most significant bits 32 to 47
  of a signed value.
ENUM
  BFD_RELOC_AARCH64_MOVW_PREL_G3
ENUMDOC
  AArch64 MOVK instruction with most significant bits 47 to 63
  of a signed value.
ENUM
  BFD_RELOC_AARCH64_LD_LO19_PCREL
ENUMDOC
  AArch64 Load Literal instruction, holding a 19 bit pc-relative word
  offset.  The lowest two bits must be zero and are not stored in the
  instruction, giving a 21 bit signed byte offset.
ENUM
  BFD_RELOC_AARCH64_ADR_LO21_PCREL
ENUMDOC
  AArch64 ADR instruction, holding a simple 21 bit pc-relative byte offset.
ENUM
  BFD_RELOC_AARCH64_ADR_HI21_PCREL
ENUMDOC
  AArch64 ADRP instruction, with bits 12 to 32 of a pc-relative page
  offset, giving a 4KB aligned page base address.
ENUM
  BFD_RELOC_AARCH64_ADR_HI21_NC_PCREL
ENUMDOC
  AArch64 ADRP instruction, with bits 12 to 32 of a pc-relative page
  offset, giving a 4KB aligned page base address, but with no overflow
  checking.
ENUM
  BFD_RELOC_AARCH64_ADD_LO12
ENUMDOC
  AArch64 ADD immediate instruction, holding bits 0 to 11 of the address.
  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_LDST8_LO12
ENUMDOC
  AArch64 8-bit load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_TSTBR14
ENUMDOC
  AArch64 14 bit pc-relative test bit and branch.
  The lowest two bits must be zero and are not stored in the instruction,
  giving a 16 bit signed byte offset.
ENUM
  BFD_RELOC_AARCH64_BRANCH19
ENUMDOC
  AArch64 19 bit pc-relative conditional branch and compare & branch.
  The lowest two bits must be zero and are not stored in the instruction,
  giving a 21 bit signed byte offset.
ENUM
  BFD_RELOC_AARCH64_JUMP26
ENUMDOC
  AArch64 26 bit pc-relative unconditional branch.
  The lowest two bits must be zero and are not stored in the instruction,
  giving a 28 bit signed byte offset.
ENUM
  BFD_RELOC_AARCH64_CALL26
ENUMDOC
  AArch64 26 bit pc-relative unconditional branch and link.
  The lowest two bits must be zero and are not stored in the instruction,
  giving a 28 bit signed byte offset.
ENUM
  BFD_RELOC_AARCH64_LDST16_LO12
ENUMDOC
  AArch64 16-bit load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_LDST32_LO12
ENUMDOC
  AArch64 32-bit load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_LDST64_LO12
ENUMDOC
  AArch64 64-bit load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_LDST128_LO12
ENUMDOC
  AArch64 128-bit load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_GOT_LD_PREL19
ENUMDOC
  AArch64 Load Literal instruction, holding a 19 bit PC relative word
  offset of the global offset table entry for a symbol.  The lowest two
  bits must be zero and are not stored in the instruction, giving a 21
  bit signed byte offset.  This relocation type requires signed overflow
  checking.
ENUM
  BFD_RELOC_AARCH64_ADR_GOT_PAGE
ENUMDOC
  Get to the page base of the global offset table entry for a symbol as
  part of an ADRP instruction using a 21 bit PC relative value.Used in
  conjunction with BFD_RELOC_AARCH64_LD64_GOT_LO12_NC.
ENUM
  BFD_RELOC_AARCH64_LD64_GOT_LO12_NC
ENUMDOC
  Unsigned 12 bit byte offset for 64 bit load/store from the page of
  the GOT entry for this symbol.  Used in conjunction with
  BFD_RELOC_AARCH64_ADR_GOT_PAGE.  Valid in LP64 ABI only.
ENUM
  BFD_RELOC_AARCH64_LD32_GOT_LO12_NC
ENUMDOC
  Unsigned 12 bit byte offset for 32 bit load/store from the page of
  the GOT entry for this symbol.  Used in conjunction with
  BFD_RELOC_AARCH64_ADR_GOT_PAGE.  Valid in ILP32 ABI only.
 ENUM
  BFD_RELOC_AARCH64_MOVW_GOTOFF_G0_NC
ENUMDOC
  Unsigned 16 bit byte offset for 64 bit load/store from the GOT entry
  for this symbol.  Valid in LP64 ABI only.
ENUM
  BFD_RELOC_AARCH64_MOVW_GOTOFF_G1
ENUMDOC
  Unsigned 16 bit byte higher offset for 64 bit load/store from the GOT entry
  for this symbol.  Valid in LP64 ABI only.
ENUM
  BFD_RELOC_AARCH64_LD64_GOTOFF_LO15
ENUMDOC
  Unsigned 15 bit byte offset for 64 bit load/store from the page of
  the GOT entry for this symbol.  Valid in LP64 ABI only.
ENUM
  BFD_RELOC_AARCH64_LD32_GOTPAGE_LO14
ENUMDOC
  Scaled 14 bit byte offset to the page base of the global offset table.
ENUM
  BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15
ENUMDOC
  Scaled 15 bit byte offset to the page base of the global offset table.
ENUM
  BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21
ENUMDOC
  Get to the page base of the global offset table entry for a symbols
  tls_index structure as part of an adrp instruction using a 21 bit PC
  relative value.  Used in conjunction with
  BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC.
ENUM
  BFD_RELOC_AARCH64_TLSGD_ADR_PREL21
ENUMDOC
  AArch64 TLS General Dynamic
ENUM
  BFD_RELOC_AARCH64_TLSGD_ADD_LO12_NC
ENUMDOC
  Unsigned 12 bit byte offset to global offset table entry for a symbols
  tls_index structure.  Used in conjunction with
  BFD_RELOC_AARCH64_TLSGD_ADR_PAGE21.
ENUM
  BFD_RELOC_AARCH64_TLSGD_MOVW_G0_NC
ENUMDOC
  AArch64 TLS General Dynamic relocation.
ENUM
  BFD_RELOC_AARCH64_TLSGD_MOVW_G1
ENUMDOC
  AArch64 TLS General Dynamic relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_LD32_GOTTPREL_LO12_NC
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_PREL19
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSIE_MOVW_GOTTPREL_G1
ENUMDOC
  AArch64 TLS INITIAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_HI12
ENUMDOC
  bit[23:12] of byte offset to module TLS base address.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12
ENUMDOC
  Unsigned 12 bit byte offset to module TLS base address.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12_NC
ENUMDOC
  No overflow check version of BFD_RELOC_AARCH64_TLSLD_ADD_DTPREL_LO12.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADD_LO12_NC
ENUMDOC
  Unsigned 12 bit byte offset to global offset table entry for a symbols
  tls_index structure.  Used in conjunction with
  BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADR_PAGE21
ENUMDOC
  GOT entry page address for AArch64 TLS Local Dynamic, used with ADRP
  instruction.
ENUM
  BFD_RELOC_AARCH64_TLSLD_ADR_PREL21
ENUMDOC
  GOT entry address for AArch64 TLS Local Dynamic, used with ADR instruction.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12
ENUMDOC
  bit[11:1] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLD_LDST16_DTPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12
ENUMDOC
  bit[11:2] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLD_LDST32_DTPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12
ENUMDOC
  bit[11:3] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLD_LDST64_DTPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12
ENUMDOC
  bit[11:0] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLD_LDST8_DTPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0
ENUMDOC
  bit[15:0] of byte offset to module TLS base address.
ENUM
  BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0_NC
ENUMDOC
  No overflow check version of BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G0
ENUM
  BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1
ENUMDOC
  bit[31:16] of byte offset to module TLS base address.
ENUM
  BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1_NC
ENUMDOC
  No overflow check version of BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G1
ENUM
  BFD_RELOC_AARCH64_TLSLD_MOVW_DTPREL_G2
ENUMDOC
  bit[47:32] of byte offset to module TLS base address.
ENUM
  BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G2
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G1_NC
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_MOVW_TPREL_G0_NC
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_HI12
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_ADD_TPREL_LO12_NC
ENUMDOC
  AArch64 TLS LOCAL EXEC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12
ENUMDOC
  bit[11:1] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLE_LDST16_TPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12
ENUMDOC
  bit[11:2] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLE_LDST32_TPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12
ENUMDOC
  bit[11:3] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLE_LDST64_TPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12
ENUMDOC
  bit[11:0] of byte offset to module TLS base address, encoded in ldst
  instructions.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLE_LDST8_TPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_LD_PREL19
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_ADR_PREL21
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_ADR_PAGE21
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_LD64_LO12
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_LD32_LO12_NC
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_ADD_LO12
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_OFF_G1
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_OFF_G0_NC
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_LDR
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_ADD
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_CALL
ENUMDOC
  AArch64 TLS DESC relocation.
ENUM
  BFD_RELOC_AARCH64_COPY
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_GLOB_DAT
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_JUMP_SLOT
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_RELATIVE
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_TLS_DTPMOD
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_TLS_DTPREL
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_TLS_TPREL
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_TLSDESC
ENUMDOC
  AArch64 TLS relocation.
ENUM
  BFD_RELOC_AARCH64_IRELATIVE
ENUMDOC
  AArch64 support for STT_GNU_IFUNC.
ENUM
  BFD_RELOC_AARCH64_RELOC_END
ENUMDOC
  AArch64 pseudo relocation code to mark the end of the AArch64
  relocation enumerators that have direct mapping to ELF reloc codes.
  There are a few more enumerators after this one; those are mainly
  used by the AArch64 assembler for the internal fixup or to select
  one of the above enumerators.
ENUM
  BFD_RELOC_AARCH64_GAS_INTERNAL_FIXUP
ENUMDOC
  AArch64 pseudo relocation code to be used internally by the AArch64
  assembler and not (currently) written to any object files.
ENUM
  BFD_RELOC_AARCH64_LDST_LO12
ENUMDOC
  AArch64 unspecified load/store instruction, holding bits 0 to 11 of the
  address.  Used in conjunction with BFD_RELOC_AARCH64_ADR_HI21_PCREL.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12
ENUMDOC
  AArch64 pseudo relocation code for TLS local dynamic mode.  It's to be
  used internally by the AArch64 assembler and not (currently) written to
  any object files.
ENUM
  BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLD_LDST_DTPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12
ENUMDOC
  AArch64 pseudo relocation code for TLS local exec mode.  It's to be
  used internally by the AArch64 assembler and not (currently) written to
  any object files.
ENUM
  BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12_NC
ENUMDOC
  Similar as BFD_RELOC_AARCH64_TLSLE_LDST_TPREL_LO12, but no overflow check.
ENUM
  BFD_RELOC_AARCH64_LD_GOT_LO12_NC
ENUMDOC
  AArch64 pseudo relocation code to be used internally by the AArch64
  assembler and not (currently) written to any object files.
ENUM
  BFD_RELOC_AARCH64_TLSIE_LD_GOTTPREL_LO12_NC
ENUMDOC
  AArch64 pseudo relocation code to be used internally by the AArch64
  assembler and not (currently) written to any object files.
ENUM
  BFD_RELOC_AARCH64_TLSDESC_LD_LO12_NC
ENUMDOC
  AArch64 pseudo relocation code to be used internally by the AArch64
  assembler and not (currently) written to any object files.
ENUM
  BFD_RELOC_TILEPRO_COPY
ENUMX
  BFD_RELOC_TILEPRO_GLOB_DAT
ENUMX
  BFD_RELOC_TILEPRO_JMP_SLOT
ENUMX
  BFD_RELOC_TILEPRO_RELATIVE
ENUMX
  BFD_RELOC_TILEPRO_BROFF_X1
ENUMX
  BFD_RELOC_TILEPRO_JOFFLONG_X1
ENUMX
  BFD_RELOC_TILEPRO_JOFFLONG_X1_PLT
ENUMX
  BFD_RELOC_TILEPRO_IMM8_X0
ENUMX
  BFD_RELOC_TILEPRO_IMM8_Y0
ENUMX
  BFD_RELOC_TILEPRO_IMM8_X1
ENUMX
  BFD_RELOC_TILEPRO_IMM8_Y1
ENUMX
  BFD_RELOC_TILEPRO_DEST_IMM8_X1
ENUMX
  BFD_RELOC_TILEPRO_MT_IMM15_X1
ENUMX
  BFD_RELOC_TILEPRO_MF_IMM15_X1
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_LO_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_LO_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_HI_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_HI_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_HA_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_HA_PCREL
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_GOT
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_GOT
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_GOT_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_GOT_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_GOT_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_GOT_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_GOT_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_GOT_HA
ENUMX
  BFD_RELOC_TILEPRO_MMSTART_X0
ENUMX
  BFD_RELOC_TILEPRO_MMEND_X0
ENUMX
  BFD_RELOC_TILEPRO_MMSTART_X1
ENUMX
  BFD_RELOC_TILEPRO_MMEND_X1
ENUMX
  BFD_RELOC_TILEPRO_SHAMT_X0
ENUMX
  BFD_RELOC_TILEPRO_SHAMT_X1
ENUMX
  BFD_RELOC_TILEPRO_SHAMT_Y0
ENUMX
  BFD_RELOC_TILEPRO_SHAMT_Y1
ENUMX
  BFD_RELOC_TILEPRO_TLS_GD_CALL
ENUMX
  BFD_RELOC_TILEPRO_IMM8_X0_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEPRO_IMM8_X1_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEPRO_IMM8_Y0_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEPRO_IMM8_Y1_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEPRO_TLS_IE_LOAD
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_GD_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_GD_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_IE_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_IE_HA
ENUMX
  BFD_RELOC_TILEPRO_TLS_DTPMOD32
ENUMX
  BFD_RELOC_TILEPRO_TLS_DTPOFF32
ENUMX
  BFD_RELOC_TILEPRO_TLS_TPOFF32
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_LO
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HI
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X0_TLS_LE_HA
ENUMX
  BFD_RELOC_TILEPRO_IMM16_X1_TLS_LE_HA
ENUMDOC
  Tilera TILEPro Relocations.
ENUM
  BFD_RELOC_TILEGX_HW0
ENUMX
  BFD_RELOC_TILEGX_HW1
ENUMX
  BFD_RELOC_TILEGX_HW2
ENUMX
  BFD_RELOC_TILEGX_HW3
ENUMX
  BFD_RELOC_TILEGX_HW0_LAST
ENUMX
  BFD_RELOC_TILEGX_HW1_LAST
ENUMX
  BFD_RELOC_TILEGX_HW2_LAST
ENUMX
  BFD_RELOC_TILEGX_COPY
ENUMX
  BFD_RELOC_TILEGX_GLOB_DAT
ENUMX
  BFD_RELOC_TILEGX_JMP_SLOT
ENUMX
  BFD_RELOC_TILEGX_RELATIVE
ENUMX
  BFD_RELOC_TILEGX_BROFF_X1
ENUMX
  BFD_RELOC_TILEGX_JUMPOFF_X1
ENUMX
  BFD_RELOC_TILEGX_JUMPOFF_X1_PLT
ENUMX
  BFD_RELOC_TILEGX_IMM8_X0
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y0
ENUMX
  BFD_RELOC_TILEGX_IMM8_X1
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y1
ENUMX
  BFD_RELOC_TILEGX_DEST_IMM8_X1
ENUMX
  BFD_RELOC_TILEGX_MT_IMM14_X1
ENUMX
  BFD_RELOC_TILEGX_MF_IMM14_X1
ENUMX
  BFD_RELOC_TILEGX_MMSTART_X0
ENUMX
  BFD_RELOC_TILEGX_MMEND_X0
ENUMX
  BFD_RELOC_TILEGX_SHAMT_X0
ENUMX
  BFD_RELOC_TILEGX_SHAMT_X1
ENUMX
  BFD_RELOC_TILEGX_SHAMT_Y0
ENUMX
  BFD_RELOC_TILEGX_SHAMT_Y1
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW3
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW3
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW3_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW3_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_GOT
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW3_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW3_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_LE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_GD
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW2_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW2_LAST_PLT_PCREL
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW0_LAST_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW0_LAST_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X0_HW1_LAST_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_IMM16_X1_HW1_LAST_TLS_IE
ENUMX
  BFD_RELOC_TILEGX_TLS_DTPMOD64
ENUMX
  BFD_RELOC_TILEGX_TLS_DTPOFF64
ENUMX
  BFD_RELOC_TILEGX_TLS_TPOFF64
ENUMX
  BFD_RELOC_TILEGX_TLS_DTPMOD32
ENUMX
  BFD_RELOC_TILEGX_TLS_DTPOFF32
ENUMX
  BFD_RELOC_TILEGX_TLS_TPOFF32
ENUMX
  BFD_RELOC_TILEGX_TLS_GD_CALL
ENUMX
  BFD_RELOC_TILEGX_IMM8_X0_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_X1_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y0_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y1_TLS_GD_ADD
ENUMX
  BFD_RELOC_TILEGX_TLS_IE_LOAD
ENUMX
  BFD_RELOC_TILEGX_IMM8_X0_TLS_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_X1_TLS_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y0_TLS_ADD
ENUMX
  BFD_RELOC_TILEGX_IMM8_Y1_TLS_ADD
ENUMDOC
  Tilera TILE-Gx Relocations.

ENUM
  BFD_RELOC_BPF_64
ENUMX
  BFD_RELOC_BPF_DISP32
ENUMDOC
  Linux eBPF relocations.

ENUM
  BFD_RELOC_EPIPHANY_SIMM8
ENUMDOC
  Adapteva EPIPHANY - 8 bit signed pc-relative displacement
ENUM
  BFD_RELOC_EPIPHANY_SIMM24
ENUMDOC
  Adapteva EPIPHANY - 24 bit signed pc-relative displacement
ENUM
  BFD_RELOC_EPIPHANY_HIGH
ENUMDOC
  Adapteva EPIPHANY - 16 most-significant bits of absolute address
ENUM
  BFD_RELOC_EPIPHANY_LOW
ENUMDOC
  Adapteva EPIPHANY - 16 least-significant bits of absolute address
ENUM
  BFD_RELOC_EPIPHANY_SIMM11
ENUMDOC
  Adapteva EPIPHANY - 11 bit signed number - add/sub immediate
ENUM
  BFD_RELOC_EPIPHANY_IMM11
ENUMDOC
  Adapteva EPIPHANY - 11 bit sign-magnitude number (ld/st displacement)
ENUM
  BFD_RELOC_EPIPHANY_IMM8
ENUMDOC
  Adapteva EPIPHANY - 8 bit immediate for 16 bit mov instruction.

ENUM
  BFD_RELOC_VISIUM_HI16
ENUMX
  BFD_RELOC_VISIUM_LO16
ENUMX
  BFD_RELOC_VISIUM_IM16
ENUMX
  BFD_RELOC_VISIUM_REL16
ENUMX
  BFD_RELOC_VISIUM_HI16_PCREL
ENUMX
  BFD_RELOC_VISIUM_LO16_PCREL
ENUMX
  BFD_RELOC_VISIUM_IM16_PCREL
ENUMDOC
  Visium Relocations.

ENUM
  BFD_RELOC_WASM32_LEB128
ENUMX
  BFD_RELOC_WASM32_LEB128_GOT
ENUMX
  BFD_RELOC_WASM32_LEB128_GOT_CODE
ENUMX
  BFD_RELOC_WASM32_LEB128_PLT
ENUMX
  BFD_RELOC_WASM32_PLT_INDEX
ENUMX
  BFD_RELOC_WASM32_ABS32_CODE
ENUMX
  BFD_RELOC_WASM32_COPY
ENUMX
  BFD_RELOC_WASM32_CODE_POINTER
ENUMX
  BFD_RELOC_WASM32_INDEX
ENUMX
  BFD_RELOC_WASM32_PLT_SIG
ENUMDOC
  WebAssembly relocations.

ENUM
  BFD_RELOC_CKCORE_NONE
ENUMX
  BFD_RELOC_CKCORE_ADDR32
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM8BY4
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM11BY2
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM4BY2
ENUMX
  BFD_RELOC_CKCORE_PCREL32
ENUMX
  BFD_RELOC_CKCORE_PCREL_JSR_IMM11BY2
ENUMX
  BFD_RELOC_CKCORE_GNU_VTINHERIT
ENUMX
  BFD_RELOC_CKCORE_GNU_VTENTRY
ENUMX
  BFD_RELOC_CKCORE_RELATIVE
ENUMX
  BFD_RELOC_CKCORE_COPY
ENUMX
  BFD_RELOC_CKCORE_GLOB_DAT
ENUMX
  BFD_RELOC_CKCORE_JUMP_SLOT
ENUMX
  BFD_RELOC_CKCORE_GOTOFF
ENUMX
  BFD_RELOC_CKCORE_GOTPC
ENUMX
  BFD_RELOC_CKCORE_GOT32
ENUMX
  BFD_RELOC_CKCORE_PLT32
ENUMX
  BFD_RELOC_CKCORE_ADDRGOT
ENUMX
  BFD_RELOC_CKCORE_ADDRPLT
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM26BY2
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM16BY2
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM16BY4
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM10BY2
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM10BY4
ENUMX
  BFD_RELOC_CKCORE_ADDR_HI16
ENUMX
  BFD_RELOC_CKCORE_ADDR_LO16
ENUMX
  BFD_RELOC_CKCORE_GOTPC_HI16
ENUMX
  BFD_RELOC_CKCORE_GOTPC_LO16
ENUMX
  BFD_RELOC_CKCORE_GOTOFF_HI16
ENUMX
  BFD_RELOC_CKCORE_GOTOFF_LO16
ENUMX
  BFD_RELOC_CKCORE_GOT12
ENUMX
  BFD_RELOC_CKCORE_GOT_HI16
ENUMX
  BFD_RELOC_CKCORE_GOT_LO16
ENUMX
  BFD_RELOC_CKCORE_PLT12
ENUMX
  BFD_RELOC_CKCORE_PLT_HI16
ENUMX
  BFD_RELOC_CKCORE_PLT_LO16
ENUMX
  BFD_RELOC_CKCORE_ADDRGOT_HI16
ENUMX
  BFD_RELOC_CKCORE_ADDRGOT_LO16
ENUMX
  BFD_RELOC_CKCORE_ADDRPLT_HI16
ENUMX
  BFD_RELOC_CKCORE_ADDRPLT_LO16
ENUMX
  BFD_RELOC_CKCORE_PCREL_JSR_IMM26BY2
ENUMX
  BFD_RELOC_CKCORE_TOFFSET_LO16
ENUMX
  BFD_RELOC_CKCORE_DOFFSET_LO16
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM18BY2
ENUMX
  BFD_RELOC_CKCORE_DOFFSET_IMM18
ENUMX
  BFD_RELOC_CKCORE_DOFFSET_IMM18BY2
ENUMX
  BFD_RELOC_CKCORE_DOFFSET_IMM18BY4
ENUMX
  BFD_RELOC_CKCORE_GOTOFF_IMM18
ENUMX
  BFD_RELOC_CKCORE_GOT_IMM18BY4
ENUMX
  BFD_RELOC_CKCORE_PLT_IMM18BY4
ENUMX
  BFD_RELOC_CKCORE_PCREL_IMM7BY4
ENUMX
  BFD_RELOC_CKCORE_TLS_LE32
ENUMX
  BFD_RELOC_CKCORE_TLS_IE32
ENUMX
  BFD_RELOC_CKCORE_TLS_GD32
ENUMX
  BFD_RELOC_CKCORE_TLS_LDM32
ENUMX
  BFD_RELOC_CKCORE_TLS_LDO32
ENUMX
  BFD_RELOC_CKCORE_TLS_DTPMOD32
ENUMX
  BFD_RELOC_CKCORE_TLS_DTPOFF32
ENUMX
  BFD_RELOC_CKCORE_TLS_TPOFF32
ENUMX
  BFD_RELOC_CKCORE_PCREL_FLRW_IMM8BY4
ENUMX
  BFD_RELOC_CKCORE_NOJSRI
ENUMX
  BFD_RELOC_CKCORE_CALLGRAPH
ENUMX
  BFD_RELOC_CKCORE_IRELATIVE
ENUMX
  BFD_RELOC_CKCORE_PCREL_BLOOP_IMM4BY4
ENUMX
  BFD_RELOC_CKCORE_PCREL_BLOOP_IMM12BY4
ENUMDOC
  C-SKY relocations.

ENUM
  BFD_RELOC_S12Z_OPR
ENUMDOC
  S12Z relocations.

ENUM
  BFD_RELOC_LARCH_TLS_DTPMOD32
ENUMX
  BFD_RELOC_LARCH_TLS_DTPREL32
ENUMX
  BFD_RELOC_LARCH_TLS_DTPMOD64
ENUMX
  BFD_RELOC_LARCH_TLS_DTPREL64
ENUMX
  BFD_RELOC_LARCH_TLS_TPREL32
ENUMX
  BFD_RELOC_LARCH_TLS_TPREL64
ENUMX
  BFD_RELOC_LARCH_MARK_LA
ENUMX
  BFD_RELOC_LARCH_MARK_PCREL
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_PCREL
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_ABSOLUTE
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_DUP
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_GPREL
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_TLS_TPREL
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_TLS_GOT
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_TLS_GD
ENUMX
  BFD_RELOC_LARCH_SOP_PUSH_PLT_PCREL
ENUMX
  BFD_RELOC_LARCH_SOP_ASSERT
ENUMX
  BFD_RELOC_LARCH_SOP_NOT
ENUMX
  BFD_RELOC_LARCH_SOP_SUB
ENUMX
  BFD_RELOC_LARCH_SOP_SL
ENUMX
  BFD_RELOC_LARCH_SOP_SR
ENUMX
  BFD_RELOC_LARCH_SOP_ADD
ENUMX
  BFD_RELOC_LARCH_SOP_AND
ENUMX
  BFD_RELOC_LARCH_SOP_IF_ELSE
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_10_5
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_U_10_12
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_10_12
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_10_16
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_10_16_S2
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_5_20
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_0_5_10_16_S2
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_S_0_10_10_16_S2
ENUMX
  BFD_RELOC_LARCH_SOP_POP_32_U
ENUMX
  BFD_RELOC_LARCH_ADD8
ENUMX
  BFD_RELOC_LARCH_ADD16
ENUMX
  BFD_RELOC_LARCH_ADD24
ENUMX
  BFD_RELOC_LARCH_ADD32
ENUMX
  BFD_RELOC_LARCH_ADD64
ENUMX
  BFD_RELOC_LARCH_SUB8
ENUMX
  BFD_RELOC_LARCH_SUB16
ENUMX
  BFD_RELOC_LARCH_SUB24
ENUMX
  BFD_RELOC_LARCH_SUB32
ENUMX
  BFD_RELOC_LARCH_SUB64

ENUMX
  BFD_RELOC_LARCH_B16
ENUMX
  BFD_RELOC_LARCH_B21
ENUMX
  BFD_RELOC_LARCH_B26

ENUMX
  BFD_RELOC_LARCH_ABS_HI20
ENUMX
  BFD_RELOC_LARCH_ABS_LO12
ENUMX
  BFD_RELOC_LARCH_ABS64_LO20
ENUMX
  BFD_RELOC_LARCH_ABS64_HI12

ENUMX
  BFD_RELOC_LARCH_PCALA_HI20
ENUMX
  BFD_RELOC_LARCH_PCALA_LO12
ENUMX
  BFD_RELOC_LARCH_PCALA64_LO20
ENUMX
  BFD_RELOC_LARCH_PCALA64_HI12

ENUMX
  BFD_RELOC_LARCH_GOT_PC_HI20
ENUMX
  BFD_RELOC_LARCH_GOT_PC_LO12
ENUMX
  BFD_RELOC_LARCH_GOT64_PC_LO20
ENUMX
  BFD_RELOC_LARCH_GOT64_PC_HI12
ENUMX
  BFD_RELOC_LARCH_GOT_HI20
ENUMX
  BFD_RELOC_LARCH_GOT_LO12
ENUMX
  BFD_RELOC_LARCH_GOT64_LO20
ENUMX
  BFD_RELOC_LARCH_GOT64_HI12

ENUMX
  BFD_RELOC_LARCH_TLS_LE_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_LE_LO12
ENUMX
  BFD_RELOC_LARCH_TLS_LE64_LO20
ENUMX
  BFD_RELOC_LARCH_TLS_LE64_HI12
ENUMX
  BFD_RELOC_LARCH_TLS_IE_PC_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_IE_PC_LO12
ENUMX
  BFD_RELOC_LARCH_TLS_IE64_PC_LO20
ENUMX
  BFD_RELOC_LARCH_TLS_IE64_PC_HI12
ENUMX
  BFD_RELOC_LARCH_TLS_IE_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_IE_LO12
ENUMX
  BFD_RELOC_LARCH_TLS_IE64_LO20
ENUMX
  BFD_RELOC_LARCH_TLS_IE64_HI12
ENUMX
  BFD_RELOC_LARCH_TLS_LD_PC_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_LD_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_GD_PC_HI20
ENUMX
  BFD_RELOC_LARCH_TLS_GD_HI20

ENUMX
  BFD_RELOC_LARCH_32_PCREL

ENUMX
  BFD_RELOC_LARCH_RELAX

ENUMX
  BFD_RELOC_LARCH_DELETE

ENUMX
  BFD_RELOC_LARCH_ALIGN

ENUMX
  BFD_RELOC_LARCH_PCREL20_S2

ENUMX
  BFD_RELOC_LARCH_CFA

ENUMX
  BFD_RELOC_LARCH_ADD6
ENUMX
  BFD_RELOC_LARCH_SUB6

ENUMX
  BFD_RELOC_LARCH_ADD_ULEB128
ENUMX
  BFD_RELOC_LARCH_SUB_ULEB128

ENUMX
  BFD_RELOC_LARCH_64_PCREL

ENUMDOC
  LARCH relocations.

ENDSENUM
  BFD_RELOC_UNUSED

CODE_FRAGMENT
.typedef enum bfd_reloc_code_real bfd_reloc_code_real_type;
.
*/

/*
FUNCTION
	bfd_reloc_type_lookup
	bfd_reloc_name_lookup

SYNOPSIS
	reloc_howto_type *bfd_reloc_type_lookup
	  (bfd *abfd, bfd_reloc_code_real_type code);
	reloc_howto_type *bfd_reloc_name_lookup
	  (bfd *abfd, const char *reloc_name);

DESCRIPTION
	Return a pointer to a howto structure which, when
	invoked, will perform the relocation @var{code} on data from the
	architecture noted.
*/

reloc_howto_type *
bfd_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  return BFD_SEND (abfd, reloc_type_lookup, (abfd, code));
}

reloc_howto_type *
bfd_reloc_name_lookup (bfd *abfd, const char *reloc_name)
{
  return BFD_SEND (abfd, reloc_name_lookup, (abfd, reloc_name));
}

static reloc_howto_type bfd_howto_32 =
HOWTO (0, 00, 4, 32, false, 0, complain_overflow_dont, 0, "VRT32", false, 0xffffffff, 0xffffffff, true);

/*
INTERNAL_FUNCTION
	bfd_default_reloc_type_lookup

SYNOPSIS
	reloc_howto_type *bfd_default_reloc_type_lookup
	  (bfd *abfd, bfd_reloc_code_real_type  code);

DESCRIPTION
	Provides a default relocation lookup routine for any architecture.
*/

reloc_howto_type *
bfd_default_reloc_type_lookup (bfd *abfd, bfd_reloc_code_real_type code)
{
  /* Very limited support is provided for relocs in generic targets
     such as elf32-little.  FIXME: Should we always return NULL?  */
  if (code == BFD_RELOC_CTOR
      && bfd_arch_bits_per_address (abfd) == 32)
    return &bfd_howto_32;
  return NULL;
}

/*
FUNCTION
	bfd_get_reloc_code_name

SYNOPSIS
	const char *bfd_get_reloc_code_name (bfd_reloc_code_real_type code);

DESCRIPTION
	Provides a printable name for the supplied relocation code.
	Useful mainly for printing error messages.
*/

const char *
bfd_get_reloc_code_name (bfd_reloc_code_real_type code)
{
  if (code > BFD_RELOC_UNUSED)
    return 0;
  return bfd_reloc_code_real_names[code];
}

/*
INTERNAL_FUNCTION
	bfd_generic_relax_section

SYNOPSIS
	bool bfd_generic_relax_section
	  (bfd *abfd,
	   asection *section,
	   struct bfd_link_info *,
	   bool *);

DESCRIPTION
	Provides default handling for relaxing for back ends which
	don't do relaxing.
*/

bool
bfd_generic_relax_section (bfd *abfd ATTRIBUTE_UNUSED,
			   asection *section ATTRIBUTE_UNUSED,
			   struct bfd_link_info *link_info ATTRIBUTE_UNUSED,
			   bool *again)
{
  if (bfd_link_relocatable (link_info))
    (*link_info->callbacks->einfo)
      (_("%P%F: --relax and -r may not be used together\n"));

  *again = false;
  return true;
}

/*
INTERNAL_FUNCTION
	bfd_generic_gc_sections

SYNOPSIS
	bool bfd_generic_gc_sections
	  (bfd *, struct bfd_link_info *);

DESCRIPTION
	Provides default handling for relaxing for back ends which
	don't do section gc -- i.e., does nothing.
*/

bool
bfd_generic_gc_sections (bfd *abfd ATTRIBUTE_UNUSED,
			 struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  return true;
}

/*
INTERNAL_FUNCTION
	bfd_generic_lookup_section_flags

SYNOPSIS
	bool bfd_generic_lookup_section_flags
	  (struct bfd_link_info *, struct flag_info *, asection *);

DESCRIPTION
	Provides default handling for section flags lookup
	-- i.e., does nothing.
	Returns FALSE if the section should be omitted, otherwise TRUE.
*/

bool
bfd_generic_lookup_section_flags (struct bfd_link_info *info ATTRIBUTE_UNUSED,
				  struct flag_info *flaginfo,
				  asection *section ATTRIBUTE_UNUSED)
{
  if (flaginfo != NULL)
    {
      _bfd_error_handler (_("INPUT_SECTION_FLAGS are not supported"));
      return false;
    }
  return true;
}

/*
INTERNAL_FUNCTION
	bfd_generic_merge_sections

SYNOPSIS
	bool bfd_generic_merge_sections
	  (bfd *, struct bfd_link_info *);

DESCRIPTION
	Provides default handling for SEC_MERGE section merging for back ends
	which don't have SEC_MERGE support -- i.e., does nothing.
*/

bool
bfd_generic_merge_sections (bfd *abfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *link_info ATTRIBUTE_UNUSED)
{
  return true;
}

/*
INTERNAL_FUNCTION
	bfd_generic_get_relocated_section_contents

SYNOPSIS
	bfd_byte *bfd_generic_get_relocated_section_contents
	  (bfd *abfd,
	   struct bfd_link_info *link_info,
	   struct bfd_link_order *link_order,
	   bfd_byte *data,
	   bool relocatable,
	   asymbol **symbols);

DESCRIPTION
	Provides default handling of relocation effort for back ends
	which can't be bothered to do it efficiently.
*/

bfd_byte *
bfd_generic_get_relocated_section_contents (bfd *abfd,
					    struct bfd_link_info *link_info,
					    struct bfd_link_order *link_order,
					    bfd_byte *data,
					    bool relocatable,
					    asymbol **symbols)
{
  bfd *input_bfd = link_order->u.indirect.section->owner;
  asection *input_section = link_order->u.indirect.section;
  long reloc_size;
  arelent **reloc_vector;
  long reloc_count;

  reloc_size = bfd_get_reloc_upper_bound (input_bfd, input_section);
  if (reloc_size < 0)
    return NULL;

  /* Read in the section.  */
  bfd_byte *orig_data = data;
  if (!bfd_get_full_section_contents (input_bfd, input_section, &data))
    return NULL;

  if (data == NULL)
    return NULL;

  if (reloc_size == 0)
    return data;

  reloc_vector = (arelent **) bfd_malloc (reloc_size);
  if (reloc_vector == NULL)
    goto error_return;

  reloc_count = bfd_canonicalize_reloc (input_bfd,
					input_section,
					reloc_vector,
					symbols);
  if (reloc_count < 0)
    goto error_return;

  if (reloc_count > 0)
    {
      arelent **parent;

      for (parent = reloc_vector; *parent != NULL; parent++)
	{
	  char *error_message = NULL;
	  asymbol *symbol;
	  bfd_reloc_status_type r;

	  symbol = *(*parent)->sym_ptr_ptr;
	  /* PR ld/19628: A specially crafted input file
	     can result in a NULL symbol pointer here.  */
	  if (symbol == NULL)
	    {
	      link_info->callbacks->einfo
		/* xgettext:c-format */
		(_("%X%P: %pB(%pA): error: relocation for offset %V has no value\n"),
		 abfd, input_section, (* parent)->address);
	      goto error_return;
	    }

	  /* Zap reloc field when the symbol is from a discarded
	     section, ignoring any addend.  Do the same when called
	     from bfd_simple_get_relocated_section_contents for
	     undefined symbols in debug sections.  This is to keep
	     debug info reasonably sane, in particular so that
	     DW_FORM_ref_addr to another file's .debug_info isn't
	     confused with an offset into the current file's
	     .debug_info.  */
	  if ((symbol->section != NULL && discarded_section (symbol->section))
	      || (symbol->section == bfd_und_section_ptr
		  && (input_section->flags & SEC_DEBUGGING) != 0
		  && link_info->input_bfds == link_info->output_bfd))
	    {
	      bfd_vma off;
	      static reloc_howto_type none_howto
		= HOWTO (0, 0, 0, 0, false, 0, complain_overflow_dont, NULL,
			 "unused", false, 0, 0, false);

	      off = ((*parent)->address
		     * bfd_octets_per_byte (input_bfd, input_section));
	      _bfd_clear_contents ((*parent)->howto, input_bfd,
				   input_section, data, off);
	      (*parent)->sym_ptr_ptr = bfd_abs_section_ptr->symbol_ptr_ptr;
	      (*parent)->addend = 0;
	      (*parent)->howto = &none_howto;
	      r = bfd_reloc_ok;
	    }
	  else
	    r = bfd_perform_relocation (input_bfd,
					*parent,
					data,
					input_section,
					relocatable ? abfd : NULL,
					&error_message);

	  if (relocatable)
	    {
	      asection *os = input_section->output_section;

	      /* A partial link, so keep the relocs.  */
	      os->orelocation[os->reloc_count] = *parent;
	      os->reloc_count++;
	    }

	  if (r != bfd_reloc_ok)
	    {
	      switch (r)
		{
		case bfd_reloc_undefined:
		  (*link_info->callbacks->undefined_symbol)
		    (link_info, bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     input_bfd, input_section, (*parent)->address, true);
		  break;
		case bfd_reloc_dangerous:
		  BFD_ASSERT (error_message != NULL);
		  (*link_info->callbacks->reloc_dangerous)
		    (link_info, error_message,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_overflow:
		  (*link_info->callbacks->reloc_overflow)
		    (link_info, NULL,
		     bfd_asymbol_name (*(*parent)->sym_ptr_ptr),
		     (*parent)->howto->name, (*parent)->addend,
		     input_bfd, input_section, (*parent)->address);
		  break;
		case bfd_reloc_outofrange:
		  /* PR ld/13730:
		     This error can result when processing some partially
		     complete binaries.  Do not abort, but issue an error
		     message instead.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" goes out of range\n"),
		     abfd, input_section, * parent);
		  goto error_return;

		case bfd_reloc_notsupported:
		  /* PR ld/17512
		     This error can result when processing a corrupt binary.
		     Do not abort.  Issue an error message instead.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" is not supported\n"),
		     abfd, input_section, * parent);
		  goto error_return;

		default:
		  /* PR 17512; file: 90c2a92e.
		     Report unexpected results, without aborting.  */
		  link_info->callbacks->einfo
		    /* xgettext:c-format */
		    (_("%X%P: %pB(%pA): relocation \"%pR\" returns an unrecognized value %x\n"),
		     abfd, input_section, * parent, r);
		  break;
		}

	    }
	}
    }

  free (reloc_vector);
  return data;

 error_return:
  free (reloc_vector);
  if (orig_data == NULL)
    free (data);
  return NULL;
}

/*
INTERNAL_FUNCTION
	_bfd_generic_set_reloc

SYNOPSIS
	void _bfd_generic_set_reloc
	  (bfd *abfd,
	   sec_ptr section,
	   arelent **relptr,
	   unsigned int count);

DESCRIPTION
	Installs a new set of internal relocations in SECTION.
*/

void
_bfd_generic_set_reloc (bfd *abfd ATTRIBUTE_UNUSED,
			sec_ptr section,
			arelent **relptr,
			unsigned int count)
{
  section->orelocation = relptr;
  section->reloc_count = count;
  if (count != 0)
    section->flags |= SEC_RELOC;
  else
    section->flags &= ~SEC_RELOC;
}

/*
INTERNAL_FUNCTION
	_bfd_unrecognized_reloc

SYNOPSIS
	bool _bfd_unrecognized_reloc
	  (bfd * abfd,
	   sec_ptr section,
	   unsigned int r_type);

DESCRIPTION
	Reports an unrecognized reloc.
	Written as a function in order to reduce code duplication.
	Returns FALSE so that it can be called from a return statement.
*/

bool
_bfd_unrecognized_reloc (bfd * abfd, sec_ptr section, unsigned int r_type)
{
   /* xgettext:c-format */
  _bfd_error_handler (_("%pB: unrecognized relocation type %#x in section `%pA'"),
		      abfd, r_type, section);

  /* PR 21803: Suggest the most likely cause of this error.  */
  _bfd_error_handler (_("is this version of the linker - %s - out of date ?"),
		      BFD_VERSION_STRING);

  bfd_set_error (bfd_error_bad_value);
  return false;
}

reloc_howto_type *
_bfd_norelocs_bfd_reloc_type_lookup
    (bfd *abfd,
     bfd_reloc_code_real_type code ATTRIBUTE_UNUSED)
{
  return (reloc_howto_type *) _bfd_ptr_bfd_null_error (abfd);
}

reloc_howto_type *
_bfd_norelocs_bfd_reloc_name_lookup (bfd *abfd,
				     const char *reloc_name ATTRIBUTE_UNUSED)
{
  return (reloc_howto_type *) _bfd_ptr_bfd_null_error (abfd);
}

long
_bfd_nodynamic_canonicalize_dynamic_reloc (bfd *abfd,
					   arelent **relp ATTRIBUTE_UNUSED,
					   asymbol **symp ATTRIBUTE_UNUSED)
{
  return _bfd_long_bfd_n1_error (abfd);
}
