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

#ifndef _PERFAN_DEBUG_H
#define _PERFAN_DEBUG_H

extern unsigned int mpmt_debug_opt;
// To set mpmt_debug_opt use:
//   MPMT_DEBUG=4095 ; export MPMT_DEBUG
#define DEBUG_FLAG          (mpmt_debug_opt & 1)
#define DUMP_ELF_SEC        (mpmt_debug_opt & 2)
#define DUMP_ELF_SYM        (mpmt_debug_opt & 4)
#define DUMP_RELA_SEC       (mpmt_debug_opt & 8)
#define DUMP_ELF_RELOC      DUMP_RELA_SEC
#define DUMP_DWARFLIB       (mpmt_debug_opt & 16)
#define DUMP_DWR_LINE_REGS  (mpmt_debug_opt & 32)
#define DUMP_USER_LABELS    (mpmt_debug_opt & 64)
#define DEBUG_MAPS          (mpmt_debug_opt & 128)
#define DEBUG_DBE_FILE      (mpmt_debug_opt & 256)
#define DEBUG_DATA_WINDOW   (mpmt_debug_opt & 512)
#define DEBUG_STABS         (mpmt_debug_opt & 1024)
#define DEBUG_DATAOBJ       (mpmt_debug_opt & 2048)
#define DEBUG_LOADOBJ       (mpmt_debug_opt & 4096)
#define DEBUG_SAXPARSER     (mpmt_debug_opt & 8192)
#define DUMP_JAVA_CLASS     (mpmt_debug_opt & 16384)
#define DEBUG_COMPARISON    (mpmt_debug_opt & 32768)
#define DEBUG_READ_AR       (mpmt_debug_opt & 65536)
#define DEBUG_ERR_MSG       (mpmt_debug_opt & 131072)
#define DUMP_JCLASS_READER  (mpmt_debug_opt & 262144)
#define DEBUG_DBE           (mpmt_debug_opt & 524288)
#define DEBUG_ARCHIVE       (mpmt_debug_opt & 1048576)
#define DEBUG_IO            (mpmt_debug_opt & 2097152)
#define DUMP_DYN_FILE       (mpmt_debug_opt & 4194304)
#define DUMP_JAR_FILE       (mpmt_debug_opt & 8388608)
#define DUMP_CALL_STACK     (mpmt_debug_opt & 16777216)
#define DEBUG_THREADS       (mpmt_debug_opt & 33554432)
#define DBE_USE_MMAP        (mpmt_debug_opt & 67108864)

#ifdef DEBUG

// Turn on assertion checking whenever debugging
#define ASSERTS 1

// debug macro - provides a clean way of inserting debugging code without
//  having the distracting #ifdef DEBUG ... #else ... #endif directives
//  interspersed throughout the code.  It also provides an easy way
//  to turn them off with no loss of efficiency.  It is not limited
//  to printf() commands; any code may be inserted.  Variables
//  needed only by the debugging code can be declared inside a
//  debug { ... } statement.
//
// usage:
//      debug <statement>
//  or,	debug { <statements> }
// If DEBUG is on, map "DEBUG_CODE" to nothing!
// This results in the <statement> being executed normally

#define DEBUG_CODE

#else
// If DEBUG is off, map "DEBUG_CODE" to something harmless.
// The clever hack used here is to use a conditional with a
// constant condition, which is optimized out by the compiler,
// so that <statement> is not present in the compiled code!

#define DEBUG_CODE if (0)

#endif /*DEBUG*/

#define Dprintf(x, ...) DEBUG_CODE if(x) fprintf(stderr, __VA_ARGS__)

#endif /* ! _DEBUG_H */
