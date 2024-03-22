/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "mme_tu104_sim.h"

#include <inttypes.h>

#include "mme_tu104.h"
#include "util/u_math.h"

#include "nvk_clc597.h"

struct mme_tu104_sim {
   uint32_t param_count;
   const uint32_t *params;

   uint32_t load[2];

   /* Bound memory ranges */
   uint32_t mem_count;
   struct mme_tu104_sim_mem *mems;

   /* SET_MME_MEM_ADDRESS_A/B */
   uint64_t mem_addr;

   /* RAM, accessed by DREAD/DWRITE */
   struct {
      uint32_t data[MME_TU104_DRAM_COUNT];

      /* SET_MME_MEM_RAM_ADDRESS */
      uint32_t addr;
   } ram;

   struct {
      struct {
         uint32_t data[1024];
         uint64_t count;
      } read_fifo;
   } dma;

   /* NVC597_SET_MME_SHADOW_SCRATCH(i) */
   uint32_t scratch[256];

   struct {
      unsigned mthd:16;
      unsigned inc:4;
      bool has_mthd:1;
      unsigned _pad:5;
      unsigned data_len:8;
      uint32_t data[8];
   } mthd;

   uint32_t set_regs;
   uint32_t regs[23];
   uint32_t alu_res[2];
   uint32_t alu_carry;

   uint16_t ip;
   uint16_t next_ip;
   bool stop;

   uint32_t loop_count;
   uint16_t loop_start;
   uint16_t loop_end;
};

static bool
inst_loads_reg(const struct mme_tu104_inst *inst,
               enum mme_tu104_reg reg)
{
   return inst->pred == reg ||
          inst->alu[0].src[0] == reg ||
          inst->alu[0].src[1] == reg ||
          inst->alu[1].src[0] == reg ||
          inst->alu[1].src[1] == reg;
}

static bool
inst_loads_out(const struct mme_tu104_inst *inst,
               enum mme_tu104_out_op out)
{
   return inst->out[0].mthd == out ||
          inst->out[0].emit == out ||
          inst->out[1].mthd == out ||
          inst->out[1].emit == out;
}

static void
load_params(struct mme_tu104_sim *sim,
            const struct mme_tu104_inst *inst)
{
   const bool has_load0 = inst_loads_reg(inst, MME_TU104_REG_LOAD0) ||
                          inst_loads_out(inst, MME_TU104_OUT_OP_LOAD0);
   const bool has_load1 = inst_loads_reg(inst, MME_TU104_REG_LOAD1) ||
                          inst_loads_out(inst, MME_TU104_OUT_OP_LOAD1);
   assert(has_load0 || !has_load1);

   if (has_load0) {
      sim->load[0] = *sim->params;
      sim->params++;
      sim->param_count--;
   }

   if (has_load1) {
      sim->load[1] = *sim->params;
      sim->params++;
      sim->param_count--;
   }
}

static uint32_t
load_state(struct mme_tu104_sim *sim, uint16_t state)
{
   assert(state % 4 == 0);

   if (NVC597_SET_MME_SHADOW_SCRATCH(0) <= state &&
       state < NVC597_CALL_MME_MACRO(0)) {
      uint32_t i = (state - NVC597_SET_MME_SHADOW_SCRATCH(0)) / 4;
      assert(i <= ARRAY_SIZE(sim->scratch));
      return sim->scratch[i];
   }

   return 0;
}

static uint32_t *
find_mem(struct mme_tu104_sim *sim, uint64_t addr, const char *op_desc)
{
   for (uint32_t i = 0; i < sim->mem_count; i++) {
      if (addr < sim->mems[i].addr)
         continue;

      uint64_t offset = addr - sim->mems[i].addr;
      if (offset >= sim->mems[i].size)
         continue;

      assert(sim->mems[i].data != NULL);
      return (uint32_t *)((char *)sim->mems[i].data + offset);
   }

   fprintf(stderr, "FAULT in %s at address 0x%"PRIx64"\n", op_desc, addr);
   abort();
}

