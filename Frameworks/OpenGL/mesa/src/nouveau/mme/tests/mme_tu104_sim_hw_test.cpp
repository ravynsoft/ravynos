/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include "mme_runner.h"
#include "mme_tu104_sim.h"

#include "nvk_clc597.h"

class mme_tu104_sim_test : public ::testing::Test, public mme_hw_runner {
public:
   mme_tu104_sim_test();
   ~mme_tu104_sim_test();

   void SetUp();
   void test_macro(const mme_builder *b,
                   const std::vector<uint32_t>& macro,
                   const std::vector<uint32_t>& params);
};

mme_tu104_sim_test::mme_tu104_sim_test() :
   ::testing::Test(),
   mme_hw_runner()
{ }

mme_tu104_sim_test::~mme_tu104_sim_test()
{ }

void
mme_tu104_sim_test::SetUp()
{
   ASSERT_TRUE(set_up_hw(TURING_A, UINT16_MAX));
}

void
mme_tu104_sim_test::test_macro(const mme_builder *b,
                               const std::vector<uint32_t>& macro,
                               const std::vector<uint32_t>& params)
{
   const uint32_t data_dwords = DATA_BO_SIZE / sizeof(uint32_t);

   std::vector<mme_tu104_inst> insts(macro.size() / 3);
   mme_tu104_decode(&insts[0], &macro[0], macro.size() / 3);

   /* First, make a copy of the data and simulate the macro */
   std::vector<uint32_t> sim_data(data, data + (DATA_BO_SIZE / 4));
   mme_tu104_sim_mem sim_mem = {
      .addr = data_addr,
      .data = &sim_data[0],
      .size = DATA_BO_SIZE,
   };
   mme_tu104_sim(insts.size(), &insts[0],
                 params.size(), &params[0],
                 1, &sim_mem);

   /* Now run the macro on the GPU */
   push_macro(0, macro);

   P_1INC(p, NVC597, CALL_MME_MACRO(0));
   if (params.empty()) {
      P_NVC597_CALL_MME_MACRO(p, 0, 0);
   } else {
      P_INLINE_ARRAY(p, &params[0], params.size());
   }

   submit_push();

   /* Check the results */
   for (uint32_t i = 0; i < data_dwords; i++)
      ASSERT_EQ(data[i], sim_data[i]);
}

static mme_tu104_reg
mme_value_as_reg(mme_value val)
{
   assert(val.type == MME_VALUE_TYPE_REG);
   return (mme_tu104_reg)(MME_TU104_REG_R0 + val.reg);
}

TEST_F(mme_tu104_sim_test, sanity)
{
   const uint32_t canary = 0xc0ffee01;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_store_imm_addr(&b, data_addr, mme_imm(canary));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, multi_param)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value v0 = mme_alloc_reg(&b);
   mme_value v1 = mme_alloc_reg(&b);

   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v0);
      i.alu[0].src[0] = MME_TU104_REG_LOAD1;
      i.alu[1].dst = mme_value_as_reg(v1);
      i.alu[1].src[0] = MME_TU104_REG_LOAD0;
      i.imm[0] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(12) >> 2);
      i.out[0].mthd = MME_TU104_OUT_OP_IMM0;
      i.out[0].emit = MME_TU104_OUT_OP_LOAD0;
      i.imm[1] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(35) >> 2);
      i.out[1].mthd = MME_TU104_OUT_OP_IMM1;
      i.out[1].emit = MME_TU104_OUT_OP_LOAD1;
   }

   mme_value v2 = mme_state(&b, NVC597_SET_MME_SHADOW_SCRATCH(12));
   mme_value v3 = mme_state(&b, NVC597_SET_MME_SHADOW_SCRATCH(35));

   mme_store_imm_addr(&b, data_addr + 0, v0);
   mme_store_imm_addr(&b, data_addr + 4, v1);
   mme_store_imm_addr(&b, data_addr + 8, v2);
   mme_store_imm_addr(&b, data_addr + 12, v3);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(2581);
   params.push_back(3048);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, pred_param)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value v0 = mme_load(&b);
   mme_value v1 = mme_mov(&b, mme_imm(240));

   mme_tu104_asm(&b, i) {
      i.pred_mode = MME_TU104_PRED_TTTT;
      i.alu[0].dst = mme_value_as_reg(v1);
      i.alu[0].src[0] = MME_TU104_REG_LOAD0;
   }

   mme_value v2 = mme_load(&b);

   mme_store_imm_addr(&b, data_addr + 0, v0);
   mme_store_imm_addr(&b, data_addr + 4, v1);
   mme_store_imm_addr(&b, data_addr + 8, v2);

   auto macro = mme_builder_finish_vec(&b);

   for (uint32_t j = 0; j < 4; j++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back((j & 1) * 2043);
      params.push_back((j & 2) * 523);
      params.push_back(2581);
      params.push_back(3048);

      test_macro(&b, macro, params);
   }
}

