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

#ifndef _EMU_H_
#define _EMU_H_

#include <stdbool.h>
#include <stdint.h>

#include "util/bitset.h"

#include "afuc.h"

extern int gpuver;

#define EMU_NUM_GPR_REGS 32

struct emu_gpr_regs {
   BITSET_DECLARE(written, EMU_NUM_GPR_REGS);
   uint32_t pc;
   uint32_t val[EMU_NUM_GPR_REGS];
};

#define EMU_NUM_CONTROL_REGS 0x1000

struct emu_control_regs {
   BITSET_DECLARE(written, EMU_NUM_CONTROL_REGS);
   uint32_t val[EMU_NUM_CONTROL_REGS];
};

#define EMU_NUM_SQE_REGS 0x10

struct emu_sqe_regs {
   BITSET_DECLARE(written, EMU_NUM_SQE_REGS);
   uint32_t val[EMU_NUM_SQE_REGS];
};

#define EMU_NUM_GPU_REGS 0x10000

struct emu_gpu_regs {
   BITSET_DECLARE(written, EMU_NUM_GPU_REGS);
   uint32_t val[EMU_NUM_GPU_REGS];
};

#define EMU_NUM_PIPE_REGS 0x100

struct emu_pipe_regs {
   BITSET_DECLARE(written, EMU_NUM_PIPE_REGS);
   uint32_t val[EMU_NUM_PIPE_REGS];
};

/**
 * A simple queue implementation to buffer up cmdstream for the
 * emulated firmware to consume
 */
struct emu_queue {
   unsigned head, tail, count;
   uint32_t fifo[0x100];
};

static inline bool
emu_queue_push(struct emu_queue *q, uint32_t val)
{
   if (q->count >= ARRAY_SIZE(q->fifo))
      return false;

   q->count++;
   q->head++;
   q->head %= ARRAY_SIZE(q->fifo);

   q->fifo[q->head] = val;

   return true;
}

static inline bool
emu_queue_pop(struct emu_queue *q, uint32_t *val)
{
   if (!q->count)
      return false;

   q->count--;
   q->tail++;
   q->tail %= ARRAY_SIZE(q->fifo);

   *val = q->fifo[q->tail];

   return true;
}

/**
 * Draw-state (ie. CP_SET_DRAW_STATE) related emulation
 */
struct emu_draw_state {
   unsigned prev_draw_state_sel;
   struct {
      union {
         uint32_t hdr;
         struct {
            uint16_t count;       /* # of dwords */
            uint16_t mode_mask;
         };
      };
      union {
         uint32_t base_lohi[2];
         uint64_t base;
      };
      uint64_t sds_base;
      uint32_t sds_dwords;
   } state[32];
};

/**
 * The GPU memory size:
 *
 * The size is a bit arbitrary, and could be increased.  The backing
 * storage is a MAP_ANONYMOUS mapping so untouched pages should not
 * have a cost other than consuming virtual address space.
 *
 * Use something >4gb so we can test that anything doing GPU pointer
 * math correctly handles rollover
 */
#define EMU_MEMORY_SIZE 0x200000000

/**
 * The GPU "address" of the instructions themselves:
 *
 * Note address is kind of arbitrary, but should be something non-
 * zero to sanity check the bootstrap process and packet-table
 * loading
 */
#define EMU_INSTR_BASE 0x1000

/**
 * Emulated hw state.
 */
struct emu {
   /**
    * In bootstrap mode, execute bootstrap without outputting anything.
    * Useful to (for example) extract packet-table.
    */
   bool quiet;

   enum {
      EMU_PROC_SQE,
      EMU_PROC_BV,
      EMU_PROC_LPAC,
   } processor;

   uint32_t *instrs;
   unsigned sizedwords;
   unsigned gpu_id;

   struct emu_control_regs control_regs;
   struct emu_sqe_regs     sqe_regs;
   struct emu_pipe_regs    pipe_regs;
   struct emu_gpu_regs     gpu_regs;
   struct emu_gpr_regs     gpr_regs;

   struct emu_draw_state   draw_state;

   /* branch target to jump to after next instruction (ie. after delay-
    * slot):
    */
   uint32_t branch_target;

   /* executed waitin, jump to handler after next instruction (ie. after
    * delay-slot):
    */
   bool waitin;

