/* tls.s Test file for AArch64 TLS relocations.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GAS.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

func:

	// R_AARCH64_TLSDESC_ADR_PAGE21  var
	adrp  x0, :tlsdesc:var
	// R_AARCH64_TLSDESC_LD64_LO12 var
	ldr   x1, [x0, #:tlsdesc_lo12:var]
	// R_AARCH64_TLSDESC_ADD_LO12  var
	add   x0, x0, #:tlsdesc_lo12:var
	// R_AARCH64_TLSDESC_CALL      var
	.tlsdesccall var
	blr   x1

	// R_AARCH64_TLSGD_ADR_PAGE21         var
	adrp x0, :tlsgd:var
	// R_AARCH64_TLSGD_ADD_LO12_NC        var
	add  x0, x0, #:tlsgd_lo12:var
	// R_AARCH64_CALL26
	bl   __tls_get_addr

	// R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21 var
	adrp x0, :gottprel:var
	// R_AARCH64_TLSUE_GOTTPREL_LO12_NC    var
	ldr  x0, [x0, #:gottprel_lo12:var]

	// R_AARCH64_TLSLE_ADD_TPREL_LO12       var
	add  x0, x1, #:tprel_lo12:var
	// R_AARCH64_TLSLE_ADD_TPREL_HI12       var
	add  x0, x1, #:tprel_hi12:var
	// R_AARCH64_TLSLE_ADD_TPREL_HI12       var
	add  x0, x1, #:tprel_hi12:var, lsl #12
	// R_AARCH64_TLSLE_ADD_TPREL_LO12_NC    var
	add  x0, x1, #:tprel_lo12_nc:var

	movz x0, #:tprel_g1:var
	movk x0, #:tprel_g0_nc:var
