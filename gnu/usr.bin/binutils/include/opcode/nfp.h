/* nfp.h.  NFP opcode list.
   Copyright (C) 2017-2023 Free Software Foundation, Inc.
   Contributed by Francois H. Theron <francois.theron@netronome.com>

   This file is part of the GNU opcodes library.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING3.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _NFP_H_
#define _NFP_H_

#include "bfd.h"
#include <stdint.h>
#include "elf/nfp.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* We use ME versions for most of this rather than NFP family and revision
   numbers.  The version numbers are currently 2.7 and 2.8 and to avoid long
   names with many underscores we'll just use 27 and 28 until some feature
   number makes it necessary to do something different.  */

#define NFP_ME27_INSTR_MASK_CMD	        ((uint64_t) 0x008000000000)
#define NFP_ME27_INSTR_CMD		((uint64_t) 0x000000000000)
#define NFP_ME27_INSTR_IS_CMD(instr) \
	((instr & NFP_ME27_INSTR_MASK_CMD) == NFP_ME27_INSTR_CMD)

#define NFP_ME27_INSTR_MASK_ALU_SHF	((uint64_t) 0x1ee000000000)
#define NFP_ME27_INSTR_ALU_SHF		((uint64_t) 0x008000000000)
#define NFP_ME27_INSTR_IS_ALU_SHF(instr) \
	((instr & NFP_ME27_INSTR_MASK_ALU_SHF) == NFP_ME27_INSTR_ALU_SHF)

#define NFP_ME27_INSTR_MASK_ALU	        ((uint64_t) 0x1ee000000000)
#define NFP_ME27_INSTR_ALU		((uint64_t) 0x00a000000000)
#define NFP_ME27_INSTR_IS_ALU(instr) \
		((instr & NFP_ME27_INSTR_MASK_ALU) == NFP_ME27_INSTR_ALU)

#define NFP_ME27_INSTR_MASK_IMMED	((uint64_t) 0x1ff900000000)
#define NFP_ME27_INSTR_IMMED		((uint64_t) 0x00f000000000)
#define NFP_ME27_INSTR_IS_IMMED(instr) \
	((instr & NFP_ME27_INSTR_MASK_IMMED) == NFP_ME27_INSTR_IMMED)

#define NFP_ME27_INSTR_MASK_LD_FIELD	((uint64_t) 0x1ffa00e00000)
#define NFP_ME27_INSTR_LD_FIELD	        ((uint64_t) 0x00c000000000)
#define NFP_ME27_INSTR_IS_LD_FIELD(instr) \
	((instr & NFP_ME27_INSTR_MASK_LD_FIELD) == NFP_ME27_INSTR_LD_FIELD)

#define NFP_ME27_INSTR_MASK_CTX_ARB	((uint64_t) 0x00f800000000)
#define NFP_ME27_INSTR_CTX_ARB		((uint64_t) 0x00e000000000)
#define NFP_ME27_INSTR_IS_CTX_ARB(instr) \
	((instr & NFP_ME27_INSTR_MASK_CTX_ARB) == NFP_ME27_INSTR_CTX_ARB)

#define NFP_ME27_INSTR_MASK_LOCAL_CSR	((uint64_t) 0x1ffe00100000)
#define NFP_ME27_INSTR_LOCAL_CSR	((uint64_t) 0x00fc00000000)
#define NFP_ME27_INSTR_IS_LOCAL_CSR(instr) \
	((instr & NFP_ME27_INSTR_MASK_LOCAL_CSR) == NFP_ME27_INSTR_LOCAL_CSR)

#define NFP_ME27_INSTR_MASK_BRANCH	((uint64_t) 0x00f8000c3ce0)
#define NFP_ME27_INSTR_BRANCH		((uint64_t) 0x00d800000020)
#define NFP_ME27_INSTR_IS_BRANCH(instr) \
	((instr & NFP_ME27_INSTR_MASK_BRANCH) == NFP_ME27_INSTR_BRANCH)

#define NFP_ME27_INSTR_MASK_BR_BYTE	((uint64_t) 0x00f800000000)
#define NFP_ME27_INSTR_BR_BYTE		((uint64_t) 0x00c800000000)
#define NFP_ME27_INSTR_IS_BR_BYTE(instr) \
	((instr & NFP_ME27_INSTR_MASK_BR_BYTE) == NFP_ME27_INSTR_BR_BYTE)

#define NFP_ME27_INSTR_MASK_BR_BIT	((uint64_t) 0x00f800080300)
#define NFP_ME27_INSTR_BR_BIT		((uint64_t) 0x00d000000000)
#define NFP_ME27_INSTR_IS_BR_BIT(instr) \
	((instr & NFP_ME27_INSTR_MASK_BR_BIT) == NFP_ME27_INSTR_BR_BIT)

#define NFP_ME27_INSTR_MASK_BR_ALU	((uint64_t) 0x1fff80000000)
#define NFP_ME27_INSTR_BR_ALU		((uint64_t) 0x00e800000000)
#define NFP_ME27_INSTR_IS_BR_ALU(instr) \
	((instr & NFP_ME27_INSTR_MASK_BR_ALU) == NFP_ME27_INSTR_BR_ALU)