TEST_F(mme_tu104_sim_test, out_imm0)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 0)));
   mme_emit(&b, mme_imm(low32(data_addr + 0)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x1234;
      i.out[0].emit = MME_TU104_OUT_OP_IMM0;
   }
   mme_emit(&b, mme_imm(0x10000000));

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 4)));
   mme_emit(&b, mme_imm(low32(data_addr + 4)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x8765;
      i.out[0].emit = MME_TU104_OUT_OP_IMM0;
   }
   mme_emit(&b, mme_imm(0x10000000));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, out_imm1)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 0)));
   mme_emit(&b, mme_imm(low32(data_addr + 0)));
   mme_tu104_asm(&b, i) {
      i.imm[1] = 0x1234;
      i.out[0].emit = MME_TU104_OUT_OP_IMM1;
   }
   mme_emit(&b, mme_imm(0x10000000));

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 4)));
   mme_emit(&b, mme_imm(low32(data_addr + 4)));
   mme_tu104_asm(&b, i) {
      i.imm[1] = 0x8765;
      i.out[0].emit = MME_TU104_OUT_OP_IMM1;
   }
   mme_emit(&b, mme_imm(0x10000000));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, out_immhigh0)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 0)));
   mme_emit(&b, mme_imm(low32(data_addr + 0)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x1234;
      i.out[0].emit = MME_TU104_OUT_OP_IMMHIGH0;
   }
   mme_emit(&b, mme_imm(0x10000000));

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 4)));
   mme_emit(&b, mme_imm(low32(data_addr + 4)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x8765;
      i.out[1].emit = MME_TU104_OUT_OP_IMMHIGH0;
   }
   mme_emit(&b, mme_imm(0x10000000));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, out_immhigh1)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 0)));
   mme_emit(&b, mme_imm(low32(data_addr + 0)));
   mme_tu104_asm(&b, i) {
      i.imm[1] = 0x1234;
      i.out[0].emit = MME_TU104_OUT_OP_IMMHIGH1;
   }
   mme_emit(&b, mme_imm(0x10000000));

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 4)));
   mme_emit(&b, mme_imm(low32(data_addr + 4)));
   mme_tu104_asm(&b, i) {
      i.imm[1] = 0x8765;
      i.out[1].emit = MME_TU104_OUT_OP_IMMHIGH1;
   }
   mme_emit(&b, mme_imm(0x10000000));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, out_imm32)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 0)));
   mme_emit(&b, mme_imm(low32(data_addr + 0)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x1234;
      i.imm[1] = 0x7654;
      i.out[0].emit = MME_TU104_OUT_OP_IMM32;
   }
   mme_emit(&b, mme_imm(0x10000000));

   mme_mthd(&b, NVC597_SET_REPORT_SEMAPHORE_A);
   mme_emit(&b, mme_imm(high32(data_addr + 4)));
   mme_emit(&b, mme_imm(low32(data_addr + 4)));
   mme_tu104_asm(&b, i) {
      i.imm[0] = 0x1234;
      i.imm[1] = 0x7654;
      i.out[1].emit = MME_TU104_OUT_OP_IMM32;
   }
   mme_emit(&b, mme_imm(0x10000000));

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, reg_imm32)
{
   const uint32_t canary = 0xc0ffee01;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value v = mme_alloc_reg(&b);

   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = MME_TU104_REG_IMM32,
      i.imm[0] = (uint16_t)canary;
      i.imm[1] = (uint16_t)(canary >> 16);
   }

   mme_store_imm_addr(&b, data_addr, v);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, pred_alu)
{
   static const enum mme_tu104_pred preds[] = {
      MME_TU104_PRED_UUUU,
      MME_TU104_PRED_TTTT,
      MME_TU104_PRED_FFFF,
      MME_TU104_PRED_TTUU,
      MME_TU104_PRED_FFUU,
      MME_TU104_PRED_TFUU,
      MME_TU104_PRED_TUUU,
      MME_TU104_PRED_FUUU,
      MME_TU104_PRED_UUTT,
      MME_TU104_PRED_UUTF,
      MME_TU104_PRED_UUTU,
      MME_TU104_PRED_UUFT,
      MME_TU104_PRED_UUFF,
      MME_TU104_PRED_UUFU,
      MME_TU104_PRED_UUUT,
      MME_TU104_PRED_UUUF,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(preds); i++) {
      mme_builder b;
      mme_builder_init(&b, devinfo);

      mme_value pred = mme_load(&b);
      mme_value v0 = mme_mov(&b, mme_imm(i * 100 + 13));
      mme_value v1 = mme_mov(&b, mme_imm(i * 100 + 62));

      mme_tu104_asm(&b, inst) {
         inst.pred = mme_value_as_reg(pred);
         inst.pred_mode = preds[i];
         inst.alu[0].dst = mme_value_as_reg(v0);
         inst.alu[0].src[0] = MME_TU104_REG_IMM;
         inst.imm[0] = i * 100 + 25;
         inst.alu[1].dst = mme_value_as_reg(v1);
         inst.alu[1].src[0] = MME_TU104_REG_IMM;
         inst.imm[1] = i * 100 + 73;
      }

      mme_store_imm_addr(&b, data_addr + i * 8 + 0, v0);
      mme_store_imm_addr(&b, data_addr + i * 8 + 4, v1);

      auto macro = mme_builder_finish_vec(&b);

      for (uint32_t j = 0; j < 2; j++) {
         reset_push();

         std::vector<uint32_t> params;
         params.push_back(j * 25894);

         test_macro(&b, macro, params);
      }
   }
}

TEST_F(mme_tu104_sim_test, pred_out)
{
   static const enum mme_tu104_pred preds[] = {
      MME_TU104_PRED_UUUU,
      MME_TU104_PRED_TTTT,
      MME_TU104_PRED_FFFF,
      MME_TU104_PRED_TTUU,
      MME_TU104_PRED_FFUU,
      MME_TU104_PRED_TFUU,
      MME_TU104_PRED_TUUU,
      MME_TU104_PRED_FUUU,
      MME_TU104_PRED_UUTT,
      MME_TU104_PRED_UUTF,
      MME_TU104_PRED_UUTU,
      MME_TU104_PRED_UUFT,
      MME_TU104_PRED_UUFF,
      MME_TU104_PRED_UUFU,
      MME_TU104_PRED_UUUT,
      MME_TU104_PRED_UUUF,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(preds); i++) {
      mme_builder b;
      mme_builder_init(&b, devinfo);

      mme_value pred = mme_load(&b);

      mme_tu104_asm(&b, inst) {
         inst.imm[0] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 0) >> 2);
         inst.imm[1] = i * 100 + 25;
         inst.out[0].mthd = MME_TU104_OUT_OP_IMM0;
         inst.out[0].emit = MME_TU104_OUT_OP_IMM1;
      }

      mme_tu104_asm(&b, inst) {
         inst.imm[0] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 1) >> 2);
         inst.imm[1] = i * 100 + 75;
         inst.out[0].mthd = MME_TU104_OUT_OP_IMM0;
         inst.out[0].emit = MME_TU104_OUT_OP_IMM1;
      }

      mme_tu104_asm(&b, inst) {
         inst.pred = mme_value_as_reg(pred);
         inst.pred_mode = preds[i];
         inst.imm[0] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 0) >> 2);
         inst.imm[1] = (1<<12) | (NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 1) >> 2);
         inst.out[0].mthd = MME_TU104_OUT_OP_IMM0;
         inst.out[0].emit = MME_TU104_OUT_OP_IMM1;
         inst.out[1].mthd = MME_TU104_OUT_OP_IMM1;
         inst.out[1].emit = MME_TU104_OUT_OP_IMM0;
      }

      mme_value v0 = mme_state(&b, NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 0));
      mme_value v1 = mme_state(&b, NVC597_SET_MME_SHADOW_SCRATCH(i*2 + 1));

      mme_store_imm_addr(&b, data_addr + i * 8 + 0, v0);
      mme_store_imm_addr(&b, data_addr + i * 8 + 4, v1);

      auto macro = mme_builder_finish_vec(&b);

      for (uint32_t j = 0; j < 2; j++) {
         reset_push();

         std::vector<uint32_t> params;
         params.push_back(j * 25894);

         test_macro(&b, macro, params);
      }
   }
}

