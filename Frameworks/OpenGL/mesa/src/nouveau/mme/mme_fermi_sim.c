/*
 * Copyright © 2022 Mary Guillemard
 * SPDX-License-Identifier: MIT
 */
#include "mme_fermi_sim.h"

#include <inttypes.h>

#include "mme_fermi.h"
#include "util/u_math.h"

#include "nvk_cl9097.h"
#include "nvk_cl902d.h"

struct mme_fermi_sim {
   uint32_t param_count;
   const uint32_t *params;

   /* Bound memory ranges */
   uint32_t mem_count;
   struct mme_fermi_sim_mem *mems;

   /* SET_MME_MEM_ADDRESS_A/B */
   uint64_t mem_addr;

   /* RAM, accessed by STATE */
   struct {
      uint32_t data[MME_FERMI_DRAM_COUNT];

      /* SET_MME_MEM_RAM_ADDRESS */
      uint32_t addr;
   } ram;

   struct {
      // TODO: check if read_fifo is supported.
      struct {
         uint32_t data[1024];
         uint64_t count;
      } read_fifo;
   } dma;

   struct {
      unsigned mthd:16;
      unsigned inc:4;
      bool has_mthd:1;
   } mthd;

   /* SET_MME_SHADOW_SCRATCH(i) */
   uint32_t scratch[MME_FERMI_SCRATCH_COUNT];

   uint32_t regs[7];
   uint32_t alu_carry;
   uint16_t ip;
   uint16_t next_ip;
};

static uint32_t *
find_mem(struct mme_fermi_sim *sim, uint64_t addr, const char *op_desc)
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

static uint32_t
read_dmem(struct mme_fermi_sim *sim, uint64_t addr, const char *op_desc)
{
   uint32_t ram_index = addr / 4;

   if (ram_index < ARRAY_SIZE(sim->ram.data)) {
      return sim->ram.data[ram_index];
   }

   if (addr >= NV9097_SET_MME_SHADOW_SCRATCH(0) && addr < NV9097_CALL_MME_MACRO(0)) {
      return sim->scratch[ram_index - NV9097_SET_MME_SHADOW_SCRATCH(0) / 4];
   }

   fprintf(stderr, "FAULT in %s at DMEM address 0x%"PRIx64"\n (READ)", op_desc, addr);
   abort();
}

static void
write_dmem(struct mme_fermi_sim *sim, uint64_t addr, uint32_t val, const char *op_desc)
{
   uint32_t ram_index = addr / 4;

   if (ram_index < ARRAY_SIZE(sim->ram.data)) {
      sim->ram.data[ram_index] = val;
   }
   else if (addr >= NV9097_SET_MME_SHADOW_SCRATCH(0) && addr < NV9097_CALL_MME_MACRO(0)) {
      sim->scratch[ram_index - NV9097_SET_MME_SHADOW_SCRATCH(0) / 4] = val;
   } else {
      fprintf(stderr, "FAULT in %s at DMEM address 0x%"PRIx64" (WRITE)\n", op_desc, addr);
      abort();
   }
}

static uint64_t
read_dmem64(struct mme_fermi_sim *sim, uint64_t addr, const char *op_desc)
{
   return ((uint64_t)read_dmem(sim, addr, op_desc) << 32) | read_dmem(sim, addr + 4, op_desc);
}

static uint32_t load_param(struct mme_fermi_sim *sim)
{
   if (sim->param_count == 0) {
      // TODO: know what happens on hardware here
      return 0;
   }

   uint32_t param = *sim->params;

   sim->params++;
   sim->param_count--;

   return param;
}

static uint32_t load_reg(struct mme_fermi_sim *sim, enum mme_fermi_reg reg)
{
   if (reg == MME_FERMI_REG_ZERO) {
      return 0;
   }

   return sim->regs[reg - 1];
}

static void store_reg(struct mme_fermi_sim *sim, enum mme_fermi_reg reg, uint32_t val)
{
   if (reg == MME_FERMI_REG_ZERO) {
      return;
   }

   sim->regs[reg - 1] = val;
}

