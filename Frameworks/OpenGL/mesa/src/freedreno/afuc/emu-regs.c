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
#include <stdio.h>
#include <stdlib.h>

#include "emu.h"
#include "util.h"

/*
 * Emulator Registers:
 *
 * Handles access to GPR, GPU, control, and pipe registers.
 */

static bool
is_draw_state_control_reg(unsigned n)
{
   char *reg_name = afuc_control_reg_name(n);
   if (!reg_name)
      return false;
   bool ret = !!strstr(reg_name, "DRAW_STATE");
   free(reg_name);
   return ret;
}

uint32_t
emu_get_control_reg(struct emu *emu, unsigned n)
{
   assert(n < ARRAY_SIZE(emu->control_regs.val));
   if (is_draw_state_control_reg(n))
      return emu_get_draw_state_reg(emu, n);
   return emu->control_regs.val[n];
}

void
emu_set_control_reg(struct emu *emu, unsigned n, uint32_t val)
{
   EMU_CONTROL_REG(PACKET_TABLE_WRITE);
   EMU_CONTROL_REG(PACKET_TABLE_WRITE_ADDR);
   EMU_CONTROL_REG(REG_WRITE);
   EMU_CONTROL_REG(REG_WRITE_ADDR);
   EMU_CONTROL_REG(BV_CNTL);
   EMU_CONTROL_REG(LPAC_CNTL);
   EMU_CONTROL_REG(THREAD_SYNC);

   assert(n < ARRAY_SIZE(emu->control_regs.val));
   BITSET_SET(emu->control_regs.written, n);
   emu->control_regs.val[n] = val;

   /* Some control regs have special action on write: */
   if (n == emu_reg_offset(&PACKET_TABLE_WRITE)) {
      unsigned write_addr = emu_get_reg32(emu, &PACKET_TABLE_WRITE_ADDR);

      assert(write_addr < ARRAY_SIZE(emu->jmptbl));
      emu->jmptbl[write_addr] = val;

      emu_set_reg32(emu, &PACKET_TABLE_WRITE_ADDR, write_addr + 1);
   } else if (n == emu_reg_offset(&REG_WRITE)) {
      uint32_t write_addr = emu_get_reg32(emu, &REG_WRITE_ADDR);

      /* Upper bits seem like some flags, not part of the actual
       * register offset.. not sure what they mean yet:
       */
      uint32_t flags = write_addr >> 16;
      write_addr &= 0xffff;

      emu_set_gpu_reg(emu, write_addr++, val);
      emu_set_reg32(emu, &REG_WRITE_ADDR, write_addr | (flags << 16));
   } else if (gpuver >= 7 && n == emu_reg_offset(&BV_CNTL)) {
      /* This is sort-of a hack, but emulate what the BV bootstrap routine
       * does so that the main bootstrap routine doesn't get stuck.
       */
      emu_set_reg32(emu, &THREAD_SYNC,
                    emu_get_reg32(emu, &THREAD_SYNC) & ~(1u << 1));
   } else if (gpuver >= 7 && n == emu_reg_offset(&LPAC_CNTL)) {
      /* This is sort-of a hack, but emulate what the LPAC bootstrap routine
       * does so that the main bootstrap routine doesn't get stuck.
       */
      emu_set_reg32(emu, &THREAD_SYNC,
                    emu_get_reg32(emu, &THREAD_SYNC) & ~(1u << 2));
   } else if (is_draw_state_control_reg(n)) {
      emu_set_draw_state_reg(emu, n, val);
   }
}

uint32_t
emu_get_sqe_reg(struct emu *emu, unsigned n)
{
   assert(n < ARRAY_SIZE(emu->sqe_regs.val));
   return emu->sqe_regs.val[n];
}

void
emu_set_sqe_reg(struct emu *emu, unsigned n, uint32_t val)
{
   assert(n < ARRAY_SIZE(emu->sqe_regs.val));
   BITSET_SET(emu->sqe_regs.written, n);
   emu->sqe_regs.val[n] = val;
}

static uint32_t
emu_get_pipe_reg(struct emu *emu, unsigned n)
{
   assert(n < ARRAY_SIZE(emu->pipe_regs.val));
   return emu->pipe_regs.val[n];
}

