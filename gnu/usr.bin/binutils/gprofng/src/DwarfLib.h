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

#ifndef _DWARFLIB_H_
#define _DWARFLIB_H_

#include "dwarf2.h"

class ElfReloc;
class Dwr_type;
class SourceFile;

template <class ITEM> class Vector;
template <class ITEM> class DbeArray;
template <typename Key_t, typename Value_t> class DefaultMap;

typedef uint64_t ULEB128;
typedef int64_t SLEB128;
typedef unsigned short Dwarf_Half;
typedef unsigned char Dwarf_Small;
typedef uint64_t Dwarf_Off;
typedef uint64_t Dwarf_Addr;
typedef uint64_t Dwarf_Unsigned;
typedef int64_t Dwarf_Die;
typedef int32_t Dwarf_Debug;
typedef int32_t Dwarf_Attribute;


class DwrSec
{
public:
  DwrSec (unsigned char *_data, uint64_t _size, bool _need_swap_endian, bool _addr32);
  DwrSec (DwrSec *secp, uint64_t _offset);
  ~DwrSec ();
  unsigned char Get_8 ();
  unsigned short Get_16 ();
  uint32_t Get_32 ();
  uint64_t Get_64 ();
  uint64_t GetRef ();
  uint64_t GetADDR ();
  uint64_t GetADDR_32 ();
  uint64_t GetADDR_64 ();
  uint64_t GetLong ();
  uint64_t ReadLength ();
  SLEB128 GetSLEB128 ();
  ULEB128 GetULEB128 ();
  char *GetString ();
  char *GetData (uint64_t len);
  uint32_t Get_24 ();
  uint64_t get_value (int dw_form);
  void dump (char *msg);

  inline uint32_t
  GetULEB128_32 ()
  {
    return (uint32_t) GetULEB128 ();
  }

  bool
  inRange (uint64_t left, uint64_t right)
  {
    return (offset >= left) && (offset < right);
  };

  ElfReloc *reloc;
  uint64_t sizeSec;
  uint64_t size;
  uint64_t offset;
  bool fmt64;
  bool addr32;
  bool need_swap_endian;
  int address_size;
  int segment_selector_size; // DWARF 5

private:
  bool isCopy;
  unsigned char *data;
  bool bounds_violation (uint64_t sz);
};

class DwrFileName
{
public:
  DwrFileName (char *_fname);
  ~DwrFileName ();
  uint64_t timestamp;
  uint64_t file_size;
  int dir_index;
  char *fname;
  char *path;
  bool isUsed;
};

class DwrLine
{
public:
  DwrLine ();
  ~DwrLine ();
  uint64_t address;
  uint32_t file;
  uint32_t line;
  uint32_t column;
};

class DwrInlinedSubr
{
public:
  DwrInlinedSubr (int64_t _abstract_origin, uint64_t _low_pc, uint64_t _high_pc,
		  int _file, int _line, int _level);
  void dump ();
  int64_t abstract_origin;
  uint64_t low_pc;
  uint64_t high_pc;
  int file;
  int line;
  int level;
};

class DwrLineRegs
{
public:
  DwrLineRegs (Dwarf *_dwarf, DwrSec *_secp, char *dirName);
  ~DwrLineRegs ();
  char *getPath (int fn);
  Vector<DwrLine *> *get_lines ();
  void dump ();

  Vector<DwrFileName *> *file_names;

private:
  void DoExtendedOpcode ();
  void DoStandardOpcode (int opcode);
  void DoSpecialOpcode (int opcode);
  void EmitLine ();
  void reset ();
  Vector <DwrFileName *> *read_file_names_dwarf5 ();

  Dwarf *dwarf;
  char *fname;
  uint64_t dir_index;
  uint64_t timestamp;
  uint64_t file_size;
  uint64_t address;
  int file;
  int line;
  int column;
  Dwarf_Half version;
  uint64_t op_index_register;
  Dwarf_Small maximum_operations_per_instruction;
  Dwarf_Small minimum_instruction_length;
  Dwarf_Small default_is_stmt;
  Dwarf_Small line_range;
  Dwarf_Small opcode_base;
  signed char line_base;
  bool is_stmt;
  bool basic_block;
  bool end_sequence;
  Vector<DwrLine *> *lines;
  Vector<DwrFileName *> *dir_names;
  Dwarf_Small *standard_opcode_length;
  DwrSec *debug_lineSec;
  uint64_t header_length;
  uint64_t opcode_start;
};

