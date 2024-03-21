/* Definitions for decoding the moxie opcode table.
   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by Anthony Green (green@moxielogic.com).

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Form 1 instructions come in different flavors:

    Some have no arguments                          (MOXIE_F1_NARG)
    Some only use the A operand                     (MOXIE_F1_A)
    Some use A and B registers                      (MOXIE_F1_AB)
    Some use A and consume a 4 byte immediate value (MOXIE_F1_A4)
    Some use just a 4 byte immediate value          (MOXIE_F1_4)
    Some use just a 4 byte memory address           (MOXIE_F1_M)
    Some use B and an indirect A                    (MOXIE_F1_AiB)
    Some use A and an indirect B                    (MOXIE_F1_ABi)
    Some consume a 4 byte immediate value and use X (MOXIE_F1_4A)
    Some use B and an indirect A plus 2 byte offset (MOXIE_F1_AiB2)
    Some use A and an indirect B plus 2 byte offset (MOXIE_F1_ABi2)

  Form 2 instructions also come in different flavors:

    Some have no arguments                          (MOXIE_F2_NARG)
    Some use the A register and an 8-bit value      (MOXIE_F2_A8V)

  Form 3 instructions also come in different flavors:

    Some have no arguments                          (MOXIE_F3_NARG)
    Some have a 10-bit PC relative operand          (MOXIE_F3_PCREL).  */

#define MOXIE_F1_NARG 0x100
#define MOXIE_F1_A    0x101
#define MOXIE_F1_AB   0x102
/* #define MOXIE_F1_ABC  0x103 */
#define MOXIE_F1_A4   0x104
#define MOXIE_F1_4    0x105
#define MOXIE_F1_AiB  0x106
#define MOXIE_F1_ABi  0x107
#define MOXIE_F1_4A   0x108
#define MOXIE_F1_AiB2 0x109
#define MOXIE_F1_ABi2 0x10a
#define MOXIE_F1_M    0x10b

#define MOXIE_F2_NARG 0x200
#define MOXIE_F2_A8V  0x201

#define MOXIE_F3_NARG  0x300
#define MOXIE_F3_PCREL 0x301

#define MOXIE_BAD     0x400

typedef struct moxie_opc_info_t
{
  short         opcode;
  unsigned      itype;
  const char *  name;
} moxie_opc_info_t;

extern const moxie_opc_info_t moxie_form1_opc_info[128];
extern const moxie_opc_info_t moxie_form2_opc_info[4];
extern const moxie_opc_info_t moxie_form3_opc_info[16];
