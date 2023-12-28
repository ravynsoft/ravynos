/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "config.h"
#include <ctype.h>

#include "util.h"
#include "Dwarf.h"
#include "DwarfLib.h"
#include "Elf.h"
#include "Function.h"
#include "Module.h"
#include "StringBuilder.h"
#include "DbeArray.h"
#include "DbeSession.h"

#define NO_STMT_LIST ((uint64_t) -1)
#define CASE_S(x)   case x: s = (char *) #x; break

static char *
gelf_st_type2str (int type)
{
  static char buf[128];
  char *s;
  switch (type)
    {
      CASE_S (STT_NOTYPE);
      CASE_S (STT_OBJECT);
      CASE_S (STT_FUNC);
      CASE_S (STT_SECTION);
      CASE_S (STT_FILE);
      CASE_S (STT_COMMON);
      CASE_S (STT_TLS);
      //    CASE_S(STT_NUM);
      CASE_S (STT_LOPROC);
      CASE_S (STT_HIPROC);
    default: s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, type);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

static char *
special_opcode2str (int opcode)
{
  static char buf[128];
  snprintf (buf, sizeof (buf), NTXT ("SpecialOpcode: %3d"), opcode);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

static char *
extended_opcode2str (int opcode)
{
  static char buf[128];
  char *s;
  switch (opcode)
    {
      CASE_S (DW_LNE_end_sequence);
      CASE_S (DW_LNE_set_address);
      CASE_S (DW_LNE_define_file);
    default:
      snprintf (buf, sizeof (buf), NTXT ("??? (%d)"), opcode);
      buf[sizeof (buf) - 1] = 0;
      s = buf;
      break;
    }
  return s;
}

static char *
standard_opcode2str (int opcode)
{
  static char buf[128];
  char *s;
  switch (opcode)
    {
      CASE_S (DW_LNS_copy);
      CASE_S (DW_LNS_advance_pc);
      CASE_S (DW_LNS_advance_line);
      CASE_S (DW_LNS_set_file);
      CASE_S (DW_LNS_set_column);
      CASE_S (DW_LNS_negate_stmt);
      CASE_S (DW_LNS_set_basic_block);
      CASE_S (DW_LNS_const_add_pc);
      CASE_S (DW_LNS_fixed_advance_pc);
    default:
      snprintf (buf, sizeof (buf), NTXT ("??? (%d)"), opcode);
      buf[sizeof (buf) - 1] = 0;
      s = buf;
      break;
    }
  return s;
}

template<> void Vector<DwrInlinedSubr *>
::dump (const char *msg)
{
  Dprintf (1, NTXT ("%s Vector<DwrInlinedSubr *> [%lld]\n"),
	   msg ? msg : NTXT (""), (long long) size ());
  for (long i = 0, sz = size (); i < sz; i++)
    {
      DwrInlinedSubr *p = get (i);
      Dprintf (1, NTXT ("%ld: "), (long) i);
      p->dump ();
    }
}

template<> void Vector<DwrLine *>
::dump (const char *msg)
{
  Dprintf (1, "%s Vector<DwrLine *> [%lld]:\n    address [file line column]\n",
	   msg ? msg : NTXT (""), (long long) size ());
  for (long i = 0, sz = size (); i < sz; i++)
    {
      DwrLine *lnp = get (i);
      Dprintf (1, NTXT (" %2lld 0x%08llx  [ %2lld, %lld, %lld ] \n"),
	       (long long) i, (long long) lnp->address, (long long) lnp->file,
	       (long long) lnp->line, (long long) lnp->column);
    }
  Dprintf (1, NTXT ("\n\n"));
}

template<> void Vector<DwrFileName *>
::dump (const char *msg)
{
  Dprintf (1, "\n%s Vector<DwrFileName *> [%lld]:  [dir_ind tstamp fsize]\n",
	   msg ? msg : NTXT (""), (long long) size ());
  for (long i = 0, sz = size (); i < sz; i++)
    {
      DwrFileName *fnp = get (i);
      Dprintf (1, " %2ld %3lld %8lld %8lld %s\n", i, (long long) fnp->dir_index,
	       (long long) fnp->timestamp, (long long) fnp->file_size,
	       STR (fnp->fname));
    }
  Dprintf (1, "\n");
}

static char *
get_string (DwrSec *sec, uint64_t off)
{
  if (sec)
    {
      sec->offset = off;
      return sec->GetString ();
    }
  return NULL;
}

  
//////////////////////////////////////////////////////////
//  class ElfReloc

ElfReloc::ElfReloc (Elf *_elf)
{
  elf = _elf;
  reloc = NULL;
  cur_reloc_ind = 0;
}

ElfReloc::~ElfReloc ()
{
  if (reloc)
    {
      reloc->destroy ();
      delete reloc;
    }
}

void
ElfReloc::dump_rela_debug_sec (int sec)
{
  if (!DUMP_RELA_SEC)
    return;
  Elf_Internal_Shdr *shdr = elf->get_shdr (sec);
  if (shdr == NULL)
    return;

  Elf_Data *data = elf->elf_getdata (sec);
  if (data == NULL)
    return;

  uint64_t ScnSize = data->d_size;
  uint64_t EntSize = shdr->sh_entsize;
  if (ScnSize == 0 || EntSize == 0)
    return;

  Elf_Internal_Shdr *shdr_sym = elf->get_shdr (shdr->sh_link);
  if (shdr_sym == NULL)
    return;
  Elf_Data *data_sym = elf->elf_getdata (shdr->sh_link);
  Elf_Data *data_str = elf->elf_getdata (shdr_sym->sh_link);
  char *Strtab = data_str ? (char*) data_str->d_buf : NULL;
  Elf_Internal_Rela rela;
  int n, cnt = (int) (ScnSize / EntSize);

  char *sec_name = elf->get_sec_name (sec);
  if (sec_name == NULL) // It can not be, but let's check
    return;
  Dprintf (DUMP_RELA_SEC,
	   "======= DwarfLib::dump_rela_debug_sec  Section:%2d  '%s'\n",
	   sec, sec_name);
  Dprintf (DUMP_RELA_SEC,
	   " N |addend|   offset   |       r_info      |    stt_type   |\n");
  for (n = 0; n < cnt; n++)
    {
      if (strncmp (sec_name, NTXT (".rela."), 6) == 0)
	elf->elf_getrela (data, n, &rela);
      else
	{
	  elf->elf_getrel (data, n, &rela);
	  rela.r_addend = 0;
	}
      int ndx = (int) GELF_R_SYM (rela.r_info);
      Elf_Internal_Shdr *secHdr;
      Elf_Internal_Sym sym;
      elf->elf_getsym (data_sym, ndx, &sym);
      Dprintf (DUMP_RELA_SEC, NTXT ("%3d:%5d |%11lld |0x%016llx | %-15s|"),
	       n, (int) rela.r_addend,
	       (long long) rela.r_offset, (long long) rela.r_info,
	       gelf_st_type2str ((int) GELF_ST_TYPE (sym.st_info)));
      switch (GELF_ST_TYPE (sym.st_info))
	{
	case STT_FUNC:
	case STT_OBJECT:
	case STT_NOTYPE:
	  secHdr = elf->get_shdr (sym.st_shndx);
	  if (secHdr)
	    Dprintf (DUMP_RELA_SEC, NTXT (" img_offset=0x%llx"),
		     (long long) (sym.st_value + secHdr->sh_offset));
	  if (Strtab && sym.st_name)
	    Dprintf (DUMP_RELA_SEC, NTXT ("  %s"), Strtab + sym.st_name);
	  break;
	case STT_SECTION:
	  secHdr = elf->get_shdr (sym.st_shndx);
	  if (secHdr)
	    {
	      Dprintf (DUMP_RELA_SEC, NTXT ("       value=0x%016llx (%lld)"),
		       (long long) (secHdr->sh_offset + rela.r_addend),
		       (long long) (secHdr->sh_offset + rela.r_addend));
	    }
	  break;
	default:
	  break;
	}
      Dprintf (DUMP_RELA_SEC, NTXT ("\n"));
    }
  Dprintf (DUMP_RELA_SEC, NTXT ("\n"));
}

void
ElfReloc::dump ()
{
  if (!DUMP_ELF_RELOC || (reloc == NULL) || (reloc->size () == 0))
    return;
  Dprintf (DUMP_ELF_RELOC, NTXT ("======= ElfReloc::dump\n"));
  Dprintf (DUMP_ELF_RELOC, NTXT (" N |   offset   |    value   | STT_TYPE\n"));
  for (int i = 0; i < reloc->size (); i++)
    {
      Sreloc *srlc = reloc->fetch (i);
      Dprintf (DUMP_ELF_RELOC, NTXT ("%3d:%11lld |%11lld | %s\n"),
	       i, (long long) srlc->offset, (long long) srlc->value,
	       gelf_st_type2str (srlc->stt_type));
    }
  Dprintf (DUMP_ELF_RELOC, NTXT ("\n"));
}

static int
DwrRelocOffsetCmp (const void *a, const void *b)
{
  ElfReloc::Sreloc *item1 = *((ElfReloc::Sreloc **) a);
  ElfReloc::Sreloc *item2 = *((ElfReloc::Sreloc **) b);
  return item1->offset < item2->offset ? -1 :
	 item1->offset == item2->offset ? 0 : 1;
}

ElfReloc *
ElfReloc::get_elf_reloc (Elf *elfp, char *sec_name, ElfReloc *rlc)
{
  int et = elfp->elf_getehdr ()->e_type;
  if (et == ET_EXEC || et == ET_DYN)
    return rlc;
  int sec = elfp->elf_get_sec_num (sec_name);
  if (sec == 0)
    return rlc;
  Elf_Internal_Shdr *shdr = elfp->get_shdr (sec);
  if (shdr == NULL || shdr->sh_entsize == 0)
    return rlc;

  Elf_Data *data = elfp->elf_getdata (sec);
  if (data == NULL || data->d_size == 0)
    return rlc;

  int cnt = (int) (data->d_size / shdr->sh_entsize);
  Elf_Internal_Shdr *shdr_sym = elfp->get_shdr (shdr->sh_link);
  if (shdr_sym == NULL)
    return rlc;
  Elf_Data *data_sym = elfp->elf_getdata (shdr->sh_link);
  Vector<Sreloc *> *vp = NULL;

  for (int n = 0; n < cnt; n++)
    {
      Elf_Internal_Shdr *secHdr;
      Sreloc *srlc;
      Elf_Internal_Rela rela;
      if (strncmp (sec_name, NTXT (".rela."), 6) == 0)
	elfp->elf_getrela (data, n, &rela);
      else
	{
	  elfp->elf_getrel (data, n, &rela);
	  rela.r_addend = 0;
	}
      int ndx = (int) GELF_R_SYM (rela.r_info);
      Elf_Internal_Sym sym;
      elfp->elf_getsym (data_sym, ndx, &sym);

      srlc = new Sreloc;
      srlc->offset = rela.r_offset;
      srlc->value = 0;
      srlc->stt_type = (int) GELF_ST_TYPE (sym.st_info);
      switch (GELF_ST_TYPE (sym.st_info))
	{
	case STT_FUNC:
	  secHdr = elfp->get_shdr (sym.st_shndx);
	  if (secHdr)
	    srlc->value = secHdr->sh_offset + sym.st_value;
	  break;
	case STT_OBJECT:
	case STT_NOTYPE:
	  secHdr = elfp->get_shdr (shdr->sh_info);
	  if (secHdr)
	    {
	      srlc->offset = rela.r_info;
	      srlc->value = secHdr->sh_offset + rela.r_addend;
	    }
	  break;
	case STT_SECTION:
	  secHdr = elfp->get_shdr (sym.st_shndx);
	  if (secHdr)
	    srlc->value = rela.r_addend;
	  break;
	default:
	  srlc->value = 0;
	  break;
	}
      if (rlc == NULL)
	{
	  rlc = new ElfReloc (elfp);
	  vp = rlc->reloc;
	}
      if (vp == NULL)
	{
	  vp = new Vector<Sreloc*>;
	  rlc->reloc = vp;
	}
      vp->append (srlc);
    }
  if (vp)
    vp->sort (DwrRelocOffsetCmp);
  if (rlc)
    {
      rlc->dump_rela_debug_sec (sec);
      rlc->dump ();
    }
  return rlc;
}

long long
ElfReloc::get_reloc_addr (long long offset)
{
  Sreloc *srlc;
  int i = cur_reloc_ind - 1;
  if (i >= 0 && i < reloc->size ())
    {
      srlc = reloc->fetch (i);
      if (srlc->offset > offset)  // need to reset
	cur_reloc_ind = 0;
    }
  for (; cur_reloc_ind < reloc->size (); cur_reloc_ind++)
    {
      srlc = reloc->fetch (cur_reloc_ind);
      if (srlc->offset == offset)
	return srlc->value;
      if (srlc->offset > offset)
	return 0;
    }
  return 0;
}

DwrLocation *
DwrCU::dwr_get_location (DwrSec *secp, DwrLocation *lp)
{
  lp->offset = secp->offset;
  lp->lc_number = 0;
  lp->lc_number2 = 0;
  lp->op = secp->Get_8 ();
  switch (lp->op)
    {
      // registers
    case DW_OP_reg0:
    case DW_OP_reg1:
    case DW_OP_reg2:
    case DW_OP_reg3:
    case DW_OP_reg4:
    case DW_OP_reg5:
    case DW_OP_reg6:
    case DW_OP_reg7:
    case DW_OP_reg8:
    case DW_OP_reg9:
    case DW_OP_reg10:
    case DW_OP_reg11:
    case DW_OP_reg12:
    case DW_OP_reg13:
    case DW_OP_reg14:
    case DW_OP_reg15:
    case DW_OP_reg16:
    case DW_OP_reg17:
    case DW_OP_reg18:
    case DW_OP_reg19:
    case DW_OP_reg20:
    case DW_OP_reg21:
    case DW_OP_reg22:
    case DW_OP_reg23:
    case DW_OP_reg24:
    case DW_OP_reg25:
    case DW_OP_reg26:
    case DW_OP_reg27:
    case DW_OP_reg28:
    case DW_OP_reg29:
    case DW_OP_reg30:
    case DW_OP_reg31:
      break;
    case DW_OP_regx:
      lp->lc_number = secp->GetULEB128 ();
      break;
    case DW_OP_breg0:
    case DW_OP_breg1:
    case DW_OP_breg2:
    case DW_OP_breg3:
    case DW_OP_breg4:
    case DW_OP_breg5:
    case DW_OP_breg6:
    case DW_OP_breg7:
    case DW_OP_breg8:
    case DW_OP_breg9:
    case DW_OP_breg10:
    case DW_OP_breg11:
    case DW_OP_breg12:
    case DW_OP_breg13:
    case DW_OP_breg14:
    case DW_OP_breg15:
    case DW_OP_breg16:
    case DW_OP_breg17:
    case DW_OP_breg18:
    case DW_OP_breg19:
    case DW_OP_breg20:
    case DW_OP_breg21:
    case DW_OP_breg22:
    case DW_OP_breg23:
    case DW_OP_breg24:
    case DW_OP_breg25:
    case DW_OP_breg26:
    case DW_OP_breg27:
    case DW_OP_breg28:
    case DW_OP_breg29:
    case DW_OP_breg30:
    case DW_OP_breg31:
      lp->lc_number = secp->GetSLEB128 ();
      break;
    case DW_OP_fbreg:
      lp->lc_number = secp->GetSLEB128 ();
      break;
    case DW_OP_bregx:
      lp->lc_number = secp->GetULEB128 ();
      lp->lc_number2 = secp->GetSLEB128 ();
      break;
    case DW_OP_lit0:
    case DW_OP_lit1:
    case DW_OP_lit2:
    case DW_OP_lit3:
    case DW_OP_lit4:
    case DW_OP_lit5:
    case DW_OP_lit6:
    case DW_OP_lit7:
    case DW_OP_lit8:
    case DW_OP_lit9:
    case DW_OP_lit10:
    case DW_OP_lit11:
    case DW_OP_lit12:
    case DW_OP_lit13:
    case DW_OP_lit14:
    case DW_OP_lit15:
    case DW_OP_lit16:
    case DW_OP_lit17:
    case DW_OP_lit18:
    case DW_OP_lit19:
    case DW_OP_lit20:
    case DW_OP_lit21:
    case DW_OP_lit22:
    case DW_OP_lit23:
    case DW_OP_lit24:
    case DW_OP_lit25:
    case DW_OP_lit26:
    case DW_OP_lit27:
    case DW_OP_lit28:
    case DW_OP_lit29:
    case DW_OP_lit30:
    case DW_OP_lit31:
      lp->lc_number = lp->op - DW_OP_lit0;
      break;
    case DW_OP_addr:
      lp->lc_number = secp->GetADDR ();
      break;
    case DW_OP_const1u:
      lp->lc_number = secp->Get_8 ();
      break;
    case DW_OP_const1s:
      {
	signed char x;
	x = secp->Get_8 ();
	lp->lc_number = x;
      }
      break;
    case DW_OP_const2u:
      lp->lc_number = secp->Get_16 ();
      break;
    case DW_OP_const2s:
      {
	signed short x;
	x = secp->Get_16 ();
	lp->lc_number = x;
      }
      break;
    case DW_OP_const4u:
      lp->lc_number = secp->Get_32 ();
      break;
    case DW_OP_const4s:
      {
	signed int x;
	x = secp->Get_32 ();
	lp->lc_number = x;
      }
      break;
    case DW_OP_const8u:
      lp->lc_number = secp->Get_64 ();
      break;
    case DW_OP_const8s:
      {
	signed long long x;
	x = secp->Get_64 ();
	lp->lc_number = x;
      }
      break;
    case DW_OP_plus_uconst:
    case DW_OP_constu:
      lp->lc_number = secp->GetULEB128 ();
      break;
    case DW_OP_consts:
      lp->lc_number = secp->GetSLEB128 ();
      break;

      // Stack operations
    case DW_OP_pick:
    case DW_OP_deref_size:
    case DW_OP_xderef_size:
      lp->lc_number = secp->Get_8 ();
      break;
    case DW_OP_dup:
    case DW_OP_drop:
    case DW_OP_over:
    case DW_OP_swap:
    case DW_OP_rot:
    case DW_OP_deref:
    case DW_OP_xderef:
      // Arithmetic and Logical Operations
    case DW_OP_abs:
    case DW_OP_and:
    case DW_OP_div:
    case DW_OP_minus:
    case DW_OP_mod:
    case DW_OP_mul:
    case DW_OP_neg:
    case DW_OP_not:
    case DW_OP_or:
    case DW_OP_plus:
    case DW_OP_shl:
    case DW_OP_shr:
    case DW_OP_shra:
    case DW_OP_xor:
    case DW_OP_le:
    case DW_OP_ge:
    case DW_OP_eq:
    case DW_OP_lt:
    case DW_OP_gt:
    case DW_OP_ne:
    case DW_OP_nop:
      break;
    case DW_OP_skip:
    case DW_OP_bra:
      lp->lc_number = secp->Get_16 ();
      break;
    case DW_OP_piece:
      lp->lc_number = secp->GetULEB128 ();
      break;
    case DW_OP_push_object_address: /* DWARF3 */
      break;
    case DW_OP_call2: /* DWARF3 */
      lp->lc_number = secp->Get_16 ();
      break;
    case DW_OP_call4: /* DWARF3 */
      lp->lc_number = secp->Get_32 ();
      break;
    case DW_OP_call_ref: /* DWARF3 */
      lp->lc_number = secp->GetADDR ();
      break;
    default:
      return (NULL);
    }
  return lp;
}

char *
DwrCU::tag2str (int tag)
{
  static char buf[128];
  char *s;

  switch (tag)
    {
      CASE_S (DW_TAG_array_type);
      CASE_S (DW_TAG_class_type);
      CASE_S (DW_TAG_entry_point);
      CASE_S (DW_TAG_enumeration_type);
      CASE_S (DW_TAG_formal_parameter);
      CASE_S (DW_TAG_imported_declaration);
      CASE_S (DW_TAG_label);
      CASE_S (DW_TAG_lexical_block);
      CASE_S (DW_TAG_member);
      CASE_S (DW_TAG_pointer_type);
      CASE_S (DW_TAG_reference_type);
      CASE_S (DW_TAG_compile_unit);
      CASE_S (DW_TAG_string_type);
      CASE_S (DW_TAG_structure_type);
      CASE_S (DW_TAG_subroutine_type);
      CASE_S (DW_TAG_typedef);
      CASE_S (DW_TAG_union_type);
      CASE_S (DW_TAG_unspecified_parameters);
      CASE_S (DW_TAG_variant);
      CASE_S (DW_TAG_common_block);
      CASE_S (DW_TAG_common_inclusion);
      CASE_S (DW_TAG_inheritance);
      CASE_S (DW_TAG_inlined_subroutine);
      CASE_S (DW_TAG_module);
      CASE_S (DW_TAG_ptr_to_member_type);
      CASE_S (DW_TAG_set_type);
      CASE_S (DW_TAG_subrange_type);
      CASE_S (DW_TAG_with_stmt);
      CASE_S (DW_TAG_access_declaration);
      CASE_S (DW_TAG_base_type);
      CASE_S (DW_TAG_catch_block);
      CASE_S (DW_TAG_const_type);
      CASE_S (DW_TAG_constant);
      CASE_S (DW_TAG_enumerator);
      CASE_S (DW_TAG_file_type);
      CASE_S (DW_TAG_friend);
      CASE_S (DW_TAG_namelist);
      CASE_S (DW_TAG_namelist_item);
      CASE_S (DW_TAG_packed_type);
      CASE_S (DW_TAG_subprogram);
      CASE_S (DW_TAG_template_type_param);
      CASE_S (DW_TAG_template_value_param);
      CASE_S (DW_TAG_thrown_type);
      CASE_S (DW_TAG_try_block);
      CASE_S (DW_TAG_variant_part);
      CASE_S (DW_TAG_variable);
      CASE_S (DW_TAG_volatile_type);
      CASE_S (DW_TAG_dwarf_procedure);
      CASE_S (DW_TAG_restrict_type);
      CASE_S (DW_TAG_interface_type);
      CASE_S (DW_TAG_namespace);
      CASE_S (DW_TAG_imported_module);
      CASE_S (DW_TAG_unspecified_type);
      CASE_S (DW_TAG_partial_unit);
      CASE_S (DW_TAG_imported_unit);
      CASE_S (DW_TAG_lo_user);
      CASE_S (DW_TAG_MIPS_loop);
      CASE_S (DW_TAG_format_label);
      CASE_S (DW_TAG_function_template);
      CASE_S (DW_TAG_class_template);
      CASE_S (DW_TAG_GNU_BINCL);
      CASE_S (DW_TAG_GNU_EINCL);
      CASE_S (DW_TAG_GNU_call_site);
      CASE_S (DW_TAG_GNU_call_site_parameter);
      CASE_S (DW_TAG_SUN_codeflags);
      CASE_S (DW_TAG_SUN_memop_info);
      CASE_S (DW_TAG_hi_user);
      CASE_S (DW_TAG_icc_compile_unit);
      CASE_S (DW_TAG_rvalue_reference_type);
      CASE_S (DW_TAG_coarray_type);
      CASE_S (DW_TAG_generic_subrange);
      CASE_S (DW_TAG_dynamic_type);
      CASE_S (DW_TAG_atomic_type);
      CASE_S (DW_TAG_call_site);
      CASE_S (DW_TAG_call_site_parameter);
      CASE_S (DW_TAG_skeleton_unit);
      CASE_S (DW_TAG_immutable_type);
      CASE_S (0);
    default: s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, tag);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

char *
DwrCU::at2str (int tag)
{
  static char buf[128];
  char *s;
  switch (tag)
    {
      CASE_S (DW_AT_sibling);
      CASE_S (DW_AT_location);
      CASE_S (DW_AT_name);
      CASE_S (DW_AT_ordering);
      CASE_S (DW_AT_subscr_data);
      CASE_S (DW_AT_byte_size);
      CASE_S (DW_AT_bit_offset);
      CASE_S (DW_AT_bit_size);
      CASE_S (DW_AT_element_list);
      CASE_S (DW_AT_stmt_list);
      CASE_S (DW_AT_low_pc);
      CASE_S (DW_AT_high_pc);
      CASE_S (DW_AT_language);
      CASE_S (DW_AT_member);
      CASE_S (DW_AT_discr);
      CASE_S (DW_AT_discr_value);
      CASE_S (DW_AT_visibility);
      CASE_S (DW_AT_import);
      CASE_S (DW_AT_string_length);
      CASE_S (DW_AT_common_reference);
      CASE_S (DW_AT_comp_dir);
      CASE_S (DW_AT_const_value);
      CASE_S (DW_AT_containing_type);
      CASE_S (DW_AT_default_value);
      CASE_S (DW_AT_inline);
      CASE_S (DW_AT_is_optional);
      CASE_S (DW_AT_lower_bound);
      CASE_S (DW_AT_producer);
      CASE_S (DW_AT_prototyped);
      CASE_S (DW_AT_return_addr);
      CASE_S (DW_AT_start_scope);
      CASE_S (DW_AT_stride_size);
      CASE_S (DW_AT_upper_bound);
      CASE_S (DW_AT_abstract_origin);
      CASE_S (DW_AT_accessibility);
      CASE_S (DW_AT_address_class);
      CASE_S (DW_AT_artificial);
      CASE_S (DW_AT_base_types);
      CASE_S (DW_AT_calling_convention);
      CASE_S (DW_AT_count);
      CASE_S (DW_AT_data_member_location);
      CASE_S (DW_AT_decl_column);
      CASE_S (DW_AT_decl_file);
      CASE_S (DW_AT_decl_line);
      CASE_S (DW_AT_declaration);
      CASE_S (DW_AT_discr_list);
      CASE_S (DW_AT_encoding);
      CASE_S (DW_AT_external);
      CASE_S (DW_AT_frame_base);
      CASE_S (DW_AT_friend);
      CASE_S (DW_AT_identifier_case);
      CASE_S (DW_AT_macro_info);
      CASE_S (DW_AT_namelist_item);
      CASE_S (DW_AT_priority);
      CASE_S (DW_AT_segment);
      CASE_S (DW_AT_specification);
      CASE_S (DW_AT_static_link);
      CASE_S (DW_AT_type);
      CASE_S (DW_AT_use_location);
      CASE_S (DW_AT_variable_parameter);
      CASE_S (DW_AT_virtuality);
      CASE_S (DW_AT_vtable_elem_location);
      CASE_S (DW_AT_allocated);
      CASE_S (DW_AT_associated);
      CASE_S (DW_AT_data_location);
      CASE_S (DW_AT_byte_stride);
      CASE_S (DW_AT_entry_pc);
      CASE_S (DW_AT_use_UTF8);
      CASE_S (DW_AT_extension);
      CASE_S (DW_AT_ranges);
      CASE_S (DW_AT_trampoline);
      CASE_S (DW_AT_call_column);
      CASE_S (DW_AT_call_file);
      CASE_S (DW_AT_call_line);
      CASE_S (DW_AT_description);
      CASE_S (DW_AT_binary_scale);
      CASE_S (DW_AT_decimal_scale);
      CASE_S (DW_AT_small);
      CASE_S (DW_AT_decimal_sign);
      CASE_S (DW_AT_digit_count);
      CASE_S (DW_AT_picture_string);
      CASE_S (DW_AT_mutable);
      CASE_S (DW_AT_threads_scaled);
      CASE_S (DW_AT_explicit);
      CASE_S (DW_AT_object_pointer);
      CASE_S (DW_AT_endianity);
      CASE_S (DW_AT_elemental);
      CASE_S (DW_AT_pure);
      CASE_S (DW_AT_recursive);
      CASE_S (DW_AT_signature);
      CASE_S (DW_AT_main_subprogram);
      CASE_S (DW_AT_data_bit_offset);
      CASE_S (DW_AT_const_expr);
      CASE_S (DW_AT_enum_class);
      CASE_S (DW_AT_linkage_name);
      CASE_S (DW_AT_lo_user);
      CASE_S (DW_AT_MIPS_fde);
      CASE_S (DW_AT_MIPS_loop_begin);
      CASE_S (DW_AT_MIPS_tail_loop_begin);
      CASE_S (DW_AT_MIPS_epilog_begin);
      CASE_S (DW_AT_MIPS_loop_unroll_factor);
      CASE_S (DW_AT_MIPS_software_pipeline_depth);
      CASE_S (DW_AT_MIPS_linkage_name);
      CASE_S (DW_AT_MIPS_stride);
      CASE_S (DW_AT_MIPS_abstract_name);
      CASE_S (DW_AT_MIPS_clone_origin);
      CASE_S (DW_AT_MIPS_has_inlines);
      CASE_S (DW_AT_sf_names);
      CASE_S (DW_AT_src_info);
      CASE_S (DW_AT_mac_info);
      CASE_S (DW_AT_src_coords);
      CASE_S (DW_AT_body_begin);
      CASE_S (DW_AT_body_end);
      CASE_S (DW_AT_GNU_vector);
      CASE_S (DW_AT_GNU_guarded_by);
      CASE_S (DW_AT_GNU_pt_guarded_by);
      CASE_S (DW_AT_GNU_guarded);
      CASE_S (DW_AT_GNU_pt_guarded);
      CASE_S (DW_AT_GNU_locks_excluded);
      CASE_S (DW_AT_GNU_exclusive_locks_required);
      CASE_S (DW_AT_GNU_shared_locks_required);
      CASE_S (DW_AT_GNU_odr_signature);
      CASE_S (DW_AT_GNU_template_name);
      CASE_S (DW_AT_GNU_call_site_value);
      CASE_S (DW_AT_GNU_call_site_data_value);
      CASE_S (DW_AT_GNU_call_site_target);
      CASE_S (DW_AT_GNU_call_site_target_clobbered);
      CASE_S (DW_AT_GNU_tail_call);
      CASE_S (DW_AT_GNU_all_tail_call_sites);
      CASE_S (DW_AT_GNU_all_call_sites);
      CASE_S (DW_AT_GNU_all_source_call_sites);
      CASE_S (DW_AT_GNU_locviews);
      CASE_S (DW_AT_GNU_entry_view);
      CASE_S (DW_AT_SUN_command_line);
      CASE_S (DW_AT_SUN_func_offsets);
      CASE_S (DW_AT_SUN_cf_kind);
      CASE_S (DW_AT_SUN_func_offset);
      CASE_S (DW_AT_SUN_memop_type_ref);
      CASE_S (DW_AT_SUN_profile_id);
      CASE_S (DW_AT_SUN_memop_signature);
      CASE_S (DW_AT_SUN_obj_dir);
      CASE_S (DW_AT_SUN_obj_file);
      CASE_S (DW_AT_SUN_original_name);
      CASE_S (DW_AT_SUN_link_name);
      CASE_S (DW_AT_hi_user);
      CASE_S (DW_AT_icc_flags);
      CASE_S (DW_AT_string_length_bit_size);
      CASE_S (DW_AT_string_length_byte_size);
      CASE_S (DW_AT_rank);
      CASE_S (DW_AT_str_offsets_base);
      CASE_S (DW_AT_addr_base);
      CASE_S (DW_AT_rnglists_base);
      CASE_S (DW_AT_dwo_name);
      CASE_S (DW_AT_reference);
      CASE_S (DW_AT_rvalue_reference);
      CASE_S (DW_AT_macros);
      CASE_S (DW_AT_call_all_calls);
      CASE_S (DW_AT_call_all_source_calls);
      CASE_S (DW_AT_call_all_tail_calls);
      CASE_S (DW_AT_call_return_pc);
      CASE_S (DW_AT_call_value);
      CASE_S (DW_AT_call_origin);
      CASE_S (DW_AT_call_parameter);
      CASE_S (DW_AT_call_pc);
      CASE_S (DW_AT_call_tail_call);
      CASE_S (DW_AT_call_target);
      CASE_S (DW_AT_call_target_clobbered);
      CASE_S (DW_AT_call_data_location);
      CASE_S (DW_AT_call_data_value);
      CASE_S (DW_AT_noreturn);
      CASE_S (DW_AT_alignment);
      CASE_S (DW_AT_export_symbols);
      CASE_S (DW_AT_deleted);
      CASE_S (DW_AT_defaulted);
      CASE_S (DW_AT_loclists_base);

    default: s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, tag);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

char *
DwrCU::form2str (int tag)
{
  static char buf[128];
  char *s;
  switch (tag)
    {
      CASE_S (DW_FORM_addr);
      CASE_S (DW_FORM_block2);
      CASE_S (DW_FORM_block4);
      CASE_S (DW_FORM_data2);
      CASE_S (DW_FORM_data4);
      CASE_S (DW_FORM_data8);
      CASE_S (DW_FORM_data16);
      CASE_S (DW_FORM_line_strp);
      CASE_S (DW_FORM_implicit_const);
      CASE_S (DW_FORM_string);
      CASE_S (DW_FORM_block);
      CASE_S (DW_FORM_block1);
      CASE_S (DW_FORM_data1);
      CASE_S (DW_FORM_flag);
      CASE_S (DW_FORM_sdata);
      CASE_S (DW_FORM_strp);
      CASE_S (DW_FORM_udata);
      CASE_S (DW_FORM_ref_addr);
      CASE_S (DW_FORM_ref1);
      CASE_S (DW_FORM_ref2);
      CASE_S (DW_FORM_ref4);
      CASE_S (DW_FORM_ref8);
      CASE_S (DW_FORM_ref_udata);
      CASE_S (DW_FORM_indirect);
      CASE_S (DW_FORM_sec_offset);
      CASE_S (DW_FORM_exprloc);
      CASE_S (DW_FORM_flag_present);
      CASE_S (DW_FORM_ref_sig8);
    default: s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, tag);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

char *
DwrCU::lnct2str (int ty)
{
  static char buf[128];
  char *s;
  switch (ty)
    {
      CASE_S (DW_LNCT_path);
      CASE_S (DW_LNCT_directory_index);
      CASE_S (DW_LNCT_timestamp);
      CASE_S (DW_LNCT_size);
      CASE_S (DW_LNCT_MD5);
      CASE_S (DW_LNCT_lo_user);
      CASE_S (DW_LNCT_hi_user);
    default: s = NTXT ("???");
      break;
    }
  snprintf (buf, sizeof (buf), NTXT ("%s(%d)"), s, ty);
  buf[sizeof (buf) - 1] = 0;
  return buf;
}

void
Dwr_Tag::dump ()
{
  Dprintf (DUMP_DWARFLIB,
	   "\n<%2d>:<0x%08llx> %-30s <abbrev %lld> offset=0x%llx %s\n",
	   (int) level, (long long) die, DwrCU::tag2str (tag), (long long) num,
	   (long long) offset,
	   hasChild ? NTXT ("DW_children_yes") : NTXT ("DW_children_no"));
  for (int i1 = firstAttribute; i1 < lastAttribute; i1++)
    {
      Dwr_Attr *atrp = abbrevAtForm->get (i1);
      Dprintf (DUMP_DWARFLIB, "       %-30s ", DwrCU::at2str (atrp->at_name));
      switch (atrp->at_form)
	{
	case DW_FORM_strp:
	case DW_FORM_string:
	case DW_FORM_line_strp:
	case DW_FORM_strp_sup:
	case DW_FORM_implicit_const:
	  Dprintf (DUMP_DWARFLIB, "  \"%s\"", atrp->u.str ? atrp->u.str : "<NULL>");
	  break;
	case DW_FORM_block:
	case DW_FORM_block1:
	case DW_FORM_block2:
	case DW_FORM_block4:
	case DW_FORM_data16:
	  Dprintf (DUMP_DWARFLIB, "  len=%3ld  %p", (long) atrp->len,
		   atrp->u.str);
	  break;
	case DW_FORM_addr:
	case DW_FORM_data2:
	case DW_FORM_data4:
	case DW_FORM_data8:
	case DW_FORM_data1:
	case DW_FORM_flag:
	case DW_FORM_sdata:
	case DW_FORM_udata:
	case DW_FORM_ref_addr:
	case DW_FORM_ref1:
	case DW_FORM_ref2:
	case DW_FORM_ref4:
	case DW_FORM_ref8:
	case DW_FORM_ref_udata:
	case DW_FORM_indirect:
	case DW_FORM_sec_offset:
	case DW_FORM_exprloc:
	case DW_FORM_ref_sig8:
	case DW_FORM_flag_present:
	  Dprintf (DUMP_DWARFLIB, "  0x%llx (%lld)", (long long) atrp->u.val,
		   (long long) atrp->u.val);
	  break;
	default:
	  DEBUG_CODE
	  {
	    Dprintf (1, "Attribute form 0x%llx (%lld) is not implemented\n",
		     (long long) atrp->at_form, (long long) atrp->at_form);
	    assert (false);
	  }
	}
      Dprintf (DUMP_DWARFLIB, NTXT ("\n"));
    }
}


//////////////////////////////////////////////////////////
//  class DwrSec

DwrSec::DwrSec (unsigned char *_data, uint64_t _size, bool _need_swap_endian, bool _addr32)
{
  isCopy = false;
  data = _data;
  sizeSec = _size;
  size = (data ? _size : 0);
  offset = 0;
  fmt64 = false;
  reloc = NULL;
  need_swap_endian = _need_swap_endian;
  addr32 = _addr32;
}

DwrSec::DwrSec (DwrSec *secp, uint64_t _offset)
{
  isCopy = true;
  data = secp->data;
  sizeSec = secp->sizeSec;
  size = secp->size;
  offset = _offset;
  fmt64 = secp->fmt64;
  reloc = secp->reloc;
  need_swap_endian = secp->need_swap_endian;
  addr32 = secp->addr32;
}

DwrSec::~DwrSec ()
{
  if (!isCopy)
    delete reloc;
}

bool
DwrSec::bounds_violation (uint64_t sz)
{
  if (offset + sz > size)
    {
      Dprintf (DEBUG_ERR_MSG, "DwrSec::bounds_violation: offset=%lld + sz=%lld > size=%lld\n",
	       (long long) offset, (long long) sz, (long long) size);
      return true;
    }
  return false;
}

uint64_t
DwrSec::ReadLength ()
{
  fmt64 = false;
  uint64_t val = Get_32 ();
  if (((uint32_t) val) == 0xffffffff)
    {
      fmt64 = true;
      val = Get_64 ();
    }
  size = (val + offset < sizeSec) ? val + offset : sizeSec;
  return size;
}

unsigned char
DwrSec::Get_8 ()
{
  unsigned char n = 0;
  if (bounds_violation (sizeof (char)))
    return n;
  n = data[offset];
  offset += sizeof (char);
  return n;
}

unsigned short
DwrSec::Get_16 ()
{
  unsigned short n = 0;
  if (bounds_violation (sizeof (short)))
    return n;
  memcpy ((char *) &n, data + offset, sizeof (short));
  offset += sizeof (short);
  if (need_swap_endian)
    SWAP_ENDIAN (n);
  return n;
}

uint32_t
DwrSec::Get_24 ()
{
  uint32_t n = 0;
  if (bounds_violation (3))
    return n;
  memcpy ((char *) &n, data + offset, 3);
  offset += 3;
  if (need_swap_endian)
    SWAP_ENDIAN (n);
  return n;
}

uint32_t
DwrSec::Get_32 ()
{
  uint32_t n = 0;
  if (bounds_violation (sizeof (uint32_t)))
    return n;
  memcpy ((char *) &n, data + offset, sizeof (uint32_t));
  offset += sizeof (uint32_t);
  if (need_swap_endian)
    SWAP_ENDIAN (n);
  return n;
}

uint64_t
DwrSec::Get_64 ()
{
  uint64_t n = 0;
  if (bounds_violation (sizeof (uint64_t)))
    return n;
  memcpy ((char *) &n, data + offset, sizeof (uint64_t));
  offset += sizeof (uint64_t);
  if (need_swap_endian)
    SWAP_ENDIAN (n);
  return n;
}

char *
DwrSec::GetData (uint64_t len)
{
  char *s = ((char *) data) + offset;
  if (bounds_violation (len))
    s = NULL;
  offset += len;
  return s;
}

char *
DwrSec::GetString ()
{
  uint64_t off = offset;
  while (offset < size)
    if (data[offset++] == 0)
      { // '\0' is inside section
	if (off + 1 == offset)
	  return NULL;
	return ((char *) data) + off;
      }
  return NULL; // The section is not '\0' terminated
}

uint64_t
DwrSec::GetLong ()
{
  if (fmt64)
    return Get_64 ();
  return Get_32 ();
}

uint64_t
DwrSec::GetADDR_32 ()
{
  uint64_t res = reloc ? reloc->get_reloc_addr (offset) : 0;
  res += Get_32 ();
  return res;
}

uint64_t
DwrSec::GetADDR_64 ()
{
  uint64_t res = reloc ? reloc->get_reloc_addr (offset) : 0;
  res += Get_64 ();
  return res;
}

uint64_t
DwrSec::GetADDR ()
{
  if (addr32)
    return GetADDR_32 ();
  return GetADDR_64 ();
}

uint64_t
DwrSec::GetRef ()
{
  if (fmt64)
    return GetADDR_64 ();
  return GetADDR_32 ();
}

ULEB128
DwrSec::GetULEB128 ()
{
  ULEB128 res = 0;
  for (int shift = 0;; shift += 7)
    {
      ULEB128 val = Get_8 ();
      res |= (val & 0x7f) << shift;
      if ((val & 0x80) == 0)
	break;
    }
  return res;
}

SLEB128
DwrSec::GetSLEB128 ()
{
  ULEB128 res = 0, val = 0;
  size_t shift;
  for (shift = 0;;)
    {
      val = Get_8 ();
      res |= (val & 0x7f) << shift;
      shift += 7;
      if ((val & 0x80) == 0)
	break;
    }
  if ((val & 0x40) && (shift < 8 * sizeof (res)))
    res |= -(((ULEB128) 1) << shift);
  return (SLEB128) res;
}

uint64_t
DwrSec::get_value (int dw_form)
{
  uint64_t v;
  switch (dw_form)
    {
    case DW_FORM_line_strp:
    case DW_FORM_strp:
    case DW_FORM_strp_sup:
      return GetRef ();
    case DW_FORM_data1:
      return Get_8 ();
    case DW_FORM_data2:
      return Get_16 ();
    case DW_FORM_data4:
      return Get_32 ();
    case DW_FORM_data8:
      return Get_64 ();
    case DW_FORM_udata:
      return GetULEB128 ();
    case DW_FORM_data16:
      offset += 16;
      return offset - 16;
    case DW_FORM_block:
      v = GetULEB128 ();
      offset += v;
      return offset - v;
    }
  return 0;
}

static void
fillBuf (unsigned char *s, int len, int col, unsigned char *buf)
{
  const char *nameX = "0123456789abcdef";
  int i, n, posCh = 2 * col + col / 4 + 5;

  if (len >= col)
    len = col;
  for (i = n = 0; i < len; i++, n += 2)
    {
      if ((i % 4) == 0 && i > 0)
	{
	  buf[n] = ' ';
	  n++;
	}
      buf[n] = nameX[s[i] >> 4];
      buf[n + 1] = nameX[s[i] & 0xf];
      buf[posCh + i] = isprint (s[i]) ? s[i] : ' ';
    }
  buf[posCh + i] = 0;
  for (i = n; i < posCh; i++)
    buf[i] = ' ';
}

static void
dumpArr (unsigned char *s, int len, int col, int num)
{
  unsigned char buf[128];
  if (col <= 0)
    return;
  for (int i = 0; i < len; i += col, num += col)
    {
      fillBuf (s + i, len - i, col, buf);
      Dprintf (DUMP_DWARFLIB, "%5d: %s\n", num, buf);
    }
}

void
DwrSec::dump (char *msg)
{
  if (sizeSec > 0)
    {
      Dprintf (DUMP_DWARFLIB, NTXT ("======= DwrSec::dump\n"));
      if (msg)
	Dprintf (DUMP_DWARFLIB, NTXT ("%s:\n"), msg);
      dumpArr (data, (int) sizeSec, 32, 0);
      Dprintf (DUMP_DWARFLIB, NTXT ("\n"));
    }
}

//////////////////////////////////////////////////////////
//  class DwrFileNames

DwrFileName::DwrFileName (char *_fname)
{
  path = NULL;
  fname = dbe_strdup (_fname);
  dir_index = 0;
  timestamp = 0;
  file_size = 0;
  isUsed = false;
}

DwrFileName::~DwrFileName ()
{
  if (path != fname)
    free (path);
}


//////////////////////////////////////////////////////////
//  class DwrLine
DwrLine::DwrLine ()
{
  address = 0;
  file = 0;
  line = 0;
  column = 0;
}

DwrLine::~DwrLine () { }


//////////////////////////////////////////////////////////
//  class DwrLineRegs
static int
LineRegsCmp (const void *a, const void *b)
{
  DwrLine *item1 = *((DwrLine **) a);
  DwrLine *item2 = *((DwrLine **) b);
  return item1->address == item2->address ? 0 :
	  item1->address > item2->address ? 1 : -1;
}

DwrLineRegs::DwrLineRegs (Dwarf *_dwarf, DwrSec *secp, char *dirName)
{
  dwarf = _dwarf;
  dir_names = NULL;
  file_names = NULL;
  lines = NULL;
  fname = NULL;
  // `dwarfdump -vv -l` shows a line section (.debug_line)
  debug_lineSec = secp;
  uint64_t stmt_offset = debug_lineSec->offset;
  uint64_t next_cu_offset = debug_lineSec->ReadLength ();
  uint64_t header_offset = debug_lineSec->offset;
  debug_lineSec->size = next_cu_offset;
  version = debug_lineSec->Get_16 ();
  if (version == 5)
    {
      debug_lineSec->address_size = debug_lineSec->Get_8();
      debug_lineSec->segment_selector_size = debug_lineSec->Get_8();
    }
  header_length = debug_lineSec->GetLong ();
  opcode_start = debug_lineSec->offset + header_length;
  minimum_instruction_length = debug_lineSec->Get_8 ();
  op_index_register = 0;
  if (version >= 4)
    maximum_operations_per_instruction = debug_lineSec->Get_8 ();
  else
    maximum_operations_per_instruction = 1;
  default_is_stmt = debug_lineSec->Get_8 ();
  is_stmt = (default_is_stmt != 0);
  line_base = debug_lineSec->Get_8 ();
  line_range = debug_lineSec->Get_8 ();
  opcode_base = debug_lineSec->Get_8 ();
  standard_opcode_length = (Dwarf_Small*) debug_lineSec->GetData (opcode_base - 1);

  if (DUMP_DWR_LINE_REGS)
    {
      Dprintf (DUMP_DWR_LINE_REGS,
	       "\n.debug_line  version=%d stmt_offset=0x%llx"
	       "  header_offset=0x%llx size=%lld dirname='%s'\n"
	       "    header_length=0x%llx  opcode_start=0x%llx"
	       "  minimum_instruction_length=%d default_is_stmt=%d\n"
	       "    line_base=%d  line_range=%d  opcode_base=%d\n",
	       (int) version, (long long) stmt_offset,
	       (long long) header_offset,
	       (long long) (next_cu_offset - header_offset), STR (dirName),
	       (long long) header_length, (long long) opcode_start,
	       (int) minimum_instruction_length, (int) default_is_stmt,
	       (int) line_base, (int) line_range, (int) opcode_base);
      if (standard_opcode_length == NULL)
	Dprintf (DUMP_DWR_LINE_REGS, "ERROR: standard_opcode_length is NULL\n");
      for (int i = 0, sz = standard_opcode_length ? opcode_base - 1 : 0;
	      i < sz; i++)
	Dprintf (DUMP_DWR_LINE_REGS, "  opcode[%2d] length %2d\n", i,
		 (int) standard_opcode_length[i]);
    }

  if (version == 5)
    {
      dir_names = read_file_names_dwarf5 ();
      file_names = read_file_names_dwarf5 ();
    }
  else
    {
      dir_names = new Vector<DwrFileName *>;
      dir_names->append (new DwrFileName (dirName));
      while (true)
	{
	  char *s = debug_lineSec->GetString ();
	  if (s == NULL)
	    break;
	  dir_names->append (new DwrFileName (s));
	}

      file_names = new Vector<DwrFileName *>;
      file_names->append (new DwrFileName (dirName));
      while (true)
	{
	  char *s = debug_lineSec->GetString ();
	  if (s == NULL)
	    break;
	  DwrFileName *fnp = new DwrFileName (s);
	  fnp->dir_index = debug_lineSec->GetULEB128_32 ();
	  fnp->timestamp = debug_lineSec->GetULEB128 ();
	  fnp->file_size = debug_lineSec->GetULEB128 ();
	  file_names->append (fnp);
	}
    }
  dump ();
}

DwrLineRegs::~DwrLineRegs ()
{
  Destroy (dir_names);
  Destroy (file_names);
  Destroy (lines);
  delete debug_lineSec;
}

Vector <DwrFileName *> *
DwrLineRegs::read_file_names_dwarf5 ()
{

  typedef struct
  {
    int type_code;
    int form_code;
  } t_entry_fmt;

  int efmt_cnt = debug_lineSec->Get_8 ();
  Dprintf (DUMP_DWR_LINE_REGS, "\nRead names: offset=0x%llx entry_fmt_cnt=%d\n",
	   (long long) debug_lineSec->offset, efmt_cnt);
  if (efmt_cnt == 0)
    return NULL;
  t_entry_fmt *efmt = (t_entry_fmt *) malloc (sizeof (t_entry_fmt) * efmt_cnt);
  for (int i = 0; i < efmt_cnt; i++)
    {
      efmt[i].type_code = debug_lineSec->GetULEB128 ();
      efmt[i].form_code = debug_lineSec->GetULEB128 ();
      Dprintf (DUMP_DWR_LINE_REGS, "  %2d  %20s  %s\n", i,
	       DwrCU::lnct2str (efmt[i].type_code),
	       DwrCU::form2str (efmt[i].form_code));
    }
  
  int cnt = debug_lineSec->GetULEB128_32 ();
  Dprintf (DUMP_DWR_LINE_REGS, "\nRead names: offset=0x%llx names_cnt=%d\n",
	   (long long) debug_lineSec->offset, cnt);
  Vector<DwrFileName *> *fnames = new Vector<DwrFileName *> (cnt);
  for (int i = 0; i < cnt; i++)
    {
      int ind = 0;
      uint64_t off = 0;
      uint64_t tstamp = 0;
      uint64_t fsize = 0;
      char *nm = NULL;
      for (int k = 0; k < efmt_cnt; k++)
	switch (efmt[k].type_code)
	  {
	  case DW_LNCT_path:
	    if (efmt[k].form_code == DW_FORM_string)
	      nm = debug_lineSec->GetString ();
	    else
	      {
		off = debug_lineSec->get_value (efmt[k].form_code);
		if (efmt[k].form_code == DW_FORM_line_strp)
		  nm = get_string (dwarf->debug_line_strSec, off);
		else if (efmt[k].form_code == DW_FORM_strp)
		  nm = get_string (dwarf->debug_strSec, off);
	      }
	    break;
	  case DW_LNCT_directory_index:
	    ind = debug_lineSec->get_value (efmt[k].form_code);
	    break;
	  case DW_LNCT_timestamp:
	    tstamp = debug_lineSec->get_value (efmt[k].form_code);
	    break;
	  case DW_LNCT_size:
	    fsize = debug_lineSec->get_value (efmt[k].form_code);
	    break;
	  case DW_LNCT_MD5:
	    (void) debug_lineSec->get_value (efmt[k].form_code);
	    break;
	  }
      Dprintf (DUMP_DWR_LINE_REGS, " %3d ind=%d off=0x%08llx  %s\n",
	       i, ind, (long long) off, STR (nm));
      DwrFileName *fnp = new DwrFileName (nm);
      fnp->dir_index = ind;
      fnp->timestamp = tstamp;
      fnp->file_size = fsize;
      fnames->append (fnp);
    }
  free (efmt);
  return fnames;
}

void
DwrLineRegs::dump ()
{
  if (!DUMP_DWR_LINE_REGS)
    return;
  if (dir_names)
    dir_names->dump ("dir_names");
  if (file_names)
    file_names->dump ("file_names");

  Dprintf (DUMP_DWR_LINE_REGS, NTXT ("\nfile_names size=%lld\n"), (long long) VecSize (file_names));
  for (long i = 0, sz = VecSize (file_names); i < sz; i++)
    {
      DwrFileName *fnp = file_names->get (i);
      Dprintf (DUMP_DWR_LINE_REGS, NTXT (" %2lld %-40s dir_index=%4lld  timestamp=%8lld file_size=%lld\n"),
	       (long long) i, STR (fnp->fname),
	       (long long) fnp->dir_index, (long long) fnp->timestamp, (long long) fnp->file_size);
    }
  if (lines)
    lines->dump (fname);
  Dprintf (DUMP_DWR_LINE_REGS, NTXT ("\n\n"));
}

void
DwrLineRegs::DoExtendedOpcode ()
{
  uint64_t size = debug_lineSec->GetULEB128 ();
  if (size == 0)
    {
      Dprintf (DUMP_DWR_LINE_REGS, NTXT ("%-20s"), NTXT ("ExtendedOpCode: size=0"));
      return;
    }
  Dwarf_Small opcode = debug_lineSec->Get_8 ();
  Dprintf (DUMP_DWR_LINE_REGS, NTXT ("%-20s"), extended_opcode2str (opcode));
  switch (opcode)
    {
    case DW_LNE_end_sequence:
      end_sequence = true;
      reset ();
      break;
    case DW_LNE_set_address:
      address = debug_lineSec->GetADDR ();
      break;
    case DW_LNE_define_file:
      // TODO, add file to file list
      fname = debug_lineSec->GetString ();
      dir_index = debug_lineSec->GetULEB128 ();
      timestamp = debug_lineSec->GetULEB128 ();
      file_size = debug_lineSec->GetULEB128 ();
      break;
    default:
      debug_lineSec->GetData (size - 1); // skip unknown opcode
      break;
    }
}

void
DwrLineRegs::DoStandardOpcode (int opcode)
{
  switch (opcode)
    {
    case DW_LNS_copy:
      basic_block = false;
      EmitLine ();
      break;
    case DW_LNS_advance_pc:
      address += debug_lineSec->GetULEB128 () * minimum_instruction_length;
      break;
    case DW_LNS_advance_line:
      line += (int) debug_lineSec->GetSLEB128 ();
      break;
    case DW_LNS_set_file:
      file = debug_lineSec->GetULEB128_32 ();
      break;
    case DW_LNS_set_column:
      column = debug_lineSec->GetULEB128_32 ();
      break;
    case DW_LNS_negate_stmt:
      is_stmt = -is_stmt;
      break;
    case DW_LNS_set_basic_block:
      basic_block = true;
      break;
    case DW_LNS_const_add_pc:
      address += ((255 - opcode_base) / line_range) * minimum_instruction_length;
      break;
    case DW_LNS_fixed_advance_pc:
      address += debug_lineSec->Get_16 ();
      break;
    default:    // skip unknown opcode/operands
      debug_lineSec->GetData (standard_opcode_length ?
			      standard_opcode_length[opcode] : 1);
      break;
    }
}

void
DwrLineRegs::DoSpecialOpcode (int opcode)
{
  int max_op_per_instr = maximum_operations_per_instruction == 0 ? 1
	  : maximum_operations_per_instruction;
  int operation_advance = (opcode / line_range);
  address += minimum_instruction_length * ((op_index_register + operation_advance) / max_op_per_instr);
  op_index_register = (op_index_register + operation_advance) % max_op_per_instr;
  line += line_base + (opcode % line_range);
  basic_block = false;
  EmitLine ();
}

void
DwrLineRegs::reset ()
{
  dir_index = 0;
  timestamp = 0;
  file_size = 0;
  address = 0;
  file = 1;
  line = 1;
  column = 0;
  is_stmt = (default_is_stmt != 0);
  basic_block = false;
  end_sequence = false;
}

void
DwrLineRegs::EmitLine ()
{
  DwrLine *lnp = new DwrLine;

  lnp->file = file;
  lnp->line = line;
  lnp->column = column;
  lnp->address = address;
  lines->append (lnp);
  if ((file > 0) && (file < VecSize (file_names)))
    {
      DwrFileName *fnp = file_names->get (file);
      fnp->isUsed = true;
    }
}

Vector<DwrLine *> *
DwrLineRegs::get_lines ()
{
  if (lines == NULL)
    {
      lines = new Vector<DwrLine *>;
      debug_lineSec->offset = opcode_start;
      reset ();
      Dprintf (DUMP_DWR_LINE_REGS, "\n  offset        code             address (file, line, column) stmt blck end_seq \n");
      while (debug_lineSec->offset < debug_lineSec->size)
	{
	  Dprintf (DUMP_DWR_LINE_REGS, NTXT ("0x%08llx "),
		   (long long) debug_lineSec->offset);
	  Dwarf_Small opcode = debug_lineSec->Get_8 ();
	  if (opcode == 0)
	    DoExtendedOpcode ();
	  else if (opcode < opcode_base)
	    {
	      DoStandardOpcode (opcode);
	      Dprintf (DUMP_DWR_LINE_REGS, NTXT ("%-20s"), standard_opcode2str (opcode));
	    }
	  else
	    {
	      DoSpecialOpcode (opcode - opcode_base);
	      Dprintf (DUMP_DWR_LINE_REGS, NTXT ("%-20s"),
		       special_opcode2str (opcode - opcode_base));
	    }
	  Dprintf (DUMP_DWR_LINE_REGS,
		   "  0x%08llx  (%lld, %lld, %lld)  %c %c %c\n",
		   (long long) address, (long long) file, (long long) line,
		   (long long) column, is_stmt ? 'T' : 'F',
		   basic_block ? 'T' : 'F', end_sequence ? 'T' : 'F');
	}
      lines->sort (LineRegsCmp);
      if (DUMP_DWR_LINE_REGS)
	lines->dump (fname);
    }
  return lines;
}

char *
DwrLineRegs::getPath (int fn)
{
  if (fn >= VecSize (file_names) || fn < 0)
    {
      Dprintf (DEBUG_ERR_MSG, NTXT ("DwrLineRegs::getPath: fn=0x%lld file_names->size()=%lld\n"),
	       (long long) fn, (long long) VecSize (file_names));
      return NULL;
    }
  DwrFileName *fnp = file_names->fetch (fn);
  if (fnp->fname == NULL)
    return NULL;
  if (fnp->path)
    return fnp->path;

  fnp->path = fnp->fname;
  if (fnp->fname[0] == '/')
    return fnp->path;

  char *dir = NULL;
  if (dir_names)
    {
      if (fnp->dir_index < dir_names->size () && fnp->dir_index >= 0)
	dir = dir_names->get (fnp->dir_index)->fname;
    }
  if (dir == NULL || *dir == 0)
    return fnp->path;

  char *dir1 = NULL;
  if (*dir != '/')
    dir1 = dir_names->get(0)->fname;
  if (dir1 && *dir != 0)
    fnp->path = dbe_sprintf ("%s/%s/%s", dir1, dir, fnp->fname);
  else 
    fnp->path = dbe_sprintf ("%s/%s", dir, fnp->fname);
  fnp->path = canonical_path (fnp->path);
  return fnp->path;
}

DwrCU::DwrCU (Dwarf *_dwarf)
{
  dwarf = _dwarf;
  cu_offset = dwarf->debug_infoSec->offset;
  debug_infoSec = new DwrSec (dwarf->debug_infoSec, cu_offset);
  next_cu_offset = debug_infoSec->ReadLength ();
  if (next_cu_offset > debug_infoSec->sizeSec)
    {
      Dprintf (DEBUG_ERR_MSG,
	"DwrCU::DwrCU: next_cu_offset(0x%llx) > debug_infoSec->sizeSec(%llx)\n",
	       (long long) next_cu_offset, (long long) debug_infoSec->sizeSec);
      next_cu_offset = debug_infoSec->sizeSec;
    }
  debug_infoSec->size = next_cu_offset;
  version = debug_infoSec->Get_16 ();
  if (version == 5)
    {
      unit_type = debug_infoSec->Get_8 ();
      address_size = debug_infoSec->Get_8 ();
      debug_abbrev_offset = debug_infoSec->GetLong ();
    }
  else
    {
      unit_type = DW_UT_compile;
      debug_abbrev_offset = debug_infoSec->GetLong ();
      address_size = debug_infoSec->Get_8 ();
    }
  cu_header_offset = debug_infoSec->offset;
  comp_dir = NULL;
  module = NULL;
  abbrevTable = NULL;
  dwrInlinedSubrs = NULL;
  srcFiles = NULL;
  stmt_list_offset = NO_STMT_LIST;
  dwrLineReg = NULL;
  isMemop = false;
  isGNU = false;
  dwrTag.level = 0;

  build_abbrevTable (dwarf->debug_abbrevSec, debug_abbrev_offset);
#ifdef DEBUG
  if (DUMP_DWARFLIB)
    {
      Dprintf (DUMP_DWARFLIB,
	       "CU_HEADER: header_offset = 0x%08llx %lld"
	       " next_header_offset=0x%08llx %lld\n"
	       "    abbrev_offset = 0x%08llx %lld\n"
	       "    unit_length   = %lld\n"
	       "    version       = %d\n"
	       "    address_size  = %d\n"
	       "    fmt64         = %s\n"
	       "debug_info:   need_swap_endian=%s  fmt64=%s addr32=%s\n",
	       (long long) cu_offset, (long long) cu_offset,
	       (long long) next_cu_offset, (long long) next_cu_offset,
	       (long long) debug_abbrev_offset, (long long) debug_abbrev_offset,
	       (long long) (next_cu_offset - cu_offset),
	       (int) version, (int) address_size,
	       debug_infoSec->fmt64 ? "true" : "false",
	       debug_infoSec->need_swap_endian ? "true" : "false",
	       debug_infoSec->fmt64 ? "true" : "false",
	       debug_infoSec->addr32 ? "true" : "false");
      Dprintf (DUMP_DWARFLIB, "\n.debug_abbrev  cnt=%d  offset=0x%08llx %lld\n",
	       (int) VecSize (abbrevTable), (long long) debug_abbrev_offset,
	       (long long) debug_abbrev_offset);
      for (int i = 1, sz = VecSize (abbrevTable); i < sz; i++)
	{
	  DwrAbbrevTable *abbTbl = abbrevTable->get (i);
	  Dprintf (DUMP_DWARFLIB, NTXT ("%5d: %-30s %-20s offset=0x%08llx\n"),
		   (int) i, DwrCU::tag2str (abbTbl->tag),
		   abbTbl->hasChild ? "DW_children_yes" : "DW_children_no",
		   (long long) abbTbl->offset);
	  for (int i1 = abbTbl->firstAtForm; i1 < abbTbl->lastAtForm; i1++)
	    {
	      Dwr_Attr *atf = abbrevAtForm->get (i1);
	      Dprintf (DUMP_DWARFLIB, "       %-30s %s\n",
		       DwrCU::at2str (atf->at_name),
		       DwrCU::form2str (atf->at_form));
	    }
	}
    }
#endif
}

DwrCU::~DwrCU ()
{
  delete debug_infoSec;
  delete abbrevTable;
  delete abbrevAtForm;
  Destroy (dwrInlinedSubrs);
  delete srcFiles;
  delete dwrLineReg;
  free (comp_dir);
}

void
DwrCU::build_abbrevTable (DwrSec *_debug_abbrevSec, uint64_t _offset)
{
  if (abbrevTable)
    return;
  DwrSec *debug_abbrevSec = new DwrSec (_debug_abbrevSec, _offset);
  abbrevTable = new DbeArray <DwrAbbrevTable>(128);
  abbrevAtForm = new DbeArray <Dwr_Attr>(512);
  abbrevTable->allocate (1); // skip first
  abbrevAtForm->allocate (1); // skip first
  for (int i = 1; debug_abbrevSec->offset < debug_abbrevSec->size; i++)
    {
      DwrAbbrevTable abbTbl;
      abbTbl.offset = debug_abbrevSec->offset;
      abbTbl.code = debug_abbrevSec->GetULEB128_32 ();
      if (abbTbl.code == 0)
	break;
      else if (i != abbTbl.code)
	{
	  dwarf->elf->append_msg (CMSG_ERROR, GTXT ("%s: the abbreviations table is corrupted (%lld <--> %lld)\n"),
				  get_basename (dwarf->elf->get_location ()),
				  (long long) i, (long long) abbTbl.code);
	  break;
	}
      abbTbl.tag = debug_abbrevSec->GetULEB128_32 ();
      abbTbl.hasChild = (DW_children_yes == debug_abbrevSec->Get_8 ());
      abbTbl.firstAtForm = abbrevAtForm->size ();
      while (debug_abbrevSec->offset < debug_abbrevSec->size)
	{
	  Dwr_Attr atf;
	  atf.len = 0;
	  atf.u.str = NULL;
	  atf.at_name = debug_abbrevSec->GetULEB128_32 ();
	  atf.at_form = debug_abbrevSec->GetULEB128_32 ();
	  if (atf.at_name == 0 && atf.at_form == 0)
	    break;
	  switch (atf.at_form)
	    {
	    case DW_FORM_implicit_const:
	      atf.len = debug_abbrevSec->GetSLEB128 ();
	      break;
	    }
	  abbrevAtForm->append (atf);
	}
      abbTbl.lastAtForm = abbrevAtForm->size ();
      abbrevTable->append (abbTbl);
    }
  delete debug_abbrevSec;
}

int
DwrCU::set_die (Dwarf_Die die)
{
  if (die > 0)
    debug_infoSec->offset = die;
  if (debug_infoSec->offset < cu_header_offset
      || debug_infoSec->offset >= debug_infoSec->size)
    return DW_DLV_ERROR;
  dwrTag.offset = debug_infoSec->offset;
  dwrTag.die = debug_infoSec->offset - cu_offset;
  dwrTag.num = debug_infoSec->GetULEB128_32 ();
  if (dwrTag.num == 0)
    return DW_DLV_NO_ENTRY;
  dwrTag.abbrevAtForm = abbrevAtForm;
  DwrAbbrevTable *abbTbl = abbrevTable->get (dwrTag.num);
  if (abbTbl == NULL)
    { // corrupt dwarf
      dwarf->elf->append_msg (CMSG_ERROR, GTXT ("%s: the abbreviation code (%lld) does not match for the Dwarf entry (0x%llx)\n"),
			      get_basename (dwarf->elf->get_location ()),
			     (long long) dwrTag.num, (long long) dwrTag.offset);
      return DW_DLV_ERROR;
    }
  dwrTag.tag = abbTbl->tag;
  dwrTag.hasChild = abbTbl->hasChild;
  dwrTag.firstAttribute = abbTbl->firstAtForm;
  dwrTag.lastAttribute = abbTbl->lastAtForm;
  for (int k = abbTbl->firstAtForm; k < abbTbl->lastAtForm; k++)
    {
      Dwr_Attr *atf = abbrevAtForm->get (k);
      int at_form = atf->at_form;
      if (at_form == DW_FORM_indirect)
	at_form = debug_infoSec->GetULEB128_32 ();
      switch (at_form)
	{
	case DW_FORM_addr:
	  atf->u.offset = (address_size == 4) ? debug_infoSec->GetADDR_32 ()
		  : debug_infoSec->GetADDR_64 ();
	  break;
	case DW_FORM_flag:
	  atf->u.offset = debug_infoSec->Get_8 ();
	  break;
	case DW_FORM_block:
	  atf->len = debug_infoSec->GetULEB128 ();
	  atf->u.str = debug_infoSec->GetData (atf->len);
	  break;
	case DW_FORM_block1:
	  atf->len = debug_infoSec->Get_8 ();
	  atf->u.str = debug_infoSec->GetData (atf->len);
	  break;
	case DW_FORM_block2:
	  atf->len = debug_infoSec->Get_16 ();
	  atf->u.str = debug_infoSec->GetData (atf->len);
	  break;
	case DW_FORM_block4:
	  atf->len = debug_infoSec->Get_32 ();
	  atf->u.str = debug_infoSec->GetData (atf->len);
	  break;
	case DW_FORM_ref1:
	  atf->u.offset = debug_infoSec->Get_8 ();
	  break;
	case DW_FORM_ref2:
	  atf->u.offset = debug_infoSec->Get_16 ();
	  break;
	case DW_FORM_ref4:
	  atf->u.offset = debug_infoSec->Get_32 ();
	  break;
	case DW_FORM_ref8:
	  atf->u.offset = debug_infoSec->Get_64 ();
	  break;
	case DW_FORM_ref_udata:
	  atf->u.offset = debug_infoSec->GetULEB128 ();
	  break;
	case DW_FORM_data1:
	  atf->u.offset = debug_infoSec->Get_8 ();
	  break;
	case DW_FORM_data2:
	  atf->u.offset = debug_infoSec->Get_16 ();
	  break;
	case DW_FORM_data4:
	  atf->u.offset = debug_infoSec->Get_32 ();
	  break;
	case DW_FORM_data8:
	  atf->u.offset = debug_infoSec->Get_64 ();
	  break;
	case DW_FORM_string:
	  atf->u.offset = debug_infoSec->offset;
	  atf->u.str = debug_infoSec->GetString ();
	  break;
	case DW_FORM_strp:
	  atf->u.offset = debug_infoSec->GetRef ();
	  atf->u.str = get_string (dwarf->debug_strSec, atf->u.offset);
	  break;
	case DW_FORM_sdata:
	  atf->u.val = debug_infoSec->GetSLEB128 ();
	  break;
	case DW_FORM_udata:
	  atf->u.offset = debug_infoSec->GetULEB128 ();
	  break;
	case DW_FORM_ref_addr:
	  atf->u.offset = debug_infoSec->GetADDR ();
	  break;
	case DW_FORM_sec_offset:
	  atf->u.offset = debug_infoSec->GetRef ();
	  break;
	case DW_FORM_exprloc:
	  atf->u.offset = debug_infoSec->GetULEB128 ();
	  debug_infoSec->offset += atf->u.offset;
	  break;
	case DW_FORM_flag_present:
	  atf->u.val = 1;
	  break;
	case DW_FORM_ref_sig8:
	  atf->u.offset = debug_infoSec->GetADDR_64 ();
	  break;
	case DW_FORM_data16:	// we never use this data. Skip 16 bytes
	  atf->len = 16;
	  (void) debug_infoSec->Get_64 ();
	  (void) debug_infoSec->Get_64 ();
	  break;
	case DW_FORM_addrx:
	case DW_FORM_strx:
	case DW_FORM_loclistx:
	case DW_FORM_rnglistx:
	  atf->u.offset = debug_infoSec->GetULEB128 ();
	  break;
	case DW_FORM_addrx1:
	case DW_FORM_strx1:
	  atf->u.offset = debug_infoSec->Get_8 ();
	  break;
	case DW_FORM_addrx2:
	case DW_FORM_strx2:
	  atf->u.offset = debug_infoSec->Get_16 ();
	  break;
	case DW_FORM_addrx3:
	case DW_FORM_strx3:
	  atf->u.offset = debug_infoSec->Get_24 ();
	  break;
	case DW_FORM_addrx4:
	case DW_FORM_strx4:
	case DW_FORM_ref_sup4:
	  atf->u.offset = debug_infoSec->Get_32 ();
	  break;
	case DW_FORM_ref_sup8:
	  atf->u.offset = debug_infoSec->Get_64 ();
	  break;
	case DW_FORM_line_strp:
	  atf->u.offset = debug_infoSec->GetRef ();
	  atf->u.str = get_string (dwarf->debug_line_strSec, atf->u.offset);
	  break;
	case DW_FORM_strp_sup:
	  atf->u.offset = debug_infoSec->GetRef ();
	  atf->u.str = NULL;
	  atf->len = 0;
	  break;
	case DW_FORM_implicit_const:
	  atf->u.str = NULL;
	  break;
	default:
	  DEBUG_CODE
	  {
	    Dprintf (1, "Attribute form 0x%llx (%lld) is not implemented\n",
		     (long long) atf->at_form, (long long) atf->at_form);
	    assert (0);
	  }
	  atf->u.str = NULL;
	  atf->len = 0;
	  break;
	}
    }
  dwrTag.dump ();
  return DW_DLV_OK;
}

static char *
composePath (char *dname, char *fname)
{
  char *s;
  if (*fname == '/' || dname == NULL)
    s = dbe_sprintf (NTXT ("%s"), fname);
  else
    s = dbe_sprintf (NTXT ("%s/%s"), dname, fname);
  return canonical_path (s);
}

Module *
DwrCU::parse_cu_header (LoadObject *lo)
{
  // Is tag always DW_TAG_compile_unit?
  if (dwrTag.tag != DW_TAG_compile_unit)
    {
      Dprintf (DEBUG_ERR_MSG,
	    "parse_cu_header: die=0x%llx tag=%lld is not DW_TAG_compile_unit\n",
	       (long long) cu_offset, (long long) dwrTag.tag);
      return NULL;
    }

  char *name = Dwarf_string (DW_AT_name);
  if (name == NULL)
    name = NTXT ("UnnamedUnit");
  int64_t v;
  if (read_data_attr(DW_AT_stmt_list, &v) == DW_DLV_OK)
    stmt_list_offset = v;
  comp_dir = dbe_strdup (Dwarf_string (DW_AT_comp_dir));
  char *dir_name = comp_dir ? StrChr (comp_dir, ':') : NULL;
  char *orig_name = Dwarf_string (DW_AT_SUN_original_name);
  char *path = composePath (dir_name, orig_name ? orig_name : name);

  module = dwarf->stabs->append_Module (lo, path);
  free (path);
  if (module == NULL)
    return NULL;
  module->hasDwarf = true;
  if (orig_name)
    module->linkerStabName = composePath (dir_name, name);
  module->lang_code = Dwarf_lang ();
  module->comp_flags = dbe_strdup (Dwarf_string (DW_AT_SUN_command_line));
  if (module->comp_flags == NULL)
    module->comp_flags = dbe_strdup (Dwarf_string (DW_AT_icc_flags));
  module->comp_dir = dbe_strdup (dir_name);

  char *obj_file = Dwarf_string (DW_AT_SUN_obj_file);
  char *obj_dir = Dwarf_string (DW_AT_SUN_obj_dir);
  if (obj_dir && obj_file)
    {
      // object information may not be available
      dir_name = StrChr (obj_dir, ':');
      path = composePath (dir_name, obj_file);
      if (module->dot_o_file == NULL)
	module->dot_o_file = module->createLoadObject (path);
    }
  else
    path = dbe_strdup (dwarf->stabs->path);
  module->set_name (path);
  return module;
}

Dwr_Attr *
Dwr_Tag::get_attr (Dwarf_Half attr)
{
  for (long i = firstAttribute; i < lastAttribute; i++)
    {
      Dwr_Attr *atf = abbrevAtForm->get (i);
      if (atf->at_name == attr)
	return atf;
    }
  return NULL;
}

char *
DwrCU::Dwarf_string (Dwarf_Half attr)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (attr);
  return dwrAttr ? dwrAttr->u.str : NULL;
}

uint64_t
DwrCU::get_high_pc (uint64_t low_pc)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (DW_AT_high_pc);
  if (dwrAttr)
    switch (dwrAttr->at_form)
      {
      case DW_FORM_addr:
	return dwrAttr->u.offset;
      default:
	return dwrAttr->u.offset + low_pc;
      }
  return 0;
}

Dwarf_Addr
DwrCU::Dwarf_addr (Dwarf_Half attr)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (attr);
  if (dwrAttr)
    switch (dwrAttr->at_form)
      {
      case DW_FORM_addr:
	return dwrAttr->u.offset;
      }
  return 0;
}

DwrSec*
DwrCU::Dwarf_block (Dwarf_Half attr)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (attr);
  if (dwrAttr && dwrAttr->u.block)
    switch (dwrAttr->at_form)
      {
      case DW_FORM_block:
      case DW_FORM_block1:
      case DW_FORM_block2:
      case DW_FORM_block4:
	return new DwrSec (dwrAttr->u.block, dwrAttr->len,
			   dwarf->elf->need_swap_endian,
			   dwarf->elf->elf_getclass () == ELFCLASS32);
      }
  return NULL;
}

int
DwrCU::read_data_attr (Dwarf_Half attr, int64_t *retVal)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (attr);
  if (dwrAttr)
    switch (dwrAttr->at_form)
      {
      case DW_FORM_data1:
      case DW_FORM_data2:
      case DW_FORM_data4:
      case DW_FORM_data8:
      case DW_FORM_data16:
      case DW_FORM_udata:
      case DW_FORM_sec_offset:
	*retVal = dwrAttr->u.val;
	return DW_DLV_OK;

      }
  return DW_DLV_ERROR;
}

