/* od-pe.c -- dump information about a PE object file.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold, Adacore and Nick Clifton, Red Hat.

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

#include "sysdep.h"
#include <stddef.h>
#include <time.h>
#include "safe-ctype.h"
#include "bfd.h"
#include "objdump.h"
#include "bucomm.h"
#include "bfdlink.h"
#include "coff/internal.h"
#define L_LNNO_SIZE 4 /* FIXME: which value should we use ?  */
#include "coff/external.h"
#include "coff/pe.h"
#include "libcoff.h"
#include "libpei.h"
#include "libiberty.h"

/* Index of the options in the options[] array.  */
#define OPT_FILE_HEADER 0
#define OPT_AOUT 1
#define OPT_SECTIONS 2
#define OPT_SYMS 3
#define OPT_RELOCS 4
#define OPT_LINENO 5
#define OPT_LOADER 6
#define OPT_EXCEPT 7
#define OPT_TYPCHK 8
#define OPT_TRACEBACK 9
#define OPT_TOC 10
#define OPT_LDINFO 11

/* List of actions.  */
static struct objdump_private_option options[] =
{
  { "header", 0 },
  { "aout", 0 },
  { "sections", 0 },
  { "syms", 0 },
  { "relocs", 0 },
  { "lineno", 0 },
  { "loader", 0 },
  { "except", 0 },
  { "typchk", 0 },
  { "traceback", 0 },
  { "toc", 0 },
  { "ldinfo", 0 },
  { NULL, 0 }
};

/* Simplified section header.  */
struct pe_section
{
  /* NUL terminated name.  */
  char name[9];

  /* Section flags.  */
  unsigned int flags;

  /* Offsets in file.  */
  ufile_ptr scnptr;
  ufile_ptr relptr;
  ufile_ptr lnnoptr;

  /* Number of relocs and line numbers.  */
  unsigned int nreloc;
  unsigned int nlnno;
};

/* Translation entry type.  The last entry must be {0, NULL}.  */

struct xlat_table
{
  unsigned int  val;
  const char *  name;
};

/* PE file flags.  */
static const struct xlat_table file_flag_xlat[] =
{
  { IMAGE_FILE_RELOCS_STRIPPED,     "RELOCS STRIPPED"},
  { IMAGE_FILE_EXECUTABLE_IMAGE,    "EXECUTABLE"},
  { IMAGE_FILE_LINE_NUMS_STRIPPED,  "LINE NUMS STRIPPED"},
  { IMAGE_FILE_LOCAL_SYMS_STRIPPED, "LOCAL SYMS STRIPPED"},
  { IMAGE_FILE_AGGRESSIVE_WS_TRIM,  "AGGRESSIVE WS TRIM"},
  { IMAGE_FILE_LARGE_ADDRESS_AWARE, "LARGE ADDRESS AWARE"},
  { IMAGE_FILE_16BIT_MACHINE,       "16BIT MACHINE"},
  { IMAGE_FILE_BYTES_REVERSED_LO,   "BYTES REVERSED LO"},
  { IMAGE_FILE_32BIT_MACHINE,       "32BIT MACHINE"},
  { IMAGE_FILE_DEBUG_STRIPPED,      "DEBUG STRIPPED"},
  { IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP, "REMOVABLE RUN FROM SWAP"},
  { IMAGE_FILE_NET_RUN_FROM_SWAP,   "NET RUN FROM SWAP"},
  { IMAGE_FILE_SYSTEM,              "SYSTEM"},
  { IMAGE_FILE_DLL,                 "DLL"},
  { IMAGE_FILE_UP_SYSTEM_ONLY,      "UP SYSTEM ONLY"},
  { IMAGE_FILE_BYTES_REVERSED_HI,   "BYTES REVERSED HI"},
  { 0, NULL }
};