TEST_F(mme_tu104_sim_test, add)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);
   mme_value sum = mme_add(&b, x, y);
   mme_store_imm_addr(&b, data_addr, sum);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(25);
   params.push_back(138);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, add_imm)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);

   mme_value v0 = mme_add(&b, x, mme_imm(0x00000001));
   mme_value v1 = mme_add(&b, x, mme_imm(0xffffffff));
   mme_value v2 = mme_add(&b, x, mme_imm(0xffff8000));
   mme_value v3 = mme_add(&b, mme_imm(0x00000001), x);
   mme_value v4 = mme_add(&b, mme_imm(0xffffffff), x);
   mme_value v5 = mme_add(&b, mme_imm(0xffff8000), x);
   mme_value v6 = mme_add(&b, mme_zero(), mme_imm(0x00000001));
   mme_value v7 = mme_add(&b, mme_zero(), mme_imm(0xffffffff));
   mme_value v8 = mme_add(&b, mme_zero(), mme_imm(0xffff8000));

   mme_store_imm_addr(&b, data_addr + 0,  v0);
   mme_store_imm_addr(&b, data_addr + 4,  v1);
   mme_store_imm_addr(&b, data_addr + 8,  v2);
   mme_store_imm_addr(&b, data_addr + 12, v3);
   mme_store_imm_addr(&b, data_addr + 16, v4);
   mme_store_imm_addr(&b, data_addr + 20, v5);
   mme_store_imm_addr(&b, data_addr + 24, v6);
   mme_store_imm_addr(&b, data_addr + 28, v7);
   mme_store_imm_addr(&b, data_addr + 32, v8);

   auto macro = mme_builder_finish_vec(&b);

   uint32_t vals[] = {
      0x0000ffff,
      0x00008000,
      0x0001ffff,
      0xffffffff,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(vals); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(vals[i]);

      test_macro(&b, macro, params);
   }
}

TEST_F(mme_tu104_sim_test, addc)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   struct mme_value64 x = { mme_load(&b), mme_load(&b) };
   struct mme_value64 y = { mme_load(&b), mme_load(&b) };

   struct mme_value64 sum = mme_add64(&b, x, y);

   mme_store_imm_addr(&b, data_addr + 0, sum.lo);
   mme_store_imm_addr(&b, data_addr + 4, sum.hi);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x80008650);
   params.push_back(0x596);
   params.push_back(0x8000a8f6);
   params.push_back(0x836);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, addc_imm)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x_lo = mme_load(&b);
   mme_value x_hi = mme_load(&b);

   mme_value v1_lo = mme_alloc_reg(&b);
   mme_value v1_hi = mme_alloc_reg(&b);
   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v1_lo);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = mme_value_as_reg(x_lo);
      i.alu[0].src[1] = MME_TU104_REG_IMM;
      i.imm[0] = 0x0001;
      i.alu[1].dst = mme_value_as_reg(v1_hi);
      i.alu[1].op = MME_TU104_ALU_OP_ADDC;
      i.alu[1].src[0] = mme_value_as_reg(x_hi);
      i.alu[1].src[1] = MME_TU104_REG_IMM;
      i.imm[1] = 0x0000;
   }

   mme_value v2_lo = mme_alloc_reg(&b);
   mme_value v2_hi = mme_alloc_reg(&b);
   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v2_lo);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = mme_value_as_reg(x_lo);
      i.alu[0].src[1] = MME_TU104_REG_IMM;
      i.imm[0] = 0x0000;
      i.alu[1].dst = mme_value_as_reg(v2_hi);
      i.alu[1].op = MME_TU104_ALU_OP_ADDC;
      i.alu[1].src[0] = mme_value_as_reg(x_hi);
      i.alu[1].src[1] = MME_TU104_REG_IMM;
      i.imm[1] = 0x0001;
   }

   mme_value v3_lo = mme_alloc_reg(&b);
   mme_value v3_hi = mme_alloc_reg(&b);
   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v3_lo);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = mme_value_as_reg(x_lo);
      i.alu[0].src[1] = MME_TU104_REG_IMM;
      i.imm[0] = 0x0000;
      i.alu[1].dst = mme_value_as_reg(v3_hi);
      i.alu[1].op = MME_TU104_ALU_OP_ADDC;
      i.alu[1].src[0] = mme_value_as_reg(x_hi);
      i.alu[1].src[1] = MME_TU104_REG_IMM;
      i.imm[1] = 0xffff;
   }

   mme_value v4_lo = mme_alloc_reg(&b);
   mme_value v4_hi = mme_alloc_reg(&b);
   mme_tu104_asm(&b, i) {
      i.alu[0].dst = mme_value_as_reg(v4_lo);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = mme_value_as_reg(x_lo);
      i.alu[0].src[1] = MME_TU104_REG_IMM;
      i.imm[0] = 0x0000;
      i.alu[1].dst = mme_value_as_reg(v4_hi);
      i.alu[1].op = MME_TU104_ALU_OP_ADDC;
      i.alu[1].src[0] = mme_value_as_reg(x_hi);
      i.alu[1].src[1] = MME_TU104_REG_IMM;
      i.imm[1] = 0x8000;
   }

   mme_store_imm_addr(&b, data_addr + 0,  v1_lo);
   mme_store_imm_addr(&b, data_addr + 4,  v1_hi);
   mme_store_imm_addr(&b, data_addr + 8,  v2_lo);
   mme_store_imm_addr(&b, data_addr + 12, v2_hi);
   mme_store_imm_addr(&b, data_addr + 16, v3_lo);
   mme_store_imm_addr(&b, data_addr + 20, v3_hi);
   mme_store_imm_addr(&b, data_addr + 24, v4_lo);
   mme_store_imm_addr(&b, data_addr + 28, v4_hi);

   auto macro = mme_builder_finish_vec(&b);

   uint64_t vals[] = {
      0x0000ffffffffffffull,
      0x0000ffffffff8000ull,
      0x0000ffff00000000ull,
      0x0000800000000000ull,
      0x00008000ffffffffull,
      0x0001ffff00000000ull,
      0xffffffff00000000ull,
      0xffffffffffffffffull,
   };

   for (uint32_t i = 0; i < ARRAY_SIZE(vals); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(low32(vals[i]));
      params.push_back(high32(vals[i]));

      test_macro(&b, macro, params);
   }
}

