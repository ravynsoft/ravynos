/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef ROGUE_BUILDER_H
#define ROGUE_BUILDER_H

/**
 * \file rogue_builder.h
 *
 * \brief Contains helper functions for building Rogue shaders.
 */

#include "rogue.h"

/** Rogue builder context. */
typedef struct rogue_builder {
   rogue_shader *shader; /** The shader being built. */
   rogue_cursor cursor; /** The current position in the shader. */
} rogue_builder;

/**
 * \brief Initialises a Rogue builder context.
 *
 * \param[in] b The builder context.
 * \param[in] shader The shader.
 */
static inline void rogue_builder_init(rogue_builder *b, rogue_shader *shader)
{
   b->shader = shader;
   b->cursor = rogue_cursor_before_shader(shader);
}

/**
 * \brief Inserts a basic block at the current builder context position.
 *
 * \param[in] b The builder context.
 * \param[in] block The basic block to insert.
 */
static inline void rogue_builder_insert_block(rogue_builder *b,
                                              rogue_block *block)
{
   rogue_block_insert(block, b->cursor);
   b->cursor = rogue_cursor_after_block(block);
}

/**
 * \brief Inserts a new basic block at the current builder context position.
 *
 * \param[in] b The builder context.
 * \param[in] label The (optional) basic block label.
 * \return The new block.
 */
static inline rogue_block *rogue_push_block_labelled(rogue_builder *b,
                                                     const char *label)
{
   rogue_block *block = rogue_block_create(b->shader, label);
   rogue_builder_insert_block(b, block);

   return block;
}

/**
 * \brief Inserts a new basic block at the current builder context position.
 *
 * \param[in] b The builder context.
 * \return The new block.
 */
static inline rogue_block *rogue_push_block(rogue_builder *b)
{
   return rogue_push_block_labelled(b, NULL);
}

/* ALU instructions. */
#define ROGUE_BUILDER_DEFINE_ALU11(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref src0);

#define ROGUE_BUILDER_DEFINE_ALU12(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref src0,   \
                               rogue_ref src1);

#define ROGUE_BUILDER_DEFINE_ALU13(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref src0,   \
                               rogue_ref src1,   \
                               rogue_ref src2);

#define ROGUE_BUILDER_DEFINE_ALU22(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref dst1,   \
                               rogue_ref src0,   \
                               rogue_ref src1);

#define ROGUE_BUILDER_DEFINE_ALU23(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref dst1,   \
                               rogue_ref src0,   \
                               rogue_ref src1,   \
                               rogue_ref src2);

#define ROGUE_BUILDER_DEFINE_ALU35(op)           \
   rogue_alu_instr *rogue_##op(rogue_builder *b, \
                               rogue_ref dst0,   \
                               rogue_ref dst1,   \
                               rogue_ref dst2,   \
                               rogue_ref src0,   \
                               rogue_ref src1,   \
                               rogue_ref src2,   \
                               rogue_ref src3,   \
                               rogue_ref src4);

#include "rogue_alu_instrs.def"

/* Backend instructions. */
#define ROGUE_BUILDER_DEFINE_BACKEND00(op) \
   rogue_backend_instr *rogue_##op(rogue_builder *b);

#define ROGUE_BUILDER_DEFINE_BACKEND02(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref src0,   \
                                   rogue_ref src1);

#define ROGUE_BUILDER_DEFINE_BACKEND11(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref dst0,   \
                                   rogue_ref src0);

#define ROGUE_BUILDER_DEFINE_BACKEND13(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref dst0,   \
                                   rogue_ref src0,   \
                                   rogue_ref src1,   \
                                   rogue_ref src2);

#define ROGUE_BUILDER_DEFINE_BACKEND14(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref dst0,   \
                                   rogue_ref src0,   \
                                   rogue_ref src1,   \
                                   rogue_ref src2,   \
                                   rogue_ref src3);

#define ROGUE_BUILDER_DEFINE_BACKEND06(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref src0,   \
                                   rogue_ref src1,   \
                                   rogue_ref src2,   \
                                   rogue_ref src3,   \
                                   rogue_ref src4,   \
                                   rogue_ref src5);

#define ROGUE_BUILDER_DEFINE_BACKEND16(op)           \
   rogue_backend_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref dst0,   \
                                   rogue_ref src0,   \
                                   rogue_ref src1,   \
                                   rogue_ref src2,   \
                                   rogue_ref src3,   \
                                   rogue_ref src4,   \
                                   rogue_ref src5);

#include "rogue_backend_instrs.def"

/* Ctrl instructions. */
#define ROGUE_BUILDER_DEFINE_CTRLB(op) \
   rogue_ctrl_instr *rogue_##op(rogue_builder *b, rogue_block *block);

#define ROGUE_BUILDER_DEFINE_CTRL00(op) \
   rogue_ctrl_instr *rogue_##op(rogue_builder *b);

#define ROGUE_BUILDER_DEFINE_CTRL01(op) \
   rogue_ctrl_instr *rogue_##op(rogue_builder *b, rogue_ref src0);

#include "rogue_ctrl_instrs.def"

/* Bitwise instructions. */
#define ROGUE_BUILDER_DEFINE_BITWISE22(op)           \
   rogue_bitwise_instr *rogue_##op(rogue_builder *b, \
                                   rogue_ref dst0,   \
                                   rogue_ref dst1,   \
                                   rogue_ref src0,   \
                                   rogue_ref src1);

#include "rogue_bitwise_instrs.def"

#endif /* ROGUE_BUILDER_H */
