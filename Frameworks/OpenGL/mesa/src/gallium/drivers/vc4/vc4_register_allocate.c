/*
 * Copyright © 2014 Broadcom
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "util/ralloc.h"
#include "util/register_allocate.h"
#include "vc4_context.h"
#include "vc4_qir.h"
#include "vc4_qpu.h"

#define QPU_R(file, index) { QPU_MUX_##file, index }

static const struct qpu_reg vc4_regs[] = {
        { QPU_MUX_R0, 0},
        { QPU_MUX_R1, 0},
        { QPU_MUX_R2, 0},
        { QPU_MUX_R3, 0},
        { QPU_MUX_R4, 0},
        QPU_R(A, 0),
        QPU_R(B, 0),
        QPU_R(A, 1),
        QPU_R(B, 1),
        QPU_R(A, 2),
        QPU_R(B, 2),
        QPU_R(A, 3),
        QPU_R(B, 3),
        QPU_R(A, 4),
        QPU_R(B, 4),
        QPU_R(A, 5),
        QPU_R(B, 5),
        QPU_R(A, 6),
        QPU_R(B, 6),
        QPU_R(A, 7),
        QPU_R(B, 7),
        QPU_R(A, 8),
        QPU_R(B, 8),
        QPU_R(A, 9),
        QPU_R(B, 9),
        QPU_R(A, 10),
        QPU_R(B, 10),
        QPU_R(A, 11),
        QPU_R(B, 11),
        QPU_R(A, 12),
        QPU_R(B, 12),
        QPU_R(A, 13),
        QPU_R(B, 13),
        QPU_R(A, 14),
        QPU_R(B, 14),
        QPU_R(A, 15),
        QPU_R(B, 15),
        QPU_R(A, 16),
        QPU_R(B, 16),
        QPU_R(A, 17),
        QPU_R(B, 17),
        QPU_R(A, 18),
        QPU_R(B, 18),
        QPU_R(A, 19),
        QPU_R(B, 19),
        QPU_R(A, 20),
        QPU_R(B, 20),
        QPU_R(A, 21),
        QPU_R(B, 21),
        QPU_R(A, 22),
        QPU_R(B, 22),
        QPU_R(A, 23),
        QPU_R(B, 23),
        QPU_R(A, 24),
        QPU_R(B, 24),
        QPU_R(A, 25),
        QPU_R(B, 25),
        QPU_R(A, 26),
        QPU_R(B, 26),
        QPU_R(A, 27),
        QPU_R(B, 27),
        QPU_R(A, 28),
        QPU_R(B, 28),
        QPU_R(A, 29),
        QPU_R(B, 29),
        QPU_R(A, 30),
        QPU_R(B, 30),
        QPU_R(A, 31),
        QPU_R(B, 31),
};
#define ACC_INDEX     0
#define ACC_COUNT     5
#define AB_INDEX      (ACC_INDEX + ACC_COUNT)
#define AB_COUNT      64

static void
vc4_alloc_reg_set(struct vc4_context *vc4)
{
        assert(vc4_regs[AB_INDEX].addr == 0);
        assert(vc4_regs[AB_INDEX + 1].addr == 0);
        STATIC_ASSERT(ARRAY_SIZE(vc4_regs) == AB_INDEX + 64);

        if (vc4->regs)
                return;

        vc4->regs = ra_alloc_reg_set(vc4, ARRAY_SIZE(vc4_regs), false);

        /* The physical regfiles split us into two classes, with [0] being the
         * whole space and [1] being the bottom half (for threaded fragment
         * shaders).
         */
        for (int i = 0; i < 2; i++) {
                vc4->reg_class_any[i] = ra_alloc_contig_reg_class(vc4->regs, 1);
                vc4->reg_class_a_or_b[i] = ra_alloc_contig_reg_class(vc4->regs, 1);
                vc4->reg_class_a_or_b_or_acc[i] = ra_alloc_contig_reg_class(vc4->regs, 1);
                vc4->reg_class_r4_or_a[i] = ra_alloc_contig_reg_class(vc4->regs, 1);
                vc4->reg_class_a[i] = ra_alloc_contig_reg_class(vc4->regs, 1);
        }
        vc4->reg_class_r0_r3 = ra_alloc_contig_reg_class(vc4->regs, 1);

        /* r0-r3 */
        for (uint32_t i = ACC_INDEX; i < ACC_INDEX + 4; i++) {
                ra_class_add_reg(vc4->reg_class_r0_r3, i);
                ra_class_add_reg(vc4->reg_class_a_or_b_or_acc[0], i);
                ra_class_add_reg(vc4->reg_class_a_or_b_or_acc[1], i);
        }

        /* R4 gets a special class because it can't be written as a general
         * purpose register. (it's TMU_NOSWAP as a write address).
         */
        for (int i = 0; i < 2; i++) {
                ra_class_add_reg(vc4->reg_class_r4_or_a[i], ACC_INDEX + 4);
                ra_class_add_reg(vc4->reg_class_any[i], ACC_INDEX + 4);
        }

        /* A/B */
        for (uint32_t i = AB_INDEX; i < AB_INDEX + 64; i ++) {
                /* Reserve ra14/rb14 for spilling fixup_raddr_conflict() in
                 * vc4_qpu_emit.c
                 */
                if (vc4_regs[i].addr == 14)
                        continue;

                ra_class_add_reg(vc4->reg_class_any[0], i);
                ra_class_add_reg(vc4->reg_class_a_or_b[0], i);
                ra_class_add_reg(vc4->reg_class_a_or_b_or_acc[0], i);

                if (vc4_regs[i].addr < 16) {
                        ra_class_add_reg(vc4->reg_class_any[1], i);
                        ra_class_add_reg(vc4->reg_class_a_or_b[1], i);
                        ra_class_add_reg(vc4->reg_class_a_or_b_or_acc[1], i);
                }


                /* A only */
                if (((i - AB_INDEX) & 1) == 0) {
                        ra_class_add_reg(vc4->reg_class_a[0], i);
                        ra_class_add_reg(vc4->reg_class_r4_or_a[0], i);

                        if (vc4_regs[i].addr < 16) {
                                ra_class_add_reg(vc4->reg_class_a[1], i);
                                ra_class_add_reg(vc4->reg_class_r4_or_a[1], i);
                        }
                }
        }

        ra_set_finalize(vc4->regs, NULL);
}