TEST_F(mme_tu104_sim_test, sub)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);
   mme_value diff = mme_sub(&b, x, y);
   mme_store_imm_addr(&b, data_addr, diff);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(25);
   params.push_back(138);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, subb)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   struct mme_value64 x = { mme_load(&b), mme_load(&b) };
   struct mme_value64 y = { mme_load(&b), mme_load(&b) };

   struct mme_value64 diff = mme_sub64(&b, x, y);

   mme_store_imm_addr(&b, data_addr + 0, diff.lo);
   mme_store_imm_addr(&b, data_addr + 4, diff.hi);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x80008650);
   params.push_back(0x596);
   params.push_back(0x8000a8f6);
   params.push_back(0x836);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, mul)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);
   mme_value sum = mme_mul(&b, x, y);
   mme_store_imm_addr(&b, data_addr, sum);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(25);
   params.push_back(138);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, mul_imm)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);

   mme_value v0 = mme_mul(&b, x, mme_imm(0x00000001));
   mme_value v1 = mme_mul(&b, x, mme_imm(0xffffffff));
   mme_value v2 = mme_mul(&b, x, mme_imm(0xffff8000));
   mme_value v3 = mme_mul(&b, mme_imm(0x00000001), x);
   mme_value v4 = mme_mul(&b, mme_imm(0xffffffff), x);
   mme_value v5 = mme_mul(&b, mme_imm(0xffff8000), x);

   mme_store_imm_addr(&b, data_addr + 0,  v0);
   mme_store_imm_addr(&b, data_addr + 4,  v1);
   mme_store_imm_addr(&b, data_addr + 8,  v2);
   mme_store_imm_addr(&b, data_addr + 12, v3);
   mme_store_imm_addr(&b, data_addr + 16, v4);
   mme_store_imm_addr(&b, data_addr + 20, v5);

   auto macro = mme_builder_finish_vec(&b);

   int32_t vals[] = { 1, -5, -1, 5 };

   for (uint32_t i = 0; i < ARRAY_SIZE(vals); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(vals[i]);

      test_macro(&b, macro, params);
   }
}

TEST_F(mme_tu104_sim_test, mul_mulh)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   struct mme_value64 prod = mme_imul_32x32_64(&b, x, y);

   mme_store_imm_addr(&b, data_addr + 0, prod.lo);
   mme_store_imm_addr(&b, data_addr + 4, prod.hi);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x80008650);
   params.push_back(0x596);

   test_macro(&b, macro, params);
}

static inline struct mme_value
mme_mulu(struct mme_builder *b, struct mme_value x, struct mme_value y)
{
   return mme_alu(b, MME_ALU_OP_MULU, x, y);
}

TEST_F(mme_tu104_sim_test, mulu_imm)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);

   mme_value v0 = mme_mulu(&b, x, mme_imm(0x00000001));
   mme_value v1 = mme_mulu(&b, x, mme_imm(0xffffffff));
   mme_value v2 = mme_mulu(&b, x, mme_imm(0xffff8000));
   mme_value v3 = mme_mulu(&b, mme_imm(0x00000001), x);
   mme_value v4 = mme_mulu(&b, mme_imm(0xffffffff), x);
   mme_value v5 = mme_mulu(&b, mme_imm(0xffff8000), x);

   mme_store_imm_addr(&b, data_addr + 0,  v0);
   mme_store_imm_addr(&b, data_addr + 4,  v1);
   mme_store_imm_addr(&b, data_addr + 8,  v2);
   mme_store_imm_addr(&b, data_addr + 12, v3);
   mme_store_imm_addr(&b, data_addr + 16, v4);
   mme_store_imm_addr(&b, data_addr + 20, v5);

   auto macro = mme_builder_finish_vec(&b);

   int32_t vals[] = { 1, -5, -1, 5 };

   for (uint32_t i = 0; i < ARRAY_SIZE(vals); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(vals[i]);

      test_macro(&b, macro, params);
   }
}

TEST_F(mme_tu104_sim_test, mulu_mulh)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   struct mme_value64 prod = mme_umul_32x32_64(&b, x, y);

   mme_store_imm_addr(&b, data_addr + 0, prod.lo);
   mme_store_imm_addr(&b, data_addr + 4, prod.hi);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x80008650);
   params.push_back(0x596);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, clz)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value bits = mme_clz(&b, mme_load(&b));
   mme_store_imm_addr(&b, data_addr, bits);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x00406fe0);

   test_macro(&b, macro, params);
}