/* PE section flags.  */
static const struct xlat_table section_flag_xlat[] =
{
  { IMAGE_SCN_MEM_DISCARDABLE, "DISCARDABLE" },
  { IMAGE_SCN_MEM_EXECUTE,     "EXECUTE" },
  { IMAGE_SCN_MEM_READ,        "READ" },
  { IMAGE_SCN_MEM_WRITE,       "WRITE" },
  { IMAGE_SCN_TYPE_NO_PAD,     "NO PAD" },
  { IMAGE_SCN_CNT_CODE,        "CODE" },
  { IMAGE_SCN_CNT_INITIALIZED_DATA,   "INITIALIZED DATA" },
  { IMAGE_SCN_CNT_UNINITIALIZED_DATA, "UNINITIALIZED DATA" },
  { IMAGE_SCN_LNK_OTHER,       "OTHER" },
  { IMAGE_SCN_LNK_INFO,        "INFO" },
  { IMAGE_SCN_LNK_REMOVE,      "REMOVE" },
  { IMAGE_SCN_LNK_COMDAT,      "COMDAT" },
  { IMAGE_SCN_MEM_FARDATA,     "FARDATA" },
  { IMAGE_SCN_MEM_PURGEABLE,   "PURGEABLE" },
  { IMAGE_SCN_MEM_LOCKED,      "LOCKED" },
  { IMAGE_SCN_MEM_PRELOAD,     "PRELOAD" },
  { IMAGE_SCN_LNK_NRELOC_OVFL, "NRELOC OVFL" },
  { IMAGE_SCN_MEM_NOT_CACHED,  "NOT CACHED" },
  { IMAGE_SCN_MEM_NOT_PAGED,   "NOT PAGED" },
  { IMAGE_SCN_MEM_SHARED,      "SHARED" },    
  { 0, NULL }
};

typedef struct target_specific_info
{
  unsigned int  machine_number;
  const char *  name;
  unsigned int  aout_hdr_size;
} target_specific_info;

const struct target_specific_info targ_info[] =
{
  { IMAGE_FILE_MACHINE_ALPHA, "ALPHA", 80 },
  { IMAGE_FILE_MACHINE_ALPHA64, "ALPHA64", 80 },
  { IMAGE_FILE_MACHINE_AM33, "AM33", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_AMD64, "AMD64", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_ARM, "ARM", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_ARM64, "ARM64", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_ARMNT, "ARM NT", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_CEE, "CEE", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_CEF, "CEF", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_EBC, "EBC", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_I386, "I386", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_IA64, "IA64", 108 },
  { IMAGE_FILE_MACHINE_LOONGARCH64, "LOONGARCH64", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_M32R, "M32R", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_M68K, "M68K", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_MIPS16, "MIPS16", 56 },
  { IMAGE_FILE_MACHINE_MIPSFPU, "MIPSFPU", 56 },
  { IMAGE_FILE_MACHINE_MIPSFPU16, "MIPSFPU16", 56 },
  { IMAGE_FILE_MACHINE_POWERPC, "POWERPC", 72 },
  { IMAGE_FILE_MACHINE_POWERPCFP, "POWERPCFP", 72 },
  { IMAGE_FILE_MACHINE_R10000, "R10000", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_R3000, "R3000", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_R4000, "R4000", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_SH3, "SH3", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_SH3DSP, "SH3DSP", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_SH3E, "SH3E", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_SH4, "SH4", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_SH5, "SH5", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_THUMB, "THUMB", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_TRICORE, "TRICORE", AOUTHDRSZ },
  { IMAGE_FILE_MACHINE_WCEMIPSV2, "WCEMIPSV2", AOUTHDRSZ },

  { 0x0093, "TI C4X", 28 },
  { 0x00C1, "TI C4X", 28 },
  { 0x00C2, "TI C4X", 28 },
  { 0x0500, "SH (big endian)", AOUTHDRSZ },
  { 0x0550, "SH (little endian)", AOUTHDRSZ },
  { 0x0a00, "ARM", AOUTHDRSZ },
  { 0x0b00, "MCore", AOUTHDRSZ }
};