static void
finish_dma_read_fifo(struct mme_tu104_sim *sim)
{
   if (sim->dma.read_fifo.count == 0)
      return;

   for (uint32_t i = 0; i < sim->dma.read_fifo.count; i++) {
      uint32_t *src = find_mem(sim, sim->mem_addr + i * 4,
                               "MME_DMA_READ_FIFOED");
      assert(src != NULL);
      sim->dma.read_fifo.data[i] = *src;
   }

   sim->param_count = sim->dma.read_fifo.count;
   sim->params = sim->dma.read_fifo.data;
}

static void
flush_mthd(struct mme_tu104_sim *sim)
{
   if (!sim->mthd.has_mthd)
      return;

   uint16_t mthd = sim->mthd.mthd;
   const uint32_t *p = sim->mthd.data;
   const uint32_t *end = sim->mthd.data + sim->mthd.data_len;
   while (p < end) {
      uint32_t dw_used = 1;
      if (NVC597_SET_MME_SHADOW_SCRATCH(0) <= mthd &&
          mthd < NVC597_CALL_MME_MACRO(0)) {
         uint32_t i = (mthd - NVC597_SET_MME_SHADOW_SCRATCH(0)) / 4;
         assert(i <= ARRAY_SIZE(sim->scratch));
         sim->scratch[i] = *p;
      } else {
         switch (mthd) {
         case NVC597_SET_REPORT_SEMAPHORE_A: {
            assert(p + 4 <= end);
            uint64_t addr = ((uint64_t)p[0] << 32) | p[1];
            uint32_t data = p[2];
            assert(p[3] == 0x10000000);
            dw_used = 4;

            uint32_t *mem = find_mem(sim, addr, "SET_REPORT_SEMAPHORE");
            *mem = data;
            break;
         }
         case NVC597_SET_MME_DATA_RAM_ADDRESS:
            sim->ram.addr = *p;
            break;
         case NVC597_SET_MME_MEM_ADDRESS_A:
            assert(p + 2 <= end);
            sim->mem_addr = ((uint64_t)p[0] << 32) | p[1];
            dw_used = 2;
            break;
         case NVC597_MME_DMA_READ_FIFOED:
            sim->dma.read_fifo.count = *p;
            break;
         default:
            fprintf(stdout, "%s:\n", P_PARSE_NVC597_MTHD(mthd));
            P_DUMP_NVC597_MTHD_DATA(stdout, mthd, *p, "    ");
            break;
         }
      }

      p += dw_used;
      assert(sim->mthd.inc == 1);
      mthd += dw_used * 4;
   }

   sim->mthd.has_mthd = false;
}

static void
eval_extended(struct mme_tu104_sim *sim,
              uint32_t x, uint32_t y)
{
   /* The only extended method we know about appears to be some sort of
    * barrier required when using READ_FIFOED.
    */
   assert(x == 0x1000);
   assert(y == 1);
   flush_mthd(sim);
   finish_dma_read_fifo(sim);
}

static uint32_t
load_reg(struct mme_tu104_sim *sim,
         const struct mme_tu104_inst *inst,
         uint32_t imm_idx, enum mme_tu104_reg reg)
{
   if (reg <= MME_TU104_REG_R23) {
      assert(sim->set_regs & BITFIELD_BIT(reg));
      return sim->regs[reg];
   }

   switch (reg) {
   case MME_TU104_REG_ZERO:
      return 0;
   case MME_TU104_REG_IMM:
      assert(imm_idx < 2);
      /* Immediates are treated as signed for ALU ops */
      return (int16_t)inst->imm[imm_idx];
   case MME_TU104_REG_IMMPAIR:
      assert(imm_idx < 2);
      /* Immediates are treated as signed for ALU ops */
      return (int16_t)inst->imm[1 - imm_idx];
   case MME_TU104_REG_IMM32:
      return ((uint32_t)inst->imm[0] << 16) | inst->imm[1];
   case MME_TU104_REG_LOAD0:
      return sim->load[0];
   case MME_TU104_REG_LOAD1:
      return sim->load[1];
   default:
      unreachable("Unhandled register type");
   }
}

