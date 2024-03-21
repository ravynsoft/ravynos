/* Disassemble Xilinx microblaze instructions.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef MICROBLAZE_DIS_H
#define MICROBLAZE_DIS_H 1

#ifdef __cplusplus
extern "C" {
#endif

extern enum microblaze_instr microblaze_decode_insn (long, int *, int *,
						     int *, int *);
extern unsigned long microblaze_get_target_address (long, bool, int,
						    long, long, long,
						    bool *, bool *);
extern enum microblaze_instr get_insn_microblaze (long, bool *,
						  enum microblaze_instr_type *,
  		     				  short *);

#ifdef __cplusplus
}
#endif

#endif /* microblaze-dis.h */
