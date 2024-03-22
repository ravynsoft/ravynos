/*
 * Copyright © 2012 Intel Corporation
 * Copyright © 2016 Broadcom
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

#define MAX_INSTRUCTION (1 << 30)

#include "util/ralloc.h"
#include "util/register_allocate.h"
#include "vc4_context.h"
#include "vc4_qir.h"

struct partial_update_state {
        struct qinst *insts[4];
        uint8_t channels;
};

static int
qir_reg_to_var(struct qreg reg)
{
        if (reg.file == QFILE_TEMP)
                return reg.index;

        return -1;
}

static void
qir_setup_use(struct vc4_compile *c, struct qblock *block, int ip,
              struct qreg src)
{
        int var = qir_reg_to_var(src);
        if (var == -1)
                return;

        c->temp_start[var] = MIN2(c->temp_start[var], ip);
        c->temp_end[var] = MAX2(c->temp_end[var], ip);

        /* The use[] bitset marks when the block makes
         * use of a variable without having completely
         * defined that variable within the block.
         */
        if (!BITSET_TEST(block->def, var))
                BITSET_SET(block->use, var);
}

static struct partial_update_state *
get_partial_update_state(struct hash_table *partial_update_ht,
                         struct qinst *inst)
{
        struct hash_entry *entry =
                _mesa_hash_table_search(partial_update_ht,
                                        &inst->dst.index);
        if (entry)
                return entry->data;

        struct partial_update_state *state =
                rzalloc(partial_update_ht, struct partial_update_state);

        _mesa_hash_table_insert(partial_update_ht, &inst->dst.index, state);

        return state;
}

static void
qir_setup_def(struct vc4_compile *c, struct qblock *block, int ip,
              struct hash_table *partial_update_ht, struct qinst *inst)
{
        /* The def[] bitset marks when an initialization in a
         * block completely screens off previous updates of
         * that variable.
         */
        int var = qir_reg_to_var(inst->dst);
        if (var == -1)
                return;

        c->temp_start[var] = MIN2(c->temp_start[var], ip);
        c->temp_end[var] = MAX2(c->temp_end[var], ip);

        /* If we've already tracked this as a def, or already used it within
         * the block, there's nothing to do.
         */
        if (BITSET_TEST(block->use, var) || BITSET_TEST(block->def, var))
                return;

        /* Easy, common case: unconditional full register update.
         *
         * We treat conditioning on the exec mask as the same as not being
         * conditional.  This makes sure that if the register gets set on
         * either side of an if, it is treated as being screened off before
         * the if.  Otherwise, if there was no intervening def, its live
         * interval doesn't extend back to the start of he program, and if too
         * many registers did that we'd fail to register allocate.
         */
        if ((inst->cond == QPU_COND_ALWAYS ||
             inst->cond_is_exec_mask) && !inst->dst.pack) {
                BITSET_SET(block->def, var);
                return;
        }

        /* Finally, look at the condition code and packing and mark it as a
         * def.  We need to make sure that we understand sequences
         * instructions like:
         *
         *     mov.zs t0, t1
         *     mov.zc t0, t2
         *
         * or:
         *
         *     mmov t0.8a, t1
         *     mmov t0.8b, t2
         *     mmov t0.8c, t3
         *     mmov t0.8d, t4
         *
         * as defining the temp within the block, because otherwise dst's live
         * range will get extended up the control flow to the top of the
         * program.
         */
        struct partial_update_state *state =
                get_partial_update_state(partial_update_ht, inst);
        uint8_t mask = qir_channels_written(inst);

        if (inst->cond == QPU_COND_ALWAYS) {
                state->channels |= mask;
        } else {
                for (int i = 0; i < 4; i++) {
                        if (!(mask & (1 << i)))
                                continue;

                        if (state->insts[i] &&
                            state->insts[i]->cond ==
                            qpu_cond_complement(inst->cond))
                                state->channels |= 1 << i;
                        else
                                state->insts[i] = inst;
                }
        }

        if (state->channels == 0xf)
                BITSET_SET(block->def, var);
}

static void
sf_state_clear(struct hash_table *partial_update_ht)
{
        hash_table_foreach(partial_update_ht, entry) {
                struct partial_update_state *state = entry->data;

                for (int i = 0; i < 4; i++) {
                        if (state->insts[i] && state->insts[i]->cond)
                                state->insts[i] = NULL;
                }
        }
}

/* Sets up the def/use arrays for when variables are used-before-defined or
 * defined-before-used in the block.
 *
 * Also initializes the temp_start/temp_end to cover just the instruction IPs
 * where the variable is used, which will be extended later in
 * qir_compute_start_end().
 */
