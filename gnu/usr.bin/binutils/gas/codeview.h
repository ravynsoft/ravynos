/* codeview.h - CodeView debug support
   Copyright (C) 2022-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* Header files referred to below can be found in Microsoft's PDB
   repository: https://github.com/microsoft/microsoft-pdb.  */

#ifndef GAS_CODEVIEW_H
#define GAS_CODEVIEW_H

#define CV_SIGNATURE_C13	4

#define DEBUG_S_SYMBOLS		0xf1
#define DEBUG_S_LINES		0xf2
#define DEBUG_S_STRINGTABLE	0xf3
#define DEBUG_S_FILECHKSMS	0xf4

#define S_OBJNAME		0x1101
#define S_COMPILE3		0x113c

#define CV_CFL_MASM		0x03

#define CV_CFL_80386		0x03
#define CV_CFL_X64		0xD0
#define CV_CFL_ARM64		0xF6

#define CHKSUM_TYPE_MD5		1

/* OBJNAMESYM in cvinfo.h */
struct OBJNAMESYM
{
  uint16_t length;
  uint16_t type;
  uint32_t signature;
};

/* COMPILESYM3 in cvinfo.h */
struct COMPILESYM3
{
  uint16_t length;
  uint16_t type;
  uint32_t flags;
  uint16_t machine;
  uint16_t frontend_major;
  uint16_t frontend_minor;
  uint16_t frontend_build;
  uint16_t frontend_qfe;
  uint16_t backend_major;
  uint16_t backend_minor;
  uint16_t backend_build;
  uint16_t backend_qfe;
} ATTRIBUTE_PACKED;

/* filedata in dumpsym7.cpp */
struct file_checksum
{
  uint32_t file_id;
  uint8_t checksum_length;
  uint8_t checksum_type;
} ATTRIBUTE_PACKED;

/* CV_DebugSLinesHeader_t in cvinfo.h */
struct cv_lines_header
{
  uint32_t offset;
  uint16_t section;
  uint16_t flags;
  uint32_t length;
};

/* CV_DebugSLinesFileBlockHeader_t in cvinfo.h */
struct cv_lines_block
{
  uint32_t file_id;
  uint32_t num_lines;
  uint32_t length;
};

/* CV_Line_t in cvinfo.h */
struct cv_line
{
  uint32_t offset;
  uint32_t line_no;
};

extern void codeview_finish (void);
extern void codeview_generate_asm_lineno (void);

#endif
