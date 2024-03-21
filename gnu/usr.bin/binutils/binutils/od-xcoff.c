/* od-xcoff.c -- dump information about an xcoff object file.
   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold, Adacore.

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
/* Force the support of weak symbols.  */
#ifndef AIX_WEAK_SUPPORT
#define AIX_WEAK_SUPPORT 1
#endif
#include "coff/internal.h"
#include "coff/rs6000.h"
#include "coff/xcoff.h"
#include "libcoff.h"
#include "libxcoff.h"

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

/* Display help.  */

static void
xcoff_help (FILE *stream)
{
  fprintf (stream, _("\
For XCOFF files:\n\
  header      Display the file header\n\
  aout        Display the auxiliary header\n\
  sections    Display the section headers\n\
  syms        Display the symbols table\n\
  relocs      Display the relocation entries\n\
  lineno      Display the line number entries\n\
  loader      Display loader section\n\
  except      Display exception table\n\
  typchk      Display type-check section\n\
  traceback   Display traceback tags\n\
  toc         Display toc symbols\n\
  ldinfo      Display loader info in core files\n\
"));
}

/* Return TRUE if ABFD is handled.  */

static int
xcoff_filter (bfd *abfd)
{
  return bfd_get_flavour (abfd) == bfd_target_xcoff_flavour;
}

/* Translation entry type.  The last entry must be {0, NULL}.  */

struct xlat_table {
  unsigned int val;
  const char *name;
};

/* Display the list of name (from TABLE) for FLAGS, using comma to separate
   them.  A name is displayed if FLAGS & VAL is not 0.  */

static void
dump_flags (const struct xlat_table *table, unsigned int flags)
{
  unsigned int r = flags;
  int first = 1;
  const struct xlat_table *t;

  for (t = table; t->name; t++)
    if ((flags & t->val) != 0)
      {
        r &= ~t->val;

        if (first)
          first = 0;
        else
          putchar (',');
        fputs (t->name, stdout);
      }

  /* Not decoded flags.  */
  if (r != 0)
    {
      if (!first)
        putchar (',');
      printf ("0x%x", r);
    }
}

/* Display the name corresponding to VAL from TABLE, using at most
   MAXLEN char (possibly passed with spaces).  */

static void
dump_value (const struct xlat_table *table, unsigned int val, int maxlen)
{
  const struct xlat_table *t;

  for (t = table; t->name; t++)
    if (t->val == val)
      {
        printf ("%-*s", maxlen, t->name);
        return;
      }
  printf ("(%*x)", maxlen - 2, val);
}

/* Names of f_flags.  */
static const struct xlat_table f_flag_xlat[] =
  {
    { F_RELFLG,    "no-rel" },
    { F_EXEC,      "exec" },
    { F_LNNO,      "lineno" },
    { F_LSYMS,     "lsyms" },

    { F_FDPR_PROF, "fdpr-prof" },
    { F_FDPR_OPTI, "fdpr-opti" },
    { F_DSA,       "dsa" },

    { F_VARPG,     "varprg" },

    { F_DYNLOAD,   "dynload" },
    { F_SHROBJ,    "shrobj" },
    { F_NONEXEC,   "nonexec" },

    { 0, NULL }
  };

/* Names of s_flags.  */
static const struct xlat_table s_flag_xlat[] =
  {
    { STYP_PAD,    "pad" },
    { STYP_DWARF,  "dwarf" },
    { STYP_TEXT,   "text" },
    { STYP_DATA,   "data" },
    { STYP_BSS,    "bss" },

    { STYP_EXCEPT, "except" },
    { STYP_INFO,   "info" },
    { STYP_TDATA,  "tdata" },
    { STYP_TBSS,   "tbss" },

    { STYP_LOADER, "loader" },
    { STYP_DEBUG,  "debug" },
    { STYP_TYPCHK, "typchk" },
    { STYP_OVRFLO, "ovrflo" },
    { 0, NULL }
  };

/* Names of storage class.  */
static const struct xlat_table sc_xlat[] =
  {
#define SC_ENTRY(X) { C_##X, #X }
    SC_ENTRY(NULL),
    SC_ENTRY(AUTO),
    SC_ENTRY(EXT),
    SC_ENTRY(STAT),
    SC_ENTRY(REG),
    SC_ENTRY(EXTDEF),
    SC_ENTRY(LABEL),
    SC_ENTRY(ULABEL),
    SC_ENTRY(MOS),
    SC_ENTRY(ARG),
    /*    SC_ENTRY(STRARG), */
    SC_ENTRY(MOU),
    SC_ENTRY(UNTAG),
    SC_ENTRY(TPDEF),
    SC_ENTRY(USTATIC),
    SC_ENTRY(ENTAG),
    SC_ENTRY(MOE),
    SC_ENTRY(REGPARM),
    SC_ENTRY(FIELD),
    SC_ENTRY(BLOCK),
    SC_ENTRY(FCN),
    SC_ENTRY(EOS),
    SC_ENTRY(FILE),
    SC_ENTRY(LINE),
    SC_ENTRY(ALIAS),
    SC_ENTRY(HIDDEN),
    SC_ENTRY(HIDEXT),
    SC_ENTRY(BINCL),
    SC_ENTRY(EINCL),
    SC_ENTRY(INFO),
    SC_ENTRY(WEAKEXT),
    SC_ENTRY(DWARF),

    /* Stabs.  */
    SC_ENTRY (GSYM),
    SC_ENTRY (LSYM),
    SC_ENTRY (PSYM),
    SC_ENTRY (RSYM),
    SC_ENTRY (RPSYM),
    SC_ENTRY (STSYM),
    SC_ENTRY (TCSYM),
    SC_ENTRY (BCOMM),
    SC_ENTRY (ECOML),
    SC_ENTRY (ECOMM),
    SC_ENTRY (DECL),
    SC_ENTRY (ENTRY),
    SC_ENTRY (FUN),
    SC_ENTRY (BSTAT),
    SC_ENTRY (ESTAT),

    { 0, NULL }
#undef SC_ENTRY
  };

/* Names for symbol type.  */
static const struct xlat_table smtyp_xlat[] =
  {
    { XTY_ER, "ER" },
    { XTY_SD, "SD" },
    { XTY_LD, "LD" },
    { XTY_CM, "CM" },
    { XTY_EM, "EM" },
    { XTY_US, "US" },
    { 0, NULL }
  };

/* Names for storage-mapping class.  */
static const struct xlat_table smclas_xlat[] =
  {
#define SMCLAS_ENTRY(X) { XMC_##X, #X }
    SMCLAS_ENTRY (PR),
    SMCLAS_ENTRY (RO),
    SMCLAS_ENTRY (DB),
    SMCLAS_ENTRY (TC),
    SMCLAS_ENTRY (UA),
    SMCLAS_ENTRY (RW),
    SMCLAS_ENTRY (GL),
    SMCLAS_ENTRY (XO),
    SMCLAS_ENTRY (SV),
    SMCLAS_ENTRY (BS),
    SMCLAS_ENTRY (DS),
    SMCLAS_ENTRY (UC),
    SMCLAS_ENTRY (TI),
    SMCLAS_ENTRY (TB),
    SMCLAS_ENTRY (TC0),
    SMCLAS_ENTRY (TD),
    SMCLAS_ENTRY (SV64),
    SMCLAS_ENTRY (SV3264),
    { 0, NULL }
#undef SMCLAS_ENTRY
  };