struct node_to_temp_map {
        uint32_t temp;
        uint32_t priority;
};

static int
node_to_temp_priority(const void *in_a, const void *in_b)
{
        const struct node_to_temp_map *a = in_a;
        const struct node_to_temp_map *b = in_b;

        return a->priority - b->priority;
}

#define CLASS_BIT_A			(1 << 0)
#define CLASS_BIT_B			(1 << 1)
#define CLASS_BIT_R4			(1 << 2)
#define CLASS_BIT_R0_R3			(1 << 4)

struct vc4_ra_select_callback_data {
        uint32_t next_acc;
        uint32_t next_ab;
};

static unsigned int
vc4_ra_select_callback(unsigned int n, BITSET_WORD *regs, void *data)
{
        struct vc4_ra_select_callback_data *vc4_ra = data;

        /* If r4 is available, always choose it -- few other things can go
         * there, and choosing anything else means inserting a mov.
         */
        if (BITSET_TEST(regs, ACC_INDEX + 4))
                return ACC_INDEX + 4;

        /* Choose an accumulator if possible (no delay between write and
         * read), but round-robin through them to give post-RA instruction
         * selection more options.
         */
        for (int i = 0; i < ACC_COUNT; i++) {
                int acc_off = (vc4_ra->next_acc + i) % ACC_COUNT;
                int acc = ACC_INDEX + acc_off;

                if (BITSET_TEST(regs, acc)) {
                        vc4_ra->next_acc = acc_off + 1;
                        return acc;
                }
        }

        for (int i = 0; i < AB_COUNT; i++) {
                int ab_off = (vc4_ra->next_ab + i) % AB_COUNT;
                int ab = AB_INDEX + ab_off;

                if (BITSET_TEST(regs, ab)) {
                        vc4_ra->next_ab = ab_off + 1;
                        return ab;
                }
        }

        unreachable("RA must pass us at least one possible reg.");
}

