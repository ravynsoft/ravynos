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

#ifndef _Elf_h_
#define	_Elf_h_

#include <string.h>
#include "ansidecl.h"
#include "bfd.h"
#include "elf/common.h"
#include "elf/internal.h"

#include "Data_window.h"
#include "Emsg.h"

class Symbol;
class DbeFile;
template <class ITEM> class Vector;
template <typename Key_t, typename Value_t> class Map;

#define GELF_R_SYM(info)    ((info)>>32)
#define GELF_ST_TYPE(info)  ((info) & 0xf)
#define GELF_ST_BIND(info)  ((info) >> 4)
#define GELF_R_TYPE(info)   ((((uint64_t)(info))<<56)>>56)

#define	SHF_SUNW_ABSENT		0x00200000	/* section data not present */

// Ancillary values.
#define ANC_SUNW_NULL       0
#define ANC_SUNW_CHECKSUM   1   /* elf checksum */
#define ANC_SUNW_MEMBER     2   /* name of ancillary group object */
#define ANC_SUNW_NUM        3


typedef struct S_Elf64_Dyn Elf64_Dyn;
typedef struct S_Elf64_Ancillary Elf64_Ancillary;

typedef struct
{
  void *d_buf;
  uint64_t d_flags;
  uint64_t d_size;
  uint64_t d_off;       // offset into section
  uint64_t d_align;     // alignment in section
} Elf_Data;

class Elf : public DbeMessages, public Data_window
{
public:
  enum Elf_status
  {
    ELF_ERR_NONE,
    ELF_ERR_CANT_OPEN_FILE,
    ELF_ERR_CANT_MMAP,
    ELF_ERR_BIG_FILE,
    ELF_ERR_BAD_ELF_FORMAT,
    ELF_ERR_READ_FILE
  };

  Elf (char *_fname);
  ~Elf ();

  static void elf_init ();
  static unsigned elf_version (unsigned ver);
  static Elf *elf_begin (char *_fname, Elf_status *stp = NULL);

  unsigned int elf_get_sec_num (const char *sec_name);
  char *get_sec_name (unsigned int sec);
  Elf_Internal_Ehdr *elf_getehdr ();
  Elf_Internal_Phdr *get_phdr (unsigned int ndx);
  Elf_Internal_Shdr *get_shdr (unsigned int ndx);
  Elf64_Dyn *elf_getdyn (Elf_Internal_Phdr *phdr, unsigned int ndx, Elf64_Dyn *pdyn);
  Elf_Data *elf_getdata (unsigned int sec);
  int64_t elf_checksum ();
  uint64_t get_baseAddr();
  char *elf_strptr (unsigned int sec, uint64_t off);
  Elf_Internal_Sym *elf_getsym (Elf_Data *edta, unsigned int ndx, Elf_Internal_Sym *dst);
  Elf_Internal_Rela *elf_getrel (Elf_Data *edta, unsigned int ndx, Elf_Internal_Rela *dst);
  Elf_Internal_Rela *elf_getrela (Elf_Data *edta, unsigned int ndx, Elf_Internal_Rela *dst);
  Elf64_Ancillary *elf_getancillary (Elf_Data *edta, unsigned int ndx, Elf64_Ancillary *dst);
  Elf *find_ancillary_files (char *lo_name); // read the .gnu_debuglink and .SUNW_ancillary seections
  char *get_location ();
  char *dump ();
  void dump_elf_sec ();

  static inline int64_t
  normalize_checksum (int64_t chk)
  {
    return (chk == 0xffffffff || chk == -1) ? 0 : chk;
  };

  inline bool
  is_Intel ()
  {
    return elf_datatype == ELFDATA2LSB;
  };

  inline int
  elf_getclass ()
  {
    return elf_class;
  };

  inline int
  elf_getdatatype ()
  {
    return elf_datatype;
  };

  Elf_status status;
  Vector<Elf*> *ancillary_files;
  Elf *gnu_debug_file;
  DbeFile *dbeFile;
  Map<const char*, Symbol*> *elfSymbols;
  unsigned int gnuLink, analyzerInfo, SUNW_ldynsym, stab, stabStr, symtab, dynsym;
  unsigned int stabIndex, stabIndexStr, stabExcl, stabExclStr, info, plt;
  bool dwarf;

protected:
  Elf *get_related_file (const char *lo_name, const char *nm);
  int elf_class;
  int elf_datatype;
  Elf_Internal_Ehdr *ehdrp;
  Elf_Data **data;
  bfd *abfd;
  static int bfd_status;
};


class ElfReloc
{
public:
  struct Sreloc
  {
    long long offset;
    long long value;
    int stt_type;
  };

  static ElfReloc *get_elf_reloc (Elf *elf, char *sec_name, ElfReloc *rlc);
  ElfReloc (Elf *_elf);
  ~ElfReloc ();
  long long get_reloc_addr (long long offset);
  void dump ();
  void dump_rela_debug_sec (int sec);

private:
  Elf *elf;
  Vector<Sreloc *> *reloc;
  int cur_reloc_ind;
};

#endif