/* Names for relocation type.  */
static const struct xlat_table rtype_xlat[] =
  {
#define RTYPE_ENTRY(X) { R_##X, #X }
    RTYPE_ENTRY (POS),
    RTYPE_ENTRY (NEG),
    RTYPE_ENTRY (REL),
    RTYPE_ENTRY (TOC),
    RTYPE_ENTRY (TRL),
    RTYPE_ENTRY (GL),
    RTYPE_ENTRY (TCL),
    RTYPE_ENTRY (BA),
    RTYPE_ENTRY (BR),
    RTYPE_ENTRY (RL),
    RTYPE_ENTRY (RLA),
    RTYPE_ENTRY (REF),
    RTYPE_ENTRY (TRLA),
    RTYPE_ENTRY (RRTBI),
    RTYPE_ENTRY (RRTBA),
    RTYPE_ENTRY (CAI),
    RTYPE_ENTRY (CREL),
    RTYPE_ENTRY (RBA),
    RTYPE_ENTRY (RBAC),
    RTYPE_ENTRY (RBR),
    RTYPE_ENTRY (RBRC),
    RTYPE_ENTRY (TLS),
    RTYPE_ENTRY (TLS_IE),
    RTYPE_ENTRY (TLS_LD),
    RTYPE_ENTRY (TLS_LE),
    RTYPE_ENTRY (TLSM),
    RTYPE_ENTRY (TLSML),
    RTYPE_ENTRY (TOCU),
    RTYPE_ENTRY (TOCL),
    { 0, NULL }
  };

/* Simplified section header.  */
struct xcoff32_section
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

/* Simplified symbol.  */

union xcoff32_symbol
{
  union external_auxent aux;

  struct sym
  {
    /* Pointer to the NUL-terminated name.  */
    char *name;

    /* XCOFF symbol fields.  */
    unsigned int val;
    unsigned short scnum;
    unsigned short ntype;
    unsigned char sclass;
    unsigned char numaux;

    /* Buffer in case the name is local.  */
    union
    {
      char name[9];
      unsigned int off;
    } raw;
  } sym;
};

/* Important fields to dump the file.  */

struct xcoff_dump
{
  /* From file header.  */
  unsigned short nscns;
  unsigned int symptr;
  unsigned int nsyms;
  unsigned short opthdr;

  /* Sections.  */
  struct xcoff32_section *sects;

  /* Symbols.  */
  union xcoff32_symbol *syms;
  char *strings;
  unsigned int strings_size;
};

/* Print a symbol (if possible).  */

static void
xcoff32_print_symbol (struct xcoff_dump *data, unsigned int symndx)
{
  if (data->syms != NULL
      && symndx < data->nsyms
      && data->syms[symndx].sym.name != NULL)
    printf ("%s", data->syms[symndx].sym.name);
  else
    printf ("%u", symndx);
}

/* Dump the file header.  */

static void
dump_xcoff32_file_header (bfd *abfd, struct external_filehdr *fhdr,
                          struct xcoff_dump *data)
{
  unsigned int timdat = bfd_h_get_32 (abfd, fhdr->f_timdat);
  unsigned short flags = bfd_h_get_16 (abfd, fhdr->f_flags);

  printf (_("  nbr sections:  %d\n"), data->nscns);
  printf (_("  time and date: 0x%08x  - "), timdat);
  if (timdat == 0)
    printf (_("not set\n"));
  else
    {
      /* Not correct on all platforms, but works on unix.  */
      time_t t = timdat;
      fputs (ctime (&t), stdout);
    }
  printf (_("  symbols off:   0x%08x\n"), data->symptr);
  printf (_("  nbr symbols:   %d\n"), data->nsyms);
  printf (_("  opt hdr sz:    %d\n"), data->opthdr);
  printf (_("  flags:         0x%04x "), flags);
  dump_flags (f_flag_xlat, flags);
  putchar ('\n');
}

/* Dump the a.out header.  */

