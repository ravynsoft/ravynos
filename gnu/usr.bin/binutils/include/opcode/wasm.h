/* WebAssembly assembler/disassembler support.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.

   This file is part of GAS, the GNU assembler.

   GAS is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING3.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */

/* WebAssembly opcodes.  Each opcode invokes the WASM_OPCODE macro
   with the following arguments:

   1. Code byte.
   2. Mnemonic.
   3. Input type.
   4. Output type.
   5. Opcode class.
   6. Signedness information.
*/

WASM_OPCODE (0x00, "unreachable", void, void, special, agnostic)
WASM_OPCODE (0x01, "nop", void, void, special, agnostic)
WASM_OPCODE (0x02, "block", void, void, typed, agnostic)
WASM_OPCODE (0x03, "loop", void, void, typed, agnostic)
WASM_OPCODE (0x04, "if", void, void, typed, agnostic)
WASM_OPCODE (0x05, "else", void, void, special, agnostic)
WASM_OPCODE (0x0b, "end", void, void, special, agnostic)
WASM_OPCODE (0x0c, "br", void, void, break, agnostic)
WASM_OPCODE (0x0d, "br_if", void, void, break_if, agnostic)
WASM_OPCODE (0x0e, "br_table", void, void, break_table, agnostic)
WASM_OPCODE (0x0f, "return", void, void, return, agnostic)

WASM_OPCODE (0x10, "call", any, any, call, agnostic)
WASM_OPCODE (0x11, "call_indirect", any, any, call_indirect, agnostic)

WASM_OPCODE (0x1a, "drop", any, any, drop, agnostic)
WASM_OPCODE (0x1b, "select", any, any, select, agnostic)

WASM_OPCODE (0x20, "get_local", any, any, get_local, agnostic)
WASM_OPCODE (0x21, "set_local", any, any, set_local, agnostic)
WASM_OPCODE (0x22, "tee_local", any, any, tee_local, agnostic)
WASM_OPCODE (0x23, "get_global", any, any, get_local, agnostic)
WASM_OPCODE (0x24, "set_global", any, any, set_local, agnostic)

WASM_OPCODE (0x28, "i32.load", i32, i32, load, agnostic)
WASM_OPCODE (0x29, "i64.load", i32, i64, load, agnostic)
WASM_OPCODE (0x2a, "f32.load", i32, f32, load, agnostic)
WASM_OPCODE (0x2b, "f64.load", i32, f64, load, agnostic)
WASM_OPCODE (0x2c, "i32.load8_s", i32, i32, load, signed)
WASM_OPCODE (0x2d, "i32.load8_u", i32, i32, load, unsigned)
WASM_OPCODE (0x2e, "i32.load16_s", i32, i32, load, signed)
WASM_OPCODE (0x2f, "i32.load16_u", i32, i32, load, unsigned)
WASM_OPCODE (0x30, "i64.load8_s", i32, i64, load, signed)
WASM_OPCODE (0x31, "i64.load8_u", i32, i64, load, unsigned)
WASM_OPCODE (0x32, "i64.load16_s", i32, i64, load, signed)
WASM_OPCODE (0x33, "i64.load16_u", i32, i64, load, unsigned)
WASM_OPCODE (0x34, "i64.load32_s", i32, i64, load, signed)
WASM_OPCODE (0x35, "i64.load32_u", i32, i64, load, unsigned)
WASM_OPCODE (0x36, "i32.store", i32, void, store, agnostic)
WASM_OPCODE (0x37, "i64.store", i64, void, store, agnostic)
WASM_OPCODE (0x38, "f32.store", f32, void, store, agnostic)
WASM_OPCODE (0x39, "f64.store", f64, void, store, agnostic)
WASM_OPCODE (0x3a, "i32.store8", i32, void, store, agnostic)
WASM_OPCODE (0x3b, "i32.store16", i32, void, store, agnostic)
WASM_OPCODE (0x3c, "i64.store8", i64, void, store, agnostic)
WASM_OPCODE (0x3d, "i64.store16", i64, void, store, agnostic)
WASM_OPCODE (0x3e, "i64.store32", i64, void, store, agnostic)

