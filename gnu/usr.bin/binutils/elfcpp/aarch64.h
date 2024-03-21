// aarch64.h -- ELF definitions specific to AARCH64  -*- C++ -*-

// Copyright (C) 2014-2023 Free Software Foundation, Inc.
// Written by Jing Yu (jingyu@google.com)

// This file is part of elfcpp.

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// as published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// In addition to the permissions in the GNU Library General Public
// License, the Free Software Foundation gives you unlimited
// permission to link the compiled version of this file into
// combinations with other programs, and to distribute those
// combinations without any restriction coming from the use of this
// file.  (The Library Public License restrictions do apply in other
// respects; for example, they cover modification of the file, and
/// distribution when not linked into a combined executable.)

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
// 02110-1301, USA.

#ifndef ELFCPP_AARCH64_H
#define ELFCPP_AARCH64_H

namespace elfcpp
{

// The relocation type information is taken from:
//
//   ELF for the ARM 64-bit Architecture (AArch64)
//   Document number: ARM IHI 0056B, current through AArch64 ABI release 1.0
//   Date of Issue: 22nd May, 2013
//

enum
{
  // Null relocation codes
  R_AARCH64_NONE = 0,		// None
  R_AARCH64_withdrawn = 256,	// Treat as R_AARCH64_NONE

  // Static relocations
  R_AARCH64_ABS64 = 257,	// S + A
  R_AARCH64_ABS32 = 258,	// S + A
  R_AARCH64_ABS16 = 259,	// S + A
  R_AARCH64_PREL64 = 260,	// S + A - P
  R_AARCH64_PREL32 = 261,	// S + A - P
  R_AARCH64_PREL16 = 262,	// S + A - P
  R_AARCH64_MOVW_UABS_G0 = 263,		// S + A
  R_AARCH64_MOVW_UABS_G0_NC = 264,	// S + A
  R_AARCH64_MOVW_UABS_G1 = 265,		// S + A
  R_AARCH64_MOVW_UABS_G1_NC = 266,	// S + A
  R_AARCH64_MOVW_UABS_G2 = 267,		// S + A
  R_AARCH64_MOVW_UABS_G2_NC = 268,	// S + A
  R_AARCH64_MOVW_UABS_G3 = 269,		// S + A
  R_AARCH64_MOVW_SABS_G0 = 270,		// S + A
  R_AARCH64_MOVW_SABS_G1 = 271,		// S + A
  R_AARCH64_MOVW_SABS_G2 = 272,		// S + A
  R_AARCH64_LD_PREL_LO19 = 273,		// S + A - P
  R_AARCH64_ADR_PREL_LO21 = 274,	// S + A - P
  R_AARCH64_ADR_PREL_PG_HI21 = 275,	// Page(S+A) - Page(P)
  R_AARCH64_ADR_PREL_PG_HI21_NC = 276,	// Page(S+A) - Page(P)
  R_AARCH64_ADD_ABS_LO12_NC = 277,	// S + A
  R_AARCH64_LDST8_ABS_LO12_NC = 278,	// S + A
  R_AARCH64_TSTBR14 = 279,		// S + A - P
  R_AARCH64_CONDBR19 = 280,		// S + A - P
  R_AARCH64_JUMP26 = 282,		// S + A - P
  R_AARCH64_CALL26 = 283,		// S + A - P
  R_AARCH64_LDST16_ABS_LO12_NC = 284,	// S + A
  R_AARCH64_LDST32_ABS_LO12_NC = 285,	// S + A
  R_AARCH64_LDST64_ABS_LO12_NC = 286,	// S + A
  R_AARCH64_MOVW_PREL_G0 = 287,		// S + A - P
  R_AARCH64_MOVW_PREL_G0_NC = 288,	// S + A - P
  R_AARCH64_MOVW_PREL_G1 = 289,		// S + A - P
  R_AARCH64_MOVW_PREL_G1_NC = 290,	// S + A - P
  R_AARCH64_MOVW_PREL_G2 = 291,		// S + A - P
  R_AARCH64_MOVW_PREL_G2_NC = 292,	// S + A - P
  R_AARCH64_MOVW_PREL_G3 = 293,		// S + A - P
  R_AARCH64_LDST128_ABS_LO12_NC = 299,	// S + A
  R_AARCH64_MOVW_GOTOFF_G0 = 300,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G0_NC = 301,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G1 = 302,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G1_NC = 303,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G2 = 304,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G2_NC = 305,	// G(GDAT(S+A))-GOT
  R_AARCH64_MOVW_GOTOFF_G3 = 306,	// G(GDAT(S+A))-GOT
  R_AARCH64_GOTREL64 = 307,		// S + A - GOT
  R_AARCH64_GOTREL32 = 308,		// S + A - GOT
  R_AARCH64_GOT_LD_PREL19 = 309,	// G(GDAT(S+A))-P
  R_AARCH64_LD64_GOTOFF_LO15 = 310,	// G(GDAT(S+A))-GOT
  R_AARCH64_ADR_GOT_PAGE = 311,		// Page(G(GDAT(S+A)))-Page(P)
  R_AARCH64_LD64_GOT_LO12_NC = 312,	// G(GDAT(S+A))
  R_AARCH64_LD64_GOTPAGE_LO15 = 313,	// G(GDAT(S+A))-Page(GOT)

