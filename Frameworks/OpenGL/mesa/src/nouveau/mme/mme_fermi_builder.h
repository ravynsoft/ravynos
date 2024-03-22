/*
 * Copyright © 2022 Mary Guillemard
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_BUILDER_H
#error "This file must only be included by mme_builder.h"
#endif

#include "mme_fermi.h"
#include "mme_value.h"

#include "util/bitscan.h"
#include "util/enum_operators.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MME_FERMI_BUILDER_MAX_INSTS 128

enum mme_fermi_instr_parts {
   MME_FERMI_INSTR_PART_OP     = BITFIELD_BIT(0),
   MME_FERMI_INSTR_PART_ASSIGN = BITFIELD_BIT(1)
};

struct mme_fermi_builder {
   bool first_loaded;
   uint32_t inst_count;
   enum mme_fermi_instr_parts inst_parts;
   struct mme_fermi_inst insts[MME_FERMI_BUILDER_MAX_INSTS];
   uint32_t cf_depth;
   struct mme_value loop_counter;
   struct mme_cf cf_stack[8];
};

void mme_fermi_builder_init(struct mme_builder *b);

uint32_t * mme_fermi_builder_finish(struct mme_fermi_builder *b, size_t *size_out);

void mme_fermi_builder_dump(struct mme_builder *b, FILE *fp);

void mme_fermi_add_inst(struct mme_builder *b,
                        const struct mme_fermi_inst *inst);

static inline bool
mme_fermi_is_empty(struct mme_fermi_builder *b)
{
   return b->inst_count == 0;
}

#define mme_fermi_asm(b, __inst)                                     \
   for (struct mme_fermi_inst __inst = { MME_FERMI_INST_DEFAULTS };  \
        !__inst.end_next;                                            \
        mme_fermi_add_inst((b), &__inst), __inst.end_next = true)

void mme_fermi_mthd_arr(struct mme_builder *b,
                        uint16_t mthd,
                        struct mme_value index);

void mme_fermi_emit(struct mme_builder *b,
                    struct mme_value data);

void mme_fermi_start_loop(struct mme_builder *b,
                          struct mme_value count);
void mme_fermi_end_loop(struct mme_builder *b);

void mme_fermi_start_if(struct mme_builder *b,
                        enum mme_cmp_op op,
                        bool if_true,
                        struct mme_value x,
                        struct mme_value y);
void mme_fermi_end_if(struct mme_builder *b);

void mme_fermi_start_while(struct mme_builder *b);
void mme_fermi_end_while(struct mme_builder *b,
                         enum mme_cmp_op op,
                         bool if_true,
                         struct mme_value x,
                         struct mme_value y);

void mme_fermi_load_to(struct mme_builder *b,
                       struct mme_value dst);

struct mme_value mme_fermi_load(struct mme_builder *b);

void
mme_fermi_alu_to(struct mme_builder *b,
                 struct mme_value dst,
                 enum mme_alu_op op,
                 struct mme_value x,
                 struct mme_value y);

static inline void
mme_fermi_alu64_to(struct mme_builder *b,
                   struct mme_value64 dst,
                   enum mme_alu_op op_lo,
                   enum mme_alu_op op_hi,
                   struct mme_value64 x,
                   struct mme_value64 y)
{
   assert(dst.lo.type == MME_VALUE_TYPE_REG);
   assert(dst.hi.type == MME_VALUE_TYPE_REG);

   mme_fermi_alu_to(b, dst.lo, op_lo, x.lo, y.lo);
   mme_fermi_alu_to(b, dst.hi, op_hi, x.hi, y.hi);
}

void
mme_fermi_bfe_to(struct mme_builder *b, struct mme_value dst,
                 struct mme_value x, struct mme_value pos, uint8_t bits);

void
mme_fermi_merge_to(struct mme_builder *b, struct mme_value dst,
                   struct mme_value x, struct mme_value y,
                   uint16_t dst_pos, uint16_t bits, uint16_t src_pos);

void mme_fermi_state_arr_to(struct mme_builder *b,
                            struct mme_value dst,
                            uint16_t state,
                            struct mme_value index);

#ifdef __cplusplus
}
#endif

