/*
 * Copyright © 2021 Google, Inc.
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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "util/u_math.h"

#include "freedreno_pm4.h"

#include "isaspec.h"

#include "emu.h"
#include "util.h"

#define rotl32(x,r) (((x) << (r)) | ((x) >> (32 - (r))))
#define rotl64(x,r) (((x) << (r)) | ((x) >> (64 - (r))))

EMU_SQE_REG(SP);
EMU_SQE_REG(STACK0);
EMU_CONTROL_REG(DRAW_STATE_SET_HDR);

/**
 * AFUC emulator.  Currently only supports a6xx
 *
 * TODO to add a5xx it might be easier to compile this multiple times
 * with conditional compile to deal with differences between generations.
 */

static uint32_t
emu_alu(struct emu *emu, afuc_opc opc, uint32_t src1, uint32_t src2)
{
   uint64_t tmp;
   switch (opc) {
   case OPC_ADD:
      tmp = (uint64_t)src1 + (uint64_t)src2;
      emu->carry = tmp >> 32;
      return (uint32_t)tmp;
   case OPC_ADDHI:
      return src1 + src2 + emu->carry;
   case OPC_SUB:
      tmp = (uint64_t)src1 - (uint64_t)src2;
      emu->carry = tmp >> 32;
      return (uint32_t)tmp;
   case OPC_SUBHI:
      return src1 - src2 + emu->carry;
   case OPC_AND:
      return src1 & src2;
   case OPC_OR:
      return src1 | src2;
   case OPC_XOR:
      return src1 ^ src2;
   case OPC_NOT:
      return ~src1;
   case OPC_SHL:
      return src1 << src2;
   case OPC_USHR:
      return src1 >> src2;
   case OPC_ISHR:
      return (int32_t)src1 >> src2;
   case OPC_ROT:
      if (src2 & 0x80000000)
         return rotl64(src1, -*(int32_t *)&src2);
      else
         return rotl32(src1, src2);
   case OPC_MUL8:
      return (src1 & 0xff) * (src2 & 0xff);
   case OPC_MIN:
      return MIN2(src1, src2);
   case OPC_MAX:
      return MAX2(src1, src2);
   case OPC_CMP:
      if (src1 > src2)
         return 0x00;
      else if (src1 == src2)
         return 0x2b;
      return 0x1e;
   case OPC_BIC:
      return src1 & ~src2;
   case OPC_MSB:
      if (!src2)
         return 0;
      return util_last_bit(src2) - 1;
   case OPC_SETBIT: {
      unsigned bit = src2 >> 1;
      unsigned val = src2 & 1;
      return (src1 & ~(1u << bit)) | (val << bit);
   }
   default:
      printf("unhandled alu opc: 0x%02x\n", opc);
      exit(1);
   }
}

/**
 * Helper to calculate load/store address based on LOAD_STORE_HI
 */
static uintptr_t
load_store_addr(struct emu *emu, unsigned gpr)
{
   EMU_CONTROL_REG(LOAD_STORE_HI);

   uintptr_t addr = emu_get_reg32(emu, &LOAD_STORE_HI);
   addr <<= 32;

   return addr + emu_get_gpr_reg(emu, gpr);
}