   /* (r)un mode, don't stop for input until next waitin: */
   bool run_mode;

   /* carry-bits for add/sub for addhi/subhi
    * TODO: this is probably in a SQE register somewhere
    */
   uint32_t carry;

   /* packet table (aka jmptable) has offsets for pm4 packet handlers: */
   uint32_t jmptbl[0x80];

   /* In reality ROQ is actually multiple queues, but we don't try
    * to model the hw that exactly (but instead only model the behavior)
    * so we just use this to buffer up cmdstream input
    */
   struct emu_queue roq;

   /* Mode for writes to $data: */
   enum {
      DATA_ADDR,
      DATA_USRADDR,
      DATA_PIPE,
   } data_mode;

   /* GPU address space: */
   void *gpumem;

   /* A bitset would be prohibitively large to track memory writes, to
    * show in the state-change dump.  But we can only write a single
    * dword per instruction (given that for (rep) and/or (xmov) we
    * dump state change at each "step" of the instruction.
    *
    * ~0 means no memory write
    */
   uintptr_t gpumem_written;
};

/*
 * API for disasm to use:
 */
void emu_step(struct emu *emu);
void emu_run_bootstrap(struct emu *emu);
void emu_init(struct emu *emu);
void emu_fini(struct emu *emu);

/*
 * Internal APIs
 */

uint32_t emu_mem_read_dword(struct emu *emu, uintptr_t gpuaddr);
void emu_mem_write_dword(struct emu *emu, uintptr_t gpuaddr, uint32_t val);

/* UI: */
void emu_main_prompt(struct emu *emu);
void emu_clear_state_change(struct emu *emu);
void emu_dump_state_change(struct emu *emu);

/* Registers: */
uint32_t emu_get_gpr_reg(struct emu *emu, unsigned n);
void emu_set_gpr_reg(struct emu *emu, unsigned n, uint32_t val);

void emu_set_gpu_reg(struct emu *emu, unsigned n, uint32_t val);

uint32_t emu_get_control_reg(struct emu *emu, unsigned n);
void emu_set_control_reg(struct emu *emu, unsigned n, uint32_t val);

uint32_t emu_get_sqe_reg(struct emu *emu, unsigned n);
void emu_set_sqe_reg(struct emu *emu, unsigned n, uint32_t val);

/* Register helpers for fixed fxn emulation, to avoid lots of boilerplate
 * for accessing other pipe/control registers.
 *
 * Example:
 *    EMU_CONTROL_REG(REG_NAME);
 *    val = emu_get_reg32(emu, &SOME_REG);
 */

struct emu_reg_accessor;

struct emu_reg {
   const char *name;
   const struct emu_reg_accessor *accessor;
   unsigned offset;
};

extern const struct emu_reg_accessor emu_control_accessor;
extern const struct emu_reg_accessor emu_sqe_accessor;
extern const struct emu_reg_accessor emu_pipe_accessor;
extern const struct emu_reg_accessor emu_gpu_accessor;

#define EMU_CONTROL_REG(name) static struct emu_reg name = { #name, &emu_control_accessor, ~0 }
#define EMU_SQE_REG(name) static struct emu_reg name = { #name, &emu_sqe_accessor, ~0 }
#define EMU_PIPE_REG(name)    static struct emu_reg name = { #name, &emu_pipe_accessor, ~0 }
#define EMU_GPU_REG(name)     static struct emu_reg name = { #name, &emu_gpu_accessor, ~0 }

unsigned emu_reg_offset(struct emu_reg *reg);
uint32_t emu_get_reg32(struct emu *emu, struct emu_reg *reg);
uint64_t emu_get_reg64(struct emu *emu, struct emu_reg *reg);
void emu_set_reg32(struct emu *emu, struct emu_reg *reg, uint32_t val);
void emu_set_reg64(struct emu *emu, struct emu_reg *reg, uint64_t val);

/* Draw-state control reg emulation: */
uint32_t emu_get_draw_state_reg(struct emu *emu, unsigned n);
void emu_set_draw_state_reg(struct emu *emu, unsigned n, uint32_t val);
void emu_set_draw_state_base(struct emu *emu, unsigned n, uint32_t val);

/* Helpers: */
#define printdelta(fmt, ...) afuc_printc(AFUC_ERR, fmt, ##__VA_ARGS__)

#endif /* _ASM_H_ */