static void
emu_set_pipe_reg(struct emu *emu, unsigned n, uint32_t val)
{
   EMU_PIPE_REG(NRT_DATA);
   EMU_PIPE_REG(NRT_ADDR);

   assert(n < ARRAY_SIZE(emu->pipe_regs.val));
   BITSET_SET(emu->pipe_regs.written, n);
   emu->pipe_regs.val[n] = val;

   /* Some pipe regs have special action on write: */
   if (n == emu_reg_offset(&NRT_DATA)) {
      uintptr_t addr = emu_get_reg64(emu, &NRT_ADDR);

      emu_mem_write_dword(emu, addr, val);

      emu_set_reg64(emu, &NRT_ADDR, addr + 4);
   }
}

static uint32_t
emu_get_gpu_reg(struct emu *emu, unsigned n)
{
   if (n >= ARRAY_SIZE(emu->gpu_regs.val))
      return 0;
   assert(n < ARRAY_SIZE(emu->gpu_regs.val));
   return emu->gpu_regs.val[n];
}

void
emu_set_gpu_reg(struct emu *emu, unsigned n, uint32_t val)
{
   if (n >= ARRAY_SIZE(emu->gpu_regs.val))
      return;
   assert(n < ARRAY_SIZE(emu->gpu_regs.val));
   BITSET_SET(emu->gpu_regs.written, n);
   emu->gpu_regs.val[n] = val;
}

static bool
is_pipe_reg_addr(unsigned regoff)
{
   return regoff > 0xffff;
}

static unsigned
get_reg_addr(struct emu *emu)
{
   switch (emu->data_mode) {
   case DATA_PIPE:
   case DATA_ADDR:    return REG_ADDR;
   case DATA_USRADDR: return REG_USRADDR;
   default:
      unreachable("bad data_mode");
      return 0;
   }
}

/* Handle reads for special streaming regs: */
static uint32_t
emu_get_fifo_reg(struct emu *emu, unsigned n)
{
   /* TODO the fifo regs are slurping out of a FIFO that the hw is filling
    * in parallel.. we can use `struct emu_queue` to emulate what is actually
    * happening more accurately
    */

   if (n == REG_MEMDATA) {
      /* $memdata */
      EMU_CONTROL_REG(MEM_READ_DWORDS);
      EMU_CONTROL_REG(MEM_READ_ADDR);

      unsigned  read_dwords = emu_get_reg32(emu, &MEM_READ_DWORDS);
      uintptr_t read_addr   = emu_get_reg64(emu, &MEM_READ_ADDR);

      if (read_dwords > 0) {
         emu_set_reg32(emu, &MEM_READ_DWORDS, read_dwords - 1);
         emu_set_reg64(emu, &MEM_READ_ADDR,   read_addr + 4);
      }

      return emu_mem_read_dword(emu, read_addr);
   } else if (n == REG_REGDATA) {
      /* $regdata */
      EMU_CONTROL_REG(REG_READ_DWORDS);
      EMU_CONTROL_REG(REG_READ_ADDR);

      unsigned read_dwords = emu_get_reg32(emu, &REG_READ_DWORDS);
      unsigned read_addr   = emu_get_reg32(emu, &REG_READ_ADDR);

      /* I think if the fw doesn't write REG_READ_DWORDS before
       * REG_READ_ADDR, it just ends up with a single value written
       * into the FIFO that $regdata is consuming from:
       */
      if (read_dwords > 0) {
         emu_set_reg32(emu, &REG_READ_DWORDS, read_dwords - 1);
         emu_set_reg32(emu, &REG_READ_ADDR,   read_addr + 1);
      }

      return emu_get_gpu_reg(emu, read_addr);
   } else if (n == REG_DATA) {
      /* $data */
      do {
         uint32_t rem = emu->gpr_regs.val[REG_REM];
         assert(rem >= 0);

         uint32_t val;
         if (emu_queue_pop(&emu->roq, &val)) {
            emu_set_gpr_reg(emu, REG_REM, --rem);
            return val;
         }

         /* If FIFO is empty, prompt for more input: */
         printf("FIFO empty, input a packet!\n");
         emu->run_mode = false;
         emu_main_prompt(emu);
      } while (true);
   } else {
      unreachable("not a FIFO reg");
      return 0;
   }
}

