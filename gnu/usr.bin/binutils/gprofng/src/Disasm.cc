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
#include <stdio.h>
#include <string.h>
#include <sys/param.h>

#include "disassemble.h"
#include "dis-asm.h"
#include "demangle.h"
#include "dbe_types.h"
#include "DbeSession.h"
#include "Elf.h"
#include "Disasm.h"
#include "Stabs.h"
#include "i18n.h"
#include "util.h"
#include "StringBuilder.h"

struct DisContext
{
  bool is_Intel;
  Stabs *stabs;
  uint64_t pc;          // first_pc <= pc < last_pc
  uint64_t first_pc;
  uint64_t last_pc;
  uint64_t f_offset;    // file offset for first_pc
  int codeptr[4];       // longest instruction length may not be > 16
  Data_window *elf;
};

static const int MAX_DISASM_STR     = 2048;
static const int MAX_INSTR_SIZE     = 8;

Disasm::Disasm (char *fname)
{
  dwin = NULL;
  dis_str = NULL;
  need_swap_endian = false;
  my_stabs = Stabs::NewStabs (fname, fname);
  if (my_stabs == NULL)
    return;
  stabs = my_stabs;
  platform = stabs->get_platform ();
  disasm_open ();
}

Disasm::Disasm (Platform_t _platform, Stabs *_stabs)
{
  dwin = NULL;
  dis_str = NULL;
  need_swap_endian = false;
  stabs = _stabs;
  platform = _platform;
  my_stabs = NULL;
  disasm_open ();
}

static int
fprintf_func (void *arg, const char *fmt, ...)
{
  char buf[512];
  va_list vp;
  va_start (vp, fmt);
  int cnt = vsnprintf (buf, sizeof (buf), fmt, vp);
  va_end (vp);

  Disasm *dis = (Disasm *) arg;
  dis->dis_str->append (buf);
  return cnt;
}

static int
fprintf_styled_func (void *arg, enum disassembler_style st ATTRIBUTE_UNUSED,
		      const char *fmt, ...)
{
  char buf[512];
  va_list vp;
  va_start (vp, fmt);
  int cnt = vsnprintf (buf, sizeof (buf), fmt, vp);
  va_end (vp);

  Disasm *dis = (Disasm *) arg;
  dis->dis_str->append (buf);
  return cnt;
}

/* Get LENGTH bytes from info's buffer, at target address memaddr.
   Transfer them to myaddr.  */
static int
read_memory_func (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length,
		  disassemble_info *info)
{
  unsigned int opb = info->octets_per_byte;
  size_t end_addr_offset = length / opb;
  size_t max_addr_offset = info->buffer_length / opb;
  size_t octets = (memaddr - info->buffer_vma) * opb;
  if (memaddr < info->buffer_vma
      || memaddr - info->buffer_vma > max_addr_offset
      || memaddr - info->buffer_vma + end_addr_offset > max_addr_offset
      || (info->stop_vma && (memaddr >= info->stop_vma
			     || memaddr + end_addr_offset > info->stop_vma)))
    return -1;
  memcpy (myaddr, info->buffer + octets, length);
  return 0;
}

static void
print_address_func (bfd_vma addr, disassemble_info *info)
{
  (*info->fprintf_func) (info->stream, "0x%llx", (unsigned long long) addr);
}

static asymbol *
symbol_at_address_func (bfd_vma addr ATTRIBUTE_UNUSED,
			disassemble_info *info ATTRIBUTE_UNUSED)
{
  return NULL;
}

static bfd_boolean
symbol_is_valid (asymbol * sym ATTRIBUTE_UNUSED,
		 disassemble_info *info ATTRIBUTE_UNUSED)
{
  return TRUE;
}

static void
memory_error_func (int status, bfd_vma addr, disassemble_info *info)
{
  info->fprintf_func (info->stream, "Address 0x%llx is out of bounds.\n",
		      (unsigned long long) addr);
}