static int32_t load_imm(const struct mme_fermi_inst *inst)
{
   return util_mask_sign_extend(inst->imm, 18);
}

static uint32_t eval_bfe_lsl(uint32_t value, uint32_t src_bit, uint32_t dst_bit, uint8_t size)
{
   if (dst_bit > 31 || src_bit > 31) {
      return 0;
   }

   return ((value >> src_bit) & BITFIELD_MASK(size)) << dst_bit;
}

static uint32_t eval_op(struct mme_fermi_sim *sim, const struct mme_fermi_inst *inst) {
   assert(inst->op != MME_FERMI_OP_BRANCH);

   uint32_t x = load_reg(sim, inst->src[0]);
   uint32_t y = load_reg(sim, inst->src[1]);

   switch (inst->op) {
      case MME_FERMI_OP_ALU_REG: {
         uint32_t res = 0;

         switch (inst->alu_op) {
            case MME_FERMI_ALU_OP_ADD:
               res = x + y;
               sim->alu_carry = res < x;
               break;
            case MME_FERMI_ALU_OP_ADDC:
               res = x + y + sim->alu_carry;
               sim->alu_carry = res < x;
               break;
            case MME_FERMI_ALU_OP_SUB:
               res = x - y;
               sim->alu_carry = res > x;
               break;
            case MME_FERMI_ALU_OP_SUBB:
               res = x - y - sim->alu_carry;
               sim->alu_carry = res > x;
               break;
            case MME_FERMI_ALU_OP_XOR:
               res = x ^ y;
               break;
            case MME_FERMI_ALU_OP_OR:
               res = x | y;
               break;
            case MME_FERMI_ALU_OP_AND:
               res = x & y;
               break;
            case MME_FERMI_ALU_OP_AND_NOT:
               res = x & ~y;
               break;
            case MME_FERMI_ALU_OP_NAND:
               res = ~(x & y);
               break;
            default:
               unreachable("Unhandled ALU op");
         }

         return res;
      }
      case MME_FERMI_OP_ADD_IMM:
         return x + load_imm(inst);
      case MME_FERMI_OP_MERGE:
         return (x & ~(BITFIELD_MASK(inst->bitfield.size) << inst->bitfield.dst_bit)) | (((y >> inst->bitfield.src_bit) & BITFIELD_MASK(inst->bitfield.size)) << inst->bitfield.dst_bit);
      case MME_FERMI_OP_BFE_LSL_IMM:
         return eval_bfe_lsl(y, x, inst->bitfield.dst_bit, inst->bitfield.size);
      case MME_FERMI_OP_BFE_LSL_REG:
         return eval_bfe_lsl(y, inst->bitfield.src_bit, x, inst->bitfield.size);
      case MME_FERMI_OP_STATE:
         return read_dmem(sim, (x + load_imm(inst)) * 4, "STATE");
      // TODO: reverse MME_FERMI_OP_UNK6
      default:
         unreachable("Unhandled op");
   }
}

static void
set_mthd(struct mme_fermi_sim *sim, uint32_t val)
{
   sim->mthd.mthd = (val & 0xfff) << 2;
   sim->mthd.inc = (val >> 12) & 0xf;
   sim->mthd.has_mthd = true;
}

static void
emit_mthd(struct mme_fermi_sim *sim, uint32_t val)
{
   // TODO: understand what happens on hardware when no mthd has been set.
   if (!sim->mthd.has_mthd)
      return;

   uint16_t mthd = sim->mthd.mthd;

   write_dmem(sim, mthd, val, "EMIT");

   switch (mthd) {
   case NV9097_SET_REPORT_SEMAPHORE_D: {
      assert(val == 0x10000000);

      uint64_t addr = read_dmem64(sim, NV9097_SET_REPORT_SEMAPHORE_A, "SET_REPORT_SEMAPHORE");
      uint32_t data = read_dmem(sim, NV9097_SET_REPORT_SEMAPHORE_C, "SET_REPORT_SEMAPHORE");

      uint32_t *mem = find_mem(sim, addr, "SET_REPORT_SEMAPHORE");
      *mem = data;
      break;
   }
   case NV902D_SET_MME_DATA_RAM_ADDRESS:
      sim->ram.addr = val;
      break;
   case NV902D_SET_MME_MEM_ADDRESS_B:
      sim->mem_addr = read_dmem64(sim, NV902D_SET_MME_MEM_ADDRESS_A, "SET_MME_MEM_ADDRESS");
      break;
   case NV902D_MME_DMA_READ_FIFOED:
      sim->dma.read_fifo.count = val;
      break;
   default:
      break;
   }

   sim->mthd.mthd += sim->mthd.inc * 4;
}