#define SHIFT_TEST(op)                                               \
TEST_F(mme_tu104_sim_test, op)                                       \
{                                                                    \
   mme_builder b;                                                    \
   mme_builder_init(&b, devinfo);                                 \
                                                                     \
   mme_value val = mme_load(&b);                                     \
   mme_value shift1 = mme_load(&b);                                  \
   mme_value shift2 = mme_load(&b);                                  \
   mme_store_imm_addr(&b, data_addr + 0, mme_##op(&b, val, shift1)); \
   mme_store_imm_addr(&b, data_addr + 4, mme_##op(&b, val, shift2)); \
                                                                     \
   auto macro = mme_builder_finish_vec(&b);                          \
                                                                     \
   std::vector<uint32_t> params;                                     \
   params.push_back(0x0c406fe0);                                     \
   params.push_back(5);                                              \
   params.push_back(51);                                             \
                                                                     \
   test_macro(&b, macro, params);                                    \
}

SHIFT_TEST(sll)
SHIFT_TEST(srl)
SHIFT_TEST(sra)

#undef SHIFT_TEST

TEST_F(mme_tu104_sim_test, bfe)
{
   const uint32_t canary = 0xc0ffee01;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value val = mme_load(&b);
   mme_value pos = mme_load(&b);

   mme_store_imm_addr(&b, data_addr + 0, mme_bfe(&b, val, pos, 1), true);
   mme_store_imm_addr(&b, data_addr + 4, mme_bfe(&b, val, pos, 2), true);
   mme_store_imm_addr(&b, data_addr + 8, mme_bfe(&b, val, pos, 5), true);

   auto macro = mme_builder_finish_vec(&b);

   for (unsigned i = 0; i < 31; i++) {
      std::vector<uint32_t> params;
      params.push_back(canary);
      params.push_back(i);

      test_macro(&b, macro, params);

      ASSERT_EQ(data[0], (canary >> i) & 0x1);
      ASSERT_EQ(data[1], (canary >> i) & 0x3);
      ASSERT_EQ(data[2], (canary >> i) & 0x1f);
   }
}

#define BITOP_TEST(op)                                               \
TEST_F(mme_tu104_sim_test, op)                                       \
{                                                                    \
   mme_builder b;                                                    \
   mme_builder_init(&b, devinfo);                                 \
                                                                     \
   mme_value x = mme_load(&b);                                       \
   mme_value y = mme_load(&b);                                       \
   mme_value v1 = mme_##op(&b, x, y);                                \
   mme_value v2 = mme_##op(&b, x, mme_imm(0xffff8000));              \
   mme_value v3 = mme_##op(&b, x, mme_imm(0xffffffff));              \
   mme_store_imm_addr(&b, data_addr + 0, v1);                        \
   mme_store_imm_addr(&b, data_addr + 4, v2);                        \
   mme_store_imm_addr(&b, data_addr + 8, v3);                        \
                                                                     \
   auto macro = mme_builder_finish_vec(&b);                          \
                                                                     \
   std::vector<uint32_t> params;                                     \
   params.push_back(0x0c406fe0);                                     \
   params.push_back(0x00fff0c0);                                     \
                                                                     \
   test_macro(&b, macro, params);                                    \
}

BITOP_TEST(and)
BITOP_TEST(nand)
BITOP_TEST(or)
BITOP_TEST(xor)

#undef BITOP_TEST

TEST_F(mme_tu104_sim_test, merge)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   mme_value m1 = mme_merge(&b, x, y, 12, 12, 20);
   mme_value m2 = mme_merge(&b, x, y, 12, 8,  20);
   mme_value m3 = mme_merge(&b, x, y, 8,  12, 20);
   mme_value m4 = mme_merge(&b, x, y, 12, 16, 8);
   mme_value m5 = mme_merge(&b, x, y, 24, 12, 8);

   mme_store_imm_addr(&b, data_addr + 0,  m1);
   mme_store_imm_addr(&b, data_addr + 4,  m2);
   mme_store_imm_addr(&b, data_addr + 8,  m3);
   mme_store_imm_addr(&b, data_addr + 12, m4);
   mme_store_imm_addr(&b, data_addr + 16, m5);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(0x0c406fe0);
   params.push_back(0x76543210u);

   test_macro(&b, macro, params);
}

#define COMPARISON_TEST(op)                     \
TEST_F(mme_tu104_sim_test, op)                  \
{                                               \
   mme_builder b;                               \
   mme_builder_init(&b, devinfo);            \
                                                \
   mme_value x = mme_load(&b);                  \
   mme_value y = mme_load(&b);                  \
   mme_value z = mme_load(&b);                  \
   mme_value w = mme_load(&b);                  \
                                                \
   mme_value v1 = mme_##op(&b, x, y);           \
   mme_value v2 = mme_##op(&b, y, x);           \
   mme_value v3 = mme_##op(&b, y, z);           \
   mme_value v4 = mme_##op(&b, z, y);           \
   mme_value v5 = mme_##op(&b, w, z);           \
   mme_value v6 = mme_##op(&b, z, w);           \
   mme_value v7 = mme_##op(&b, w, w);           \
                                                \
   mme_store_imm_addr(&b, data_addr + 0,  v1);  \
   mme_store_imm_addr(&b, data_addr + 4,  v2);  \
   mme_store_imm_addr(&b, data_addr + 8,  v3);  \
   mme_store_imm_addr(&b, data_addr + 12, v4);  \
   mme_store_imm_addr(&b, data_addr + 16, v5);  \
   mme_store_imm_addr(&b, data_addr + 20, v6);  \
   mme_store_imm_addr(&b, data_addr + 24, v7);  \
                                                \
   auto macro = mme_builder_finish_vec(&b);     \
                                                \
   std::vector<uint32_t> params;                \
   params.push_back(-5);                        \
   params.push_back(-10);                       \
   params.push_back(5);                         \
   params.push_back(10);                        \
                                                \
   test_macro(&b, macro, params);               \
}

COMPARISON_TEST(slt)
COMPARISON_TEST(sltu)
COMPARISON_TEST(sle)
COMPARISON_TEST(sleu)
COMPARISON_TEST(seq)

#undef COMPARISON_TEST

static inline void
mme_inc_whole_inst(mme_builder *b, mme_value val)
{
   mme_tu104_asm(b, i) {
      i.alu[0].dst = mme_value_as_reg(val);
      i.alu[0].op = MME_TU104_ALU_OP_ADD;
      i.alu[0].src[0] = mme_value_as_reg(val);
      i.alu[0].src[1] = MME_TU104_REG_IMM;
      i.imm[0] = 1;
   }
}

TEST_F(mme_tu104_sim_test, loop)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value count = mme_load(&b);

   mme_value x = mme_mov(&b, mme_zero());
   mme_value y = mme_mov(&b, mme_zero());

   mme_loop(&b, count) {
      mme_tu104_asm(&b, i) { } /* noop */
      mme_add_to(&b, x, x, count);
   }
   mme_add_to(&b, y, y, mme_imm(1));
   mme_tu104_asm(&b, i) { } /* noop */
   mme_tu104_asm(&b, i) { } /* noop */
   mme_tu104_asm(&b, i) { } /* noop */

   mme_store_imm_addr(&b, data_addr + 0,  count);
   mme_store_imm_addr(&b, data_addr + 4,  x);
   mme_store_imm_addr(&b, data_addr + 8,  y);

   auto macro = mme_builder_finish_vec(&b);

   uint32_t counts[] = {0, 1, 5, 9};

   for (uint32_t i = 0; i < ARRAY_SIZE(counts); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(counts[i]);

      test_macro(&b, macro, params);
      ASSERT_EQ(data[0], counts[i]);
      ASSERT_EQ(data[1], counts[i] * counts[i]);
      ASSERT_EQ(data[2], 1);
   }
}