static void
dump_xcoff32_aout_header (bfd *abfd, struct xcoff_dump *data)
{
  AOUTHDR auxhdr;
  unsigned short magic;
  unsigned int sz = data->opthdr;

  printf (_("Auxiliary header:\n"));
  if (data->opthdr == 0)
    {
      printf (_("  No aux header\n"));
      return;
    }
  if (data->opthdr > sizeof (auxhdr))
    {
      printf (_("warning: optional header size too large (> %d)\n"),
              (int)sizeof (auxhdr));
      sz = sizeof (auxhdr);
    }
  if (bfd_bread (&auxhdr, sz, abfd) != sz)
    {
      non_fatal (_("cannot read auxhdr"));
      return;
    }

  magic = bfd_h_get_16 (abfd, auxhdr.magic);
  /* We don't translate these strings as they are fields name.  */
  printf ("  o_mflag (magic): 0x%04x 0%04o\n", magic, magic);
  printf ("  o_vstamp:        0x%04x\n",
          (unsigned short)bfd_h_get_16 (abfd, auxhdr.vstamp));
  printf ("  o_tsize:         0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.tsize));
  printf ("  o_dsize:         0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.dsize));
  printf ("  o_entry:         0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.entry));
  printf ("  o_text_start:    0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.text_start));
  printf ("  o_data_start:    0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.data_start));
  if (sz == offsetof (AOUTHDR, o_toc))
    return;
  printf ("  o_toc:           0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.o_toc));
  printf ("  o_snentry:       0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_snentry));
  printf ("  o_sntext:        0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_sntext));
  printf ("  o_sndata:        0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_sndata));
  printf ("  o_sntoc:         0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_sntoc));
  printf ("  o_snloader:      0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_snloader));
  printf ("  o_snbss:         0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_snbss));
  printf ("  o_algntext:      %u\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_algntext));
  printf ("  o_algndata:      %u\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_algndata));
  printf ("  o_modtype:       0x%04x",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_modtype));
  if (ISPRINT (auxhdr.o_modtype[0]) && ISPRINT (auxhdr.o_modtype[1]))
    printf (" (%c%c)", auxhdr.o_modtype[0], auxhdr.o_modtype[1]);
  putchar ('\n');
  printf ("  o_cputype:       0x%04x\n",
          (unsigned int)bfd_h_get_16 (abfd, auxhdr.o_cputype));
  printf ("  o_maxstack:      0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.o_maxstack));
  printf ("  o_maxdata:       0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.o_maxdata));
#if 0
  printf ("  o_debugger:      0x%08x\n",
          (unsigned int)bfd_h_get_32 (abfd, auxhdr.o_debugger));
#endif
}

/* Dump the sections header.  */

static void
dump_xcoff32_sections_header (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;
  unsigned int off;

  off = sizeof (struct external_filehdr) + data->opthdr;
  printf (_("Section headers (at %u+%u=0x%08x to 0x%08x):\n"),
          (unsigned int)sizeof (struct external_filehdr), data->opthdr, off,
          off + (unsigned int)sizeof (struct external_scnhdr) * data->nscns);
  if (data->nscns == 0)
    {
      printf (_("  No section header\n"));
      return;
    }
  if (bfd_seek (abfd, off, SEEK_SET) != 0)
    {
      non_fatal (_("cannot read section header"));
      return;
    }
  /* We don't translate this string as it consists in fields name.  */
  printf (" # Name     paddr    vaddr    size     scnptr   relptr   lnnoptr  nrel  nlnno\n");
  for (i = 0; i < data->nscns; i++)
    {
      struct external_scnhdr scn;
      unsigned int flags;

      if (bfd_bread (&scn, sizeof (scn), abfd) != sizeof (scn))
        {
          non_fatal (_("cannot read section header"));
          return;
        }
      flags = bfd_h_get_32 (abfd, scn.s_flags);
      printf ("%2d %-8.8s %08x %08x %08x %08x %08x %08x %-5d %-5d\n",
              i + 1, scn.s_name,
              (unsigned int)bfd_h_get_32 (abfd, scn.s_paddr),
              (unsigned int)bfd_h_get_32 (abfd, scn.s_vaddr),
              (unsigned int)bfd_h_get_32 (abfd, scn.s_size),
              (unsigned int)bfd_h_get_32 (abfd, scn.s_scnptr),
              (unsigned int)bfd_h_get_32 (abfd, scn.s_relptr),
              (unsigned int)bfd_h_get_32 (abfd, scn.s_lnnoptr),
              (unsigned int)bfd_h_get_16 (abfd, scn.s_nreloc),
              (unsigned int)bfd_h_get_16 (abfd, scn.s_nlnno));
      printf (_("            Flags: %08x "), flags);

      if (~flags == 0)
        {
          /* Stripped executable ?  */
          putchar ('\n');
        }
      else if (flags & STYP_OVRFLO)
        printf (_("overflow - nreloc: %u, nlnno: %u\n"),
                (unsigned int)bfd_h_get_32 (abfd, scn.s_paddr),
                (unsigned int)bfd_h_get_32 (abfd, scn.s_vaddr));
      else
        {
          dump_flags (s_flag_xlat, flags);
          putchar ('\n');
        }
    }
}

/* Read section table.  */

static void
xcoff32_read_sections (bfd *abfd, struct xcoff_dump *data)
{
  int i;

  if (bfd_seek (abfd, sizeof (struct external_filehdr) + data->opthdr,
                SEEK_SET) != 0)
    {
      non_fatal (_("cannot read section headers"));
      return;
    }

  data->sects = xmalloc (data->nscns * sizeof (struct xcoff32_section));
  for (i = 0; i < data->nscns; i++)
    {
      struct external_scnhdr scn;
      struct xcoff32_section *s = &data->sects[i];

      if (bfd_bread (&scn, sizeof (scn), abfd) != sizeof (scn))
        {
          non_fatal (_("cannot read section header"));
          free (data->sects);
          data->sects = NULL;
          return;
        }
      memcpy (s->name, scn.s_name, 8);
      s->name[8] = 0;
      s->flags = bfd_h_get_32 (abfd, scn.s_flags);

      s->scnptr = bfd_h_get_32 (abfd, scn.s_scnptr);
      s->relptr = bfd_h_get_32 (abfd, scn.s_relptr);
      s->lnnoptr = bfd_h_get_32 (abfd, scn.s_lnnoptr);

      s->nreloc = bfd_h_get_16 (abfd, scn.s_nreloc);
      s->nlnno = bfd_h_get_16 (abfd, scn.s_nlnno);

      if (s->flags == STYP_OVRFLO)
        {
          if (s->nreloc > 0 && s->nreloc <= data->nscns)
            data->sects[s->nreloc - 1].nreloc =
              bfd_h_get_32 (abfd, scn.s_paddr);
          if (s->nlnno > 0 && s->nlnno <= data->nscns)
            data->sects[s->nlnno - 1].nlnno =
              bfd_h_get_32 (abfd, scn.s_vaddr);
        }
    }
}

/* Read symbols.  */

static void
xcoff32_read_symbols (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;
  char stsz_arr[4];
  unsigned int stptr;

  if (data->nsyms == 0)
    return;

  stptr = data->symptr
    + data->nsyms * (unsigned)sizeof (struct external_syment);

  /* Read string table.  */
  if (bfd_seek (abfd, stptr, SEEK_SET) != 0
      || bfd_bread (&stsz_arr, sizeof (stsz_arr), abfd) != sizeof (stsz_arr))
    {
      non_fatal (_("cannot read strings table length"));
      data->strings_size = 0;
    }
  else
    {
      data->strings_size = bfd_h_get_32 (abfd, stsz_arr);
      if (data->strings_size > sizeof (stsz_arr))
        {
          unsigned int remsz = data->strings_size - sizeof (stsz_arr);

          data->strings = xmalloc (data->strings_size);

          memcpy (data->strings, stsz_arr, sizeof (stsz_arr));
          if (bfd_bread (data->strings + sizeof (stsz_arr), remsz, abfd)
              != remsz)
            {
              non_fatal (_("cannot read strings table"));
              goto clean;
            }
        }
    }

  if (bfd_seek (abfd, data->symptr, SEEK_SET) != 0)
    {
      non_fatal (_("cannot read symbol table"));
      goto clean;
    }

  data->syms = (union xcoff32_symbol *)
    xmalloc (data->nsyms * sizeof (union xcoff32_symbol));

  for (i = 0; i < data->nsyms; i++)
    {
      struct external_syment sym;
      int j;
      union xcoff32_symbol *s = &data->syms[i];

      if (bfd_bread (&sym, sizeof (sym), abfd) != sizeof (sym))
        {
          non_fatal (_("cannot read symbol entry"));
          goto clean;
        }

      s->sym.val = bfd_h_get_32 (abfd, sym.e_value);
      s->sym.scnum = bfd_h_get_16 (abfd, sym.e_scnum);
      s->sym.ntype = bfd_h_get_16 (abfd, sym.e_type);
      s->sym.sclass = bfd_h_get_8 (abfd, sym.e_sclass);
      s->sym.numaux = bfd_h_get_8 (abfd, sym.e_numaux);

      if (sym.e.e_name[0])
        {
          memcpy (s->sym.raw.name, sym.e.e_name, sizeof (sym.e.e_name));
          s->sym.raw.name[8] = 0;
          s->sym.name = s->sym.raw.name;
        }
      else
        {
          unsigned int soff = bfd_h_get_32 (abfd, sym.e.e.e_offset);

          if ((s->sym.sclass & DBXMASK) == 0 && soff < data->strings_size)
            s->sym.name = data->strings + soff;
          else
            {
              s->sym.name = NULL;
              s->sym.raw.off = soff;
            }
        }

      for (j = 0; j < s->sym.numaux; j++, i++)
        {
           if (bfd_bread (&s[j + 1].aux,
                          sizeof (union external_auxent), abfd)
               != sizeof (union external_auxent))
            {
              non_fatal (_("cannot read symbol aux entry"));
              goto clean;
            }
        }
    }
  return;
 clean:
  free (data->syms);
  data->syms = NULL;
  free (data->strings);
  data->strings = NULL;
}

/* Dump xcoff symbols.  */

static void
dump_xcoff32_symbols (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;
  asection *debugsec;
  char *debug = NULL;

  printf (_("Symbols table (strtable at 0x%08x)"),
          data->symptr
          + data->nsyms * (unsigned)sizeof (struct external_syment));
  if (data->nsyms == 0 || data->syms == NULL)
    {
      printf (_(":\n  No symbols\n"));
      return;
    }

  /* Read strings table.  */
  if (data->strings_size == 0)
    printf (_(" (no strings):\n"));
  else
    printf (_(" (strings size: %08x):\n"), data->strings_size);

  /* Read debug section.  */
  debugsec = bfd_get_section_by_name (abfd, ".debug");
  if (debugsec != NULL)
    {
      bfd_size_type size;

      size = bfd_section_size (debugsec);
      debug = (char *) xmalloc (size);
      bfd_get_section_contents (abfd, debugsec, debug, 0, size);
    }

  /* Translators: 'sc' is for storage class, 'off' for offset.  */
  printf (_("  # sc         value    section  type aux name/off\n"));
  for (i = 0; i < data->nsyms; i++)
    {
      union xcoff32_symbol *s = &data->syms[i];
      int j;

      printf ("%3u ", i);
      dump_value (sc_xlat, s->sym.sclass, 10);
      printf (" %08x ", s->sym.val);
      if (s->sym.scnum > 0 && s->sym.scnum <= data->nscns)
        {
          if (data->sects != NULL)
            printf ("%-8s", data->sects[s->sym.scnum - 1].name);
          else
            printf ("%-8u", s->sym.scnum);
        }
      else
        switch ((signed short)s->sym.scnum)
          {
          case N_DEBUG:
            printf ("N_DEBUG ");
            break;
          case N_ABS:
            printf ("N_ABS   ");
            break;
          case N_UNDEF:
            printf ("N_UNDEF ");
            break;
          default:
            printf ("(%04x)  ", s->sym.scnum);
          }
      printf (" %04x %3u ", s->sym.ntype, s->sym.numaux);
      if (s->sym.name != NULL)
        printf ("%s", s->sym.name);
      else
        {
          if ((s->sym.sclass & DBXMASK) != 0 && debug != NULL)
            printf ("%s", debug + s->sym.raw.off);
          else
            printf ("%08x", s->sym.raw.off);
        }
      putchar ('\n');

      for (j = 0; j < s->sym.numaux; j++, i++)
        {
          union external_auxent *aux = &s[j + 1].aux;

          printf (" %3u ", i + 1);
          switch (s->sym.sclass)
            {
            case C_STAT:
              /* Section length, number of relocs and line number.  */
              printf (_("  scnlen: %08x  nreloc: %-6u  nlinno: %-6u\n"),
                      (unsigned)bfd_h_get_32 (abfd, aux->x_scn.x_scnlen),
                      (unsigned)bfd_h_get_16 (abfd, aux->x_scn.x_nreloc),
                      (unsigned)bfd_h_get_16 (abfd, aux->x_scn.x_nlinno));
              break;
            case C_DWARF:
              /* Section length and number of relocs.  */
              printf (_("  scnlen: %08x  nreloc: %-6u\n"),
                      (unsigned)bfd_h_get_32 (abfd, aux->x_scn.x_scnlen),
                      (unsigned)bfd_h_get_16 (abfd, aux->x_scn.x_nreloc));
              break;
            case C_EXT:
            case C_WEAKEXT:
            case C_HIDEXT:
              if (j == 0 && s->sym.numaux > 1)
                {
                  /* Function aux entry  (Do not translate).  */
                  printf ("  exptr: %08x fsize: %08x lnnoptr: %08x endndx: %u\n",
                          (unsigned)bfd_h_get_32 (abfd, aux->x_fcn.x_exptr),
                          (unsigned)bfd_h_get_32
                            (abfd, aux->x_fcn.x_fsize),
                          (unsigned)bfd_h_get_32
                            (abfd, aux->x_fcn.x_lnnoptr),
                          (unsigned)bfd_h_get_32
                            (abfd, aux->x_fcn.x_endndx));
                }
              else if (j == 1 || (j == 0 && s->sym.numaux == 1))
                {
                  /* csect aux entry.  */
                  unsigned char smtyp;
                  unsigned int scnlen;

                  smtyp = bfd_h_get_8 (abfd, aux->x_csect.x_smtyp);
                  scnlen = bfd_h_get_32 (abfd, aux->x_csect.x_scnlen);

                  if (smtyp == XTY_LD)
                    printf ("  scnsym: %-8u", scnlen);
                  else
                    printf ("  scnlen: %08x", scnlen);
                  printf (" h: parm=%08x sn=%04x al: 2**%u",
                          (unsigned)bfd_h_get_32 (abfd, aux->x_csect.x_parmhash),
                          (unsigned)bfd_h_get_16 (abfd, aux->x_csect.x_snhash),
                          SMTYP_ALIGN (smtyp));
                  printf (" typ: ");
                  dump_value (smtyp_xlat, SMTYP_SMTYP (smtyp), 2);
                  printf (" cl: ");
                  dump_value
                    (smclas_xlat,
                     (unsigned)bfd_h_get_8 (abfd, aux->x_csect.x_smclas), 6);
                  putchar ('\n');
                }
              else
                /* Do not translate - generic field name.  */
                printf ("aux\n");
              break;
            case C_FILE:
              {
                unsigned int off;

                printf (" ftype: %02x ",
                        (unsigned)bfd_h_get_8 (abfd, aux->x_file.x_ftype));
                if (aux->x_file.x_n.x_fname[0] != 0)
                  printf ("fname: %.14s", aux->x_file.x_n.x_fname);
                else
                  {
                    off = (unsigned)bfd_h_get_32
                      (abfd, aux->x_file.x_n.x_n.x_offset);
                    if (data->strings != NULL && off < data->strings_size)
                      printf (" %s", data->strings + off);
                    else
                      printf (_("offset: %08x"), off);
                  }
                putchar ('\n');
              }
              break;
            case C_BLOCK:
            case C_FCN:
              printf ("  lnno: %u\n",
                      (unsigned)bfd_h_get_16
                      (abfd, aux->x_sym.x_lnno));
              break;
            default:
              /* Do not translate - generic field name.  */
              printf ("aux\n");
              break;
            }
        }

    }
  free (debug);
}

/* Dump xcoff relocation entries.  */

static void
dump_xcoff32_relocs (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;

  if (data->sects == NULL)
    {
      non_fatal (_("cannot read section headers"));
      return;
    }

  for (i = 0; i < data->nscns; i++)
    {
      struct xcoff32_section *sect = &data->sects[i];
      unsigned int nrel = sect->nreloc;
      unsigned int j;

      if (nrel == 0)
        continue;
      printf (_("Relocations for %s (%u)\n"), sect->name, nrel);
      if (bfd_seek (abfd, sect->relptr, SEEK_SET) != 0)
        {
          non_fatal (_("cannot read relocations"));
          continue;
        }
      /* Do not translate: fields name.  */
      printf ("vaddr    sgn mod sz type  symndx symbol\n");
      for (j = 0; j < nrel; j++)
        {
          struct external_reloc rel;
          unsigned char rsize;
          unsigned int symndx;

          if (bfd_bread (&rel, sizeof (rel), abfd) != sizeof (rel))
            {
              non_fatal (_("cannot read relocation entry"));
              return;
            }
          rsize = bfd_h_get_8 (abfd, rel.r_size);
          printf ("%08x  %c   %c  %-2u ",
                  (unsigned int)bfd_h_get_32 (abfd, rel.r_vaddr),
                  rsize & 0x80 ? 'S' : 'U',
                  rsize & 0x40 ? 'm' : ' ',
                  (rsize & 0x3f) + 1);
          dump_value (rtype_xlat, bfd_h_get_8 (abfd, rel.r_type), 6);
          symndx = bfd_h_get_32 (abfd, rel.r_symndx);
          printf ("%-6u ", symndx);
          xcoff32_print_symbol (data, symndx);
          putchar ('\n');
        }
      putchar ('\n');
    }
}

/* Dump xcoff line number entries.  */

static void
dump_xcoff32_lineno (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;

  if (data->sects == NULL)
    {
      non_fatal (_("cannot read section headers"));
      return;
    }

  for (i = 0; i < data->nscns; i++)
    {
      struct xcoff32_section *sect = &data->sects[i];
      unsigned int nlnno = sect->nlnno;
      unsigned int j;

      if (nlnno == 0)
        continue;
      printf (_("Line numbers for %s (%u)\n"), sect->name, nlnno);
      if (bfd_seek (abfd, sect->lnnoptr, SEEK_SET) != 0)
        {
          non_fatal (_("cannot read line numbers"));
          continue;
        }
      /* Line number, symbol index and physical address.  */
      printf (_("lineno  symndx/paddr\n"));
      for (j = 0; j < nlnno; j++)
        {
          struct external_lineno ln;
          unsigned int no;

          if (bfd_bread (&ln, sizeof (ln), abfd) != sizeof (ln))
            {
              non_fatal (_("cannot read line number entry"));
              return;
            }
          no = bfd_h_get_16 (abfd, ln.l_lnno);
          printf (" %-6u ", no);
          if (no == 0)
            {
              unsigned int symndx = bfd_h_get_32 (abfd, ln.l_addr.l_symndx);
              xcoff32_print_symbol (data, symndx);
            }
          else
            printf ("0x%08x",
                    (unsigned int)bfd_h_get_32 (abfd, ln.l_addr.l_paddr));
          putchar ('\n');
        }
    }
}

/* Dump xcoff loader section.  */

static void
dump_xcoff32_loader (bfd *abfd)
{
  asection *loader;
  bfd_size_type size = 0;
  struct external_ldhdr *lhdr;
  struct external_ldsym *ldsym;
  struct external_ldrel *ldrel;
  bfd_byte *ldr_data;
  unsigned int version;
  unsigned int ndsyms;
  unsigned int ndrel;
  unsigned int stlen;
  unsigned int stoff;
  unsigned int impoff;
  unsigned int nimpid;
  unsigned int i;
  const char *p;

  loader = bfd_get_section_by_name (abfd, ".loader");

  if (loader == NULL)
    {
      printf (_("no .loader section in file\n"));
      return;
    }
  size = bfd_section_size (loader);
  if (size < sizeof (*lhdr))
    {
      printf (_("section .loader is too short\n"));
      return;
    }

  ldr_data = (bfd_byte *) xmalloc (size);
  bfd_get_section_contents (abfd, loader, ldr_data, 0, size);
  lhdr = (struct external_ldhdr *)ldr_data;
  printf (_("Loader header:\n"));
  version = bfd_h_get_32 (abfd, lhdr->l_version);
  printf (_("  version:           %u\n"), version);
  if (version != 1)
    {
      printf (_(" Unhandled version\n"));
      free (ldr_data);
      return;
    }
  ndsyms = bfd_h_get_32 (abfd, lhdr->l_nsyms);
  printf (_("  nbr symbols:       %u\n"), ndsyms);
  ndrel = bfd_h_get_32 (abfd, lhdr->l_nreloc);
  printf (_("  nbr relocs:        %u\n"), ndrel);
  /* Import string table length.  */
  printf (_("  import strtab len: %u\n"),
          (unsigned) bfd_h_get_32 (abfd, lhdr->l_istlen));
  nimpid = bfd_h_get_32 (abfd, lhdr->l_nimpid);
  printf (_("  nbr import files:  %u\n"), nimpid);
  impoff = bfd_h_get_32 (abfd, lhdr->l_impoff);
  printf (_("  import file off:   %u\n"), impoff);
  stlen = bfd_h_get_32 (abfd, lhdr->l_stlen);
  printf (_("  string table len:  %u\n"), stlen);
  stoff = bfd_h_get_32 (abfd, lhdr->l_stoff);
  printf (_("  string table off:  %u\n"), stoff);

  ldsym = (struct external_ldsym *)(ldr_data + sizeof (*lhdr));
  printf (_("Dynamic symbols:\n"));
  /* Do not translate: field names.  */
  printf ("     # value     sc IFEW ty class file  pa name\n");
  for (i = 0; i < ndsyms; i++, ldsym++)
    {
      unsigned char smtype;

      printf (_("  %4u %08x %3u "), i,
              (unsigned)bfd_h_get_32 (abfd, ldsym->l_value),
              (unsigned)bfd_h_get_16 (abfd, ldsym->l_scnum));
      smtype = bfd_h_get_8 (abfd, ldsym->l_smtype);
      putchar (smtype & 0x40 ? 'I' : ' ');
      putchar (smtype & 0x20 ? 'F' : ' ');
      putchar (smtype & 0x10 ? 'E' : ' ');
      putchar (smtype & 0x08 ? 'W' : ' ');
      putchar (' ');
      dump_value (smtyp_xlat, SMTYP_SMTYP (smtype), 2);
      putchar (' ');
      dump_value
        (smclas_xlat, (unsigned)bfd_h_get_8 (abfd, ldsym->l_smclas), 6);
      printf (_(" %3u %3u "),
              (unsigned)bfd_h_get_32 (abfd, ldsym->l_ifile),
              (unsigned)bfd_h_get_32 (abfd, ldsym->l_parm));
      if (ldsym->_l._l_name[0] != 0)
        printf ("%-.8s", ldsym->_l._l_name);
      else
        {
          unsigned int off = bfd_h_get_32 (abfd, ldsym->_l._l_l._l_offset);
          if (off > stlen)
            printf (_("(bad offset: %u)"), off);
          else
            printf ("%s", ldr_data + stoff + off);
        }
      putchar ('\n');
    }

  printf (_("Dynamic relocs:\n"));
  /* Do not translate fields name.  */
  printf ("  vaddr    sec    sz typ   sym\n");
  ldrel = (struct external_ldrel *)(ldr_data + sizeof (*lhdr)
                                    + ndsyms * sizeof (*ldsym));
  for (i = 0; i < ndrel; i++, ldrel++)
    {
      unsigned int rsize;
      unsigned int rtype;
      unsigned int symndx;

      rsize = bfd_h_get_8 (abfd, ldrel->l_rtype + 0);
      rtype = bfd_h_get_8 (abfd, ldrel->l_rtype + 1);

      printf ("  %08x %3u %c%c %2u ",
              (unsigned)bfd_h_get_32 (abfd, ldrel->l_vaddr),
              (unsigned)bfd_h_get_16 (abfd, ldrel->l_rsecnm),
              rsize & 0x80 ? 'S' : 'U',
              rsize & 0x40 ? 'm' : ' ',
              (rsize & 0x3f) + 1);
      dump_value (rtype_xlat, rtype, 6);
      symndx = bfd_h_get_32 (abfd, ldrel->l_symndx);
      switch (symndx)
        {
        case 0:
          printf (".text");
          break;
        case 1:
          printf (".data");
          break;
        case 2:
          printf (".bss");
          break;
        default:
          printf ("%u", symndx - 3);
          break;
        }
      putchar ('\n');
    }

  printf (_("Import files:\n"));
  p = (char *)ldr_data + impoff;
  for (i = 0; i < nimpid; i++)
    {
      int n1, n2, n3;

      n1 = strlen (p);
      n2 = strlen (p + n1 + 1);
      n3 = strlen (p + n1 + 1 + n2+ 1);
      printf (" %2u: %s,%s,%s\n", i,
              p, p + n1 + 1, p + n1 + n2 + 2);
      p += n1 + n2 + n3 + 3;
    }

  free (ldr_data);
}

/* Dump xcoff exception section.  */

static void
dump_xcoff32_except (bfd *abfd, struct xcoff_dump *data)
{
  asection *sec;
  bfd_size_type size = 0;
  bfd_byte *excp_data;
  struct external_exceptab *exceptab;
  unsigned int i;

  sec = bfd_get_section_by_name (abfd, ".except");

  if (sec == NULL)
    {
      printf (_("no .except section in file\n"));
      return;
    }
  size = bfd_section_size (sec);
  excp_data = (bfd_byte *) xmalloc (size);
  bfd_get_section_contents (abfd, sec, excp_data, 0, size);
  exceptab = (struct external_exceptab *)excp_data;

  printf (_("Exception table:\n"));
  /* Do not translate fields name.  */
  printf ("lang reason sym/addr\n");
  for (i = 0; i * sizeof (*exceptab) < size; i++, exceptab++)
    {
      unsigned int reason;
      unsigned int addr;

      addr = bfd_get_32 (abfd, exceptab->e_addr.e_paddr);
      reason = bfd_get_8 (abfd, exceptab->e_reason);
      printf ("  %02x     %02x ",
              (unsigned) bfd_get_8 (abfd, exceptab->e_lang), reason);
      if (reason == 0)
        xcoff32_print_symbol (data, addr);
      else
        printf ("@%08x", addr);
      putchar ('\n');
    }
  free (excp_data);
}

/* Dump xcoff type-check section.  */

static void
dump_xcoff32_typchk (bfd *abfd)
{
  asection *sec;
  bfd_size_type size = 0;
  bfd_byte *data;
  unsigned int i;

  sec = bfd_get_section_by_name (abfd, ".typchk");

  if (sec == NULL)
    {
      printf (_("no .typchk section in file\n"));
      return;
    }
  size = bfd_section_size (sec);
  data = (bfd_byte *) xmalloc (size);
  bfd_get_section_contents (abfd, sec, data, 0, size);

  printf (_("Type-check section:\n"));
  /* Do not translate field names.  */
  printf ("offset    len  lang-id general-hash language-hash\n");
  for (i = 0; i < size;)
    {
      unsigned int len;

      len = bfd_get_16 (abfd, data + i);
      printf ("%08x: %-4u ", i, len);
      i += 2;

      if (len == 10)
        {
          /* Expected format.  */
          printf ("%04x    %08x     %08x\n",
                  (unsigned) bfd_get_16 (abfd, data + i),
                  (unsigned) bfd_get_32 (abfd, data + i + 2),
                  (unsigned) bfd_get_32 (abfd, data + i + 2 + 4));
        }
      else
        {
          unsigned int j;

          for (j = 0; j < len; j++)
            {
              if (j % 16 == 0)
                printf ("\n    ");
              printf (" %02x", (unsigned char)data[i + j]);
            }
          putchar ('\n');
        }
      i += len;
    }
  free (data);
}

/* Dump xcoff traceback tags section.  */

static void
dump_xcoff32_tbtags (bfd *abfd,
                     const char *text, bfd_size_type text_size,
                     unsigned int text_start, unsigned int func_start)
{
  unsigned int i;

  if (func_start - text_start > text_size)
    {
      printf (_(" address beyond section size\n"));
      return;
    }
  for (i = func_start - text_start; i < text_size; i+= 4)
    if (bfd_get_32 (abfd, text + i) == 0)
      {
        unsigned int tb1;
        unsigned int tb2;
        unsigned int off;

        printf (_(" tags at %08x\n"), i + 4);
        if (i + 8 >= text_size)
          goto truncated;

        tb1 = bfd_get_32 (abfd, text + i + 4);
        tb2 = bfd_get_32 (abfd, text + i + 8);
        off = i + 12;
        printf (" version: %u, lang: %u, global_link: %u, is_eprol: %u, has_tboff: %u, int_proc: %u\n",
                (tb1 >> 24) & 0xff,
                (tb1 >> 16) & 0xff,
                (tb1 >> 15) & 1,
                (tb1 >> 14) & 1,
                (tb1 >> 13) & 1,
                (tb1 >> 12) & 1);
        printf (" has_ctl: %u, tocless: %u, fp_pres: %u, log_abort: %u, int_hndl: %u\n",
                (tb1 >> 11) & 1,
                (tb1 >> 10) & 1,
                (tb1 >> 9) & 1,
                (tb1 >> 8) & 1,
                (tb1 >> 7) & 1);
        printf (" name_pres: %u, uses_alloca: %u, cl_dis_inv: %u, saves_cr: %u, saves_lr: %u\n",
                (tb1 >> 6) & 1,
                (tb1 >> 5) & 1,
                (tb1 >> 2) & 7,
                (tb1 >> 1) & 1,
                (tb1 >> 0) & 1);
        printf (" stores_bc: %u, fixup: %u, fpr_saved: %-2u, spare3: %u, gpr_saved: %-2u\n",
                (tb2 >> 31) & 1,
                (tb2 >> 30) & 1,
                (tb2 >> 24) & 63,
                (tb2 >> 22) & 3,
                (tb2 >> 16) & 63);
        printf (" fixparms: %-3u  floatparms: %-3u  parm_on_stk: %u\n",
                (tb2 >> 8) & 0xff,
                (tb2 >> 1) & 0x7f,
                (tb2 >> 0) & 1);

        if (((tb2 >> 1) & 0x7fff) != 0)
          {
            unsigned int parminfo;

            if (off >= text_size)
              goto truncated;
            parminfo = bfd_get_32 (abfd, text + off);
            off += 4;
            printf (" parminfo: 0x%08x\n", parminfo);
          }

        if ((tb1 >> 13) & 1)
          {
            unsigned int tboff;

            if (off >= text_size)
              goto truncated;
            tboff = bfd_get_32 (abfd, text + off);
            off += 4;
            printf (" tb_offset: 0x%08x (start=0x%08x)\n",
                    tboff, text_start + i - tboff);
          }
        if ((tb1 >> 7) & 1)
          {
            unsigned int hand_mask;

            if (off >= text_size)
              goto truncated;
            hand_mask = bfd_get_32 (abfd, text + off);
            off += 4;
            printf (" hand_mask_offset: 0x%08x\n", hand_mask);
          }
        if ((tb1 >> 11) & 1)
          {
            unsigned int ctl_info;
            unsigned int j;

            if (off >= text_size)
              goto truncated;
            ctl_info = bfd_get_32 (abfd, text + off);
            off += 4;
            printf (_(" number of CTL anchors: %u\n"), ctl_info);
            for (j = 0; j < ctl_info; j++)
              {
                if (off >= text_size)
                  goto truncated;
                printf ("  CTL[%u]: %08x\n",
                        j, (unsigned)bfd_get_32 (abfd, text + off));
                off += 4;
              }
          }
        if ((tb1 >> 6) & 1)
          {
            unsigned int name_len;
            unsigned int j;

            if (off >= text_size)
              goto truncated;
            name_len = bfd_get_16 (abfd, text + off);
            off += 2;
            printf (_(" Name (len: %u): "), name_len);
            if (off + name_len >= text_size)
              {
                printf (_("[truncated]\n"));
                goto truncated;
              }
            for (j = 0; j < name_len; j++)
              if (ISPRINT (text[off + j]))
                putchar (text[off + j]);
              else
                printf ("[%02x]", (unsigned char)text[off + j]);
            putchar ('\n');
            off += name_len;
          }
        if ((tb1 >> 5) & 1)
          {
            if (off >= text_size)
              goto truncated;
            printf (" alloca reg: %u\n",
                    (unsigned) bfd_get_8 (abfd, text + off));
            off++;
          }
        printf (_(" (end of tags at %08x)\n"), text_start + off);
        return;
      }
  printf (_(" no tags found\n"));
  return;

 truncated:
  printf (_(" Truncated .text section\n"));
  return;
}

static void
dump_xcoff32_traceback (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;
  unsigned int scnum_text = -1;
  unsigned int text_vma;
  asection *text_sec;
  bfd_size_type text_size;
  char *text;

  if (data->syms == NULL || data->sects == NULL)
    return;

  /* Read text section.  */
  text_sec = bfd_get_section_by_name (abfd, ".text");
  if (text_sec == NULL)
    return;
  text_vma = bfd_section_vma (text_sec);

  text_size = bfd_section_size (text_sec);
  text = (char *) xmalloc (text_size);
  bfd_get_section_contents (abfd, text_sec, text, 0, text_size);

  for (i = 0; i < data->nscns; i++)
    if (data->sects[i].flags == STYP_TEXT)
      {
        scnum_text = i + 1;
        break;
      }
  if (scnum_text == (unsigned int)-1)
    return;

  for (i = 0; i < data->nsyms; i++)
    {
      union xcoff32_symbol *s = &data->syms[i];

      switch (s->sym.sclass)
        {
        case C_EXT:
        case C_HIDEXT:
        case C_WEAKEXT:
          if (s->sym.scnum == scnum_text
              && s->sym.numaux > 0)
            {
              union external_auxent *aux = &s[s->sym.numaux].aux;

              unsigned int smtyp;
              unsigned int smclas;

              smtyp = bfd_h_get_8 (abfd, aux->x_csect.x_smtyp);
              smclas = bfd_h_get_8 (abfd, aux->x_csect.x_smclas);
              if (SMTYP_SMTYP (smtyp) == XTY_LD
                  && (smclas == XMC_PR
                      || smclas == XMC_GL
                      || smclas == XMC_XO))
                {
                  printf ("%08x: ", s->sym.val);
                  xcoff32_print_symbol (data, i);
                  putchar ('\n');
                  dump_xcoff32_tbtags (abfd, text, text_size,
                                       text_vma, s->sym.val);
                }
            }
          break;
        default:
          break;
        }
      i += s->sym.numaux;
    }
  free (text);
}

/* Dump the TOC symbols.  */

static void
dump_xcoff32_toc (bfd *abfd, struct xcoff_dump *data)
{
  unsigned int i;
  unsigned int nbr_ent;
  unsigned int size;

  printf (_("TOC:\n"));

  if (data->syms == NULL)
    return;

  nbr_ent = 0;
  size = 0;

  for (i = 0; i < data->nsyms; i++)
    {
      union xcoff32_symbol *s = &data->syms[i];

      switch (s->sym.sclass)
        {
        case C_EXT:
        case C_HIDEXT:
        case C_WEAKEXT:
          if (s->sym.numaux > 0)
            {
              union external_auxent *aux = &s[s->sym.numaux].aux;
              unsigned int smclas;
              unsigned int ent_sz;

              smclas = bfd_h_get_8 (abfd, aux->x_csect.x_smclas);
              if (smclas == XMC_TC
                  || smclas == XMC_TD
                  || smclas == XMC_TC0)
                {
                  ent_sz = bfd_h_get_32 (abfd, aux->x_scn.x_scnlen);
                  printf ("%08x %08x ",
                          s->sym.val, ent_sz);
                  xcoff32_print_symbol (data, i);
                  putchar ('\n');
                  nbr_ent++;
                  size += ent_sz;
                }
            }
          break;
        default:
          break;
        }
      i += s->sym.numaux;
    }
  printf (_("Nbr entries: %-8u Size: %08x (%u)\n"),
          nbr_ent, size, size);
}

/* Handle an rs6000 xcoff file.  */

static void
dump_xcoff32 (bfd *abfd, struct external_filehdr *fhdr)
{
  struct xcoff_dump data;

  data.nscns = bfd_h_get_16 (abfd, fhdr->f_nscns);
  data.symptr = bfd_h_get_32 (abfd, fhdr->f_symptr);
  data.nsyms = bfd_h_get_32 (abfd, fhdr->f_nsyms);
  data.opthdr = bfd_h_get_16 (abfd, fhdr->f_opthdr);
  data.sects = NULL;
  data.syms = NULL;
  data.strings = NULL;
  data.strings_size = 0;

  if (options[OPT_FILE_HEADER].selected)
    dump_xcoff32_file_header (abfd, fhdr, &data);

  if (options[OPT_AOUT].selected)
    dump_xcoff32_aout_header (abfd, &data);

  if (options[OPT_SYMS].selected
      || options[OPT_RELOCS].selected
      || options[OPT_LINENO].selected
      || options[OPT_TRACEBACK].selected)
    xcoff32_read_sections (abfd, &data);

  if (options[OPT_SECTIONS].selected)
    dump_xcoff32_sections_header (abfd, &data);

  if (options[OPT_SYMS].selected
      || options[OPT_RELOCS].selected
      || options[OPT_LINENO].selected
      || options[OPT_EXCEPT].selected
      || options[OPT_TRACEBACK].selected
      || options[OPT_TOC].selected)
    xcoff32_read_symbols (abfd, &data);

  if (options[OPT_SYMS].selected)
    dump_xcoff32_symbols (abfd, &data);

  if (options[OPT_RELOCS].selected)
    dump_xcoff32_relocs (abfd, &data);

  if (options[OPT_LINENO].selected)
    dump_xcoff32_lineno (abfd, &data);

  if (options[OPT_LOADER].selected)
    dump_xcoff32_loader (abfd);

  if (options[OPT_EXCEPT].selected)
    dump_xcoff32_except (abfd, &data);

  if (options[OPT_TYPCHK].selected)
    dump_xcoff32_typchk (abfd);

  if (options[OPT_TRACEBACK].selected)
    dump_xcoff32_traceback (abfd, &data);

  if (options[OPT_TOC].selected)
    dump_xcoff32_toc (abfd, &data);

  free (data.sects);
  free (data.strings);
  free (data.syms);
}

/* Dump ABFD (according to the options[] array).  */

static void
xcoff_dump_obj (bfd *abfd)
{
  struct external_filehdr fhdr;
  unsigned short magic;

  /* Read file header.  */
  if (bfd_seek (abfd, 0, SEEK_SET) != 0
      || bfd_bread (&fhdr, sizeof (fhdr), abfd) != sizeof (fhdr))
    {
      non_fatal (_("cannot read header"));
      return;
    }

  /* Decoding.  We don't use the bfd/coff function to get all the fields.  */
  magic = bfd_h_get_16 (abfd, fhdr.f_magic);
  if (options[OPT_FILE_HEADER].selected)
    {
      printf (_("File header:\n"));
      printf (_("  magic:         0x%04x (0%04o)  "), magic, magic);
      switch (magic)
        {
        case U802WRMAGIC:
          printf (_("(WRMAGIC: writable text segments)"));
          break;
        case U802ROMAGIC:
          printf (_("(ROMAGIC: readonly sharablee text segments)"));
          break;
        case U802TOCMAGIC:
          printf (_("(TOCMAGIC: readonly text segments and TOC)"));
          break;
        default:
          printf (_("unknown magic"));
	  break;
        }
      putchar ('\n');
    }
  if (magic == U802ROMAGIC || magic == U802WRMAGIC || magic == U802TOCMAGIC)
    dump_xcoff32 (abfd, &fhdr);
  else
    printf (_("  Unhandled magic\n"));
}

/* Handle an AIX dumpx core file.  */

static void
dump_dumpx_core (bfd *abfd, struct external_core_dumpx *hdr)
{
  if (options[OPT_FILE_HEADER].selected)
    {
      printf ("  signal:     %u\n",
	      (unsigned) bfd_h_get_8 (abfd, hdr->c_signo));
      printf ("  flags:      0x%02x\n",
	      (unsigned) bfd_h_get_8 (abfd, hdr->c_flag));
      printf ("  entries:    %u\n",
	      (unsigned) bfd_h_get_16 (abfd, hdr->c_entries));
#ifdef BFD64
      printf ("  fdsinfox:   offset: 0x%08" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_fdsinfox));
      printf ("  loader:     offset: 0x%08" PRIx64 ", "
	      "size: 0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_loader),
	      bfd_h_get_64 (abfd, hdr->c_lsize));
      printf ("  thr:        offset: 0x%08" PRIx64 ", nbr: %u\n",
	      bfd_h_get_64 (abfd, hdr->c_thr),
	      (unsigned) bfd_h_get_32 (abfd, hdr->c_n_thr));
      printf ("  segregions: offset: 0x%08" PRIx64 ", "
	      "nbr: %" PRIu64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_segregion),
	      bfd_h_get_64 (abfd, hdr->c_segs));
      printf ("  stack:      offset: 0x%08" PRIx64 ", "
	      "org: 0x%" PRIx64 ", "
	      "size: 0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_stack),
	      bfd_h_get_64 (abfd, hdr->c_stackorg),
	      bfd_h_get_64 (abfd, hdr->c_size));
      printf ("  data:       offset: 0x%08" PRIx64 ", "
	      "org: 0x%" PRIx64 ", "
	      "size: 0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_data),
	      bfd_h_get_64 (abfd, hdr->c_dataorg),
	      bfd_h_get_64 (abfd, hdr->c_datasize));
      printf ("  sdata:         org: 0x%" PRIx64 ", "
	      "size: 0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_sdorg),
	      bfd_h_get_64 (abfd, hdr->c_sdsize));
      printf ("  vmmregions: offset: 0x%" PRIx64 ", "
	      "num: 0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_vmm),
	      bfd_h_get_64 (abfd, hdr->c_vmmregions));
      printf ("  impl:       0x%08x\n",
	      (unsigned) bfd_h_get_32 (abfd, hdr->c_impl));
      printf ("  cprs:       0x%" PRIx64 "\n",
	      bfd_h_get_64 (abfd, hdr->c_cprs));
#endif
    }
  if (options[OPT_LDINFO].selected)
    {
#ifdef BFD64
      file_ptr off = (file_ptr) bfd_h_get_64 (abfd, hdr->c_loader);
      bfd_size_type len = (bfd_size_type) bfd_h_get_64 (abfd, hdr->c_lsize);
      char *ldr;

      ldr = xmalloc (len);
      if (bfd_seek (abfd, off, SEEK_SET) != 0
	  || bfd_bread (ldr, len, abfd) != len)
	non_fatal (_("cannot read loader info table"));
      else
	{
	  char *p;

	  printf ("\n"
		  "ld info:\n");
	  printf ("  next     core off textorg  textsize dataorg  datasize\n");
	  p = ldr;
	  while (1)
	    {
	      struct external_ld_info32 *l = (struct external_ld_info32 *)p;
	      unsigned int next;
	      size_t n1;

	      next = bfd_h_get_32 (abfd, l->ldinfo_next);
	      printf ("  %08x %08x %08x %08x %08x %08x\n",
		      next,
		      (unsigned) bfd_h_get_32 (abfd, l->core_offset),
		      (unsigned) bfd_h_get_32 (abfd, l->ldinfo_textorg),
		      (unsigned) bfd_h_get_32 (abfd, l->ldinfo_textsize),
		      (unsigned) bfd_h_get_32 (abfd, l->ldinfo_dataorg),
		      (unsigned) bfd_h_get_32 (abfd, l->ldinfo_datasize));
	      n1 = strlen ((char *) l->ldinfo_filename);
	      printf ("    %s %s\n",
		      l->ldinfo_filename, l->ldinfo_filename + n1 + 1);
	      if (next == 0)
		break;
	      p += next;
	    }
	}
#else
      printf (_("\n"
		"ldinfo dump not supported in 32 bits environments\n"));
#endif
    }
}

/* Dump a core file.  */

static void
xcoff_dump_core (bfd *abfd)
{
  struct external_core_dumpx hdr;
  unsigned int version;

  /* Read file header.  */
  if (bfd_seek (abfd, 0, SEEK_SET) != 0
      || bfd_bread (&hdr, sizeof (hdr), abfd) != sizeof (hdr))
    {
      non_fatal (_("cannot core read header"));
      return;
    }

  version = bfd_h_get_32 (abfd, hdr.c_version);
  if (options[OPT_FILE_HEADER].selected)
    {
      printf (_("Core header:\n"));
      printf (_("  version:    0x%08x  "), version);
      switch (version)
	{
	case CORE_DUMPX_VERSION:
	  printf (_("(dumpx format - aix4.3 / 32 bits)"));
	  break;
	case CORE_DUMPXX_VERSION:
	  printf (_("(dumpxx format - aix5.0 / 64 bits)"));
	  break;
	default:
	  printf (_("unknown format"));
	  break;
	}
      putchar ('\n');
    }
  if (version == CORE_DUMPX_VERSION)
    dump_dumpx_core (abfd, &hdr);
  else
    printf (_("  Unhandled magic\n"));
}

/* Dump an XCOFF file.  */

static void
xcoff_dump (bfd *abfd)
{
  /* We rely on BFD to decide if the file is a core file.  Note that core
     files are only supported on native environment by BFD.  */
  switch (bfd_get_format (abfd))
    {
    case bfd_core:
      xcoff_dump_core (abfd);
      break;
    default:
      xcoff_dump_obj (abfd);
      break;
    }
}

/* Vector for xcoff.  */

const struct objdump_private_desc objdump_private_desc_xcoff =
  {
    xcoff_help,
    xcoff_filter,
    xcoff_dump,
    options
  };