typedef struct Dwr_Attr
{
  union
  {
    char *str;
    unsigned char *block;
    uint64_t offset;
    int64_t val;
  } u;
  uint64_t len;     // length of u.str
  int at_form;
  int at_name;
} Dwr_Attr;

typedef struct Dwr_Tag
{
public:
  Dwr_Attr *get_attr (Dwarf_Half attr);
  void dump ();

  DbeArray<Dwr_Attr> *abbrevAtForm;
  int64_t die;
  int64_t offset;
  int firstAttribute;
  int lastAttribute;
  int tag;
  int hasChild;
  int num;
  int level;
} Dwr_Tag;

enum
{
  DW_DLV_OK,
  DW_DLV_NO_ENTRY,
  DW_DLV_ERROR,
  DW_DLV_BAD_ELF,
  DW_DLV_NO_DWARF,
  DW_DLV_WRONG_ARG
};

typedef struct DwrLocation
{
  uint64_t offset;
  uint64_t lc_number;
  uint64_t lc_number2;
  uint32_t op;
} DwrLocation;

typedef struct DwrAbbrevTable
{
  int64_t offset;
  int firstAtForm;
  int lastAtForm;
  int code;
  int tag;
  bool hasChild;
} DwrAbbrevTable;

class Dwarf_cnt
{
public:
  Dwarf_cnt ();
  int64_t cu_offset;
  int64_t parent;
  int64_t size;
  Module *module;
  char *name;
  Function *func;
  Function *fortranMAIN;
  datatype_t *dtype;
  DwrInlinedSubr *inlinedSubr;
  DefaultMap <int64_t, Dwr_type*> *dwr_types;
  int level;

  Dwr_type *get_dwr_type (int64_t cu_die_offset);
  Dwr_type *put_dwr_type (int64_t cu_die_offset, int tag);
  Dwr_type *put_dwr_type (Dwr_Tag *dwrTag);
};

class DwrCU
{
public:
  DwrCU (Dwarf *_dwarf);
  ~DwrCU ();
  Module *parse_cu_header (LoadObject *lo);
  void parseChild (Dwarf_cnt *ctx);
  void read_hwcprof_info (Dwarf_cnt *ctx);
  void map_dwarf_lines (Module *mod);
  int set_die (Dwarf_Die die);
  DwrLineRegs *get_dwrLineReg ();

  static char *at2str (int tag);
  static char *form2str (int tag);
  static char *tag2str (int tag);
  static char *lnct2str (int ty);

  uint64_t cu_header_offset;
  uint64_t cu_offset;
  uint64_t next_cu_offset;
  Vector<DwrInlinedSubr*> *dwrInlinedSubrs;
  Vector<SourceFile *> *srcFiles;
  bool isMemop;
  bool isGNU;

private:
  void build_abbrevTable (DwrSec *debug_abbrevSec, uint64_t stmt_list_offset);
  Function *append_Function (Dwarf_cnt *ctx);
  void parse_inlined_subroutine (Dwarf_cnt *ctx);
  uint64_t get_low_pc ();
  uint64_t get_high_pc (uint64_t low_pc);
  DwrLocation *dwr_get_location (DwrSec *secp, DwrLocation *lp);
  int read_data_attr (Dwarf_Half attr, int64_t *retVal);
  int read_ref_attr (Dwarf_Half attr, int64_t *retVal);
  char *get_linkage_name ();
  char *Dwarf_string (Dwarf_Half attr);
  int64_t Dwarf_data (Dwarf_Half attr);
  int64_t Dwarf_ref (Dwarf_Half attr);
  DwrSec *Dwarf_block (Dwarf_Half attr);
  Dwarf_Addr Dwarf_addr (Dwarf_Half attr);
  Dwarf_Addr Dwarf_location (Dwarf_Attribute attr);
  Sp_lang_code Dwarf_lang ();

  Dwarf *dwarf;
  DwrSec *debug_infoSec;
  uint64_t debug_abbrev_offset;
  uint64_t stmt_list_offset;  // offset in .debug_line section (DW_AT_stmt_list)
  char *comp_dir;             // compilation directory (DW_AT_comp_dir)
  Module *module;
  int unit_type;
  Dwarf_Half version;
  Dwarf_Small address_size;
  Dwr_Tag dwrTag;
  DwrLineRegs *dwrLineReg;
  DbeArray<DwrAbbrevTable> *abbrevTable;
  DbeArray<Dwr_Attr> *abbrevAtForm;
};

#endif /* _DWARFLIB_H_ */