TEST_F(mme_tu104_sim_test, jal)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_mov(&b, mme_zero());
   mme_value y = mme_mov(&b, mme_zero());

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_JAL;
      i.imm[0] = (1 << 15) | 6;
   }

   for (uint32_t j = 0; j < 10; j++)
      mme_inc_whole_inst(&b, x);

//   mme_tu104_asm(&b, i) {
//      i.alu[0].op = MME_TU104_ALU_OP_JAL;
//      i.imm[0] = 6;
//   }
//
//   for (uint32_t j = 0; j < 10; j++)
//      mme_inc_whole_inst(&b, y);

   mme_store_imm_addr(&b, data_addr + 0, x);
   mme_store_imm_addr(&b, data_addr + 4, y);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
   ASSERT_EQ(data[0], 5);
}

TEST_F(mme_tu104_sim_test, bxx_fwd)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value vals[10];
   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_zero());

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_BEQ;
      i.imm[0] = (1 << 15) | 6;
   }

   for (uint32_t j = 0; j < 10; j++)
      mme_inc_whole_inst(&b, vals[j]);

   for (uint32_t j = 0; j < 10; j++)
      mme_store_imm_addr(&b, data_addr + j * 4, vals[j]);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, bxx_bwd)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value vals[15];
   for (uint32_t i = 0; i < 15; i++)
      vals[i] = mme_mov(&b, mme_zero());

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_JAL;
      i.imm[0] = (1 << 15) | 12;
   }

   for (uint32_t j = 0; j < 10; j++)
      mme_inc_whole_inst(&b, vals[j]);

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_JAL;
      i.imm[0] = (1 << 15) | 2;
   }

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_BEQ;
      i.imm[0] = (1 << 15) | ((-8) & 0x1fff);
   }

   for (uint32_t j = 10; j < 15; j++)
      mme_inc_whole_inst(&b, vals[j]);

   for (uint32_t j = 0; j < 15; j++)
      mme_store_imm_addr(&b, data_addr + j * 4, vals[j]);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   test_macro(&b, macro, params);
   for (uint32_t j = 0; j < 3; j++)
      ASSERT_EQ(data[j], 0);
   for (uint32_t j = 3; j < 15; j++)
      ASSERT_EQ(data[j], 1);
}

TEST_F(mme_tu104_sim_test, bxx_exit)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value vals[10];
   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_zero());

   for (uint32_t i = 0; i < 10; i++)
      mme_store_imm_addr(&b, data_addr + i * 4, mme_imm(0));

   mme_tu104_asm(&b, i) {
      i.alu[0].op = MME_TU104_ALU_OP_BEQ;
      i.imm[0] = (1 << 15) | 0x1000;
   }

   /* those writes won't be visible */
   for (uint32_t j = 0; j < 10; j++)
      mme_inc_whole_inst(&b, vals[j]);

   for (uint32_t i = 0; i < 10; i++)
      mme_store_imm_addr(&b, data_addr + i * 4, vals[i]);

   std::vector<uint32_t> params;

   auto macro = mme_builder_finish_vec(&b);
   test_macro(&b, macro, params);

   uint32_t i;
   for (i = 0; i < 10; i++)
      ASSERT_EQ(data[i], 0);
}

TEST_F(mme_tu104_sim_test, mme_exit)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value vals[10];
   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_zero());

   for (uint32_t i = 0; i < 10; i++)
      mme_store_imm_addr(&b, data_addr + i * 4, mme_imm(0));

   /* abort */
   mme_exit(&b);

   /* those writes won't be visible */
   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_imm(i));

   for (uint32_t i = 0; i < 10; i++) {
      mme_store_imm_addr(&b, data_addr + i * 4, vals[i]);
   }

   std::vector<uint32_t> params;

   auto macro = mme_builder_finish_vec(&b);
   test_macro(&b, macro, params);

   uint32_t i;
   for (i = 0; i < 10; i++)
      ASSERT_EQ(data[i], 0);
}

TEST_F(mme_tu104_sim_test, mme_exit_if)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value vals[10];
   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_zero());

   for (uint32_t i = 0; i < 10; i++)
      mme_store_imm_addr(&b, data_addr + i * 4, mme_imm(0));

   /* shouldn't do anything */
   mme_exit_if(&b, ieq, mme_zero(), mme_imm(1));

   for (uint32_t i = 0; i < 10; i++)
      vals[i] = mme_mov(&b, mme_imm(i));

   for (uint32_t i = 0; i < 10; i++) {
      /* abort on reaching 5 */
      mme_exit_if(&b, ile, mme_imm(5), vals[i]);
      mme_store_imm_addr(&b, data_addr + i * 4, vals[i]);
   }

   std::vector<uint32_t> params;

   auto macro = mme_builder_finish_vec(&b);
   test_macro(&b, macro, params);

   uint32_t i;
   for (i = 0; i < 10; i++)
      ASSERT_EQ(data[i], i < 5 ? i : 0);
}

static bool c_ilt(int32_t x, int32_t y) { return x < y; };
static bool c_ult(uint32_t x, uint32_t y) { return x < y; };
static bool c_ile(int32_t x, int32_t y) { return x <= y; };
static bool c_ule(uint32_t x, uint32_t y) { return x <= y; };
static bool c_ieq(int32_t x, int32_t y) { return x == y; };
static bool c_ige(int32_t x, int32_t y) { return x >= y; };
static bool c_uge(uint32_t x, uint32_t y) { return x >= y; };
static bool c_igt(int32_t x, int32_t y) { return x > y; };
static bool c_ugt(uint32_t x, uint32_t y) { return x > y; };
static bool c_ine(int32_t x, int32_t y) { return x != y; };