static uint8_t
load_pred(struct mme_tu104_sim *sim,
          const struct mme_tu104_inst *inst)
{
   if (inst->pred_mode == MME_TU104_PRED_UUUU)
      return 0xf;

   uint32_t val = load_reg(sim, inst, -1, inst->pred);
   const char *pred = mme_tu104_pred_to_str(inst->pred_mode);

   uint8_t mask = 0;
   for (unsigned i = 0; i < 4; i++) {
      if (pred[i] != (val ? 'T' : 'F'))
         mask |= BITFIELD_BIT(i);
   }

   return mask;
}

static void
store_reg(struct mme_tu104_sim *sim,
          enum mme_tu104_reg reg,
          uint32_t val)
{
   if (reg <= MME_TU104_REG_R23) {
      sim->set_regs |= BITFIELD_BIT(reg);
      sim->regs[reg] = val;
   } else if (reg <= MME_TU104_REG_ZERO) {
      /* Do nothing */
   } else {
      unreachable("Unhandled register type");
   }
}

static bool
eval_cond(enum mme_tu104_alu_op op, uint32_t x, uint32_t y)
{
   switch (op) {
   case MME_TU104_ALU_OP_BLT:
   case MME_TU104_ALU_OP_SLT:
      return (int32_t)x < (int32_t)y;
   case MME_TU104_ALU_OP_BLTU:
   case MME_TU104_ALU_OP_SLTU:
      return (uint32_t)x < (uint32_t)y;
   case MME_TU104_ALU_OP_BLE:
   case MME_TU104_ALU_OP_SLE:
      return (int32_t)x <= (int32_t)y;
   case MME_TU104_ALU_OP_BLEU:
   case MME_TU104_ALU_OP_SLEU:
      return (uint32_t)x <= (uint32_t)y;
   case MME_TU104_ALU_OP_BEQ:
   case MME_TU104_ALU_OP_SEQ:
      return x == y;
   default:
      unreachable("Not a comparison op");
   }
}