static const struct target_specific_info unknown_info = { 0, "unknown", AOUTHDRSZ };

static const struct target_specific_info *
get_target_specific_info (unsigned int machine)
{
  unsigned int i;

  for (i = ARRAY_SIZE (targ_info); i--;)
    if (targ_info[i].machine_number == machine)
      return targ_info + i;

  return & unknown_info;
}

/* Display help.  */

static void
pe_help (FILE *stream)
{
  fprintf (stream, _("\
For PE files:\n\
  header      Display the file header\n\
  sections    Display the section headers\n\
"));
}

/* Return true if ABFD is handled.  */

static int
pe_filter (bfd *abfd)
{
  return bfd_get_flavour (abfd) == bfd_target_coff_flavour;
}

/* Display the list of name (from TABLE) for FLAGS, using comma to
   separate them.  A name is displayed if FLAGS & VAL is not 0.  */

static void
dump_flags (const struct xlat_table * table, unsigned int flags)
{
  unsigned int r = flags;
  bool first = true;
  const struct xlat_table *t;

  for (t = table; t->name; t++)
    if ((flags & t->val) != 0)
      {
        r &= ~t->val;

        if (first)
          first = false;
        else
          putchar (',');
        fputs (t->name, stdout);
      }

  /* Undecoded flags.  */
  if (r != 0)
    {
      if (!first)
        putchar (',');
      printf (_("unknown: 0x%x"), r);
    }
}

/* Dump the file header.  */

static void
dump_pe_file_header (bfd *                            abfd,
		     struct external_PEI_filehdr *    fhdr,
		     struct external_PEI_IMAGE_hdr *  ihdr)
{
  unsigned int data;
  unsigned long ldata;
  unsigned long ihdr_off = 0;