#define IF_TEST(op)                                                  \
TEST_F(mme_tu104_sim_test, if_##op)                                  \
{                                                                    \
   mme_builder b;                                                    \
   mme_builder_init(&b, devinfo);                                 \
                                                                     \
   mme_value x = mme_load(&b);                                       \
   mme_value y = mme_load(&b);                                       \
   mme_value i = mme_mov(&b, mme_zero());                            \
                                                                     \
   mme_start_if_##op(&b, x, y);                                      \
   {                                                                 \
      mme_add_to(&b, i, i, mme_imm(1));                              \
      mme_add_to(&b, i, i, mme_imm(1));                              \
   }                                                                 \
   mme_end_if(&b);                                                   \
   mme_add_to(&b, i, i, mme_imm(1));                                 \
   mme_add_to(&b, i, i, mme_imm(1));                                 \
   mme_add_to(&b, i, i, mme_imm(1));                                 \
                                                                     \
   mme_store_imm_addr(&b, data_addr + 0, i);                         \
                                                                     \
   auto macro = mme_builder_finish_vec(&b);                          \
                                                                     \
   uint32_t vals[] = {23, 56, (uint32_t)-5, (uint32_t)-10, 56, 14};  \
                                                                     \
   for (uint32_t i = 0; i < ARRAY_SIZE(vals) - 1; i++) {             \
      reset_push();                                                  \
                                                                     \
      std::vector<uint32_t> params;                                  \
      params.push_back(vals[i + 0]);                                 \
      params.push_back(vals[i + 1]);                                 \
                                                                     \
      test_macro(&b, macro, params);                                 \
                                                                     \
      ASSERT_EQ(data[0], c_##op(params[0], params[1]) ? 5 : 3);      \
   }                                                                 \
}

IF_TEST(ilt)
IF_TEST(ult)
IF_TEST(ile)
IF_TEST(ule)
IF_TEST(ieq)
IF_TEST(ige)
IF_TEST(uge)
IF_TEST(igt)
IF_TEST(ugt)
IF_TEST(ine)

#undef IF_TEST

#define WHILE_TEST(op, start, step, bound)            \
TEST_F(mme_tu104_sim_test, while_##op)                \
{                                                     \
   mme_builder b;                                     \
   mme_builder_init(&b, devinfo);                  \
                                                      \
   mme_value x = mme_mov(&b, mme_zero());             \
   mme_value y = mme_mov(&b, mme_zero());             \
   mme_value z = mme_mov(&b, mme_imm(start));         \
   mme_value w = mme_mov(&b, mme_zero());             \
   mme_value v = mme_mov(&b, mme_zero());             \
                                                      \
   for (uint32_t j = 0; j < 5; j++)                   \
      mme_inc_whole_inst(&b, x);                      \
                                                      \
   mme_while(&b, op, z, mme_imm(bound)) {             \
      for (uint32_t j = 0; j < 5; j++)                \
         mme_inc_whole_inst(&b, y);                   \
                                                      \
      mme_add_to(&b, z, z, mme_imm(step));            \
                                                      \
      for (uint32_t j = 0; j < 5; j++)                \
         mme_inc_whole_inst(&b, w);                   \
   }                                                  \
                                                      \
   for (uint32_t j = 0; j < 5; j++)                   \
      mme_inc_whole_inst(&b, v);                      \
                                                      \
   mme_store_imm_addr(&b, data_addr + 0, x);          \
   mme_store_imm_addr(&b, data_addr + 4, y);          \
   mme_store_imm_addr(&b, data_addr + 8, z);          \
   mme_store_imm_addr(&b, data_addr + 12, w);         \
   mme_store_imm_addr(&b, data_addr + 16, v);         \
                                                      \
   auto macro = mme_builder_finish_vec(&b);           \
                                                      \
   uint32_t end = (uint32_t)(start), count = 0;       \
   while (c_##op(end, (bound))) {                     \
      end += (uint32_t)(step);                        \
      count++;                                        \
   }                                                  \
                                                      \
   std::vector<uint32_t> params;                      \
   test_macro(&b, macro, params);                     \
   ASSERT_EQ(data[0], 5);                             \
   ASSERT_EQ(data[1], 5 * count);                     \
   ASSERT_EQ(data[2], end);                           \
   ASSERT_EQ(data[3], 5 * count);                     \
   ASSERT_EQ(data[4], 5);                             \
}

WHILE_TEST(ilt, 0, 1, 7)
WHILE_TEST(ult, 0, 1, 7)
WHILE_TEST(ile, -10, 2, 0)
WHILE_TEST(ule, 0, 1, 7)
WHILE_TEST(ieq, 0, 5, 0)
WHILE_TEST(ige, 5, -1, -5)
WHILE_TEST(uge, 15, -2, 2)
WHILE_TEST(igt, 7, -3, -10)
WHILE_TEST(ugt, 1604, -30, 1000)
WHILE_TEST(ine, 0, 1, 7)

#undef WHILE_TEST

TEST_F(mme_tu104_sim_test, nested_while)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value n = mme_load(&b);
   mme_value m = mme_load(&b);

   mme_value count = mme_mov(&b, mme_zero());

   mme_value i = mme_mov(&b, mme_zero());
   mme_value j = mme_mov(&b, mme_imm(0xffff));
   mme_while(&b, ine, i, n) {
      mme_mov_to(&b, j, mme_zero());
      mme_while(&b, ine, j, m) {
         mme_add_to(&b, count, count, mme_imm(1));
         mme_add_to(&b, j, j, mme_imm(1));
      }

      mme_add_to(&b, i, i, mme_imm(1));
   }

   mme_store_imm_addr(&b, data_addr + 0, i);
   mme_store_imm_addr(&b, data_addr + 4, j);
   mme_store_imm_addr(&b, data_addr + 8, count);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(3);
   params.push_back(5);

   test_macro(&b, macro, params);
   ASSERT_EQ(data[0], 3);
   ASSERT_EQ(data[1], 5);
   ASSERT_EQ(data[2], 15);
}

#if 0
TEST_F(mme_tu104_sim_test, do_ble)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_alu(&b, R5, ADD, LOAD0, ZERO);
   mme_alu(&b, R6, ADD, ZERO, ZERO);
   mme_alu(&b, R7, ADD, ZERO, ZERO);

   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);
   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);
   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);
   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);
   mme_alu_imm(&b, R6, ADD, R6, IMM, 1);
   mme_branch(&b, BLE, R6, R5, -3, 2);
   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);
   mme_alu_imm(&b, R7, ADD, R7, IMM, 1);

   mme_store_imm_addr(&b, data_addr + 0,  MME_TU104_REG_R7);

   mme_end(&b);

   uint32_t counts[] = {0, 1, 5, 9};

   for (uint32_t i = 0; i < ARRAY_SIZE(counts); i++) {
      reset_push();

      std::vector<uint32_t> params;
      params.push_back(counts[i]);

      test_macro(&b, params);
   }
}
#endif