static void
eval_alu(struct mme_tu104_sim *sim,
         const struct mme_tu104_inst *inst,
         uint32_t alu_idx)
{
   const struct mme_tu104_alu *alu = &inst->alu[alu_idx];
   const uint32_t x = load_reg(sim, inst, alu_idx, alu->src[0]);
   const uint32_t y = load_reg(sim, inst, alu_idx, alu->src[1]);

   uint32_t res = 0;
   switch (inst->alu[alu_idx].op) {
   case MME_TU104_ALU_OP_ADD:
      res = x + y;
      sim->alu_carry = res < x;
      break;
   case MME_TU104_ALU_OP_ADDC:
      assert(alu_idx == 1);
      assert(inst->alu[0].op == MME_TU104_ALU_OP_ADD);
      res = x + y + sim->alu_carry;
      break;
   case MME_TU104_ALU_OP_SUB:
      res = x - y;
      sim->alu_carry = res > x;
      break;
   case MME_TU104_ALU_OP_SUBB:
      assert(alu_idx == 1);
      assert(inst->alu[0].op == MME_TU104_ALU_OP_SUB);
      res = x - y - sim->alu_carry;
      break;
   case MME_TU104_ALU_OP_MUL: {
      /* Sign extend but use uint64_t for the multiply so that we avoid
       * undefined behavior from possible signed multiply roll-over.
       */
      const uint64_t x_u64 = (int64_t)(int32_t)x;
      const uint64_t y_u64 = (int64_t)(int32_t)y;
      const uint64_t xy_u64 = x_u64 * y_u64;
      res = xy_u64;
      sim->alu_carry = xy_u64 >> 32;
      break;
   }
   case MME_TU104_ALU_OP_MULH:
      assert(inst->alu[alu_idx].src[0] == MME_TU104_REG_ZERO);
      assert(inst->alu[alu_idx].src[1] == MME_TU104_REG_ZERO);
      res = sim->alu_carry;
      break;
   case MME_TU104_ALU_OP_MULU: {
      const uint64_t x_u64 = x;
      const uint64_t y_u64 = y;
      const uint64_t xy_u64 = x_u64 * y_u64;
      res = xy_u64;
      sim->alu_carry = xy_u64 >> 32;
      break;
   }
   case MME_TU104_ALU_OP_EXTENDED:
      eval_extended(sim, x, y);
      break;
   case MME_TU104_ALU_OP_CLZ:
      res = __builtin_clz(x);
      break;
   case MME_TU104_ALU_OP_SLL:
      res = x << (y & 31);
      break;
   case MME_TU104_ALU_OP_SRL:
      res = x >> (y & 31);
      break;
   case MME_TU104_ALU_OP_SRA:
      res = (int32_t)x >> (y & 31);
      break;
   case MME_TU104_ALU_OP_AND:
      res = x & y;
      break;
   case MME_TU104_ALU_OP_NAND:
      res = ~(x & y);
      break;
   case MME_TU104_ALU_OP_OR:
      res = x | y;
      break;
   case MME_TU104_ALU_OP_XOR:
      res = x ^ y;
      break;
   case MME_TU104_ALU_OP_MERGE: {
      uint16_t immed = inst->imm[alu_idx];
      uint32_t dst_pos  = (immed >> 10) & 0x3f;
      uint32_t bits     = (immed >> 5)  & 0x1f;
      uint32_t src_pos  = (immed >> 0)  & 0x1f;
      res = (x & ~(BITFIELD_MASK(bits) << dst_pos)) |
            (((y >> src_pos) & BITFIELD_MASK(bits)) << dst_pos);
      break;
   }
   case MME_TU104_ALU_OP_SLT:
   case MME_TU104_ALU_OP_SLTU:
   case MME_TU104_ALU_OP_SLE:
   case MME_TU104_ALU_OP_SLEU:
   case MME_TU104_ALU_OP_SEQ:
      res = eval_cond(inst->alu[alu_idx].op, x, y) ? ~0u : 0u;
      break;
   case MME_TU104_ALU_OP_STATE:
      flush_mthd(sim);
      res = load_state(sim, (uint16_t)(x + y) * 4);
      break;
   case MME_TU104_ALU_OP_LOOP:
      assert(sim->loop_count == 0);
      assert(inst->alu[alu_idx].dst == MME_TU104_REG_ZERO);
      assert(inst->alu[alu_idx].src[1] == MME_TU104_REG_ZERO);
      sim->loop_count = MAX2(1, x) - 1;
      sim->loop_start = sim->ip;
      sim->loop_end = sim->ip + inst->imm[alu_idx] - 1;
      assert(sim->loop_end > sim->ip);
      break;
   case MME_TU104_ALU_OP_JAL: {
      assert(inst->alu[alu_idx].dst == MME_TU104_REG_ZERO);
      assert(inst->alu[alu_idx].src[0] == MME_TU104_REG_ZERO);
      assert(inst->alu[alu_idx].src[1] == MME_TU104_REG_ZERO);
      /* No idea what bit 15 does.  The NVIDIA blob always sets it. */
      assert(inst->imm[alu_idx] & BITFIELD_BIT(15));
      uint16_t offset = (inst->imm[alu_idx] & BITFIELD_MASK(15));
      sim->next_ip = sim->ip + offset;
      res = 0;
      break;
   }
   case MME_TU104_ALU_OP_BLT:
   case MME_TU104_ALU_OP_BLTU:
   case MME_TU104_ALU_OP_BLE:
   case MME_TU104_ALU_OP_BLEU:
   case MME_TU104_ALU_OP_BEQ: {
      assert(inst->alu[alu_idx].dst == MME_TU104_REG_ZERO);
      bool expect = (inst->imm[alu_idx] & BITFIELD_BIT(15)) != 0;
      if (eval_cond(inst->alu[alu_idx].op, x, y) == expect) {
         int16_t offset = util_mask_sign_extend(inst->imm[alu_idx], 13);
         if ((uint16_t)offset == 0xf000) {
            sim->stop = true;
            break;
         }

         assert((int)sim->ip + offset >= 0);
         assert((int)sim->ip + offset < 0x1000);
         sim->next_ip = sim->ip + offset;
      }
      break;
   }
   case MME_TU104_ALU_OP_DREAD:
      assert(inst->alu[alu_idx].src[1] == MME_TU104_REG_ZERO);
      assert(x < ARRAY_SIZE(sim->ram.data));
      res = sim->ram.data[x];
      break;
   case MME_TU104_ALU_OP_DWRITE:
      assert(inst->alu[alu_idx].dst == MME_TU104_REG_ZERO);
      assert(x < ARRAY_SIZE(sim->ram.data));
      sim->ram.data[x] = y;
      break;
   default:
      unreachable("Unhandled ALU op");
   }

   sim->alu_res[alu_idx] = res;
   store_reg(sim, inst->alu[alu_idx].dst, res);
}