WASM_OPCODE (0x3f, "current_memory", void, i32, current_memory, agnostic)
WASM_OPCODE (0x40, "grow_memory", void, i32, grow_memory, agnostic)

WASM_OPCODE (0x41, "i32.const", i32, i32, constant_i32, agnostic)
WASM_OPCODE (0x42, "i64.const", i64, i64, constant_i64, agnostic)
WASM_OPCODE (0x43, "f32.const", f32, f32, constant_f32, agnostic)
WASM_OPCODE (0x44, "f64.const", f64, f64, constant_f64, agnostic)

WASM_OPCODE (0x45, "i32.eqz", i32, i32, eqz, agnostic)
WASM_OPCODE (0x46, "i32.eq", i32, i32, relational, agnostic)
WASM_OPCODE (0x47, "i32.ne", i32, i32, relational, agnostic)
WASM_OPCODE (0x48, "i32.lt_s", i32, i32, relational, signed)
WASM_OPCODE (0x49, "i32.lt_u", i32, i32, relational, unsigned)
WASM_OPCODE (0x4a, "i32.gt_s", i32, i32, relational, signed)
WASM_OPCODE (0x4b, "i32.gt_u", i32, i32, relational, unsigned)
WASM_OPCODE (0x4c, "i32.le_s", i32, i32, relational, signed)
WASM_OPCODE (0x4d, "i32.le_u", i32, i32, relational, unsigned)
WASM_OPCODE (0x4e, "i32.ge_s", i32, i32, relational, signed)
WASM_OPCODE (0x4f, "i32.ge_u", i32, i32, relational, unsigned)

WASM_OPCODE (0x50, "i64.eqz", i64, i32, eqz, agnostic)
WASM_OPCODE (0x51, "i64.eq", i64, i32, relational, agnostic)
WASM_OPCODE (0x52, "i64.ne", i64, i32, relational, agnostic)
WASM_OPCODE (0x53, "i64.lt_s", i64, i32, relational, signed)
WASM_OPCODE (0x54, "i64.lt_u", i64, i32, relational, unsigned)
WASM_OPCODE (0x55, "i64.gt_s", i64, i32, relational, signed)
WASM_OPCODE (0x56, "i64.gt_u", i64, i32, relational, unsigned)
WASM_OPCODE (0x57, "i64.le_s", i64, i32, relational, signed)
WASM_OPCODE (0x58, "i64.le_u", i64, i32, relational, unsigned)
WASM_OPCODE (0x59, "i64.ge_s", i64, i32, relational, signed)
WASM_OPCODE (0x5a, "i64.ge_u", i64, i32, relational, unsigned)

WASM_OPCODE (0x5b, "f32.eq", f32, i32, relational, floating)
WASM_OPCODE (0x5c, "f32.ne", f32, i32, relational, floating)
WASM_OPCODE (0x5d, "f32.lt", f32, i32, relational, floating)
WASM_OPCODE (0x5e, "f32.gt", f32, i32, relational, floating)
WASM_OPCODE (0x5f, "f32.le", f32, i32, relational, floating)
WASM_OPCODE (0x60, "f32.ge", f32, i32, relational, floating)

WASM_OPCODE (0x61, "f64.eq", f64, i32, relational, floating)
WASM_OPCODE (0x62, "f64.ne", f64, i32, relational, floating)
WASM_OPCODE (0x63, "f64.lt", f64, i32, relational, floating)
WASM_OPCODE (0x64, "f64.gt", f64, i32, relational, floating)
WASM_OPCODE (0x65, "f64.le", f64, i32, relational, floating)
WASM_OPCODE (0x66, "f64.ge", f64, i32, relational, floating)

WASM_OPCODE (0x67, "i32.clz", i32, i32, unary, agnostic)
WASM_OPCODE (0x68, "i32.ctz", i32, i32, unary, agnostic)
WASM_OPCODE (0x69, "i32.popcnt", i32, i32, unary, agnostic)

