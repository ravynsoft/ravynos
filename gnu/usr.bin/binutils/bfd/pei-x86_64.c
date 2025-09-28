/* BFD back-end for Intel 386 PE IMAGE COFF files.
   Copyright (C) 2006-2023 Free Software Foundation, Inc.

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
   MA 02110-1301, USA.

   Written by Kai Tietz, OneVision Software GmbH&CoKg.  */

#include "sysdep.h"
#include "bfd.h"

#define TARGET_SYM		x86_64_pei_vec
#define TARGET_NAME		"pei-x86-64"
#define COFF_IMAGE_WITH_PE
#define COFF_WITH_PE
#define COFF_WITH_pex64
#define PCRELOFFSET		true
#if defined (USE_MINGW64_LEADING_UNDERSCORES)
#define TARGET_UNDERSCORE	'_'
#else
#define TARGET_UNDERSCORE	0
#endif
/* Long section names not allowed in executable images, only object files.  */
#define COFF_LONG_SECTION_NAMES 0
#define COFF_SUPPORT_GNU_LINKONCE
#define COFF_LONG_FILENAMES
#define PDATA_ROW_SIZE	(3 * 4)

#define COFF_SECTION_ALIGNMENT_ENTRIES \
{ COFF_SECTION_NAME_EXACT_MATCH (".bss"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".data"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".rdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".text"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 4 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".idata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_EXACT_MATCH (".pdata"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 2 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".debug"), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }, \
{ COFF_SECTION_NAME_PARTIAL_MATCH (".gnu.linkonce.wi."), \
  COFF_ALIGNMENT_FIELD_EMPTY, COFF_ALIGNMENT_FIELD_EMPTY, 0 }

/* Note we have to make sure not to include headers twice.
   Not all headers are wrapped in #ifdef guards, so we define
   PEI_HEADERS to prevent double including in coff-x86_64.c  */
#define PEI_HEADERS
#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "coff/x86_64.h"
#include "coff/internal.h"
#include "coff/pe.h"
#include "libcoff.h"
#include "libpei.h"
#include "libiberty.h"

#undef AOUTSZ
#define AOUTSZ		PEPAOUTSZ
#define PEAOUTHDR	PEPAOUTHDR

/* Name of registers according to SEH conventions.  */

static const char * const pex_regs[16] = {
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

/* Swap in a runtime function.  */

static void
pex64_get_runtime_function (bfd *abfd, struct pex64_runtime_function *rf,
			    const void *data)
{
  const struct external_pex64_runtime_function *ex_rf =
    (const struct external_pex64_runtime_function *) data;
  rf->rva_BeginAddress = bfd_get_32 (abfd, ex_rf->rva_BeginAddress);
  rf->rva_EndAddress = bfd_get_32 (abfd, ex_rf->rva_EndAddress);
  rf->rva_UnwindData =	bfd_get_32 (abfd, ex_rf->rva_UnwindData);
}

/* Swap in unwind info header.  */

static bool
pex64_get_unwind_info (bfd *abfd, struct pex64_unwind_info *ui,
		       void *data, void *data_end)
{
  struct external_pex64_unwind_info *ex_ui =
    (struct external_pex64_unwind_info *) data;
  bfd_byte *ex_dta = (bfd_byte *) data;
  bfd_byte *ex_dta_end = (bfd_byte *) data_end;

  memset (ui, 0, sizeof (struct pex64_unwind_info));

  if (ex_dta_end - ex_dta < 4)
    return false;

  ui->Version = PEX64_UWI_VERSION (ex_ui->Version_Flags);
  ui->Flags = PEX64_UWI_FLAGS (ex_ui->Version_Flags);
  ui->SizeOfPrologue = (bfd_vma) ex_ui->SizeOfPrologue;
  ui->CountOfCodes = (bfd_vma) ex_ui->CountOfCodes;
  ui->FrameRegister = PEX64_UWI_FRAMEREG (ex_ui->FrameRegisterOffset);
  ui->FrameOffset = PEX64_UWI_FRAMEOFF (ex_ui->FrameRegisterOffset);
  ui->sizeofUnwindCodes = PEX64_UWI_SIZEOF_UWCODE_ARRAY (ui->CountOfCodes);
  ui->SizeOfBlock = ui->sizeofUnwindCodes + 4;
  ui->rawUnwindCodes = ex_dta + 4;
  ui->rawUnwindCodesEnd = ex_dta_end;

  if ((size_t) (ex_dta_end - ex_dta) < ui->SizeOfBlock)
    return false;
  ex_dta += ui->SizeOfBlock;

  switch (ui->Flags)
    {
    case UNW_FLAG_CHAININFO:
      if (ex_dta_end - ex_dta < 12)
	return false;
      ui->rva_BeginAddress = bfd_get_32 (abfd, ex_dta + 0);
      ui->rva_EndAddress = bfd_get_32 (abfd, ex_dta + 4);
      ui->rva_UnwindData = bfd_get_32 (abfd, ex_dta + 8);
      ui->SizeOfBlock += 12;
      return true;
    case UNW_FLAG_EHANDLER:
    case UNW_FLAG_UHANDLER:
    case UNW_FLAG_FHANDLER:
      if (ex_dta_end - ex_dta < 4)
	return false;
      ui->rva_ExceptionHandler = bfd_get_32 (abfd, ex_dta);
      ui->SizeOfBlock += 4;
      return true;
    default:
      return true;
    }
}

/* Display unwind codes.  */

static void
pex64_xdata_print_uwd_codes (FILE *file, bfd *abfd,
			     struct pex64_unwind_info *ui,
			     struct pex64_runtime_function *rf)
{
  unsigned int i;
  unsigned int tmp; /* At least 32 bits.  */
  int save_allowed;

  if (ui->CountOfCodes == 0 || ui->rawUnwindCodes == NULL)
    return;

  /* According to UNWIND_CODE documentation:
      If an FP reg is used, the any unwind code taking an offset must only be
      used after the FP reg is established in the prolog.
     But there are counter examples of that in system dlls...  */
  save_allowed = true;

  i = 0;

  if ((size_t) (ui->rawUnwindCodesEnd - ui->rawUnwindCodes)
      < ui->CountOfCodes * 2)
    {
      fprintf (file, _("warning: corrupt unwind data\n"));
      return;
    }

  if (ui->Version == 2
      && PEX64_UNWCODE_CODE (ui->rawUnwindCodes[1]) == UWOP_EPILOG)
    {
      /* Display epilog opcode (whose docoding is not fully documented).
	 Looks to be designed to speed-up unwinding, as there is no need
	 to decode instruction flow if outside an epilog.  */
      unsigned int func_size = rf->rva_EndAddress - rf->rva_BeginAddress;

      fprintf (file, "\tv2 epilog (length: %02x) at pc+:",
	       ui->rawUnwindCodes[0]);

      if (PEX64_UNWCODE_INFO (ui->rawUnwindCodes[1]))
	fprintf (file, " 0x%x", func_size - ui->rawUnwindCodes[0]);

      i++;
      for (; i < ui->CountOfCodes; i++)
	{
	  const bfd_byte *dta = ui->rawUnwindCodes + 2 * i;
	  unsigned int off;

	  if (PEX64_UNWCODE_CODE (dta[1]) != UWOP_EPILOG)
	    break;
	  off = dta[0] | (PEX64_UNWCODE_INFO (dta[1]) << 8);
	  if (off == 0)
	    fprintf (file, " [pad]");
	  else
	    fprintf (file, " 0x%x", func_size - off);
	}
      fputc ('\n', file);
    }

  for (; i < ui->CountOfCodes; i++)
    {
      const bfd_byte *dta = ui->rawUnwindCodes + 2 * i;
      unsigned int info = PEX64_UNWCODE_INFO (dta[1]);
      int unexpected = false;

      fprintf (file, "\t  pc+0x%02x: ", (unsigned int) dta[0]);

      switch (PEX64_UNWCODE_CODE (dta[1]))
	{
	case UWOP_PUSH_NONVOL:
	  fprintf (file, "push %s", pex_regs[info]);
	  break;

	case UWOP_ALLOC_LARGE:
	  if (info == 0)
	    {
	      if (ui->rawUnwindCodesEnd - dta < 4)
		{
		  fprintf (file, _("warning: corrupt unwind data\n"));
		  return;
		}
	      tmp = bfd_get_16 (abfd, dta + 2) * 8;
	      i++;
	    }
	  else
	    {
	      if (ui->rawUnwindCodesEnd - dta < 6)
		{
		  fprintf (file, _("warning: corrupt unwind data\n"));
		  return;
		}
	      tmp = bfd_get_32 (abfd, dta + 2);
	      i += 2;
	    }
	  fprintf (file, "alloc large area: rsp = rsp - 0x%x", tmp);
	  break;

	case UWOP_ALLOC_SMALL:
	  fprintf (file, "alloc small area: rsp = rsp - 0x%x", (info + 1) * 8);
	  break;

	case UWOP_SET_FPREG:
	  /* According to the documentation, info field is unused.  */
	  fprintf (file, "FPReg: %s = rsp + 0x%x (info = 0x%x)",
		   pex_regs[ui->FrameRegister],
		   (unsigned int) ui->FrameOffset * 16, info);
	  unexpected = ui->FrameRegister == 0;
	  save_allowed = false;
	  break;

	case UWOP_SAVE_NONVOL:
	  if (ui->rawUnwindCodesEnd - dta < 4)
	    {
	      fprintf (file, _("warning: corrupt unwind data\n"));
	      return;
	    }
	  tmp = bfd_get_16 (abfd, dta + 2) * 8;
	  i++;
	  fprintf (file, "save %s at rsp + 0x%x", pex_regs[info], tmp);
	  unexpected = !save_allowed;
	  break;

	case UWOP_SAVE_NONVOL_FAR:
	  if (ui->rawUnwindCodesEnd - dta < 6)
	    {
	      fprintf (file, _("warning: corrupt unwind data\n"));
	      return;
	    }
	  tmp = bfd_get_32 (abfd, dta + 2);
	  i += 2;
	  fprintf (file, "save %s at rsp + 0x%x", pex_regs[info], tmp);
	  unexpected = !save_allowed;
	  break;

	case UWOP_SAVE_XMM:
	  if (ui->Version == 1)
	    {
	      if (ui->rawUnwindCodesEnd - dta < 4)
		{
		  fprintf (file, _("warning: corrupt unwind data\n"));
		  return;
		}
	      tmp = bfd_get_16 (abfd, dta + 2) * 8;
	      i++;
	      fprintf (file, "save mm%u at rsp + 0x%x", info, tmp);
	      unexpected = !save_allowed;
	    }
	  else if (ui->Version == 2)
	    {
	      fprintf (file, "epilog %02x %01x", dta[0], info);
	      unexpected = true;
	    }
	  break;

	case UWOP_SAVE_XMM_FAR:
	  if (ui->rawUnwindCodesEnd - dta < 6)
	    {
	      fprintf (file, _("warning: corrupt unwind data\n"));
	      return;
	    }
	  tmp = bfd_get_32 (abfd, dta + 2) * 8;
	  i += 2;
	  fprintf (file, "save mm%u at rsp + 0x%x", info, tmp);
	  unexpected = !save_allowed;
	  break;

	case UWOP_SAVE_XMM128:
	  if (ui->rawUnwindCodesEnd - dta < 4)
	    {
	      fprintf (file, _("warning: corrupt unwind data\n"));
	      return;
	    }
	  tmp = bfd_get_16 (abfd, dta + 2) * 16;
	  i++;
	  fprintf (file, "save xmm%u at rsp + 0x%x", info, tmp);
	  unexpected = !save_allowed;
	  break;

	case UWOP_SAVE_XMM128_FAR:
	  if (ui->rawUnwindCodesEnd - dta < 6)
	    {
	      fprintf (file, _("warning: corrupt unwind data\n"));
	      return;
	    }
	  tmp = bfd_get_32 (abfd, dta + 2) * 16;
	  i += 2;
	  fprintf (file, "save xmm%u at rsp + 0x%x", info, tmp);
	  unexpected = !save_allowed;
	  break;

	case UWOP_PUSH_MACHFRAME:
	  fprintf (file, "interrupt entry (SS, old RSP, EFLAGS, CS, RIP");
	  if (info == 0)
	    fprintf (file, ")");
	  else if (info == 1)
	    fprintf (file, ",ErrorCode)");
	  else
	    fprintf (file, ", unknown(%u))", info);
	  break;

	default:
	  /* PR 17512: file: 2245-7442-0.004.  */
	  fprintf (file, _("Unknown: %x"), PEX64_UNWCODE_CODE (dta[1]));
	  break;
	}

      if (unexpected)
	fprintf (file, " [Unexpected!]");
      fputc ('\n', file);
    }
}

/* Check wether section SEC_NAME contains the xdata at address ADDR.  */

static asection *
pex64_get_section_by_rva (bfd *abfd, bfd_vma addr, const char *sec_name)
{
  asection *section = bfd_get_section_by_name (abfd, sec_name);
  bfd_vma vsize;
  bfd_size_type datasize = 0;

  if (section == NULL
      || coff_section_data (abfd, section) == NULL
      || pei_section_data (abfd, section) == NULL)
    return NULL;
  vsize = section->vma - pe_data (abfd)->pe_opthdr.ImageBase;
  datasize = section->size;
  if (!datasize || vsize > addr || (vsize + datasize) < addr)
    return NULL;
  return section;
}

/* Dump xdata at for function RF to FILE.  The argument XDATA_SECTION
   designate the bfd section containing the xdata, XDATA is its content,
   and ENDX the size if known (or NULL).  */

static void
pex64_dump_xdata (FILE *file, bfd *abfd,
		  asection *xdata_section, bfd_byte *xdata, bfd_vma *endx,
		  struct pex64_runtime_function *rf)
{
  bfd_vma vaddr;
  bfd_vma end_addr;
  bfd_vma addr = rf->rva_UnwindData;
  bfd_size_type sec_size = xdata_section->rawsize > 0 ? xdata_section->rawsize : xdata_section->size;
  struct pex64_unwind_info ui;

  vaddr = xdata_section->vma - pe_data (abfd)->pe_opthdr.ImageBase;
  addr -= vaddr;

  /* PR 17512: file: 2245-7442-0.004.  */
  if (addr >= sec_size)
    {
      fprintf (file, _("warning: xdata section corrupt\n"));
      return;
    }

  if (endx)
    {
      end_addr = endx[0] - vaddr;
      /* PR 17512: file: 2245-7442-0.004.  */
      if (end_addr > sec_size)
	{
	  fprintf (file, _("warning: xdata section corrupt\n"));
	  end_addr = sec_size;
	}
    }
  else
    end_addr = sec_size;

  if (! pex64_get_unwind_info (abfd, &ui, xdata + addr, xdata + end_addr))
    {
      fprintf (file, _("warning: xdata section corrupt\n"));
      return;
    }

  if (ui.Version != 1 && ui.Version != 2)
    {
      unsigned int i;
      fprintf (file, "\tVersion %u (unknown).\n",
	       (unsigned int) ui.Version);
      for (i = 0; addr < end_addr; addr += 1, i++)
	{
	  if ((i & 15) == 0)
	    fprintf (file, "\t  %03x:", i);
	  fprintf (file, " %02x", xdata[addr]);
	  if ((i & 15) == 15)
	    fprintf (file, "\n");
	}
      if ((i & 15) != 0)
	fprintf (file, "\n");
      return;
    }

  fprintf (file, "\tVersion: %d, Flags: ", ui.Version);
  switch (ui.Flags)
    {
    case UNW_FLAG_NHANDLER:
      fprintf (file, "none");
      break;
    case UNW_FLAG_EHANDLER:
      fprintf (file, "UNW_FLAG_EHANDLER");
      break;
    case UNW_FLAG_UHANDLER:
      fprintf (file, "UNW_FLAG_UHANDLER");
      break;
    case UNW_FLAG_FHANDLER:
      fprintf
	(file, "UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER");
      break;
    case UNW_FLAG_CHAININFO:
      fprintf (file, "UNW_FLAG_CHAININFO");
      break;
    default:
      fprintf (file, "unknown flags value 0x%x", (unsigned int) ui.Flags);
      break;
    }
  fputc ('\n', file);
  fprintf (file, "\tNbr codes: %u, ", (unsigned int) ui.CountOfCodes);
  fprintf (file, "Prologue size: 0x%02x, Frame offset: 0x%x, ",
	   (unsigned int) ui.SizeOfPrologue, (unsigned int) ui.FrameOffset);
  fprintf (file, "Frame reg: %s\n",
	   ui.FrameRegister == 0 ? "none"
	   : pex_regs[(unsigned int) ui.FrameRegister]);

  /* PR 17512: file: 2245-7442-0.004.  */
  if (ui.CountOfCodes * 2 + ui.rawUnwindCodes > xdata + xdata_section->size)
    fprintf (file, _("Too many unwind codes (%ld)\n"), (long) ui.CountOfCodes);
  else
    pex64_xdata_print_uwd_codes (file, abfd, &ui, rf);

  switch (ui.Flags)
    {
    case UNW_FLAG_EHANDLER:
    case UNW_FLAG_UHANDLER:
    case UNW_FLAG_FHANDLER:
      fprintf (file, "\tHandler: %016" PRIx64 ".\n",
	       ui.rva_ExceptionHandler + pe_data (abfd)->pe_opthdr.ImageBase);
      break;
    case UNW_FLAG_CHAININFO:
      fprintf (file, "\tChain: start: %016" PRIx64 ", end: %016" PRIx64,
	       ui.rva_BeginAddress, ui.rva_EndAddress);
      fprintf (file, "\n\t unwind data: %016" PRIx64 ".\n",
	       ui.rva_UnwindData);
      break;
    }

  /* Now we need end of this xdata block.  */
  addr += ui.SizeOfBlock;
  if (addr < end_addr)
    {
      unsigned int i;
      fprintf (file,"\tUser data:\n");
      for (i = 0; addr < end_addr; addr += 1, i++)
	{
	  if ((i & 15) == 0)
	    fprintf (file, "\t  %03x:", i);
	  fprintf (file, " %02x", xdata[addr]);
	  if ((i & 15) == 15)
	    fprintf (file, "\n");
	}
      if ((i & 15) != 0)
	fprintf (file, "\n");
    }
}

/* Helper function to sort xdata.  The entries of xdata are sorted to know
   the size of each entry.  */

static int
sort_xdata_arr (const void *l, const void *r)
{
  const bfd_vma *lp = (const bfd_vma *) l;
  const bfd_vma *rp = (const bfd_vma *) r;

  if (*lp == *rp)
    return 0;
  return (*lp < *rp ? -1 : 1);
}

/* Display unwind tables for x86-64.  */

static bool
pex64_bfd_print_pdata_section (bfd *abfd, void *vfile, asection *pdata_section)
{
  FILE *file = (FILE *) vfile;
  bfd_byte *pdata = NULL;
  bfd_byte *xdata = NULL;
  asection *xdata_section = NULL;
  bfd_vma xdata_base;
  bfd_size_type i;
  bfd_size_type datasize;
  bfd_size_type stop;
  bfd_vma prev_beginaddress = (bfd_vma) -1;
  bfd_vma prev_unwinddata_rva = (bfd_vma) -1;
  bfd_vma imagebase;
  int onaline = PDATA_ROW_SIZE;
  int seen_error = 0;
  bfd_vma *xdata_arr = NULL;
  int xdata_arr_cnt;
  bool virt_size_is_zero = false;

  /* Sanity checks.  */
  if (pdata_section == NULL
      || (pdata_section->flags & SEC_HAS_CONTENTS) == 0
      || coff_section_data (abfd, pdata_section) == NULL
      || pei_section_data (abfd, pdata_section) == NULL)
    return true;

  stop = pei_section_data (abfd, pdata_section)->virt_size;
  if ((stop % onaline) != 0)
    fprintf (file,
	     /* xgettext:c-format */
	     _("Warning: %s section size (%ld) is not a multiple of %d\n"),
	     pdata_section->name, (long) stop, onaline);

  datasize = pdata_section->size;
  if (datasize == 0)
    {
      if (stop)
	fprintf (file, _("Warning: %s section size is zero\n"),
		 pdata_section->name);
      return true;
    }

  /* virt_size might be zero for objects.  */
  if (stop == 0 && strcmp (abfd->xvec->name, "pe-x86-64") == 0)
    {
      stop = datasize;
      virt_size_is_zero = true;
    }
  else if (datasize < stop)
      {
	fprintf (file,
		 /* xgettext:c-format */
		 _("Warning: %s section size (%ld) is smaller than virtual size (%ld)\n"),
		 pdata_section->name, (unsigned long) datasize,
		 (unsigned long) stop);
	/* Be sure not to read past datasize.  */
	stop = datasize;
      }

  /* Display functions table.  */
  fprintf (file,
	   _("\nThe Function Table (interpreted %s section contents)\n"),
	   pdata_section->name);

  fprintf (file, _("vma:\t\t\tBeginAddress\t EndAddress\t  UnwindData\n"));

  if (!bfd_malloc_and_get_section (abfd, pdata_section, &pdata))
    goto done;

  /* Table of xdata entries.  */
  xdata_arr = (bfd_vma *) xmalloc (sizeof (bfd_vma) * ((stop / onaline) + 1));
  xdata_arr_cnt = 0;

  if (strcmp (abfd->xvec->name, "pei-x86-64") == 0)
    imagebase = pe_data (abfd)->pe_opthdr.ImageBase;
  else
    imagebase = 0;

  for (i = 0; i < stop; i += onaline)
    {
      struct pex64_runtime_function rf;

      if (i + PDATA_ROW_SIZE > stop)
	break;

      pex64_get_runtime_function (abfd, &rf, &pdata[i]);

      if (rf.rva_BeginAddress == 0 && rf.rva_EndAddress == 0
	  && rf.rva_UnwindData == 0)
	/* We are probably into the padding of the section now.  */
	break;
      fprintf (file, " %016" PRIx64, i + pdata_section->vma);
      fprintf (file, ":\t%016" PRIx64, imagebase + rf.rva_BeginAddress);
      fprintf (file, " %016" PRIx64, imagebase + rf.rva_EndAddress);
      fprintf (file, " %016" PRIx64 "\n", imagebase + rf.rva_UnwindData);
      if (i != 0 && rf.rva_BeginAddress <= prev_beginaddress)
	{
	  seen_error = 1;
	  fprintf (file, "  has %s begin address as predecessor\n",
	    (rf.rva_BeginAddress < prev_beginaddress ? "smaller" : "same"));
	}
      prev_beginaddress = rf.rva_BeginAddress;
      /* Now we check for negative addresses.  */
      if ((prev_beginaddress & 0x80000000) != 0)
	{
	  seen_error = 1;
	  fprintf (file, "  has negative begin address\n");
	}
      if ((rf.rva_EndAddress & 0x80000000) != 0)
	{
	  seen_error = 1;
	  fprintf (file, "  has negative end address\n");
	}
      if ((rf.rva_UnwindData & 0x80000000) != 0)
	{
	  seen_error = 1;
	  fprintf (file, "  has negative unwind address\n");
	}
      else if ((rf.rva_UnwindData && !PEX64_IS_RUNTIME_FUNCTION_CHAINED (&rf))
		|| virt_size_is_zero)
	xdata_arr[xdata_arr_cnt++] = rf.rva_UnwindData;
    }

  if (seen_error)
    goto done;

  /* Add end of list marker.  */
  xdata_arr[xdata_arr_cnt++] = ~((bfd_vma) 0);

  /* Sort start RVAs of xdata.  */
  if (xdata_arr_cnt > 1)
    qsort (xdata_arr, (size_t) xdata_arr_cnt, sizeof (bfd_vma),
	   sort_xdata_arr);

  /* Find the section containing the unwind data (.xdata).  */
  xdata_base = xdata_arr[0];
  /* For sections with long names, first look for the same
     section name, replacing .pdata by .xdata prefix.  */
  if (strcmp (pdata_section->name, ".pdata") != 0)
    {
      size_t len = strlen (pdata_section->name);
      char *xdata_name = xmalloc (len + 1);

      xdata_name = memcpy (xdata_name, pdata_section->name, len + 1);
      /* Transform .pdata prefix into .xdata prefix.  */
      if (len > 1)
	xdata_name [1] = 'x';
      xdata_section = pex64_get_section_by_rva (abfd, xdata_base,
						xdata_name);
      free (xdata_name);
    }
  /* Second, try the .xdata section itself.  */
  if (!xdata_section)
    xdata_section = pex64_get_section_by_rva (abfd, xdata_base, ".xdata");
  /* Otherwise, if xdata_base is non zero, search also inside
     other standard sections.  */
  if (!xdata_section && xdata_base)
    xdata_section = pex64_get_section_by_rva (abfd, xdata_base, ".rdata");
  if (!xdata_section && xdata_base)
    xdata_section = pex64_get_section_by_rva (abfd, xdata_base, ".data");
  if (!xdata_section && xdata_base)
    xdata_section = pex64_get_section_by_rva (abfd, xdata_base, ".pdata");
  if (!xdata_section && xdata_base)
    xdata_section = pex64_get_section_by_rva (abfd, xdata_base, ".text");
  /* Transfer xdata section into xdata array.  */
  if (!xdata_section
      || (xdata_section->flags & SEC_HAS_CONTENTS) == 0
      || !bfd_malloc_and_get_section (abfd, xdata_section, &xdata))
    goto done;

  /* Avoid "also used "... ouput for single unwind info
     in object file.  */
  prev_unwinddata_rva = (bfd_vma) -1;

  /* Do dump of pdata related xdata.  */
  for (i = 0; i < stop; i += onaline)
    {
      struct pex64_runtime_function rf;

      if (i + PDATA_ROW_SIZE > stop)
	break;

      pex64_get_runtime_function (abfd, &rf, &pdata[i]);

      if (rf.rva_BeginAddress == 0 && rf.rva_EndAddress == 0
	  && rf.rva_UnwindData == 0)
	/* We are probably into the padding of the section now.  */
	break;
      if (i == 0)
	fprintf (file, _("\nDump of %s\n"), xdata_section->name);

      fprintf (file, " %016" PRIx64, rf.rva_UnwindData + imagebase);

      if (prev_unwinddata_rva == rf.rva_UnwindData)
	{
	  /* Do not dump again the xdata for the same entry.  */
	  fprintf (file, " also used for function at %016" PRIx64 "\n",
		   rf.rva_BeginAddress + imagebase);
	  continue;
	}
      else
	prev_unwinddata_rva = rf.rva_UnwindData;

      fprintf (file, " (rva: %08x): %016" PRIx64 " - %016" PRIx64 "\n",
	       (unsigned int) rf.rva_UnwindData,
	       rf.rva_BeginAddress + imagebase,
	       rf.rva_EndAddress + imagebase);

      if (rf.rva_UnwindData != 0 || virt_size_is_zero)
	{
	  if (PEX64_IS_RUNTIME_FUNCTION_CHAINED (&rf))
	    {
	      bfd_vma altent = PEX64_GET_UNWINDDATA_UNIFIED_RVA (&rf);
	      bfd_vma pdata_vma = bfd_section_vma (pdata_section);
	      struct pex64_runtime_function arf;

	      fprintf (file, "\t shares information with ");
	      altent += imagebase;

	      if (altent >= pdata_vma
		  && altent - pdata_vma + PDATA_ROW_SIZE <= stop)
		{
		  pex64_get_runtime_function
		    (abfd, &arf, &pdata[altent - pdata_vma]);
		  fprintf (file, "pdata element at 0x%016" PRIx64,
			   arf.rva_UnwindData);
		}
	      else
		fprintf (file, "unknown pdata element");
	      fprintf (file, ".\n");
	    }
	  else
	    {
	      bfd_vma *p;

	      /* Search for the current entry in the sorted array.  */
	      p = (bfd_vma *)
		  bsearch (&rf.rva_UnwindData, xdata_arr,
			   (size_t) xdata_arr_cnt, sizeof (bfd_vma),
			   sort_xdata_arr);

	      /* Advance to the next pointer into the xdata section.  We may
		 have shared xdata entries, which will result in a string of
		 identical pointers in the array; advance past all of them.  */
	      while (p[0] <= rf.rva_UnwindData)
		++p;

	      if (p[0] == ~((bfd_vma) 0))
		p = NULL;

	      pex64_dump_xdata (file, abfd, xdata_section, xdata, p, &rf);
	    }
	}
    }

 done:
  free (pdata);
  free (xdata_arr);
  free (xdata);

  return true;
}

struct pex64_paps
{
  void *obj;
  /* Number of found pdata sections.  */
  unsigned int pdata_count;
};

/* Functionn prototype.  */
bool pex64_bfd_print_pdata (bfd *, void *);

/* Helper function for bfd_map_over_section.  */
static void
pex64_print_all_pdata_sections (bfd *abfd, asection *pdata, void *arg)
{
  struct pex64_paps *paps = arg;
  if (startswith (pdata->name, ".pdata"))
    {
      if (pex64_bfd_print_pdata_section (abfd, paps->obj, pdata))
	paps->pdata_count++;
    }
}

bool
pex64_bfd_print_pdata (bfd *abfd, void *vfile)
{
  asection *pdata_section = bfd_get_section_by_name (abfd, ".pdata");
  struct pex64_paps paps;

  if (pdata_section)
    return pex64_bfd_print_pdata_section (abfd, vfile, pdata_section);

  paps.obj = vfile;
  paps.pdata_count = 0;
  bfd_map_over_sections (abfd, pex64_print_all_pdata_sections, &paps);
  return paps.pdata_count != 0;
}

#define bfd_pe_print_pdata   pex64_bfd_print_pdata
#define bfd_coff_std_swap_table bfd_coff_pei_swap_table

#include "coff-x86_64.c"