  // Relocations for thread-local storage
  R_AARCH64_TLSGD_ADR_PREL21 = 512,		// G(GTLSIDX(S,A)) - P
  R_AARCH64_TLSGD_ADR_PAGE21 = 513,		// Page(G(GTLSIDX(S,A)))-Page(P)
  R_AARCH64_TLSGD_ADD_LO12_NC = 514,		// G(GTLSICX(S,A))
  R_AARCH64_TLSGD_MOVW_G1 = 515,		// G(GTLSIDX(S,A)) - GOT
  R_AARCH64_TLSGD_MOVW_G0_NC = 516,		// G(GTLSIDX(S,A)) - GOT

  R_AARCH64_TLSLD_ADR_PREL21 = 517,		// G(GLDM(S)) - P
  R_AARCH64_TLSLD_ADR_PAGE21 = 518,		// Page(G(GLDM(S))) - Page(P)
  R_AARCH64_TLSLD_ADD_LO12_NC = 519,		// G(GLDM(S))
  R_AARCH64_TLSLD_MOVW_G1 = 520,		// G(GLDM(S)) - GOT
  R_AARCH64_TLSLD_MOVW_G0_NC = 521,		// G(GLDM(S)) - GOT
  R_AARCH64_TLSLD_LD_PREL19 = 522,		// G(GLDM(S)) - P
  R_AARCH64_TLSLD_MOVW_DTPREL_G2 = 523,		// DTPREL(S+A)
  R_AARCH64_TLSLD_MOVW_DTPREL_G1 = 524,		// DTPREL(S+A)
  R_AARCH64_TLSLD_MOVW_DTPREL_G1_NC = 525,	// DTPREL(S+A)
  R_AARCH64_TLSLD_MOVW_DTPREL_G0 = 526,		// DTPREL(S+A)
  R_AARCH64_TLSLD_MOVW_DTPREL_G0_NC = 527,	// DTPREL(S+A)
  R_AARCH64_TLSLD_ADD_DTPREL_HI12 = 528,	// DTPREL(S+A)
  R_AARCH64_TLSLD_ADD_DTPREL_LO12 = 529,	// DTPREL(S+A)
  R_AARCH64_TLSLD_ADD_DTPREL_LO12_NC = 530,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST8_DTPREL_LO12 = 531,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST8_DTPREL_LO12_NC = 532,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST16_DTPREL_LO12 = 533,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST16_DTPREL_LO12_NC = 534,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST32_DTPREL_LO12 = 535,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST32_DTPREL_LO12_NC = 536,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST64_DTPREL_LO12 = 537,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST64_DTPREL_LO12_NC = 538,	// DTPREL(S+A)
  R_AARCH64_TLSIE_MOVW_GOTTPREL_G1 = 539,	// G(GTPREL(S+A)) - GOT
  R_AARCH64_TLSIE_MOVW_GOTTPREL_G0_NC = 540,	// G(GTPREL(S+A)) - GOT
  R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21 = 541,	// Page(G(GTPREL(S+A)))-Page(P)
  R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC = 542,	// G(GTPREL(S+A))
  R_AARCH64_TLSIE_LD_GOTTPREL_PREL19 = 543,	// G(GTPREL(S+A)) - P
  R_AARCH64_TLSLE_MOVW_TPREL_G2 = 544,		// TPREL(S+A)
  R_AARCH64_TLSLE_MOVW_TPREL_G1 = 545,		// TPREL(S+A)
  R_AARCH64_TLSLE_MOVW_TPREL_G1_NC = 546,	// TPREL(S+A)
  R_AARCH64_TLSLE_MOVW_TPREL_G0 = 547,		// TPREL(S+A)
  R_AARCH64_TLSLE_MOVW_TPREL_G0_NC = 548,	// TPREL(S+A)
  R_AARCH64_TLSLE_ADD_TPREL_HI12 = 549,		// TPREL(S+A)
  R_AARCH64_TLSLE_ADD_TPREL_LO12 = 550,		// TPREL(S+A)
  R_AARCH64_TLSLE_ADD_TPREL_LO12_NC = 551,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST8_TPREL_LO12 = 552,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST8_TPREL_LO12_NC = 553,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST16_TPREL_LO12 = 554,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST16_TPREL_LO12_NC = 555,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST32_TPREL_LO12 = 556,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST32_TPREL_LO12_NC = 557,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST64_TPREL_LO12 = 558,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST64_TPREL_LO12_NC = 559,	// TPREL(S+A)
  R_AARCH64_TLSDESC_LD_PREL19 = 560,	// G(GTLSDESC(S+A)) - P
  R_AARCH64_TLSDESC_ADR_PREL21 = 561,	// G(GTLSDESC(S+A)) - P
  R_AARCH64_TLSDESC_ADR_PAGE21 = 562,	// Page(G(GTLSDESC(S+A)))-Page(P)
  R_AARCH64_TLSDESC_LD64_LO12 = 563,	// G(GTLSDESC(S+A))
  R_AARCH64_TLSDESC_ADD_LO12 = 564,	// G(GTLSDESC(S+A))
  R_AARCH64_TLSDESC_OFF_G1 = 565,	// G(GTLSDESC(S+A)) - GOT
  R_AARCH64_TLSDESC_OFF_G0_NC = 566,	// G(GTLSDESC(S+A)) - GOT
  R_AARCH64_TLSDESC_LDR = 567,		// None
  R_AARCH64_TLSDESC_ADD = 568,		// None
  R_AARCH64_TLSDESC_CALL = 569,		// None
  R_AARCH64_TLSLE_LDST128_TPREL_LO12 = 570,	// TPREL(S+A)
  R_AARCH64_TLSLE_LDST128_TPREL_LO12_NC = 571,	// TPREL(S+A)
  R_AARCH64_TLSLD_LDST128_DTPREL_LO12 = 572,	// DTPREL(S+A)
  R_AARCH64_TLSLD_LDST128_DTPREL_LO12_NC = 573,	// DTPREL(S+A)

  // Dynamic relocations
  R_AARCH64_COPY = 1024,
  R_AARCH64_GLOB_DAT = 1025,		// S + A
  R_AARCH64_JUMP_SLOT = 1026,		// S + A
  R_AARCH64_RELATIVE = 1027,		// Delta(S) + A
  // Note (shenhan): the following 2 relocs are different from elf spec from
  // arm.  In elf docs, TLS_DTPMOD64 is defined as 1029, TLS_DTPREL64 1028.
  // While actually the bfd linker (and the dynamic linker) treates TLS_DTPMOD64
  // as 1028, TLS_DTPREL64 1029.  See binutils-gdb/include/elf/aarch64.h.
  R_AARCH64_TLS_DTPMOD64 = 1028,	// LDM(S)
  R_AARCH64_TLS_DTPREL64 = 1029,	// DTPREL(S+A)
  R_AARCH64_TLS_TPREL64 = 1030,		// TPREL(S+A)
  R_AARCH64_TLSDESC = 1031,		// TLSDESC(S+A)
  R_AARCH64_IRELATIVE = 1032,		// Indirect(Delta(S) + A)
};

} // End namespace elfcpp.

#endif // !defined(ELFCPP_AARCH64_H)