int
DwrCU::read_ref_attr (Dwarf_Half attr, int64_t *retVal)
{
  Dwr_Attr *dwrAttr = dwrTag.get_attr (attr);
  if (dwrAttr)
    switch (dwrAttr->at_form)
      {
      case DW_FORM_ref1:
      case DW_FORM_ref2:
      case DW_FORM_ref4:
      case DW_FORM_ref8:
      case DW_FORM_ref_udata:
      case DW_FORM_sec_offset:
      case DW_FORM_exprloc:
      case DW_FORM_ref_sig8:
	*retVal = dwrAttr->u.val;
	return DW_DLV_OK;
      }
  return DW_DLV_ERROR;
}

int64_t
DwrCU::Dwarf_data (Dwarf_Half attr)
{
  int64_t retVal;
  if (read_data_attr (attr, &retVal) == DW_DLV_OK)
    return retVal;
  return 0;
}

int64_t
DwrCU::Dwarf_ref (Dwarf_Half attr)
{
  int64_t retVal;
  if (read_ref_attr (attr, &retVal) == DW_DLV_OK)
    return retVal;
  return 0;
}

Dwarf_Addr
DwrCU::Dwarf_location (Dwarf_Attribute attr)
{
  DwrSec *secp = Dwarf_block (attr);
  if (secp)
    {
      DwrLocation loc;
      DwrLocation *lp = dwr_get_location (secp, &loc);
      delete secp;
      if (lp)
	return lp->lc_number;
    }
  return 0;
}