  if (fhdr == NULL)
    printf (_("\n  File header not present\n"));
  else
    {
      printf (_("\n  File Header (at offset 0):\n"));

      // The values of the following fields are normally fixed.
      // But we display them anyway, in case there are discrepancies.

      data = bfd_h_get_16 (abfd, fhdr->e_cblp);
      printf (_("Bytes on Last Page:\t\t%d\n"), data);

      data = bfd_h_get_16 (abfd, fhdr->e_cp);
      printf (_("Pages In File:\t\t\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_crlc);
      printf (_("Relocations:\t\t\t%d\n"), data);

      data = bfd_h_get_16 (abfd, fhdr->e_cparhdr);
      printf (_("Size of header in paragraphs:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_minalloc);
      printf (_("Min extra paragraphs needed:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_maxalloc);
      printf (_("Max extra paragraphs needed:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_ss);
      printf (_("Initial (relative) SS value:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_sp);
      printf (_("Initial SP value:\t\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_csum);
      printf (_("Checksum:\t\t\t%#x\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_ip);
      printf (_("Initial IP value:\t\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_cs);
      printf (_("Initial (relative) CS value:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_lfarlc);
      printf (_("File address of reloc table:\t%d\n"), data);
      
      data = bfd_h_get_16 (abfd, fhdr->e_ovno);
      printf (_("Overlay number:\t\t\t%d\n"), data);

      data = bfd_h_get_16 (abfd, fhdr->e_oemid);
      printf (_("OEM identifier:\t\t\t%d\n"), data);
  
      data = bfd_h_get_16 (abfd, fhdr->e_oeminfo);
      printf (_("OEM information:\t\t%#x\n"), data);
  
      ldata = bfd_h_get_32 (abfd, fhdr->e_lfanew);
      printf (_("File address of new exe header:\t%#lx\n"), ldata);
        
      /* Display the first string found in the stub.
	 FIXME: Look for more than one string ?
	 FIXME: Strictly speaking we may not have read the full stub, since
	 it can be longer than the dos_message array in the PEI_fileheader
	 structure.  */
      const unsigned char * message = (const unsigned char *) fhdr->dos_message;
      unsigned int len = sizeof (fhdr->dos_message);
      unsigned int i;
      unsigned int seen_count = 0;
      unsigned int string_start = 0;
  
      for (i = 0; i < len; i++)
	{
	  if (ISPRINT (message[i]))
	    {
	      if (string_start == 0)
		string_start = i;
	      ++ seen_count;
	      if (seen_count > 4)
		break;
	    }
	  else
	    {
	      seen_count = string_start = 0;
	    }
	}

      if (seen_count > 4)
	{
	  printf (_("Stub message:\t\t\t"));
	  while (string_start < len)
	    {
	      char c = message[string_start ++];
	      if (! ISPRINT (c))
		break;
	      putchar (c);
	    }
	  putchar ('\n');
	}

      ihdr_off = (long) bfd_h_get_32 (abfd, fhdr->e_lfanew);
    }

  printf (_("\n  Image Header (at offset %#lx):\n"), ihdr_off);

  /* Note - we try to make this output use the same format as the output from -p.
     But since there are multiple headers to display and the order of the fields
     in the headers do not match the order of information displayed by -p, there
     are some discrepancies.  */

  unsigned int machine = (int) bfd_h_get_16 (abfd, ihdr->f_magic);
  printf (_("Machine Number:\t\t\t%#x\t\t- %s\n"), machine,
	  get_target_specific_info (machine)->name);

  printf (_("Number of sections:\t\t\%d\n"), (int) bfd_h_get_16 (abfd, ihdr->f_nscns));

  long timedat = bfd_h_get_32 (abfd, ihdr->f_timdat);
  printf (_("Time/Date:\t\t\t%#08lx\t- "), timedat);
  if (timedat == 0)
    printf (_("not set\n"));
  else
    {
      /* Not correct on all platforms, but works on unix.  */
      time_t t = timedat;
      fputs (ctime (& t), stdout);
    }
  
  printf (_("Symbol table offset:\t\t%#08lx\n"),
	  (long) bfd_h_get_32 (abfd, ihdr->f_symptr));
  printf (_("Number of symbols:\t\t\%ld\n"),
	  (long) bfd_h_get_32 (abfd, ihdr->f_nsyms));

  unsigned int opt_header_size = (int) bfd_h_get_16 (abfd, ihdr->f_opthdr);
  printf (_("Optional header size:\t\t%#x\n"), opt_header_size);

  unsigned int flags = (int) bfd_h_get_16 (abfd, ihdr->f_flags);
  printf (_("Flags:\t\t\t\t0x%04x\t\t- "), flags);
  dump_flags (file_flag_xlat, flags);
  putchar ('\n');

  if (opt_header_size == PEPAOUTSZ)
    {
      PEPAOUTHDR xhdr;

      printf (_("\n  Optional 64-bit AOUT Header (at offset %#lx):\n"),
	      ihdr_off + sizeof (* ihdr));

      // Fortunately, it appears that the size and layout of the
      // PEPAOUTHDR header is consistent across all architectures.
      if (bfd_seek (abfd, ihdr_off + sizeof (* ihdr), SEEK_SET) != 0
	  || bfd_bread (& xhdr, sizeof (xhdr), abfd) != sizeof (xhdr))
	printf (_("error: unable to read AOUT and PE+ headers\n"));
      else
	{
	  data = (int) bfd_h_get_16 (abfd, xhdr.standard.magic);
	  printf (_("Magic:\t\t\t\t%x\t\t- %s\n"), data,
		    data == 0x020b ? "PE32+" : _("Unknown"));
	  
	  printf (_("Version:\t\t\t%x\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.standard.vstamp));

	  printf (_("Text Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.tsize));
	  printf (_("Data Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.dsize));
	  printf (_("BSS Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.bsize));
	  printf (_("Entry Point:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.entry));
	  printf (_("Text Start:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.text_start));
	  /* There is no data_start field in the PE+ standard header.  */

	  printf (_("\n  Optional PE+ Header (at offset %#lx):\n"),
		  ihdr_off + sizeof (* ihdr) + sizeof (xhdr.standard));

	  printf (_("Image Base:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.ImageBase));
	  printf (_("Section Alignment:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SectionAlignment));
	  printf (_("File Alignment:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.FileAlignment));
	  printf (_("Major OS Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorOperatingSystemVersion));
	  printf (_("Minor OS ersion:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorOperatingSystemVersion));
	  printf (_("Major Image Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorImageVersion));
	  printf (_("Minor Image Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorImageVersion));
	  printf (_("Major Subsystem Version:\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorSubsystemVersion));
	  printf (_("Minor Subsystem Version:\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorSubsystemVersion));
	  printf (_("Size Of Image:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfImage));
	  printf (_("Size Of Headers:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeaders));
	  printf (_("CheckSum:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.CheckSum));
	  printf (_("Subsystem:\t\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.Subsystem));
	  // FIXME: Decode the characteristics.
	  printf (_("DllCharacteristics:\t\t%#x\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.DllCharacteristics));
	  printf (_("Size Of Stack Reserve:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfStackReserve));
	  printf (_("Size Of Stack Commit:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfStackCommit));
	  printf (_("Size Of Heap Reserve:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeapReserve));
	  printf (_("Size Of Heap Commit:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeapCommit));
	  printf (_("Loader Flags:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.LoaderFlags));
	  printf (_("Number Of Rva and Sizes:\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.NumberOfRvaAndSizes));

	  // FIXME: Decode the Data Directory.
	}
    }
  else if (opt_header_size == AOUTSZ)
    {
      PEAOUTHDR xhdr;

      /* Different architectures have different sizes of AOUT header.  */
      unsigned int aout_hdr_size = get_target_specific_info (machine)->aout_hdr_size;

      unsigned long off  = ihdr_off + sizeof (* ihdr);
      unsigned long size = sizeof (xhdr.standard);

      printf (_("\n  Optional 32-bit AOUT Header (at offset %#lx, size %d):\n"),
	      off, aout_hdr_size);

      if (bfd_seek (abfd, off, SEEK_SET) != 0
	  || bfd_bread (& xhdr.standard, size, abfd) != size)
	printf (_("error: unable to seek to/read AOUT header\n"));
      else
	{
	  data = (int) bfd_h_get_16 (abfd, xhdr.standard.magic);
	  printf (_("Magic:\t\t\t\t%x\t\t- %s\n"), data,
		    data == 0x010b ? "PE32" : _("Unknown"));
	  
	  printf (_("Version:\t\t\t%x\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.standard.vstamp));

	  printf (_("Text Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.tsize));
	  printf (_("Data Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.dsize));
	  printf (_("BSS Size:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.bsize));
	  printf (_("Entry Point:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.entry));
	  printf (_("Text Start:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.text_start));
	  printf (_("Data Start:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.standard.data_start));
	}

      off  = ihdr_off + sizeof (* ihdr) + aout_hdr_size;
      size = sizeof (xhdr) - sizeof (xhdr.standard);

      printf (_("\n  Optional PE Header (at offset %#lx):\n"), off);

      /* FIXME: Sanitizers might complain about reading more bytes than
	 fit into the ImageBase field.  Find a way to solve this.  */
      if (bfd_seek (abfd, off, SEEK_SET) != 0
	  || bfd_bread (& xhdr.ImageBase, size, abfd) != size)
	printf (_("error: unable to seek to/read PE header\n"));
      else
	{
	  printf (_("Image Base:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.ImageBase));
	  printf (_("Section Alignment:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SectionAlignment));
	  printf (_("File Alignment:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.FileAlignment));
	  printf (_("Major OS Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorOperatingSystemVersion));
	  printf (_("Minor OS ersion:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorOperatingSystemVersion));
	  printf (_("Major Image Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorImageVersion));
	  printf (_("Minor Image Version:\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorImageVersion));
	  printf (_("Major Subsystem Version:\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MajorSubsystemVersion));
	  printf (_("Minor Subsystem Version:\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.MinorSubsystemVersion));
	  printf (_("Size Of Image:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfImage));
	  printf (_("Size Of Headers:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeaders));
	  printf (_("CheckSum:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.CheckSum));
	  printf (_("Subsystem:\t\t\t%d\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.Subsystem));
	  // FIXME: Decode the characteristics.
	  printf (_("DllCharacteristics:\t\t%#x\n"),
		  (int) bfd_h_get_16 (abfd, xhdr.DllCharacteristics));
	  printf (_("Size Of Stack Reserve:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfStackReserve));
	  printf (_("Size Of Stack Commit:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfStackCommit));
	  printf (_("Size Of Heap Reserve:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeapReserve));
	  printf (_("Size Of Heap Commit:\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.SizeOfHeapCommit));
	  printf (_("Loader Flags:\t\t\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.LoaderFlags));
	  printf (_("Number Of Rva and Sizes:\t%#lx\n"),
		  (long) bfd_h_get_32 (abfd, xhdr.NumberOfRvaAndSizes));

	  // FIXME: Decode the Data Directory.
	}
    }
  else if (opt_header_size != 0)
    {
      printf (_("\nUnsupported size of Optional Header\n"));
    }
  else
    printf (_("\n  Optional header not present\n"));
}

/* Dump the sections header.  */

static void
dump_pe_sections_header (bfd *                            abfd,
			 struct external_PEI_filehdr *    fhdr,
			 struct external_PEI_IMAGE_hdr *  ihdr)
{
  unsigned int opthdr = (int) bfd_h_get_16 (abfd, ihdr->f_opthdr);
  unsigned int n_scns = (int) bfd_h_get_16 (abfd, ihdr->f_nscns);
  unsigned int off;

  /* The section header starts after the file, image and optional headers.  */  
  if (fhdr == NULL)
    off = sizeof (struct external_filehdr) + opthdr;
  else
    off = (int) bfd_h_get_16 (abfd, fhdr->e_lfanew) + sizeof (* ihdr) + opthdr;

  printf (_("\nSection headers (at offset 0x%08x):\n"), off);

  if (n_scns == 0)
    {
      printf (_("  No section headers\n"));
      return;
    }
  if (bfd_seek (abfd, off, SEEK_SET) != 0)
    {
      non_fatal (_("cannot seek to section headers start\n"));
      return;
    }

  /* We don't translate this string as it consists of field names.  */
  if (wide_output)
    printf (" # Name     paddr    vaddr    size     scnptr   relptr   lnnoptr   nrel nlnno   Flags\n");
  else
    printf (" # Name     paddr    vaddr    size     scnptr   relptr   lnnoptr   nrel nlnno\n");

  unsigned int i;
  for (i = 0; i < n_scns; i++)
    {
      struct external_scnhdr scn;
      unsigned int flags;

      if (bfd_bread (& scn, sizeof (scn), abfd) != sizeof (scn))
        {
          non_fatal (_("cannot read section header"));
          return;
        }

      printf ("%2d %-8.8s %08x %08x %08x %08x %08x %08x %5d %5d",
              i + 1, scn.s_name,
              (unsigned int) bfd_h_get_32 (abfd, scn.s_paddr),
              (unsigned int) bfd_h_get_32 (abfd, scn.s_vaddr),
              (unsigned int) bfd_h_get_32 (abfd, scn.s_size),
              (unsigned int) bfd_h_get_32 (abfd, scn.s_scnptr),
              (unsigned int) bfd_h_get_32 (abfd, scn.s_relptr),
              (unsigned int) bfd_h_get_32 (abfd, scn.s_lnnoptr),
              (unsigned int) bfd_h_get_16 (abfd, scn.s_nreloc),
              (unsigned int) bfd_h_get_16 (abfd, scn.s_nlnno));

      flags = bfd_h_get_32 (abfd, scn.s_flags);
      if (wide_output)
	printf (_("   %08x "), flags);
      else
	printf (_("\n            Flags: %08x: "), flags);

      if (flags != 0)
        {
	  /* Skip the alignment bits.  */
	  flags &= ~ IMAGE_SCN_ALIGN_POWER_BIT_MASK;
          dump_flags (section_flag_xlat, flags);
        }

      putchar ('\n');
    }
}

/* Handle a PE format file.  */

static void
dump_pe (bfd *                            abfd,
	 struct external_PEI_filehdr *    fhdr,
	 struct external_PEI_IMAGE_hdr *  ihdr)
{
  if (options[OPT_FILE_HEADER].selected)
    dump_pe_file_header (abfd, fhdr, ihdr);
  
  if (options[OPT_SECTIONS].selected)
    dump_pe_sections_header (abfd, fhdr, ihdr);
}

/* Dump ABFD (according to the options[] array).  */

static void
pe_dump_obj (bfd *abfd)
{
  struct external_PEI_filehdr fhdr;

  /* Read file header.  */
  if (bfd_seek (abfd, 0, SEEK_SET) != 0
      || bfd_bread (& fhdr, sizeof (fhdr), abfd) != sizeof (fhdr))
    {
      non_fatal (_("cannot seek to/read file header"));
      return;
    }

  unsigned short magic = bfd_h_get_16 (abfd, fhdr.e_magic);

  /* PE format executable files have a full external_PEI_filehdr structure
     at the start.  PE format object files just have an external_filehdr
     structure at the start.  */
  if (magic == IMAGE_DOS_SIGNATURE)
    {
      unsigned int ihdr_offset = (int) bfd_h_get_16 (abfd, fhdr.e_lfanew);

      /* FIXME: We could reuse the fields in fhdr, but that might
	 confuse various sanitization and memory checker tools.  */
      struct external_PEI_IMAGE_hdr ihdr;

      if (bfd_seek (abfd, ihdr_offset, SEEK_SET) != 0
	  || bfd_bread (& ihdr, sizeof (ihdr), abfd) != sizeof (ihdr))
	{
	  non_fatal (_("cannot seek to/read image header at offset %#x"),
		     ihdr_offset);
	  return;
	}

      unsigned int signature = (int) bfd_h_get_16 (abfd, ihdr.nt_signature);
      if (signature != IMAGE_NT_SIGNATURE)
	{
	  non_fatal ("file does not have an NT format signature: %#x",
		     signature);
	  return;
	}
  
      dump_pe (abfd, & fhdr, & ihdr);
    }
  /* See if we recognise this particular PE object file.  */
  else if (get_target_specific_info (magic)->machine_number)
    {
      struct external_filehdr ehdr;

      if (bfd_seek (abfd, 0, SEEK_SET) != 0
	  || bfd_bread (& ehdr, sizeof (ehdr), abfd) != sizeof (ehdr))
	{
	  non_fatal (_("cannot seek to/read image header"));
	  return;
	}

      struct external_PEI_IMAGE_hdr ihdr;
      memcpy (& ihdr.f_magic, & ehdr, sizeof (ehdr));
      dump_pe (abfd, NULL, & ihdr);
    }
  else
    {
      non_fatal ("unknown PE format binary - unsupported magic number: %#x",
		 magic);
      return;
    }
}

/* Dump a PE file.  */

static void
pe_dump (bfd *abfd)
{
  /* We rely on BFD to decide if the file is a core file.  Note that core
     files are only supported on native environment by BFD.  */
  switch (bfd_get_format (abfd))
    {
    case bfd_core:
      // FIXME: Handle PE format core files ?
      break;
    default:
      pe_dump_obj (abfd);
      break;
    }
}

/* Vector for pe.  */

const struct objdump_private_desc objdump_private_desc_pe =
{
  pe_help,
  pe_filter,
  pe_dump,
  options
};