#define NFP_ME27_INSTR_MASK_MULT	((uint64_t) 0x1efe3f000000)
#define NFP_ME27_INSTR_MULT		((uint64_t) 0x00f800000000)
#define NFP_ME27_INSTR_IS_MULT(instr) \
	((instr & NFP_ME27_INSTR_MASK_MULT) == NFP_ME27_INSTR_MULT)


#define NFP_ME28_INSTR_MASK_CMD	        ((uint64_t) 0x008000000000)
#define NFP_ME28_INSTR_CMD		((uint64_t) 0x000000000000)
#define NFP_ME28_INSTR_IS_CMD(instr) \
	((instr & NFP_ME28_INSTR_MASK_CMD) == NFP_ME28_INSTR_CMD)

#define NFP_ME28_INSTR_MASK_ALU_SHF	((uint64_t) 0x00e000000000)
#define NFP_ME28_INSTR_ALU_SHF		((uint64_t) 0x008000000000)
#define NFP_ME28_INSTR_IS_ALU_SHF(instr) \
	((instr & NFP_ME28_INSTR_MASK_ALU_SHF) == NFP_ME28_INSTR_ALU_SHF)

#define NFP_ME28_INSTR_MASK_ALU	        ((uint64_t) 0x00e000000000)
#define NFP_ME28_INSTR_ALU		((uint64_t) 0x00a000000000)
#define NFP_ME28_INSTR_IS_ALU(instr) \
	((instr & NFP_ME28_INSTR_MASK_ALU) == NFP_ME28_INSTR_ALU)

#define NFP_ME28_INSTR_MASK_IMMED	((uint64_t) 0x01f900000000)
#define NFP_ME28_INSTR_IMMED		((uint64_t) 0x00f000000000)
#define NFP_ME28_INSTR_IS_IMMED(instr) \
	((instr & NFP_ME28_INSTR_MASK_IMMED) == NFP_ME28_INSTR_IMMED)

#define NFP_ME28_INSTR_MASK_LD_FIELD	((uint64_t) 0x01fa00e00000)
#define NFP_ME28_INSTR_LD_FIELD	        ((uint64_t) 0x00c000000000)
#define NFP_ME28_INSTR_IS_LD_FIELD(instr) \
	((instr & NFP_ME28_INSTR_MASK_LD_FIELD) == NFP_ME28_INSTR_LD_FIELD)

#define NFP_ME28_INSTR_MASK_CTX_ARB	((uint64_t) 0x00f800000000)
#define NFP_ME28_INSTR_CTX_ARB		((uint64_t) 0x00e000000000)
#define NFP_ME28_INSTR_IS_CTX_ARB(instr) \
	((instr & NFP_ME28_INSTR_MASK_CTX_ARB) == NFP_ME28_INSTR_CTX_ARB)

#define NFP_ME28_INSTR_MASK_LOCAL_CSR	((uint64_t) 0x01fe00100000)
#define NFP_ME28_INSTR_LOCAL_CSR	((uint64_t) 0x00fc00000000)
#define NFP_ME28_INSTR_IS_LOCAL_CSR(instr) \
	((instr & NFP_ME28_INSTR_MASK_LOCAL_CSR) == NFP_ME28_INSTR_LOCAL_CSR)

#define NFP_ME28_INSTR_MASK_BRANCH	((uint64_t) 0x00f8000c3ce0)
#define NFP_ME28_INSTR_BRANCH		((uint64_t) 0x00d800000020)
#define NFP_ME28_INSTR_IS_BRANCH(instr) \
	((instr & NFP_ME28_INSTR_MASK_BRANCH) == NFP_ME28_INSTR_BRANCH)

#define NFP_ME28_INSTR_MASK_BR_BYTE	((uint64_t) 0x00f800000000)
#define NFP_ME28_INSTR_BR_BYTE		((uint64_t) 0x00c800000000)
#define NFP_ME28_INSTR_IS_BR_BYTE(instr) \
	((instr & NFP_ME28_INSTR_MASK_BR_BYTE) == NFP_ME28_INSTR_BR_BYTE)

#define NFP_ME28_INSTR_MASK_BR_BIT	((uint64_t) 0x00f800080300)
#define NFP_ME28_INSTR_BR_BIT		((uint64_t) 0x00d000000000)
#define NFP_ME28_INSTR_IS_BR_BIT(instr) \
	((instr & NFP_ME28_INSTR_MASK_BR_BIT) == NFP_ME28_INSTR_BR_BIT)

#define NFP_ME28_INSTR_MASK_BR_ALU	((uint64_t) 0x00ff80000000)
#define NFP_ME28_INSTR_BR_ALU		((uint64_t) 0x00e800000000)
#define NFP_ME28_INSTR_IS_BR_ALU(instr) \
	((instr & NFP_ME28_INSTR_MASK_BR_ALU) == NFP_ME28_INSTR_BR_ALU)

#define NFP_ME28_INSTR_MASK_MULT	((uint64_t) 0x00fe3f000000)
#define NFP_ME28_INSTR_MULT		((uint64_t) 0x00f800000000)
#define NFP_ME28_INSTR_IS_MULT(instr) \
	((instr & NFP_ME28_INSTR_MASK_MULT) == NFP_ME28_INSTR_MULT)

typedef struct
{
  int cpp_target;
  int cpp_action;
  int cpp_token;
  unsigned int len_fixed;
  unsigned int len_mask;
  const char *mnemonic;
}
nfp_cmd_mnemonic;

#ifdef __cplusplus
}
#endif

#endif				/* _NFP_H_ */
