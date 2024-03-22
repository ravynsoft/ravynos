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

/**
 * @file vc4_qir_lower_uniforms.c
 *
 * This is the pre-code-generation pass for fixing up instructions that try to
 * read from multiple uniform values.
 */

#include "vc4_qir.h"
#include "util/hash_table.h"
#include "util/u_math.h"

static inline uint32_t
index_hash(const void *key)
{
        return (uintptr_t)key;
}

static inline bool
index_compare(const void *a, const void *b)
{
        return a == b;
}

static void
add_uniform(struct hash_table *ht, struct qreg reg)
{
        struct hash_entry *entry;
        void *key = (void *)(uintptr_t)(reg.index + 1);

        entry = _mesa_hash_table_search(ht, key);
        if (entry) {
                entry->data++;
        } else {
                _mesa_hash_table_insert(ht, key, (void *)(uintptr_t)1);
        }
}

static void
remove_uniform(struct hash_table *ht, struct qreg reg)
{
        struct hash_entry *entry;
        void *key = (void *)(uintptr_t)(reg.index + 1);

        entry = _mesa_hash_table_search(ht, key);
        assert(entry);
        entry->data = (void *)(((uintptr_t) entry->data) - 1);
        if (entry->data == NULL)
                _mesa_hash_table_remove(ht, entry);
}

static bool
is_lowerable_uniform(struct qinst *inst, int i)
{
        if (inst->src[i].file != QFILE_UNIF)
                return false;
        if (qir_is_tex(inst))
                return i != qir_get_tex_uniform_src(inst);
        return true;
}

/* Returns the number of different uniform values referenced by the
 * instruction.
 */
static uint32_t
qir_get_instruction_uniform_count(struct qinst *inst)
{
        uint32_t count = 0;

        for (int i = 0; i < qir_get_nsrc(inst); i++) {
                if (inst->src[i].file != QFILE_UNIF)
                        continue;

                bool is_duplicate = false;
                for (int j = 0; j < i; j++) {
                        if (inst->src[j].file == QFILE_UNIF &&
                            inst->src[j].index == inst->src[i].index) {
                                is_duplicate = true;
                                break;
                        }
                }
                if (!is_duplicate)
                        count++;
        }

        return count;
}

void
qir_lower_uniforms(struct vc4_compile *c)
{
        struct hash_table *ht =
                _mesa_hash_table_create(c, index_hash, index_compare);

        /* Walk the instruction list, finding which instructions have more
         * than one uniform referenced, and add those uniform values to the
         * ht.
         */
        qir_for_each_inst_inorder(inst, c) {
                uint32_t nsrc = qir_get_nsrc(inst);

                if (qir_get_instruction_uniform_count(inst) <= 1)
                        continue;

                for (int i = 0; i < nsrc; i++) {
                        if (is_lowerable_uniform(inst, i))
                                add_uniform(ht, inst->src[i]);
                }
        }

        while (ht->entries) {
                /* Find the most commonly used uniform in instructions that
                 * need a uniform lowered.
                 */
                uint32_t max_count = 0;
                uint32_t max_index = 0;
                hash_table_foreach(ht, entry) {
                        uint32_t count = (uintptr_t)entry->data;
                        uint32_t index = (uintptr_t)entry->key - 1;
                        if (count > max_count) {
                                max_count = count;
                                max_index = index;
                        }
                }

                struct qreg unif = qir_reg(QFILE_UNIF, max_index);

                /* Now, find the instructions using this uniform and make them
                 * reference a temp instead.
                 */
                qir_for_each_block(block, c) {
                        struct qinst *mov = NULL;

                        qir_for_each_inst(inst, block) {
                                uint32_t nsrc = qir_get_nsrc(inst);

                                uint32_t count = qir_get_instruction_uniform_count(inst);

                                if (count <= 1)
                                        continue;

                                /* If the block doesn't have a load of the
                                 * uniform yet, add it.  We could potentially
                                 * do better and CSE MOVs from multiple blocks
                                 * into dominating blocks, except that may
                                 * cause troubles for register allocation.
                                 */
                                if (!mov) {
                                        mov = qir_inst(QOP_MOV, qir_get_temp(c),
                                                       unif, c->undef);
                                        list_add(&mov->link,
                                                 &block->instructions);
                                        c->defs[mov->dst.index] = mov;
                                }

                                bool removed = false;
                                for (int i = 0; i < nsrc; i++) {
                                        if (is_lowerable_uniform(inst, i) &&
                                            inst->src[i].index == max_index) {
                                                inst->src[i] = mov->dst;
                                                remove_uniform(ht, unif);
                                                removed = true;
                                        }
                                }
                                if (removed)
                                        count--;

                                /* If the instruction doesn't need lowering any more,
                                 * then drop it from the list.
                                 */
                                if (count <= 1) {
                                        for (int i = 0; i < nsrc; i++) {
                                                if (is_lowerable_uniform(inst, i))
                                                        remove_uniform(ht, inst->src[i]);
                                        }
                                }
                        }
                }
        }

        _mesa_hash_table_destroy(ht, NULL);
}