WASM_OPCODE (0x6a, "i32.add", i32, i32, binary, agnostic)
WASM_OPCODE (0x6b, "i32.sub", i32, i32, binary, agnostic)
WASM_OPCODE (0x6c, "i32.mul", i32, i32, binary, agnostic)
WASM_OPCODE (0x6d, "i32.div_s", i32, i32, binary, signed)
WASM_OPCODE (0x6e, "i32.div_u", i32, i32, binary, unsigned)
WASM_OPCODE (0x6f, "i32.rem_s", i32, i32, binary, signed)
WASM_OPCODE (0x70, "i32.rem_u", i32, i32, binary, unsigned)
WASM_OPCODE (0x71, "i32.and", i32, i32, binary, agnostic)
WASM_OPCODE (0x72, "i32.or", i32, i32, binary, agnostic)
WASM_OPCODE (0x73, "i32.xor", i32, i32, binary, agnostic)
WASM_OPCODE (0x74, "i32.shl", i32, i32, binary, agnostic)
WASM_OPCODE (0x75, "i32.shr_s", i32, i32, binary, signed)
WASM_OPCODE (0x76, "i32.shr_u", i32, i32, binary, unsigned)
WASM_OPCODE (0x77, "i32.rotl", i32, i32, binary, agnostic)
WASM_OPCODE (0x78, "i32.rotr", i32, i32, binary, agnostic)

WASM_OPCODE (0x79, "i64.clz", i64, i64, unary, agnostic)
WASM_OPCODE (0x7a, "i64.ctz", i64, i64, unary, agnostic)
WASM_OPCODE (0x7b, "i64.popcnt", i64, i64, unary, agnostic)

WASM_OPCODE (0x7c, "i64.add", i64, i64, binary, agnostic)
WASM_OPCODE (0x7d, "i64.sub", i64, i64, binary, agnostic)
WASM_OPCODE (0x7e, "i64.mul", i64, i64, binary, agnostic)
WASM_OPCODE (0x7f, "i64.div_s", i64, i64, binary, signed)
WASM_OPCODE (0x80, "i64.div_u", i64, i64, binary, unsigned)
WASM_OPCODE (0x81, "i64.rem_s", i64, i64, binary, signed)
WASM_OPCODE (0x82, "i64.rem_u", i64, i64, binary, unsigned)
WASM_OPCODE (0x83, "i64.and", i64, i64, binary, agnostic)
WASM_OPCODE (0x84, "i64.or", i64, i64, binary, agnostic)
WASM_OPCODE (0x85, "i64.xor", i64, i64, binary, agnostic)
WASM_OPCODE (0x86, "i64.shl", i64, i64, binary, agnostic)
WASM_OPCODE (0x87, "i64.shr_s", i64, i64, binary, signed)
WASM_OPCODE (0x88, "i64.shr_u", i64, i64, binary, unsigned)
WASM_OPCODE (0x89, "i64.rotl", i64, i64, binary, agnostic)
WASM_OPCODE (0x8a, "i64.rotr", i64, i64, binary, agnostic)

WASM_OPCODE (0x8b, "f32.abs", f32, f32, unary, floating)
WASM_OPCODE (0x8c, "f32.neg", f32, f32, unary, floating)
WASM_OPCODE (0x8d, "f32.ceil", f32, f32, unary, floating)
WASM_OPCODE (0x8e, "f32.floor", f32, f32, unary, floating)
WASM_OPCODE (0x8f, "f32.trunc", f32, f32, unary, floating)
WASM_OPCODE (0x90, "f32.nearest", f32, f32, unary, floating)
WASM_OPCODE (0x91, "f32.sqrt", f32, f32, unary, floating)
WASM_OPCODE (0x92, "f32.add", f32, f32, binary, floating)
WASM_OPCODE (0x93, "f32.sub", f32, f32, binary, floating)
WASM_OPCODE (0x94, "f32.mul", f32, f32, binary, floating)
WASM_OPCODE (0x95, "f32.div", f32, f32, binary, floating)
WASM_OPCODE (0x96, "f32.min", f32, f32, binary, floating)
WASM_OPCODE (0x97, "f32.max", f32, f32, binary, floating)
WASM_OPCODE (0x98, "f32.copysign", f32, f32, binary, floating)