static void
eval_inst(struct mme_fermi_sim *sim, const struct mme_fermi_inst *inst)
{
   if (inst->op == MME_FERMI_OP_BRANCH) {
      uint32_t val = load_reg(sim, inst->src[0]);
      bool cond = inst->branch.not_zero ? val != 0 : val == 0;

      if (cond) {
         int32_t offset = load_imm(inst);
         assert((int)sim->ip + offset >= 0);
         assert((int)sim->ip + offset < 0x1000);
         sim->next_ip = sim->ip + offset;
      }
   } else {
      uint32_t scratch = eval_op(sim, inst);
      switch (inst->assign_op) {
         case MME_FERMI_ASSIGN_OP_LOAD:
            store_reg(sim, inst->dst, load_param(sim));
            break;
         case MME_FERMI_ASSIGN_OP_MOVE:
            store_reg(sim, inst->dst, scratch);
            break;
         case MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR:
            store_reg(sim, inst->dst, scratch);
            set_mthd(sim, scratch);
            break;
         case MME_FERMI_ASSIGN_OP_LOAD_EMIT:
            store_reg(sim, inst->dst, load_param(sim));
            emit_mthd(sim, scratch);
            break;
         case MME_FERMI_ASSIGN_OP_MOVE_EMIT:
            store_reg(sim, inst->dst, scratch);
            emit_mthd(sim, scratch);
            break;
         case MME_FERMI_ASSIGN_OP_LOAD_SET_MADDR:
            store_reg(sim, inst->dst, scratch);
            set_mthd(sim, scratch);
            break;
         case MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR_LOAD_EMIT:
            store_reg(sim, inst->dst, scratch);
            set_mthd(sim, scratch);
            emit_mthd(sim, load_param(sim));
            break;
         case MME_FERMI_ASSIGN_OP_MOVE_SET_MADDR_LOAD_EMIT_HIGH:
            store_reg(sim, inst->dst, scratch);
            set_mthd(sim, scratch);
            emit_mthd(sim, (scratch >> 12) & 0x3f);
            break;
         default:
            unreachable("Unhandled ASSIGN op");
      }
   }
}

void mme_fermi_sim(uint32_t inst_count, const struct mme_fermi_inst *insts,
                   uint32_t param_count, const uint32_t *params,
                   uint32_t mem_count, struct mme_fermi_sim_mem *mems)
{
   struct mme_fermi_sim sim = {
      .param_count = param_count,
      .params = params,
      .mem_count = mem_count,
      .mems = mems,
   };

   sim.ip = 0;
   /* First preload first argument in R1*/
   store_reg(&sim, MME_FERMI_REG_R1, load_param(&sim));

   bool end_next = false;
   bool ignore_next_exit = false;
   bool should_delay_branch = false;

   while (!end_next) {
      assert(sim.ip < inst_count);
      const struct mme_fermi_inst *inst = &insts[sim.ip];

      if (!should_delay_branch) {
         sim.next_ip = sim.ip + 1;
      }

      eval_inst(&sim, inst);

      should_delay_branch = inst->op == MME_FERMI_OP_BRANCH && !inst->branch.no_delay;

      if (should_delay_branch) {
         sim.ip = sim.ip + 1;
      } else {
         sim.ip = sim.next_ip;
      }

      if (inst->end_next && should_delay_branch) {
         ignore_next_exit = true;
         continue;
      }

      end_next = inst->end_next && !ignore_next_exit;
      ignore_next_exit = false;
   }

   // Handle delay slot at exit
   assert(sim.ip < inst_count);
   eval_inst(&sim, &insts[sim.ip]);
}
