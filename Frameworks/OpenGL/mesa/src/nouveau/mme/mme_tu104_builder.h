/*
 * Copyright © 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_BUILDER_H
#error "This file must only be included by mme_builder.h"
#endif

enum mme_tu104_instr_parts {
   MME_TU104_INSTR_PART_IMM0  = BITFIELD_BIT(0),
   MME_TU104_INSTR_PART_IMM1  = BITFIELD_BIT(1),
   MME_TU104_INSTR_PART_LOAD0 = BITFIELD_BIT(2),
   MME_TU104_INSTR_PART_LOAD1 = BITFIELD_BIT(3),
   MME_TU104_INSTR_PART_ALU0  = BITFIELD_BIT(4),
   MME_TU104_INSTR_PART_ALU1  = BITFIELD_BIT(5),
   MME_TU104_INSTR_PART_MTHD0 = BITFIELD_BIT(6),
   MME_TU104_INSTR_PART_MTHD1 = BITFIELD_BIT(7),
   MME_TU104_INSTR_PART_EMIT0 = BITFIELD_BIT(8),
   MME_TU104_INSTR_PART_EMIT1 = BITFIELD_BIT(9),
};

#define MME_TU104_BUILDER_MAX_INSTS 128

struct mme_tu104_builder {
   uint32_t inst_count;
   struct mme_tu104_inst insts[MME_TU104_BUILDER_MAX_INSTS];
   enum mme_tu104_instr_parts inst_parts;

   uint32_t cf_depth;
   struct mme_cf cf_stack[8];
};

void mme_tu104_builder_init(struct mme_builder *b);

void mme_tu104_add_inst(struct mme_builder *b,
                        const struct mme_tu104_inst *inst);

#define mme_tu104_asm(b, __inst)                                     \
   for (struct mme_tu104_inst __inst = { MME_TU104_INST_DEFAULTS };  \
        !__inst.end_next;                                            \
        mme_tu104_add_inst((b), &__inst), __inst.end_next = true)

void mme_tu104_alu_to(struct mme_builder *b,
                      struct mme_value dst,
                      enum mme_alu_op op,
                      struct mme_value x,
                      struct mme_value y);

void mme_tu104_alu64_to(struct mme_builder *b,
                        struct mme_value64 dst,
                        enum mme_alu_op op_lo,
                        enum mme_alu_op op_hi,
                        struct mme_value64 x,
                        struct mme_value64 y);

void mme_tu104_merge_to(struct mme_builder *b, struct mme_value dst,
                        struct mme_value x, struct mme_value y,
                        uint16_t dst_pos, uint16_t bits, uint16_t src_pos);

void mme_tu104_state_arr_to(struct mme_builder *b, struct mme_value dst,
                            uint16_t state, struct mme_value index);

void mme_tu104_load_barrier(struct mme_builder *b);

void mme_tu104_load_to(struct mme_builder *b,
                       struct mme_value dst);

void mme_tu104_mthd(struct mme_builder *b,
                    uint16_t mthd, struct mme_value index);

void mme_tu104_emit(struct mme_builder *b,
                    struct mme_value data);

void mme_tu104_start_loop(struct mme_builder *b,
                          struct mme_value count);
void mme_tu104_end_loop(struct mme_builder *b);

void mme_tu104_start_if(struct mme_builder *b,
                        enum mme_cmp_op op,
                        bool if_true,
                        struct mme_value x,
                        struct mme_value y);
void mme_tu104_end_if(struct mme_builder *b);

void mme_tu104_start_while(struct mme_builder *b);
void mme_tu104_end_while(struct mme_builder *b,
                         enum mme_cmp_op op,
                         bool if_true,
                         struct mme_value x,
                         struct mme_value y);

void mme_tu104_exit_if(struct mme_builder *b,
                       enum mme_cmp_op op,
                       bool if_true,
                       struct mme_value x,
                       struct mme_value y);

uint32_t *mme_tu104_builder_finish(struct mme_tu104_builder *b,
                                   size_t *size_out);

void mme_tu104_builder_dump(struct mme_builder *b, FILE *fp);
