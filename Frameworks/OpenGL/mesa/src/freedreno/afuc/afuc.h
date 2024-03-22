/*
 * Copyright (c) 2017 Rob Clark <robdclark@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _AFUC_H_
#define _AFUC_H_

#include <stdbool.h>

#include "util/macros.h"

/*
TODO kernel debugfs to inject packet into rb for easier experimentation.  It
should trigger reloading pfp/me and resetting gpu..

Actually maybe it should be flag on submit ioctl to be able to deal w/ relocs,
should be restricted to CAP_ADMIN and probably compile option too (default=n).
if flag set, copy cmdstream bo contents into RB instead of IB'ing to it from
RB.
 */

typedef enum {
   OPC_NOP,

#define ALU(name) \
   OPC_##name, \
   OPC_##name##I,
   ALU(ADD)   /* add immediate */
   ALU(ADDHI) /* add immediate (hi 32b of 64b) */
   ALU(SUB)   /* subtract immediate */
   ALU(SUBHI) /* subtract immediate (hi 32b of 64b) */
   ALU(AND)   /* AND immediate */
   ALU(OR)    /* OR immediate */
   ALU(XOR)   /* XOR immediate */
   ALU(NOT)   /* bitwise not of immed (src1 ignored) */
   ALU(SHL)   /* shift-left immediate */
   ALU(USHR)  /* unsigned shift right by immediate */
   ALU(ISHR)  /* signed shift right by immediate */
   ALU(ROT)   /* rotate left (left shift with wrap-around) */
   ALU(MUL8)  /* 8bit multiply by immediate */
   ALU(MIN)
   ALU(MAX)
   ALU(CMP)   /* compare src to immed */
   ALU(BIC)   /* AND with second source negated */
   OPC_SETBIT, /* Set or clear a bit dynamically */
   OPC_MOVI,  /* move immediate */
   OPC_SETBITI, /* Set a bit */
   OPC_CLRBIT, /* Clear a bit */
   OPC_UBFX, /* Unsigned BitField eXtract */
   OPC_BFI,  /* BitField Insert */
#undef ALU

   /* Return the most-significant bit of src2, or 0 if src2 == 0 (the
    * same as if src2 == 1). src1 is ignored. Note that this overlaps
    * with STORE, so it can only be used with the two-source encoding.
    */
   OPC_MSB,

   /* These seem something to do with setting some external state..
    * doesn't seem to map *directly* to registers, but I guess that
    * is where things end up.  For example, this sequence in the
    * CP_INDIRECT_BUFFER handler:
    *
    *     mov $02, $data   ; low 32b of IB target address
    *     mov $03, $data   ; high 32b of IB target
    *     mov $04, $data   ; IB size in dwords
    *     breq $04, 0x0, #l23 (#69, 04a2)
    *     and $05, $18, 0x0003
    *     shl $05, $05, 0x0002
    *     cwrite $02, [$05 + 0x0b0], 0x8
    *     cwrite $03, [$05 + 0x0b1], 0x8
    *     cwrite $04, [$05 + 0x0b2], 0x8
    *
    * Note that CP_IB1/2_BASE_LO/HI/BUFSZ in 0x0b1f->0xb21 (IB1) and
    * 0x0b22->0x0b24 (IB2).  Presumably $05 ends up w/ different value
    * for RB->IB1 vs IB1->IB2.
    */
   OPC_CWRITE,
   OPC_CREAD,

   /* A6xx added new opcodes that let you read/write directly to memory (and
    * bypass the IOMMU?).
    */
   OPC_STORE,
   OPC_LOAD,

   /* A6xx added new opcodes that let you read/write the state of the
    * SQE processor itself, like the call stack. This is mostly used by
    * preemption but is also used to set the preempt routine entrypoint.
    */
   OPC_SREAD,
   OPC_SWRITE,

   OPC_BRNEI,         /* relative branch (if $src != immed) */
   OPC_BREQI,         /* relative branch (if $src == immed) */
   OPC_BRNEB,         /* relative branch (if bit not set) */
   OPC_BREQB,         /* relative branch (if bit is set) */
   OPC_RET,           /* return */
   OPC_IRET,          /* return from preemption interrupt handler */
   OPC_CALL,          /* "function" call */
   OPC_WAITIN,        /* wait for input (ie. wait for WPTR to advance) */
   OPC_PREEMPTLEAVE,  /* try to leave preemption */
   OPC_SETSECURE,     /* switch secure mode on/off */

   /* pseudo-opcodes without an actual encoding */
   OPC_BREQ,
   OPC_BRNE,
   OPC_JUMP,
   OPC_RAW_LITERAL,
} afuc_opc;

/**
 * Special GPR registers:
 *
 * Notes:  (applicable to a6xx, double check a5xx)
 *
 *   0x1d:
 *      $addr:    writes configure GPU reg address to read/write
 *                (does not respect CP_PROTECT)
 *      $memdata: reads from FIFO filled based on MEM_READ_DWORDS/
 *                MEM_READ_ADDR
 *   0x1e: (note different mnemonic for src vs dst)
 *      $usraddr: writes configure GPU reg address to read/write,
 *                respecting CP_PROTECT
 *      $regdata: reads from FIFO filled based on REG_READ_DWORDS/
 *                REG_READ_ADDR
 *   0x1f:
 *      $data:    reads from from pm4 input stream
 *      $data:    writes to stream configured by write to $addr
 *                or $usraddr
 */
typedef enum {
   REG_REM     = 0x1c,
   REG_MEMDATA = 0x1d,  /* when used as src */
   REG_ADDR    = 0x1d,  /* when used as dst */
   REG_REGDATA = 0x1e,  /* when used as src */
   REG_USRADDR = 0x1e,  /* when used as dst */
   REG_DATA    = 0x1f,
} afuc_reg;

struct afuc_instr {
   afuc_opc opc;

   uint8_t dst;
   uint8_t src1;
   uint8_t src2;
   uint32_t immed;
   uint8_t shift;
   uint8_t bit;
   uint8_t xmov;
   uint8_t sds;
   uint32_t literal;
   int offset;
   const char *label;

   bool has_immed : 1;
   bool has_shift : 1;
   bool has_bit : 1;
   bool is_literal : 1;
   bool rep : 1;
   bool preincrement : 1;
};

void print_control_reg(uint32_t id);
void print_sqe_reg(uint32_t id);
void print_pipe_reg(uint32_t id);

#endif /* _AFUC_H_ */