void
Disasm::disasm_open ()
{
  hex_visible = 1;
  snprintf (addr_fmt, sizeof (addr_fmt), NTXT ("%s"), NTXT ("%8llx:  "));
  if (dis_str == NULL)
    dis_str = new StringBuilder;

  switch (platform)
    {
    case Aarch64:
    case Intel:
    case Amd64:
      need_swap_endian = (DbeSession::platform == Sparc);
      break;
    case Sparcv8plus:
    case Sparcv9:
    case Sparc:
    default:
      need_swap_endian = (DbeSession::platform != Sparc);
      break;
    }

  memset (&dis_info, 0, sizeof (dis_info));
  dis_info.flavour = bfd_target_unknown_flavour;
  dis_info.endian = BFD_ENDIAN_UNKNOWN;
  dis_info.endian_code = dis_info.endian;
  dis_info.octets_per_byte = 1;
  dis_info.disassembler_needs_relocs = FALSE;
  dis_info.fprintf_func = fprintf_func;
  dis_info.fprintf_styled_func = fprintf_styled_func;
  dis_info.stream = this;
  dis_info.disassembler_options = NULL;
  dis_info.read_memory_func = read_memory_func;
  dis_info.memory_error_func = memory_error_func;
  dis_info.print_address_func = print_address_func;
  dis_info.symbol_at_address_func = symbol_at_address_func;
  dis_info.symbol_is_valid = symbol_is_valid;
  dis_info.display_endian = BFD_ENDIAN_UNKNOWN;
  dis_info.symtab = NULL;
  dis_info.symtab_size = 0;
  dis_info.buffer_vma = 0;
  switch (platform)
    {
    case Aarch64:
      dis_info.arch = bfd_arch_aarch64;
      dis_info.mach = bfd_mach_aarch64;
      break;
    case Intel:
    case Amd64:
      dis_info.arch = bfd_arch_i386;
      dis_info.mach = bfd_mach_x86_64;
      break;
    case Sparcv8plus:
    case Sparcv9:
    case Sparc:
    default:
      dis_info.arch = bfd_arch_unknown;
      dis_info.endian = BFD_ENDIAN_UNKNOWN;
      break;
    }
  dis_info.display_endian = dis_info.endian = BFD_ENDIAN_BIG;
  dis_info.display_endian = dis_info.endian = BFD_ENDIAN_LITTLE;
  dis_info.display_endian = dis_info.endian = BFD_ENDIAN_UNKNOWN;
  disassemble_init_for_target (&dis_info);
}

Disasm::~Disasm ()
{
  delete my_stabs;
  delete dwin;
  delete dis_str;
}

void
Disasm::set_img_name (char *img_fname)
{
  if (stabs == NULL && img_fname && dwin == NULL)
    {
      dwin = new Data_window (img_fname);
      if (dwin->not_opened ())
	{
	  delete dwin;
	  dwin = NULL;
	  return;
	}
      dwin->need_swap_endian = need_swap_endian;
    }
}

void
Disasm::remove_disasm_hndl (void *hndl)
{
  DisContext *ctx = (DisContext *) hndl;
  delete ctx;
}

#if 0
int
Disasm::get_instr_size (uint64_t vaddr, void *hndl)
{
  DisContext *ctx = (DisContext *) hndl;
  if (ctx == NULL || vaddr < ctx->first_pc || vaddr >= ctx->last_pc)
    return -1;
  ctx->pc = vaddr;
  size_t sz = ctx->is_Intel ? sizeof (ctx->codeptr) : 4;
  if (sz > ctx->last_pc - vaddr)
    sz = (size_t) (ctx->last_pc - vaddr);
  if (ctx->elf->get_data (ctx->f_offset + (vaddr - ctx->first_pc),
			  sz, ctx->codeptr) == NULL)
    return -1;

  char buf[MAX_DISASM_STR];
  *buf = 0;
  uint64_t inst_vaddr = vaddr;
#if MEZ_NEED_TO_FIX
  size_t instrs_cnt = 0;
  disasm_err_code_t status = disasm (handle, &inst_vaddr, ctx->last_pc, 1,
				     ctx, buf, sizeof (buf), &instrs_cnt);
  if (instrs_cnt != 1 || status != disasm_err_ok)
    return -1;
#endif
  return (int) (inst_vaddr - vaddr);
}
#endif

void
Disasm::set_addr_end (uint64_t end_address)
{
  char buf[32];
  int len = snprintf (buf, sizeof (buf), "%llx", (long long) end_address);
  snprintf (addr_fmt, sizeof (addr_fmt), "%%%dllx:  ", len < 8 ? 8 : len);
}

