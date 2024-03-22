/*
 * Copyright © 2022 Collabora Ltd. and Red Hat Inc.
 * SPDX-License-Identifier: MIT
 */
#ifndef NVK_MME_H
#define NVK_MME_H 1

#include "mme_builder.h"

struct nv_device_info;

enum nvk_mme {
   NVK_MME_BIND_CBUF_DESC,
   NVK_MME_CLEAR,
   NVK_MME_DRAW,
   NVK_MME_DRAW_INDEXED,
   NVK_MME_DRAW_INDIRECT,
   NVK_MME_DRAW_INDEXED_INDIRECT,
   NVK_MME_DRAW_INDIRECT_COUNT,
   NVK_MME_DRAW_INDEXED_INDIRECT_COUNT,
   NVK_MME_ADD_CS_INVOCATIONS,
   NVK_MME_DISPATCH_INDIRECT,
   NVK_MME_WRITE_CS_INVOCATIONS,
   NVK_MME_COPY_QUERIES,
   NVK_MME_XFB_COUNTER_LOAD,
   NVK_MME_XFB_DRAW_INDIRECT,
   NVK_MME_SET_PRIV_REG,
   NVK_MME_SET_WRITE_MASK,
   NVK_MME_COUNT,
};

enum nvk_mme_scratch {
   NVK_MME_SCRATCH_CS_INVOCATIONS_HI = 0,
   NVK_MME_SCRATCH_CS_INVOCATIONS_LO,
   NVK_MME_SCRATCH_DRAW_BEGIN,
   NVK_MME_SCRATCH_DRAW_COUNT,
   NVK_MME_SCRATCH_DRAW_PAD_DW,
   NVK_MME_SCRATCH_DRAW_IDX,
   NVK_MME_SCRATCH_VIEW_MASK,
   NVK_MME_SCRATCH_WRITE_MASK_DYN,
   NVK_MME_SCRATCH_WRITE_MASK_PIPELINE,

   /* Must be at the end */
   NVK_MME_NUM_SCRATCH,
};

static inline void
_nvk_mme_load_scratch_to(struct mme_builder *b, struct mme_value val,
                         enum nvk_mme_scratch scratch)
{
   mme_state_to(b, val, 0x3400 + scratch * 4);
}

static inline struct mme_value
_nvk_mme_load_scratch(struct mme_builder *b, enum nvk_mme_scratch scratch)
{
   struct mme_value val = mme_alloc_reg(b);
   _nvk_mme_load_scratch_to(b, val, scratch);
   return val;
}
#define nvk_mme_load_scratch(b, S) \
   _nvk_mme_load_scratch(b, NVK_MME_SCRATCH_##S)

static inline void
_nvk_mme_store_scratch(struct mme_builder *b, enum nvk_mme_scratch scratch,
                       struct mme_value data)
{
   mme_mthd(b, 0x3400 + scratch * 4);
   mme_emit(b, data);
}
#define nvk_mme_store_scratch(b, S, v) \
   _nvk_mme_store_scratch(b, NVK_MME_SCRATCH_##S, v)

static inline void
_nvk_mme_load_to_scratch(struct mme_builder *b, enum nvk_mme_scratch scratch)
{
   struct mme_value val = mme_load(b);
   _nvk_mme_store_scratch(b, scratch, val);
   mme_free_reg(b, val);
}
#define nvk_mme_load_to_scratch(b, S) \
   _nvk_mme_load_to_scratch(b, NVK_MME_SCRATCH_##S)

static void
_nvk_mme_spill(struct mme_builder *b, enum nvk_mme_scratch scratch,
               struct mme_value val)
{
   if (val.type == MME_VALUE_TYPE_REG) {
      _nvk_mme_store_scratch(b, scratch, val);
      mme_free_reg(b, val);
   }
}
#define nvk_mme_spill(b, S, v) \
   _nvk_mme_spill(b, NVK_MME_SCRATCH_##S, v)

static void
_nvk_mme_unspill(struct mme_builder *b, enum nvk_mme_scratch scratch,
                 struct mme_value val)
{
   if (val.type == MME_VALUE_TYPE_REG) {
      mme_realloc_reg(b, val);
      _nvk_mme_load_scratch_to(b, val, scratch);
   }
}
#define nvk_mme_unspill(b, S, v) \
   _nvk_mme_unspill(b, NVK_MME_SCRATCH_##S, v)

typedef void (*nvk_mme_builder_func)(struct mme_builder *b);

uint32_t *nvk_build_mme(const struct nv_device_info *devinfo,
                        enum nvk_mme mme, size_t *size_out);

void nvk_test_build_all_mmes(const struct nv_device_info *devinfo);

void nvk_mme_bind_cbuf_desc(struct mme_builder *b);
void nvk_mme_clear(struct mme_builder *b);
void nvk_mme_draw(struct mme_builder *b);
void nvk_mme_draw_indexed(struct mme_builder *b);
void nvk_mme_draw_indirect(struct mme_builder *b);
void nvk_mme_draw_indexed_indirect(struct mme_builder *b);
void nvk_mme_draw_indirect_count(struct mme_builder *b);
void nvk_mme_draw_indexed_indirect_count(struct mme_builder *b);
void nvk_mme_add_cs_invocations(struct mme_builder *b);
void nvk_mme_dispatch_indirect(struct mme_builder *b);
void nvk_mme_write_cs_invocations(struct mme_builder *b);
void nvk_mme_copy_queries(struct mme_builder *b);
void nvk_mme_xfb_counter_load(struct mme_builder *b);
void nvk_mme_xfb_draw_indirect(struct mme_builder *b);
void nvk_mme_set_priv_reg(struct mme_builder *b);
void nvk_mme_set_write_mask(struct mme_builder *b);

#endif /* NVK_MME_H */