TEST_F(mme_tu104_sim_test, dread_dwrite)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   mme_dwrite(&b, mme_imm(5), x);
   mme_dwrite(&b, mme_imm(8), y);

   mme_value y2 = mme_dread(&b, mme_imm(8));
   mme_value x2 = mme_dread(&b, mme_imm(5));

   mme_store_imm_addr(&b, data_addr + 0, y2);
   mme_store_imm_addr(&b, data_addr + 4, x2);

   auto macro = mme_builder_finish_vec(&b);

   std::vector<uint32_t> params;
   params.push_back(-10);
   params.push_back(5);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, dwrite_dma)
{
   const uint32_t canary5 = 0xc0ffee01;
   const uint32_t canary8 = canary5 & 0x00ffff00;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   mme_dwrite(&b, mme_imm(5), x);
   mme_dwrite(&b, mme_imm(8), y);

   auto macro = mme_builder_finish_vec(&b);

   push_macro(0, macro);

   P_1INC(p, NVC597, CALL_MME_MACRO(0));
   P_INLINE_DATA(p, canary5);
   P_INLINE_DATA(p, canary8);

   P_MTHD(p, NVC597, SET_MME_MEM_ADDRESS_A);
   P_NVC597_SET_MME_MEM_ADDRESS_A(p, high32(data_addr));
   P_NVC597_SET_MME_MEM_ADDRESS_B(p, low32(data_addr));
   /* Start 3 dwords into MME RAM */
   P_NVC597_SET_MME_DATA_RAM_ADDRESS(p, 3);
   P_IMMD(p, NVC597, MME_DMA_WRITE, 20);

   submit_push();

   for (uint32_t i = 0; i < 20; i++) {
      if (i == 5 - 3) {
         ASSERT_EQ(data[i], canary5);
      } else if (i == 8 - 3) {
         ASSERT_EQ(data[i], canary8);
      } else {
         ASSERT_EQ(data[i], 0);
      }
   }
}

TEST_F(mme_tu104_sim_test, dram_limit)
{
   static const uint32_t chunk_size = 32;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value start = mme_load(&b);
   mme_value count = mme_load(&b);

   mme_value i = mme_mov(&b, start);
   mme_loop(&b, count) {
      mme_dwrite(&b, i, i);
      mme_add_to(&b, i, i, mme_imm(1));
   }

   mme_value j = mme_mov(&b, start);
   struct mme_value64 addr = mme_mov64(&b, mme_imm64(data_addr));

   mme_loop(&b, count) {
      mme_value x = mme_dread(&b, j);
      mme_store(&b, addr, x);
      mme_add_to(&b, j, j, mme_imm(1));
      mme_add64_to(&b, addr, addr, mme_imm64(4));
   }

   auto macro = mme_builder_finish_vec(&b);

   for (uint32_t i = 0; i < MME_TU104_DRAM_COUNT; i += chunk_size) {
      reset_push();

      push_macro(0, macro);

      P_1INC(p, NVC597, CALL_MME_MACRO(0));
      P_INLINE_DATA(p, i);
      P_INLINE_DATA(p, chunk_size);

      submit_push();

      for (uint32_t j = 0; j < chunk_size; j++)
         ASSERT_EQ(data[j], i + j);
   }
}

TEST_F(mme_tu104_sim_test, dma_read_fifoed)
{
   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_mthd(&b, NVC597_SET_MME_DATA_RAM_ADDRESS);
   mme_emit(&b, mme_zero());

   mme_mthd(&b, NVC597_SET_MME_MEM_ADDRESS_A);
   mme_emit(&b, mme_imm(high32(data_addr)));
   mme_emit(&b, mme_imm(low32(data_addr)));

   mme_mthd(&b, NVC597_MME_DMA_READ_FIFOED);
   mme_emit(&b, mme_imm(2));

   mme_tu104_load_barrier(&b);

   mme_value x = mme_load(&b);
   mme_value y = mme_load(&b);

   mme_store_imm_addr(&b, data_addr + 256 + 0, x);
   mme_store_imm_addr(&b, data_addr + 256 + 4, y);

   auto macro = mme_builder_finish_vec(&b);

   P_IMMD(p, NVC597, SET_MME_DATA_FIFO_CONFIG, FIFO_SIZE_SIZE_4KB);

   for (uint32_t i = 0; i < 64; i++)
      data[i] = 1000 + i;

   std::vector<uint32_t> params;
   params.push_back(7);

   test_macro(&b, macro, params);
}

TEST_F(mme_tu104_sim_test, scratch_limit)
{
   static const uint32_t chunk_size = 32;

   mme_builder b;
   mme_builder_init(&b, devinfo);

   mme_value start = mme_load(&b);
   mme_value count = mme_load(&b);

   mme_value i = mme_mov(&b, start);
   mme_loop(&b, count) {
      mme_mthd_arr(&b, NVC597_SET_MME_SHADOW_SCRATCH(0), i);
      mme_emit(&b, i);
      mme_add_to(&b, i, i, mme_imm(1));
   }

   mme_value j = mme_mov(&b, start);
   struct mme_value64 addr = mme_mov64(&b, mme_imm64(data_addr));

   mme_loop(&b, count) {
      mme_value x = mme_state_arr(&b, NVC597_SET_MME_SHADOW_SCRATCH(0), j);
      mme_store(&b, addr, x);
      mme_add_to(&b, j, j, mme_imm(1));
      mme_add64_to(&b, addr, addr, mme_imm64(4));
   }

   auto macro = mme_builder_finish_vec(&b);

   for (uint32_t i = 0; i < MME_TU104_SCRATCH_COUNT; i += chunk_size) {
      reset_push();

      push_macro(0, macro);

      P_1INC(p, NVC597, CALL_MME_MACRO(0));
      P_INLINE_DATA(p, i);
      P_INLINE_DATA(p, chunk_size);

      submit_push();

      for (uint32_t j = 0; j < chunk_size; j++)
         ASSERT_EQ(data[j], i + j);
   }
}
