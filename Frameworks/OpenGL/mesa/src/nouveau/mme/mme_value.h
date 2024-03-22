/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_VALUE_H
#define MME_VALUE_H

#include <stdbool.h>
#include <stdint.h>

#include "util/bitscan.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

enum mme_value_type {
   /* This must be zero, making a zero-initizlied mme_value a ZERO */
   MME_VALUE_TYPE_ZERO = 0,
   MME_VALUE_TYPE_IMM,
   MME_VALUE_TYPE_REG,
};

struct mme_value {
   enum mme_value_type type;

   union {
      uint32_t imm;
      uint32_t reg;
   };
};

struct mme_value64 {
   struct mme_value lo;
   struct mme_value hi;
};

static inline struct mme_value
mme_zero()
{
   struct mme_value val = {
      .type = MME_VALUE_TYPE_ZERO,
   };
   return val;
}

static inline struct mme_value
mme_imm(uint32_t imm)
{
   struct mme_value val = {
      .type = MME_VALUE_TYPE_IMM,
      .imm = imm,
   };
   return val;
}

static inline bool
mme_is_zero(struct mme_value x)
{
   switch (x.type) {
   case MME_VALUE_TYPE_ZERO:  return true;
   case MME_VALUE_TYPE_IMM:   return x.imm == 0;
   case MME_VALUE_TYPE_REG:   return false;
   default: unreachable("Invalid MME value type");
   }
}

static inline struct mme_value64
mme_value64(struct mme_value lo, struct mme_value hi)
{
   struct mme_value64 val = { lo, hi };
   return val;
}

static inline struct mme_value64
mme_imm64(uint64_t imm)
{
   struct mme_value64 val = {
      mme_imm((uint32_t)imm),
      mme_imm((uint32_t)(imm >> 32)),
   };
   return val;
}

struct mme_reg_alloc {
   uint32_t exists;
   uint32_t alloc;
};

static inline void
mme_reg_alloc_init(struct mme_reg_alloc *a, uint32_t exists)
{
   a->alloc = 0;
   a->exists = exists;
}

static inline struct mme_value
mme_reg_alloc_alloc(struct mme_reg_alloc *a)
{
   uint8_t reg = ffs(~a->alloc & a->exists) - 1;
   assert(reg < 32);
   assert(a->exists & (1u << reg));

   a->alloc |= (1u << reg);

   struct mme_value val = {
      .type = MME_VALUE_TYPE_REG,
      .reg = reg,
   };

   return val;
}

static inline void
mme_reg_alloc_realloc(struct mme_reg_alloc *a, struct mme_value val)
{
   assert(val.type == MME_VALUE_TYPE_REG);

   assert(val.reg < 32);
   assert(a->exists & (1u << val.reg));
   assert(!(a->alloc & (1u << val.reg)));

   a->alloc |= (1u << val.reg);
}

static inline void
mme_reg_alloc_free(struct mme_reg_alloc *a, struct mme_value val)
{
   assert(val.type == MME_VALUE_TYPE_REG);

   assert(val.reg < 32);
   assert(a->exists & (1u << val.reg));
   assert(a->alloc & (1u << val.reg));

   a->alloc &= ~(1u << val.reg);
}

#ifdef __cplusplus
}
#endif

#endif /* MME_VALUE_H */