static void
qir_setup_def_use(struct vc4_compile *c)
{
        struct hash_table *partial_update_ht =
                _mesa_hash_table_create(c, _mesa_hash_int, _mesa_key_int_equal);
        int ip = 0;

        qir_for_each_block(block, c) {
                block->start_ip = ip;

                _mesa_hash_table_clear(partial_update_ht, NULL);

                qir_for_each_inst(inst, block) {
                        for (int i = 0; i < qir_get_nsrc(inst); i++)
                                qir_setup_use(c, block, ip, inst->src[i]);

                        qir_setup_def(c, block, ip, partial_update_ht, inst);

                        if (inst->sf)
                                sf_state_clear(partial_update_ht);

                        switch (inst->op) {
                        case QOP_FRAG_Z:
                        case QOP_FRAG_W:
                                /* The payload registers have values
                                 * implicitly loaded at the start of the
                                 * program.
                                 */
                                if (inst->dst.file == QFILE_TEMP)
                                        c->temp_start[inst->dst.index] = 0;
                                break;
                        default:
                                break;
                        }
                        ip++;
                }
                block->end_ip = ip;
        }

        _mesa_hash_table_destroy(partial_update_ht, NULL);
}

static bool
qir_live_variables_dataflow(struct vc4_compile *c, int bitset_words)
{
        bool cont = false;

        qir_for_each_block_rev(block, c) {
                /* Update live_out: Any successor using the variable
                 * on entrance needs us to have the variable live on
                 * exit.
                 */
                qir_for_each_successor(succ, block) {
                        for (int i = 0; i < bitset_words; i++) {
                                BITSET_WORD new_live_out = (succ->live_in[i] &
                                                            ~block->live_out[i]);
                                if (new_live_out) {
                                        block->live_out[i] |= new_live_out;
                                        cont = true;
                                }
                        }
                }

                /* Update live_in */
                for (int i = 0; i < bitset_words; i++) {
                        BITSET_WORD new_live_in = (block->use[i] |
                                                   (block->live_out[i] &
                                                    ~block->def[i]));
                        if (new_live_in & ~block->live_in[i]) {
                                block->live_in[i] |= new_live_in;
                                cont = true;
                        }
                }
        }

        return cont;
}

/**
 * Extend the start/end ranges for each variable to account for the
 * new information calculated from control flow.
 */
static void
qir_compute_start_end(struct vc4_compile *c, int num_vars)
{
        qir_for_each_block(block, c) {
                for (int i = 0; i < num_vars; i++) {
                        if (BITSET_TEST(block->live_in, i)) {
                                c->temp_start[i] = MIN2(c->temp_start[i],
                                                        block->start_ip);
                                c->temp_end[i] = MAX2(c->temp_end[i],
                                                      block->start_ip);
                        }

                        if (BITSET_TEST(block->live_out, i)) {
                                c->temp_start[i] = MIN2(c->temp_start[i],
                                                        block->end_ip);
                                c->temp_end[i] = MAX2(c->temp_end[i],
                                                      block->end_ip);
                        }
                }
        }
}

void
qir_calculate_live_intervals(struct vc4_compile *c)
{
        int bitset_words = BITSET_WORDS(c->num_temps);

        /* If we called this function more than once, then we should be
         * freeing the previous arrays.
         */
        assert(!c->temp_start);

        c->temp_start = rzalloc_array(c, int, c->num_temps);
        c->temp_end = rzalloc_array(c, int, c->num_temps);

        for (int i = 0; i < c->num_temps; i++) {
                c->temp_start[i] = MAX_INSTRUCTION;
                c->temp_end[i] = -1;
        }

        qir_for_each_block(block, c) {
                block->def = rzalloc_array(c, BITSET_WORD, bitset_words);
                block->use = rzalloc_array(c, BITSET_WORD, bitset_words);
                block->live_in = rzalloc_array(c, BITSET_WORD, bitset_words);
                block->live_out = rzalloc_array(c, BITSET_WORD, bitset_words);
        }

        qir_setup_def_use(c);

        while (qir_live_variables_dataflow(c, bitset_words))
                ;

        qir_compute_start_end(c, c->num_temps);

        if (VC4_DBG(SHADERDB)) {
                int last_ip = 0;
                for (int i = 0; i < c->num_temps; i++)
                        last_ip = MAX2(last_ip, c->temp_end[i]);

                int reg_pressure = 0;
                int max_reg_pressure = 0;
                for (int i = 0; i < last_ip; i++) {
                        for (int j = 0; j < c->num_temps; j++) {
                                if (c->temp_start[j] == i)
                                        reg_pressure++;
                                if (c->temp_end[j] == i)
                                        reg_pressure--;
                        }
                        max_reg_pressure = MAX2(max_reg_pressure, reg_pressure);
                }
                c->max_reg_pressure = max_reg_pressure;
        }
}
