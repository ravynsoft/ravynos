/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>
#include <vector>

#include "mme_builder.h"

struct nouveau_bo;
struct nouveau_ws_contxt;
struct nouveau_ws_device;

#include "nv_push.h"
#include "nvk_cl9097.h"

#define DATA_BO_SIZE 4096
#define DATA_DWORDS 1024

class mme_runner {
public:
   mme_runner();
   virtual ~mme_runner();

   virtual void run_macro(const std::vector<uint32_t>& macro,
                          const std::vector<uint32_t>& params) = 0;

   void mme_store_data(mme_builder *b, uint32_t dw_idx,
                       mme_value data, bool free_reg = false);

   const nv_device_info *devinfo;
   uint64_t data_addr;
   uint32_t *data;
};

class mme_hw_runner : public mme_runner {
public:
   mme_hw_runner();
   virtual ~mme_hw_runner();

   bool set_up_hw(uint16_t min_cls, uint16_t max_cls);
   void push_macro(uint32_t id, const std::vector<uint32_t>& macro);
   void reset_push();
   void submit_push();

   virtual void run_macro(const std::vector<uint32_t>& macro,
                          const std::vector<uint32_t>& params);

   struct nv_push *p;

private:
   struct nouveau_ws_device *dev;
   struct nouveau_ws_context *ctx;
   struct nouveau_ws_bo *data_bo;
   struct nouveau_ws_bo *push_bo;
   uint32_t syncobj;
   void *push_map;
   struct nv_push push;
};

class mme_fermi_sim_runner : public mme_runner {
public:
   mme_fermi_sim_runner(uint64_t data_addr);
   virtual ~mme_fermi_sim_runner();

   virtual void run_macro(const std::vector<uint32_t>& macro,
                          const std::vector<uint32_t>& params);

private:
   struct nv_device_info info;
   uint32_t data_store[DATA_DWORDS];
};

class mme_tu104_sim_runner : public mme_runner {
public:
   mme_tu104_sim_runner(uint64_t data_addr);
   virtual ~mme_tu104_sim_runner();

   virtual void run_macro(const std::vector<uint32_t>& macro,
                          const std::vector<uint32_t>& params);

private:
   struct nv_device_info info;
   uint32_t data_store[DATA_DWORDS];
};

inline std::vector<uint32_t>
mme_builder_finish_vec(mme_builder *b)
{
   size_t size = 0;
   uint32_t *dw = mme_builder_finish(b, &size);
   std::vector<uint32_t> vec(dw, dw + (size / 4));
   free(dw);
   return vec;
}

inline uint32_t
high32(uint64_t x)
{
   return (uint32_t)(x >> 32);
}

inline uint32_t
low32(uint64_t x)
{
   return (uint32_t)x;
}

inline void
mme_store_imm_addr(mme_builder *b, uint64_t addr, mme_value v,
                   bool free_reg = false)
{
   mme_mthd(b, NV9097_SET_REPORT_SEMAPHORE_A);
   mme_emit(b, mme_imm(high32(addr)));
   mme_emit(b, mme_imm(low32(addr)));
   mme_emit(b, v);
   mme_emit(b, mme_imm(0x10000000));

   if (free_reg && v.type == MME_VALUE_TYPE_REG)
      mme_free_reg(b, v);
}

inline void
mme_store(mme_builder *b, struct mme_value64 addr, mme_value v,
          bool free_reg = false)
{
   mme_mthd(b, NV9097_SET_REPORT_SEMAPHORE_A);
   mme_emit(b, addr.hi);
   mme_emit(b, addr.lo);
   mme_emit(b, v);
   mme_emit(b, mme_imm(0x10000000));

   if (free_reg && v.type == MME_VALUE_TYPE_REG)
      mme_free_reg(b, v);
}