static void
emu_instr(struct emu *emu, struct afuc_instr *instr)
{
   uint32_t rem = emu_get_gpr_reg(emu, REG_REM);

   switch (instr->opc) {
   case OPC_NOP:
      break;
   case OPC_MSB:
   case OPC_ADD ... OPC_BIC: {
      uint32_t val = emu_alu(emu, instr->opc,
                             emu_get_gpr_reg(emu, instr->src1),
                             instr->has_immed ? instr->immed : 
                             emu_get_gpr_reg(emu, instr->src2));
      emu_set_gpr_reg(emu, instr->dst, val);

      if (instr->xmov) {
         unsigned m = MIN2(instr->xmov, rem);

         assert(m <= 3);

         if (m == 1) {
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, REG_DATA,
                            emu_get_gpr_reg(emu, instr->src2));
         } else if (m == 2) {
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, REG_DATA,
                            emu_get_gpr_reg(emu, instr->src2));
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, REG_DATA,
                            emu_get_gpr_reg(emu, instr->src2));
         } else if (m == 3) {
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, REG_DATA,
                            emu_get_gpr_reg(emu, instr->src2));
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, instr->dst,
                            emu_get_gpr_reg(emu, instr->src2));
            emu_set_gpr_reg(emu, REG_REM, --rem);
            emu_dump_state_change(emu);
            emu_set_gpr_reg(emu, REG_DATA,
                            emu_get_gpr_reg(emu, instr->src2));
         }
      }
      break;
   }
   case OPC_MOVI: {
      uint32_t val = instr->immed << instr->shift;
      emu_set_gpr_reg(emu, instr->dst, val);
      break;
   }
   case OPC_SETBITI: {
      uint32_t src = emu_get_gpr_reg(emu, instr->src1);
      emu_set_gpr_reg(emu, instr->dst, src | (1u << instr->bit));
      break;
   }
   case OPC_CLRBIT: {
      uint32_t src = emu_get_gpr_reg(emu, instr->src1);
      emu_set_gpr_reg(emu, instr->dst, src & ~(1u << instr->bit));
      break;
   }
   case OPC_UBFX: {
      uint32_t src = emu_get_gpr_reg(emu, instr->src1);
      unsigned lo = instr->bit, hi = instr->immed;
      uint32_t dst = (src >> lo) & BITFIELD_MASK(hi - lo + 1);
      emu_set_gpr_reg(emu, instr->dst, dst);
      break;
   }
   case OPC_BFI: {
      uint32_t src = emu_get_gpr_reg(emu, instr->src1);
      unsigned lo = instr->bit, hi = instr->immed;
      src = (src & BITFIELD_MASK(hi - lo + 1)) << lo;
      emu_set_gpr_reg(emu, instr->dst, emu_get_gpr_reg(emu, instr->dst) | src);
      break;
   }
   case OPC_CWRITE: {
      uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);
      uint32_t src2 = emu_get_gpr_reg(emu, instr->src2);
      uint32_t reg = src2 + instr->immed;

      if (instr->preincrement) {
         emu_set_gpr_reg(emu, instr->src2, reg);
      }

      emu_set_control_reg(emu, reg, src1);

      for (unsigned i = 0; i < instr->sds; i++) {
         uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);

         /* TODO: There is likely a DRAW_STATE_SET_BASE register on a6xx, as
          * there is on a7xx, and we should be writing that instead of setting
          * the base directly.
          */
         if (reg == emu_reg_offset(&DRAW_STATE_SET_HDR))
            emu_set_draw_state_base(emu, i, src1);
      }
      break;
   }
   case OPC_CREAD: {
      uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);

      if (instr->preincrement) {
         emu_set_gpr_reg(emu, instr->src1, src1 + instr->immed);
      }

      emu_set_gpr_reg(emu, instr->dst,
                      emu_get_control_reg(emu, src1 + instr->immed));
      break;
   }
   case OPC_SWRITE: {
      uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);
      uint32_t src2 = emu_get_gpr_reg(emu, instr->src2);

      if (instr->preincrement) {
         emu_set_gpr_reg(emu, instr->src2, src2 + instr->immed);
      }

      emu_set_sqe_reg(emu, src2 + instr->immed, src1);
      break;
   }
   case OPC_SREAD: {
      uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);

      if (instr->preincrement) {
         emu_set_gpr_reg(emu, instr->src1, src1 + instr->immed);
      }

      emu_set_gpr_reg(emu, instr->dst,
                      emu_get_sqe_reg(emu, src1 + instr->immed));
      break;
   }
   case OPC_LOAD: {
      uintptr_t addr = load_store_addr(emu, instr->src1) +
            instr->immed;

      if (instr->preincrement) {
         uint32_t src1 = emu_get_gpr_reg(emu, instr->src1);
         emu_set_gpr_reg(emu, instr->src1, src1 + instr->immed);
      }

      uint32_t val = emu_mem_read_dword(emu, addr);

      emu_set_gpr_reg(emu, instr->dst, val);

      break;
   }
   case OPC_STORE: {
      uintptr_t addr = load_store_addr(emu, instr->src2) +
            instr->immed;

      if (instr->preincrement) {
         uint32_t src2 = emu_get_gpr_reg(emu, instr->src2);
         emu_set_gpr_reg(emu, instr->src2, src2 + instr->immed);
      }

      uint32_t val = emu_get_gpr_reg(emu, instr->src1);

      emu_mem_write_dword(emu, addr, val);

      break;
   }
   case OPC_BRNEI ... OPC_BREQB: {
      uint32_t off = emu->gpr_regs.pc + instr->offset;
      uint32_t src = emu_get_gpr_reg(emu, instr->src1);

      if (instr->opc == OPC_BRNEI) {
         if (src != instr->immed)
            emu->branch_target = off;
      } else if (instr->opc == OPC_BREQI) {
         if (src == instr->immed)
            emu->branch_target = off;
      } else if (instr->opc == OPC_BRNEB) {
         if (!(src & (1 << instr->bit)))
            emu->branch_target = off;
      } else if (instr->opc == OPC_BREQB) {
         if (src & (1 << instr->bit))
            emu->branch_target = off;
      } else {
         assert(0);
      }
      break;
   }
   case OPC_RET: {
      unsigned sp = emu_get_reg32(emu, &SP);
      assert(sp > 0);

      /* counter-part to 'call' instruction, also has a delay slot: */
      emu->branch_target = emu_get_sqe_reg(emu, emu_reg_offset(&STACK0) + sp - 1);
      emu_set_reg32(emu, &SP, sp - 1);

      break;
   }
   case OPC_CALL: {
      unsigned sp = emu_get_reg32(emu, &SP);
      assert(sp + emu_reg_offset(&STACK0) < ARRAY_SIZE(emu->sqe_regs.val));

      /* call looks to have same delay-slot behavior as branch/etc, so
       * presumably the return PC is two instructions later:
       */
      emu_set_sqe_reg(emu, emu_reg_offset(&STACK0) + sp, emu->gpr_regs.pc + 2);
      emu_set_reg32(emu, &SP, sp + 1);
      emu->branch_target = instr->literal;

      break;
   }
   case OPC_WAITIN: {
      assert(!emu->branch_target);
      emu->run_mode = false;
      emu->waitin = true;
      break;
   }
   /* OPC_PREEMPTLEAVE6 */
   case OPC_SETSECURE: {
      // TODO this acts like a conditional branch, but in which case
      // does it branch?
      break;
   }
   default:
      printf("unhandled opc: 0x%02x\n", instr->opc);
      exit(1);
   }

   if (instr->rep) {
      assert(rem > 0);
      emu_set_gpr_reg(emu, REG_REM, --rem);
   }
}