WASM_OPCODE (0x99, "f64.abs", f64, f64, unary, floating)
WASM_OPCODE (0x9a, "f64.neg", f64, f64, unary, floating)
WASM_OPCODE (0x9b, "f64.ceil", f64, f64, unary, floating)
WASM_OPCODE (0x9c, "f64.floor", f64, f64, unary, floating)
WASM_OPCODE (0x9d, "f64.trunc", f64, f64, unary, floating)
WASM_OPCODE (0x9e, "f64.nearest", f64, f64, unary, floating)
WASM_OPCODE (0x9f, "f64.sqrt", f64, f64, unary, floating)
WASM_OPCODE (0xa0, "f64.add", f64, f64, binary, floating)
WASM_OPCODE (0xa1, "f64.sub", f64, f64, binary, floating)
WASM_OPCODE (0xa2, "f64.mul", f64, f64, binary, floating)
WASM_OPCODE (0xa3, "f64.div", f64, f64, binary, floating)
WASM_OPCODE (0xa4, "f64.min", f64, f64, binary, floating)
WASM_OPCODE (0xa5, "f64.max", f64, f64, binary, floating)
WASM_OPCODE (0xa6, "f64.copysign", f64, f64, binary, floating)

WASM_OPCODE (0xa7, "i32.wrap/i64", i64, i32, conv, agnostic)
WASM_OPCODE (0xa8, "i32.trunc_s/f32", f32, i32, conv, signed)
WASM_OPCODE (0xa9, "i32.trunc_u/f32", f32, i32, conv, unsigned)
WASM_OPCODE (0xaa, "i32.trunc_s/f64", f64, i32, conv, signed)
WASM_OPCODE (0xab, "i32.trunc_u/f64", f64, i32, conv, unsigned)
WASM_OPCODE (0xac, "i64.extend_s/i32", i32, i64, conv, signed)
WASM_OPCODE (0xad, "i64.extend_u/i32", i32, i64, conv, unsigned)
WASM_OPCODE (0xae, "i64.trunc_s/f32", f32, i64, conv, signed)
WASM_OPCODE (0xaf, "i64.trunc_u/f32", f32, i64, conv, unsigned)
WASM_OPCODE (0xb0, "i64.trunc_s/f64", f64, i64, conv, signed)
WASM_OPCODE (0xb1, "i64.trunc_u/f64", f64, i64, conv, unsigned)

WASM_OPCODE (0xb2, "f32.convert_s/i32", i32, f32, conv, signed)
WASM_OPCODE (0xb3, "f32.convert_u/i32", i32, f32, conv, unsigned)
WASM_OPCODE (0xb4, "f32.convert_s/i64", i64, f32, conv, signed)
WASM_OPCODE (0xb5, "f32.convert_u/i64", i64, f32, conv, unsigned)
WASM_OPCODE (0xb6, "f32.demote/f64", f64, f32, conv, floating)
WASM_OPCODE (0xb7, "f64.convert_s/i32", i32, f64, conv, signed)
WASM_OPCODE (0xb8, "f64.convert_u/i32", i32, f64, conv, unsigned)
WASM_OPCODE (0xb9, "f64.convert_s/i64", i64, f64, conv, signed)
WASM_OPCODE (0xba, "f64.convert_u/i64", i64, f64, conv, unsigned)
WASM_OPCODE (0xbb, "f64.promote/f32", f32, f64, conv, floating)

WASM_OPCODE (0xbc, "i32.reinterpret/f32", f32, i32, conv, agnostic)
WASM_OPCODE (0xbd, "i64.reinterpret/f64", f64, i64, conv, agnostic)
WASM_OPCODE (0xbe, "f32.reinterpret/i32", i32, f32, conv, agnostic)
WASM_OPCODE (0xbf, "f64.reinterpret/i64", i64, f64, conv, agnostic)

/* This isn't, strictly speaking, an opcode, but is treated as such by
   the assembler.  */
WASM_OPCODE (0x60, "signature", void, void, signature, agnostic)