void
DwrCU::map_dwarf_lines (Module *mod)
{
  DwrLineRegs *lineReg = get_dwrLineReg ();
  long inlinedSubrCnt = VecSize (dwrInlinedSubrs);
  if (isGNU && (inlinedSubrCnt > 0))
    {
      Function *func = NULL;
      mod->inlinedSubr = (InlinedSubr *) malloc (inlinedSubrCnt
						 * sizeof (InlinedSubr));
      for (long i = 0; i < inlinedSubrCnt; i++)
	{
	  DwrInlinedSubr *inlinedSubr = dwrInlinedSubrs->get (i);
	  uint64_t low_pc;
	  Function *f = dwarf->stabs->map_PC_to_func (inlinedSubr->low_pc,
						      low_pc, mod->functions);
	  if (f == NULL)
	    continue;
	  if (func != f)
	    {
	      func = f;
	      func->inlinedSubrCnt = 0;
	      func->inlinedSubr = mod->inlinedSubr + i;
	    }
	  InlinedSubr *p = func->inlinedSubr + func->inlinedSubrCnt;
	  func->inlinedSubrCnt++;
	  int fileno = inlinedSubr->file - 1;
	  SourceFile *sf = ((fileno >= 0) && (fileno < VecSize (srcFiles))) ?
		  srcFiles->get (fileno) : dbeSession->get_Unknown_Source ();
	  p->dbeLine = sf->find_dbeline (inlinedSubr->line);
	  p->high_pc = inlinedSubr->high_pc - low_pc;
	  p->low_pc = inlinedSubr->low_pc - low_pc;
	  p->level = inlinedSubr->level;
	  p->func = NULL;
	  p->fname = NULL;
	  if (set_die (inlinedSubr->abstract_origin) == DW_DLV_OK)
	    p->fname = dbe_strdup (Dwarf_string (DW_AT_name));
	  if (p->fname)
	    p->func = Stabs::find_func (p->fname, mod->functions,
					Stabs::is_fortran (mod->lang_code));
	}
    }
  if (lineReg == NULL)
    return;
  Vector<DwrLine *> *lines = lineReg->get_lines ();

  Include *includes = new Include;
  includes->new_src_file (mod->getMainSrc (), 0, NULL);
  char *path = NULL;
  SourceFile *cur_src = NULL;
  Function *cur_func = NULL;
  for (long i = 0, sz = VecSize (lines); i < sz; i++)
    {
      DwrLine *dwrLine = lines->get (i);
      char *filename = lineReg->getPath (dwrLine->file);
      if (filename == NULL)
	continue;
      uint64_t pc = dwrLine->address;
      int lineno = dwrLine->line;
      if (path != filename)
	{
	  path = filename;
	  char *name = StrChr (path, ':');
	  SourceFile *src = mod->setIncludeFile (name);
	  if (cur_src != src)
	    {
	      includes->new_src_file (src, lineno, cur_func);
	      cur_src = src;
	    }
	}
      uint64_t low_pc;
      Function *func = dwarf->stabs->map_PC_to_func (pc, low_pc, mod->functions);
      if (func && (func->module == mod))
	{
	  if (func != cur_func)
	    {
	      if (cur_func)
		while (cur_func->popSrcFile () != NULL)
		  ;
	      cur_func = func;
	      includes->push_src_files (cur_func);
	    }
	  cur_func->add_PC_info (pc - low_pc, lineno);
	}
    }
  if (cur_func)
    while (cur_func->popSrcFile ())
      ;
  delete includes;
}