void
emu_step(struct emu *emu)
{
   struct afuc_instr *instr;
   bool decoded = isa_decode((void *)&instr,
                             (void *)&emu->instrs[emu->gpr_regs.pc],
                             &(struct isa_decode_options) {
                              .gpu_id = gpuver,
                             });

   if (!decoded) {
      uint32_t instr_val = emu->instrs[emu->gpr_regs.pc];
      if ((instr_val >> 27) == 0) {
         /* This is printed as an undecoded literal to show the immediate
          * payload, but when executing it's just a NOP.
          */
         instr = calloc(1, sizeof(struct afuc_instr));
         instr->opc = OPC_NOP;
      } else {
         printf("unmatched instruction: 0x%08x\n", instr_val);
         exit(1);
      }
   }

   emu_main_prompt(emu);

   uint32_t branch_target = emu->branch_target;
   emu->branch_target = 0;

   bool waitin = emu->waitin;
   emu->waitin = false;

   if (instr->rep) {
      do {
         if (!emu_get_gpr_reg(emu, REG_REM))
            break;

         emu_clear_state_change(emu);
         emu_instr(emu, instr);

         /* defer last state-change dump until after any
          * post-delay-slot handling below:
          */
         if (emu_get_gpr_reg(emu, REG_REM))
            emu_dump_state_change(emu);
      } while (true);
   } else {
      emu_clear_state_change(emu);
      emu_instr(emu, instr);
   }

   emu->gpr_regs.pc++;

   if (branch_target) {
      emu->gpr_regs.pc = branch_target;
   }

   if (waitin) {
      uint32_t hdr = emu_get_gpr_reg(emu, 1);
      uint32_t id, count;

      if (pkt_is_type4(hdr)) {
         id = afuc_pm4_id("PKT4");
         count = type4_pkt_size(hdr);

         /* Possibly a hack, not sure what the hw actually
          * does here, but we want to mask out the pkt
          * type field from the hdr, so that PKT4 handler
          * doesn't see it and interpret it as part as the
          * register offset:
          */
         emu->gpr_regs.val[1] &= 0x0fffffff;
      } else if (pkt_is_type7(hdr)) {
         id = cp_type7_opcode(hdr);
         count = type7_pkt_size(hdr);
      } else {
         printf("Invalid opcode: 0x%08x\n", hdr);
         exit(1);  /* GPU goes *boom* */
      }

      assert(id < ARRAY_SIZE(emu->jmptbl));

      emu_set_gpr_reg(emu, REG_REM, count);
      emu->gpr_regs.pc = emu->jmptbl[id];
   }

   emu_dump_state_change(emu);

   free(instr);
}

