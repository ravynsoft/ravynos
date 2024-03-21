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

#ifndef _Dwarf_h_
#define _Dwarf_h_ 1

#include "dwarf2.h"

#include "Stabs.h"
#include "dbe_structs.h"
#include "DwarfLib.h"

enum
{
  /* ICC extensions */
  DW_AT_icc_flags           = 0x3b01,
  DW_TAG_icc_compile_unit   = 0x7000,

  /* Sun extensions */
  DW_ATCF_SUN_branch_target = 0x46,
  DW_AT_SUN_command_line    = 0x2205,
  DW_AT_SUN_func_offsets    = 0x2211,
  DW_AT_SUN_cf_kind         = 0x2212,
  DW_AT_SUN_func_offset     = 0x2216,
  DW_AT_SUN_memop_type_ref  = 0x2217,
  DW_AT_SUN_profile_id      = 0x2218,
  DW_AT_SUN_memop_signature = 0x2219,
  DW_AT_SUN_obj_dir         = 0x2220,
  DW_AT_SUN_obj_file        = 0x2221,
  DW_AT_SUN_original_name   = 0x2222,
  DW_AT_SUN_link_name       = 0x2226,

  DW_TAG_SUN_codeflags      = 0x4206,
  DW_TAG_SUN_memop_info     = 0x4207,
  DW_TAG_SUN_dtor_info      = 0x420a,
  DW_TAG_SUN_dtor           = 0x420b,

  DW_LANG_SUN_Assembler     = 0x9001
};


class LoadObject;
class Module;
class DwrCU;
class DwrSec;

class Dwarf
{
public:
  Dwarf (Stabs *_stabs);
  ~Dwarf ();
  bool archive_Dwarf (LoadObject *lo);
  void srcline_Dwarf (Module *module);
  void read_hwcprof_info (Module *module);

  Stabs::Stab_status status;
  Vector<DwrCU *> *dwrCUs;
  DwrSec *debug_infoSec;
  DwrSec *debug_abbrevSec;
  DwrSec *debug_strSec;
  DwrSec *debug_lineSec;
  DwrSec *debug_line_strSec;
  DwrSec *debug_rangesSec;
  Elf *elf;
  Stabs *stabs;

private:
  DwrSec *dwrGetSec (const char *sec_name);
};

#endif  /* _Dwarf_h_ */
