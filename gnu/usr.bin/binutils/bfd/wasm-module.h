/* BFD back-end for WebAssembly modules.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

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
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _WASM_MODULE_H
#define _WASM_MODULE_H

/* WebAssembly module file header.  Note that WASM_VERSION is a 32-bit
   little-endian integer, not an LEB128-encoded integer.  */
#define SIZEOF_WASM_MAGIC    4
#define WASM_MAGIC	     { 0x00, 'a', 's', 'm' }
#define SIZEOF_WASM_VERSION  4
#define WASM_VERSION	     { 0x01, 0x00, 0x00, 0x00 }

/* Prefix to use to form section names.  */
#define WASM_SECTION_PREFIX ".wasm."

/* NUMBER is currently unused, but is included for error checking purposes.  */
#define WASM_SECTION(number, name) (WASM_SECTION_PREFIX name)

/* Section names.  WASM_NAME_SECTION is the name of the named section
   named "name".  */
#define WASM_NAME_SECTION	   WASM_SECTION (0, "name")
#define WASM_RELOC_SECTION_PREFIX  WASM_SECTION (0, "reloc.")
#define WASM_LINKING_SECTION	   WASM_SECTION (0, "linking")
#define WASM_DYLINK_SECTION	   WASM_SECTION (0, "dylink")

/* Subsection indices.  Right now, that's subsections of the "name"
   section only.  */
#define WASM_FUNCTION_SUBSECTION 1 /* Function names.  */
#define WASM_LOCALS_SUBSECTION   2 /* Names of locals by function.  */

/* The section to report wasm symbols in.  */
#define WASM_SECTION_FUNCTION_INDEX ".space.function_index"

#endif /* _WASM_MODULE_H */