char *
Disasm::get_disasm (uint64_t inst_address, uint64_t end_address,
		  uint64_t start_address, uint64_t f_offset, int64_t &inst_size)
{
  inst_size = 0;
  if (inst_address >= end_address)
    return NULL;
  Data_window *dw = stabs ? stabs->openElf (false) : dwin;
  if (dw == NULL)
    return NULL;

  unsigned char buffer[MAX_DISASM_STR];
  dis_info.buffer = buffer;
  dis_info.buffer_length = end_address - inst_address;
  if (dis_info.buffer_length > sizeof (buffer))
    dis_info.buffer_length = sizeof (buffer);
  dw->get_data (f_offset + (inst_address - start_address),
		dis_info.buffer_length, dis_info.buffer);

  dis_str->setLength (0);
  bfd abfd;
  disassembler_ftype disassemble = disassembler (dis_info.arch, dis_info.endian,
						 dis_info.mach, &abfd);
  if (disassemble == NULL)
    {
      printf ("ERROR: unsupported disassemble\n");
      return NULL;
    }
  inst_size = disassemble (0, &dis_info);
  if (inst_size <= 0)
    {
      inst_size = 0;
      return NULL;
    }
  StringBuilder sb;
  sb.appendf (addr_fmt, inst_address); // Write address

  // Write hex bytes of instruction
  if (hex_visible)
    {
      char bytes[64];
      *bytes = '\0';
      for (int i = 0; i < inst_size; i++)
	{
	  unsigned int hex_value = buffer[i] & 0xff;
	  snprintf (bytes + 3 * i, sizeof (bytes) - 3 * i, "%02x ", hex_value);
	}
      const char *fmt = "%s   ";
      if (platform == Intel)
	fmt = "%-21s   "; // 21 = 3 * 7 - maximum instruction length on Intel
      sb.appendf (fmt, bytes);
    }
  sb.append (dis_str);
#if MEZ_NEED_TO_FIX
  // Write instruction
  if (ctx.is_Intel)  // longest instruction length for Intel is 7
    sb.appendf (NTXT ("%-7s %s"), parts_array[1], parts_array[2]);
  else  // longest instruction length for SPARC is 11
    sb.appendf (NTXT ("%-11s %s"), parts_array[1], parts_array[2]);
  if (strcmp (parts_array[1], NTXT ("call")) == 0)
    {
      if (strncmp (parts_array[2], NTXT ("0x"), 2) == 0)
	sb.append (GTXT ("\t! (Unable to determine target symbol)"));
    }
#endif
  return sb.toString ();
}

#if MEZ_NEED_TO_FIX
void *
Disasm::get_inst_ptr (disasm_handle_t, uint64_t vaddr, void *pass_through)
{
  // Actually it fetches only one instruction at a time for sparc,
  // and one byte at a time for intel.
  DisContext *ctx = (DisContext*) pass_through;
  size_t sz = ctx->is_Intel ? 1 : 4;
  if (vaddr + sz > ctx->last_pc)
    {
      ctx->codeptr[0] = -1;
      return ctx->codeptr;
    }
  if (ctx->elf->get_data (ctx->f_offset + (vaddr - ctx->first_pc), sz, ctx->codeptr) == NULL)
    {
      ctx->codeptr[0] = -1;
      return ctx->codeptr;
    }
  if (ctx->elf->need_swap_endian && !ctx->is_Intel)
    ctx->codeptr[0] = ctx->elf->decode (ctx->codeptr[0]);
  return ctx->codeptr;
}

// get a symbol name for an address
disasm_err_code_t
Disasm::get_sym_name (disasm_handle_t,          // an open disassembler handle
		      uint64_t target_address,  // the target virtual address
		      uint64_t inst_address,  // virtual address of instruction
					      // being disassembled
		      int use_relocation, // flag to use relocation information
		      char *buffer,             // places the symbol here
		      size_t buffer_size,       // limit on symbol length
		      int *,                    // sys/elf_{SPARC.386}.h
		      uint64_t *offset,       // from the symbol to the address
		      void *pass_through)       // disassembler context
{
  char buf[MAXPATHLEN];
  if (!use_relocation)
    return disasm_err_symbol;

  DisContext *ctxp = (DisContext*) pass_through;
  char *name = NULL;
  if (ctxp->stabs)
    {
      uint64_t addr = ctxp->f_offset + (inst_address - ctxp->first_pc);
      name = ctxp->stabs->sym_name (target_address, addr, use_relocation);
    }
  if (name == NULL)
    return disasm_err_symbol;

  char *s = NULL;
  if (*name == '_')
    s = cplus_demangle (name, DMGL_PARAMS);
  if (s)
    {
      snprintf (buffer, buffer_size, NTXT ("%s"), s);
      free (s);
    }
  else
    snprintf (buffer, buffer_size, NTXT ("%s"), name);

  *offset = 0;
  return disasm_err_ok;
}
#endif