/**
 * Returns a mapping from QFILE_TEMP indices to struct qpu_regs.
 *
 * The return value should be freed by the caller.
 */
struct qpu_reg *
vc4_register_allocate(struct vc4_context *vc4, struct vc4_compile *c)
{
        struct node_to_temp_map map[c->num_temps];
        uint32_t temp_to_node[c->num_temps];
        uint8_t class_bits[c->num_temps];
        struct qpu_reg *temp_registers = calloc(c->num_temps,
                                                sizeof(*temp_registers));
        struct vc4_ra_select_callback_data callback_data = {
                .next_acc = 0,
                .next_ab = 0,
        };

        /* If things aren't ever written (undefined values), just read from
         * r0.
         */
        for (uint32_t i = 0; i < c->num_temps; i++)
                temp_registers[i] = qpu_rn(0);

        vc4_alloc_reg_set(vc4);

        struct ra_graph *g = ra_alloc_interference_graph(vc4->regs,
                                                         c->num_temps);

        /* Compute the live ranges so we can figure out interference. */
        qir_calculate_live_intervals(c);

        ra_set_select_reg_callback(g, vc4_ra_select_callback, &callback_data);

        for (uint32_t i = 0; i < c->num_temps; i++) {
                map[i].temp = i;
                map[i].priority = c->temp_end[i] - c->temp_start[i];
        }
        qsort(map, c->num_temps, sizeof(map[0]), node_to_temp_priority);
        for (uint32_t i = 0; i < c->num_temps; i++) {
                temp_to_node[map[i].temp] = i;
        }

        /* Figure out our register classes and preallocated registers.  We
         * start with any temp being able to be in any file, then instructions
         * incrementally remove bits that the temp definitely can't be in.
         */
        memset(class_bits,
               CLASS_BIT_A | CLASS_BIT_B | CLASS_BIT_R4 | CLASS_BIT_R0_R3,
               sizeof(class_bits));

        int ip = 0;
        qir_for_each_inst_inorder(inst, c) {
                if (qir_writes_r4(inst)) {
                        /* This instruction writes r4 (and optionally moves
                         * its result to a temp), so nothing else can be
                         * stored in r4 across it.
                         */
                        for (int i = 0; i < c->num_temps; i++) {
                                if (c->temp_start[i] < ip && c->temp_end[i] > ip)
                                        class_bits[i] &= ~CLASS_BIT_R4;
                        }

                        /* If we're doing a conditional write of something
                         * writing R4 (math, tex results), then make sure that
                         * we store in a temp so that we actually
                         * conditionally move the result.
                         */
                        if (inst->cond != QPU_COND_ALWAYS)
                                class_bits[inst->dst.index] &= ~CLASS_BIT_R4;
                } else {
                        /* R4 can't be written as a general purpose
                         * register. (it's TMU_NOSWAP as a write address).
                         */
                        if (inst->dst.file == QFILE_TEMP)
                                class_bits[inst->dst.index] &= ~CLASS_BIT_R4;
                }

                switch (inst->op) {
                case QOP_FRAG_Z:
                        ra_set_node_reg(g, temp_to_node[inst->dst.index],
                                        AB_INDEX + QPU_R_FRAG_PAYLOAD_ZW * 2 + 1);
                        break;

                case QOP_FRAG_W:
                        ra_set_node_reg(g, temp_to_node[inst->dst.index],
                                        AB_INDEX + QPU_R_FRAG_PAYLOAD_ZW * 2);
                        break;

                case QOP_ROT_MUL:
                        assert(inst->src[0].file == QFILE_TEMP);
                        class_bits[inst->src[0].index] &= CLASS_BIT_R0_R3;
                        break;

                case QOP_THRSW:
                        /* All accumulators are invalidated across a thread
                         * switch.
                         */
                        for (int i = 0; i < c->num_temps; i++) {
                                if (c->temp_start[i] < ip && c->temp_end[i] > ip)
                                        class_bits[i] &= ~(CLASS_BIT_R0_R3 |
                                                           CLASS_BIT_R4);
                        }
                        break;

                default:
                        break;
                }

                if (inst->dst.pack && !qir_is_mul(inst)) {
                        /* The non-MUL pack flags require an A-file dst
                         * register.
                         */
                        class_bits[inst->dst.index] &= CLASS_BIT_A;
                }

                /* Apply restrictions for src unpacks.  The integer unpacks
                 * can only be done from regfile A, while float unpacks can be
                 * either A or R4.
                 */
                for (int i = 0; i < qir_get_nsrc(inst); i++) {
                        if (inst->src[i].file == QFILE_TEMP &&
                            inst->src[i].pack) {
                                if (qir_is_float_input(inst)) {
                                        class_bits[inst->src[i].index] &=
                                                CLASS_BIT_A | CLASS_BIT_R4;
                                } else {
                                        class_bits[inst->src[i].index] &=
                                                CLASS_BIT_A;
                                }
                        }
                }

                ip++;
        }

        for (uint32_t i = 0; i < c->num_temps; i++) {
                int node = temp_to_node[i];

                switch (class_bits[i]) {
                case CLASS_BIT_A | CLASS_BIT_B | CLASS_BIT_R4 | CLASS_BIT_R0_R3:
                        ra_set_node_class(g, node,
                                          vc4->reg_class_any[c->fs_threaded]);
                        break;
                case CLASS_BIT_A | CLASS_BIT_B:
                        ra_set_node_class(g, node,
                                          vc4->reg_class_a_or_b[c->fs_threaded]);
                        break;
                case CLASS_BIT_A | CLASS_BIT_B | CLASS_BIT_R0_R3:
                        ra_set_node_class(g, node,
                                          vc4->reg_class_a_or_b_or_acc[c->fs_threaded]);
                        break;
                case CLASS_BIT_A | CLASS_BIT_R4:
                        ra_set_node_class(g, node,
                                          vc4->reg_class_r4_or_a[c->fs_threaded]);
                        break;
                case CLASS_BIT_A:
                        ra_set_node_class(g, node,
                                          vc4->reg_class_a[c->fs_threaded]);
                        break;
                case CLASS_BIT_R0_R3:
                        ra_set_node_class(g, node, vc4->reg_class_r0_r3);
                        break;

                default:
                        /* DDX/DDY used across thread switched might get us
                         * here.
                         */
                        if (c->fs_threaded) {
                                c->failed = true;
                                free(temp_registers);
                                return NULL;
                        }

                        fprintf(stderr, "temp %d: bad class bits: 0x%x\n",
                                i, class_bits[i]);
                        abort();
                        break;
                }
        }

        for (uint32_t i = 0; i < c->num_temps; i++) {
                for (uint32_t j = i + 1; j < c->num_temps; j++) {
                        if (!(c->temp_start[i] >= c->temp_end[j] ||
                              c->temp_start[j] >= c->temp_end[i])) {
                                ra_add_node_interference(g,
                                                         temp_to_node[i],
                                                         temp_to_node[j]);
                        }
                }
        }

        bool ok = ra_allocate(g);
        if (!ok) {
                if (!c->fs_threaded) {
                        fprintf(stderr, "Failed to register allocate:\n");
                        qir_dump(c);
                }

                c->failed = true;
                free(temp_registers);
                return NULL;
        }

        for (uint32_t i = 0; i < c->num_temps; i++) {
                temp_registers[i] = vc4_regs[ra_get_node_reg(g, temp_to_node[i])];

                /* If the value's never used, just write to the NOP register
                 * for clarity in debug output.
                 */
                if (c->temp_start[i] == c->temp_end[i])
                        temp_registers[i] = qpu_ra(QPU_W_NOP);
        }

        ralloc_free(g);

        return temp_registers;
}
