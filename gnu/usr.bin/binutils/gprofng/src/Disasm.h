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

#ifndef _DISASM_H
#define _DISASM_H

#include "disassemble.h"

class Data_window;
class Stabs;
class StringBuilder;
enum Platform_t;

class Disasm
{
public:
  Disasm (char *fname);
  Disasm (Platform_t _platform, Stabs *_stabs);
  ~Disasm ();
  void remove_disasm_hndl (void *hndl);
  void *get_disasm_hndl (uint64_t vaddr, uint64_t f_offset, size_t size);
  int get_instr_size (uint64_t vaddr, void *hndl);
  void set_addr_end (uint64_t end_address);

  void
  set_hex_visible (int set)
  {
    hex_visible = set;
  }

  char *get_disasm (uint64_t inst_address, uint64_t end_address,
		 uint64_t start_address, uint64_t f_offset, int64_t &inst_size);
  void set_img_name (char *fname);  // Only for dynfunc

  StringBuilder *dis_str;

private:
  void disasm_open ();

  disassemble_info dis_info;
  Data_window *dwin;
  Stabs *stabs, *my_stabs;
  Platform_t platform;
  char addr_fmt[32];
  int hex_visible;
  bool need_swap_endian;
};

#endif  /* _DISASM_H */