static uint32_t
load_out(struct mme_tu104_sim *sim,
         const struct mme_tu104_inst *inst,
         enum mme_tu104_out_op op)
{
   switch (op) {
   case MME_TU104_OUT_OP_ALU0:
      return sim->alu_res[0];
   case MME_TU104_OUT_OP_ALU1:
      return sim->alu_res[1];
   case MME_TU104_OUT_OP_LOAD0:
      return sim->load[0];
   case MME_TU104_OUT_OP_LOAD1:
      return sim->load[1];
   case MME_TU104_OUT_OP_IMM0:
      return inst->imm[0];
   case MME_TU104_OUT_OP_IMM1:
      return inst->imm[1];
   case MME_TU104_OUT_OP_IMMHIGH0:
      return inst->imm[0] >> 12;
   case MME_TU104_OUT_OP_IMMHIGH1:
      return inst->imm[1] >> 12;
   case MME_TU104_OUT_OP_IMM32:
      return ((uint32_t)inst->imm[0] << 16) | inst->imm[1];
   default:
      unreachable("Unhandled output op");
   }
}

static void
eval_out(struct mme_tu104_sim *sim,
         const struct mme_tu104_inst *inst,
         uint32_t out_idx)
{
   if (inst->out[out_idx].mthd != MME_TU104_OUT_OP_NONE) {
      uint32_t data = load_out(sim, inst, inst->out[out_idx].mthd);

      flush_mthd(sim);
      sim->mthd.mthd = (data & 0xfff) << 2;
      sim->mthd.inc = (data >> 12) & 0xf;
      sim->mthd.has_mthd = true;
      sim->mthd.data_len = 0;
   }

   if (inst->out[out_idx].emit != MME_TU104_OUT_OP_NONE) {
      uint32_t data = load_out(sim, inst, inst->out[out_idx].emit);

      assert(sim->mthd.data_len < ARRAY_SIZE(sim->mthd.data));
      sim->mthd.data[sim->mthd.data_len++] = data;
   }
}

void
mme_tu104_sim(uint32_t inst_count, const struct mme_tu104_inst *insts,
              uint32_t param_count, const uint32_t *params,
              uint32_t mem_count, struct mme_tu104_sim_mem *mems)
{
   struct mme_tu104_sim sim = {
      .param_count = param_count,
      .params = params,
      .mem_count = mem_count,
      .mems = mems,
   };

   bool end_next = false;
   while (true) {
      assert(sim.ip < inst_count);
      const struct mme_tu104_inst *inst = &insts[sim.ip];
      sim.next_ip = sim.ip + 1;

      load_params(&sim, inst);

      uint8_t pred = load_pred(&sim, inst);

      /* No idea why the HW has this rule but it does */
      assert(inst->alu[0].op != MME_TU104_ALU_OP_STATE ||
             inst->alu[1].op != MME_TU104_ALU_OP_STATE);

      if (pred & BITFIELD_BIT(0))
         eval_alu(&sim, inst, 0);
      if (pred & BITFIELD_BIT(1))
         eval_alu(&sim, inst, 1);
      if (pred & BITFIELD_BIT(2))
         eval_out(&sim, inst, 0);
      if (pred & BITFIELD_BIT(3))
         eval_out(&sim, inst, 1);

      if (end_next || sim.stop)
         break;

      end_next = inst->end_next;

      if (sim.loop_count > 0 && sim.ip == sim.loop_end) {
         sim.loop_count--;
         sim.next_ip = sim.loop_start + 1;
      }

      sim.ip = sim.next_ip;
   }

   flush_mthd(&sim);
}