static void
emu_set_fifo_reg(struct emu *emu, unsigned n, uint32_t val)
{
   if ((n == REG_ADDR) || (n == REG_USRADDR)) {
      emu->data_mode = (n == REG_ADDR) ? DATA_ADDR : DATA_USRADDR;

      /* Treat these as normal register writes so we can see
       * updated values in the output as we step thru the
       * instructions:
       */
      emu->gpr_regs.val[n] = val;
      BITSET_SET(emu->gpr_regs.written, n);

      if (is_pipe_reg_addr(val)) {
         /* "void" pipe regs don't have a value to write, so just
          * treat it as writing zero to the pipe reg:
          */
         if (afuc_pipe_reg_is_void(val >> 24))
            emu_set_pipe_reg(emu, val >> 24, 0);
         emu->data_mode = DATA_PIPE;
      }
   } else if (n == REG_DATA) {
      unsigned reg = get_reg_addr(emu);
      unsigned regoff = emu->gpr_regs.val[reg];
      if (is_pipe_reg_addr(regoff)) {
         /* writes pipe registers: */

         assert(!(regoff & 0xfbffff));

         /* If b18 is set, don't auto-increment dest addr.. and if we
          * do auto-increment, we only increment the high 8b
          *
          * Note that we bypass emu_set_gpr_reg() in this case because
          * auto-incrementing isn't triggering a write to "void" pipe
          * regs.
          */
         if (!(regoff & 0x40000)) {
            emu->gpr_regs.val[reg] = regoff + 0x01000000;
            BITSET_SET(emu->gpr_regs.written, reg);
         }

         emu_set_pipe_reg(emu, regoff >> 24, val);
      } else {
         /* writes to gpu registers: */
         emu_set_gpr_reg(emu, reg, regoff+1);
         emu_set_gpu_reg(emu, regoff, val);
      }
   }
}

uint32_t
emu_get_gpr_reg(struct emu *emu, unsigned n)
{
   assert(n < ARRAY_SIZE(emu->gpr_regs.val));

   /* Handle special regs: */
   switch (n) {
   case 0x00:
      return 0;
   case REG_MEMDATA:
   case REG_REGDATA:
   case REG_DATA:
      return emu_get_fifo_reg(emu, n);
   default:
      return emu->gpr_regs.val[n];
   }
}

void
emu_set_gpr_reg(struct emu *emu, unsigned n, uint32_t val)
{
   assert(n < ARRAY_SIZE(emu->gpr_regs.val));

   switch (n) {
   case REG_ADDR:
   case REG_USRADDR:
   case REG_DATA:
      emu_set_fifo_reg(emu, n, val);
      break;
   default:
      emu->gpr_regs.val[n] = val;
      BITSET_SET(emu->gpr_regs.written, n);
      break;
   }
}

/*
 * Control/pipe register accessor helpers:
 */

struct emu_reg_accessor {
   unsigned (*get_offset)(const char *name);
   uint32_t (*get)(struct emu *emu, unsigned n);
   void (*set)(struct emu *emu, unsigned n, uint32_t val);
};

const struct emu_reg_accessor emu_control_accessor = {
      .get_offset = afuc_control_reg,
      .get = emu_get_control_reg,
      .set = emu_set_control_reg,
};

const struct emu_reg_accessor emu_sqe_accessor = {
      .get_offset = afuc_sqe_reg,
      .get = emu_get_sqe_reg,
      .set = emu_set_sqe_reg,
};

const struct emu_reg_accessor emu_pipe_accessor = {
      .get_offset = afuc_pipe_reg,
      .get = emu_get_pipe_reg,
      .set = emu_set_pipe_reg,
};

const struct emu_reg_accessor emu_gpu_accessor = {
      .get_offset = afuc_gpu_reg,
      .get = emu_get_gpu_reg,
      .set = emu_set_gpu_reg,
};

unsigned
emu_reg_offset(struct emu_reg *reg)
{
   if (reg->offset == ~0)
      reg->offset = reg->accessor->get_offset(reg->name);
   return reg->offset;
}

uint32_t
emu_get_reg32(struct emu *emu, struct emu_reg *reg)
{
   return reg->accessor->get(emu, emu_reg_offset(reg));
}

uint64_t
emu_get_reg64(struct emu *emu, struct emu_reg *reg)
{
   uint64_t val = reg->accessor->get(emu, emu_reg_offset(reg) + 1);
   val <<= 32;
   val |= reg->accessor->get(emu, emu_reg_offset(reg));
   return val;
}

void
emu_set_reg32(struct emu *emu, struct emu_reg *reg, uint32_t val)
{
   reg->accessor->set(emu, emu_reg_offset(reg), val);
}

void
emu_set_reg64(struct emu *emu, struct emu_reg *reg, uint64_t val)
{
   reg->accessor->set(emu, emu_reg_offset(reg),     val);
   reg->accessor->set(emu, emu_reg_offset(reg) + 1, val >> 32);
}