DwrLineRegs *
DwrCU::get_dwrLineReg ()
{
  if (dwrLineReg == NULL && stmt_list_offset != NO_STMT_LIST)
    dwrLineReg = new DwrLineRegs (dwarf, new DwrSec (dwarf->debug_lineSec,
					      stmt_list_offset), comp_dir);
  return dwrLineReg;
}

void
DwrCU::parse_inlined_subroutine (Dwarf_cnt *ctx)
{
  int64_t abstract_origin = Dwarf_ref (DW_AT_abstract_origin);
  int fileno = (int) Dwarf_data (DW_AT_call_file);
  int lineno = (int) Dwarf_data (DW_AT_call_line);
  int level = ctx->inlinedSubr ? (ctx->inlinedSubr->level + 1) : 0;
  DwrInlinedSubr *inlinedSubr_old = ctx->inlinedSubr;

  if (dwrInlinedSubrs == NULL)
    dwrInlinedSubrs = new Vector<DwrInlinedSubr*>;
  Dwr_Attr *dwrAttr = dwrTag.get_attr (DW_AT_ranges);
  if (dwrAttr)
    {
      uint64_t ranges = Dwarf_ref (DW_AT_ranges);
      if (dwarf->debug_rangesSec && (ranges < dwarf->debug_rangesSec->size))
	{
	  dwarf->debug_rangesSec->offset = ranges;
	  for (;;)
	    {
	      uint64_t low_pc = dwarf->debug_rangesSec->GetADDR ();
	      uint64_t high_pc = dwarf->debug_rangesSec->GetADDR ();
	      if ((low_pc > 0) && (low_pc <= high_pc))
		{
		  DwrInlinedSubr *p = new DwrInlinedSubr (abstract_origin,
					low_pc, high_pc, fileno, lineno, level);
		  dwrInlinedSubrs->append (p);
		  ctx->inlinedSubr = p;
		}
	      else
		break;
	    }
	}
    }
  else
    {
      uint64_t low_pc = Dwarf_addr (DW_AT_low_pc);
      uint64_t high_pc = get_high_pc (low_pc);
      if ((low_pc > 0) && (low_pc <= high_pc))
	{
	  DwrInlinedSubr *p = new DwrInlinedSubr (abstract_origin, low_pc,
						high_pc, fileno, lineno, level);
	  dwrInlinedSubrs->append (p);
	  ctx->inlinedSubr = p;
	}
    }
  parseChild (ctx);
  ctx->inlinedSubr = inlinedSubr_old;
}


//////////////////////////////////////////////////////////
//  class DwrInlinedSubr
DwrInlinedSubr::DwrInlinedSubr (int64_t _abstract_origin, uint64_t _low_pc,
			    uint64_t _high_pc, int _file, int _line, int _level)
{
  abstract_origin = _abstract_origin;
  low_pc = _low_pc;
  high_pc = _high_pc;
  file = _file;
  line = _line;
  level = _level;
}

void
DwrInlinedSubr::dump ()
{
  Dprintf (DUMP_DWARFLIB,
	   "  level=%d  0x%08llx [0x%08llx - 0x%08llx]  file=%d line=%d\n",
	   (int) level, (long long) abstract_origin, (long long) low_pc,
	   (long long) high_pc, (int) file, (int) line);
}