void
emu_run_bootstrap(struct emu *emu)
{
   EMU_CONTROL_REG(PACKET_TABLE_WRITE_ADDR);

   emu->quiet = true;
   emu->run_mode = true;

   while (emu_get_reg32(emu, &PACKET_TABLE_WRITE_ADDR) < 0x80) {
      emu_step(emu);
   }
}


static void
check_access(struct emu *emu, uintptr_t gpuaddr, unsigned sz)
{
   if ((gpuaddr % sz) != 0) {
      printf("unaligned access fault: %p\n", (void *)gpuaddr);
      exit(1);
   }

   if ((gpuaddr + sz) >= EMU_MEMORY_SIZE) {
      printf("iova fault: %p\n", (void *)gpuaddr);
      exit(1);
   }
}

uint32_t
emu_mem_read_dword(struct emu *emu, uintptr_t gpuaddr)
{
   check_access(emu, gpuaddr, 4);
   return *(uint32_t *)(emu->gpumem + gpuaddr);
}

static void
mem_write_dword(struct emu *emu, uintptr_t gpuaddr, uint32_t val)
{
   check_access(emu, gpuaddr, 4);
   *(uint32_t *)(emu->gpumem + gpuaddr) = val;
}

void
emu_mem_write_dword(struct emu *emu, uintptr_t gpuaddr, uint32_t val)
{
   mem_write_dword(emu, gpuaddr, val);
   assert(emu->gpumem_written == ~0);
   emu->gpumem_written = gpuaddr;
}

void
emu_init(struct emu *emu)
{
   emu->gpumem = mmap(NULL, EMU_MEMORY_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                      0, 0);
   if (emu->gpumem == MAP_FAILED) {
      printf("Could not allocate GPU memory: %s\n", strerror(errno));
      exit(1);
   }

   /* Copy the instructions into GPU memory: */
   for (unsigned i = 0; i < emu->sizedwords; i++) {
      mem_write_dword(emu, EMU_INSTR_BASE + (4 * i), emu->instrs[i]);
   }

   EMU_GPU_REG(CP_SQE_INSTR_BASE);
   EMU_GPU_REG(CP_LPAC_SQE_INSTR_BASE);
   EMU_CONTROL_REG(BV_INSTR_BASE);
   EMU_CONTROL_REG(LPAC_INSTR_BASE);

   /* Setup the address of the SQE fw, just use the normal CPU ptr address: */
   switch (emu->processor) {
   case EMU_PROC_SQE:
      emu_set_reg64(emu, &CP_SQE_INSTR_BASE, EMU_INSTR_BASE);
      break;
   case EMU_PROC_BV:
      emu_set_reg64(emu, &BV_INSTR_BASE, EMU_INSTR_BASE);
      break;
   case EMU_PROC_LPAC:
      if (gpuver >= 7)
         emu_set_reg64(emu, &LPAC_INSTR_BASE, EMU_INSTR_BASE);
      else
         emu_set_reg64(emu, &CP_LPAC_SQE_INSTR_BASE, EMU_INSTR_BASE);
      break;
   }

   if (emu->gpu_id == 730) {
      emu_set_control_reg(emu, 0xef, 1 << 21);
      emu_set_control_reg(emu, 0, 7 << 28);
   } else if (emu->gpu_id == 660) {
      emu_set_control_reg(emu, 0, 3 << 28);
   } else if (emu->gpu_id == 650) {
      emu_set_control_reg(emu, 0, 1 << 28);
   }
}

void
emu_fini(struct emu *emu)
{
   uint32_t *instrs = emu->instrs;
   unsigned sizedwords = emu->sizedwords;
   unsigned gpu_id = emu->gpu_id;

   munmap(emu->gpumem, EMU_MEMORY_SIZE);
   memset(emu, 0, sizeof(*emu));

   emu->instrs = instrs;
   emu->sizedwords = sizedwords;
   emu->gpu_id = gpu_id;
}
