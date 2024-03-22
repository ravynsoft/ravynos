/**************************************************************************
 * 
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/* This file contains the vertex shader transformations for SW TCL needed
 * to overcome the limitations of the r300 rasterizer.
 *
 * Transformations:
 * 1) If the secondary color output is present, the primary color must be
 *    present too.
 * 2) If any back-face color output is present, there must be all 4 color
 *    outputs and missing ones must be inserted.
 * 3) Insert a trailing texcoord output containing a copy of POS, for WPOS.
 *
 * I know this code is cumbersome, but I don't know of any nicer way
 * of transforming TGSI shaders. ~ M.
 */

#include "r300_vs.h"

#include <stdio.h>

#include "tgsi/tgsi_transform.h"
#include "tgsi/tgsi_dump.h"

#include "draw/draw_context.h"

struct vs_transform_context {
    struct tgsi_transform_context base;

    bool color_used[2];
    bool bcolor_used[2];

    /* Index of the pos output, typically 0. */
    unsigned pos_output;
    /* Index of the pos temp where all writes of pos are redirected to. */
    unsigned pos_temp;
    /* The index of the last generic output, after which we insert a new
     * output for WPOS. */
    int last_generic;

    unsigned num_outputs;
    /* Used to shift output decl. indices when inserting new ones. */
    unsigned decl_shift;
    /* Used to remap writes to output decls if their indices changed. */
    unsigned out_remap[32];

    /* First instruction processed? */
    bool first_instruction;
    /* End instruction processed? */
    bool end_instruction;

    bool temp_used[1024];
};

static void emit_temp(struct tgsi_transform_context *ctx, unsigned reg)
{
    struct tgsi_full_declaration decl;

    decl = tgsi_default_full_declaration();
    decl.Declaration.File = TGSI_FILE_TEMPORARY;
    decl.Range.First = decl.Range.Last = reg;
    ctx->emit_declaration(ctx, &decl);
}

static void emit_output(struct tgsi_transform_context *ctx,
                        unsigned name, unsigned index, unsigned interp,
                        unsigned reg)
{
    struct vs_transform_context *vsctx = (struct vs_transform_context *)ctx;
    struct tgsi_full_declaration decl;

    decl = tgsi_default_full_declaration();
    decl.Declaration.File = TGSI_FILE_OUTPUT;
    decl.Declaration.Interpolate = 1;
    decl.Declaration.Semantic = true;
    decl.Semantic.Name = name;
    decl.Semantic.Index = index;
    decl.Range.First = decl.Range.Last = reg;
    decl.Interp.Interpolate = interp;
    ctx->emit_declaration(ctx, &decl);
    ++vsctx->num_outputs;
}

static void insert_output_before(struct tgsi_transform_context *ctx,
                                 struct tgsi_full_declaration *before,
                                 unsigned name, unsigned index, unsigned interp)
{
    struct vs_transform_context *vsctx = (struct vs_transform_context *)ctx;
    unsigned i;

    /* Make a place for the new output. */
    for (i = before->Range.First; i < ARRAY_SIZE(vsctx->out_remap); i++) {
        ++vsctx->out_remap[i];
    }

    /* Insert the new output. */
    emit_output(ctx, name, index, interp,
                before->Range.First + vsctx->decl_shift);

    ++vsctx->decl_shift;
}

static void insert_output_after(struct tgsi_transform_context *ctx,
                                struct tgsi_full_declaration *after,
                                unsigned name, unsigned index, unsigned interp)
{
    struct vs_transform_context *vsctx = (struct vs_transform_context *)ctx;
    unsigned i;

    /* Make a place for the new output. */
    for (i = after->Range.First+1; i < ARRAY_SIZE(vsctx->out_remap); i++) {
        ++vsctx->out_remap[i];
    }

    /* Insert the new output. */
    emit_output(ctx, name, index, interp,
                after->Range.First + 1);

    ++vsctx->decl_shift;
}

static void transform_decl(struct tgsi_transform_context *ctx,
                           struct tgsi_full_declaration *decl)
{
    struct vs_transform_context *vsctx = (struct vs_transform_context *)ctx;
    unsigned i;

    if (decl->Declaration.File == TGSI_FILE_OUTPUT) {
        switch (decl->Semantic.Name) {
            case TGSI_SEMANTIC_POSITION:
                vsctx->pos_output = decl->Range.First;
                break;

            case TGSI_SEMANTIC_COLOR:
                assert(decl->Semantic.Index < 2);

                /* We must rasterize the first color if the second one is
                 * used, otherwise the rasterizer doesn't do the color
                 * selection correctly. Declare it, but don't write to it. */
                if (decl->Semantic.Index == 1 && !vsctx->color_used[0]) {
                    insert_output_before(ctx, decl, TGSI_SEMANTIC_COLOR, 0,
                                         TGSI_INTERPOLATE_LINEAR);
                    vsctx->color_used[0] = true;
                }
                break;

            case TGSI_SEMANTIC_BCOLOR:
                assert(decl->Semantic.Index < 2);

                /* We must rasterize all 4 colors if back-face colors are
                 * used, otherwise the rasterizer doesn't do the color
                 * selection correctly. Declare it, but don't write to it. */
                if (!vsctx->color_used[0]) {
                    insert_output_before(ctx, decl, TGSI_SEMANTIC_COLOR, 0,
                                         TGSI_INTERPOLATE_LINEAR);
                    vsctx->color_used[0] = true;
                }
                if (!vsctx->color_used[1]) {
                    insert_output_before(ctx, decl, TGSI_SEMANTIC_COLOR, 1,
                                         TGSI_INTERPOLATE_LINEAR);
                    vsctx->color_used[1] = true;
                }
                if (decl->Semantic.Index == 1 && !vsctx->bcolor_used[0]) {
                    insert_output_before(ctx, decl, TGSI_SEMANTIC_BCOLOR, 0,
                                         TGSI_INTERPOLATE_LINEAR);
                    vsctx->bcolor_used[0] = true;
                }
                break;

            case TGSI_SEMANTIC_GENERIC:
                vsctx->last_generic = MAX2(vsctx->last_generic, decl->Semantic.Index);
                break;
        }

        /* Since we're inserting new outputs in between, the following outputs
         * should be moved to the right so that they don't overlap with
         * the newly added ones. */
        decl->Range.First += vsctx->decl_shift;
        decl->Range.Last += vsctx->decl_shift;

        ++vsctx->num_outputs;
    } else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
        for (i = decl->Range.First; i <= decl->Range.Last; i++) {
           vsctx->temp_used[i] = true;
        }
    }

    ctx->emit_declaration(ctx, decl);

    /* Insert BCOLOR1 if needed. */
    if (decl->Declaration.File == TGSI_FILE_OUTPUT &&
        decl->Semantic.Name == TGSI_SEMANTIC_BCOLOR &&
        !vsctx->bcolor_used[1]) {
        insert_output_after(ctx, decl, TGSI_SEMANTIC_BCOLOR, 1,
                            TGSI_INTERPOLATE_LINEAR);
    }
}

static void transform_inst(struct tgsi_transform_context *ctx,
                           struct tgsi_full_instruction *inst)
{
    struct vs_transform_context *vsctx = (struct vs_transform_context *) ctx;
    struct tgsi_full_instruction new_inst;
    unsigned i;

    if (!vsctx->first_instruction) {
        vsctx->first_instruction = true;

        /* Insert the generic output for WPOS. */
        emit_output(ctx, TGSI_SEMANTIC_GENERIC, vsctx->last_generic + 1,
                    TGSI_INTERPOLATE_PERSPECTIVE, vsctx->num_outputs);

        /* Find a free temp for POSITION. */
        for (i = 0; i < ARRAY_SIZE(vsctx->temp_used); i++) {
            if (!vsctx->temp_used[i]) {
                emit_temp(ctx, i);
                vsctx->pos_temp = i;
                break;
            }
        }
    }

    if (inst->Instruction.Opcode == TGSI_OPCODE_END) {
        /* MOV OUT[pos_output], TEMP[pos_temp]; */
        new_inst = tgsi_default_full_instruction();
        new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
        new_inst.Instruction.NumDstRegs = 1;
        new_inst.Dst[0].Register.File = TGSI_FILE_OUTPUT;
        new_inst.Dst[0].Register.Index = vsctx->pos_output;
        new_inst.Dst[0].Register.WriteMask = TGSI_WRITEMASK_XYZW;
        new_inst.Instruction.NumSrcRegs = 1;
        new_inst.Src[0].Register.File = TGSI_FILE_TEMPORARY;
        new_inst.Src[0].Register.Index = vsctx->pos_temp;
        ctx->emit_instruction(ctx, &new_inst);

        /* MOV OUT[n-1], TEMP[pos_temp]; */
        new_inst = tgsi_default_full_instruction();
        new_inst.Instruction.Opcode = TGSI_OPCODE_MOV;
        new_inst.Instruction.NumDstRegs = 1;
        new_inst.Dst[0].Register.File = TGSI_FILE_OUTPUT;
        new_inst.Dst[0].Register.Index = vsctx->num_outputs - 1;
        new_inst.Dst[0].Register.WriteMask = TGSI_WRITEMASK_XYZW;
        new_inst.Instruction.NumSrcRegs = 1;
        new_inst.Src[0].Register.File = TGSI_FILE_TEMPORARY;
        new_inst.Src[0].Register.Index = vsctx->pos_temp;
        ctx->emit_instruction(ctx, &new_inst);

        vsctx->end_instruction = true;
    } else {
        /* Not an END instruction. */
        /* Fix writes to outputs. */
        for (i = 0; i < inst->Instruction.NumDstRegs; i++) {
            struct tgsi_full_dst_register *dst = &inst->Dst[i];
            if (dst->Register.File == TGSI_FILE_OUTPUT) {
                if (dst->Register.Index == vsctx->pos_output) {
                    /* Replace writes to OUT[pos_output] with TEMP[pos_temp]. */
                    dst->Register.File = TGSI_FILE_TEMPORARY;
                    dst->Register.Index = vsctx->pos_temp;
                } else {
                    /* Not a position, good...
                     * Since we were changing the indices of output decls,
                     * we must redirect writes into them too. */
                    dst->Register.Index = vsctx->out_remap[dst->Register.Index];
                }
            }
        }

        /* Inserting 2 instructions before the END opcode moves all following
         * labels by 2. Subroutines are always after the END opcode so
         * they're always moved. */
        if (inst->Instruction.Opcode == TGSI_OPCODE_CAL) {
            inst->Label.Label += 2;
        }
        /* The labels of the following opcodes are moved only after
         * the END opcode. */
        if (vsctx->end_instruction &&
            (inst->Instruction.Opcode == TGSI_OPCODE_IF ||
             inst->Instruction.Opcode == TGSI_OPCODE_ELSE ||
             inst->Instruction.Opcode == TGSI_OPCODE_BGNLOOP ||
             inst->Instruction.Opcode == TGSI_OPCODE_ENDLOOP)) {
            inst->Label.Label += 2;
        }
    }

    ctx->emit_instruction(ctx, inst);
}

void r300_draw_init_vertex_shader(struct r300_context *r300,
                                  struct r300_vertex_shader *vs)
{
    struct draw_context *draw = r300->draw;
    struct tgsi_shader_info info;
    struct vs_transform_context transform;
    const uint newLen = tgsi_num_tokens(vs->state.tokens) + 100;
    struct pipe_shader_state new_vs = {
        .type = PIPE_SHADER_IR_TGSI,
        .tokens = tgsi_alloc_tokens(newLen)
    };
    unsigned i;

    tgsi_scan_shader(vs->state.tokens, &info);

    memset(&transform, 0, sizeof(transform));
    for (i = 0; i < ARRAY_SIZE(transform.out_remap); i++) {
        transform.out_remap[i] = i;
    }
    transform.last_generic = -1;
    transform.base.transform_instruction = transform_inst;
    transform.base.transform_declaration = transform_decl;

    for (i = 0; i < info.num_outputs; i++) {
        unsigned index = info.output_semantic_index[i];

        switch (info.output_semantic_name[i]) {
            case TGSI_SEMANTIC_COLOR:
                assert(index < 2);
                transform.color_used[index] = true;
                break;

            case TGSI_SEMANTIC_BCOLOR:
                assert(index < 2);
                transform.bcolor_used[index] = true;
                break;
        }
    }

    new_vs.tokens = tgsi_transform_shader(vs->state.tokens, newLen, &transform.base);
    if (!new_vs.tokens)
        return;

#if 0
    printf("----------------------------------------------\norig shader:\n");
    tgsi_dump(vs->state.tokens, 0);
    printf("----------------------------------------------\nnew shader:\n");
    tgsi_dump(new_vs.tokens, 0);
    printf("----------------------------------------------\n");
#endif

    /* Free old tokens. */
    FREE((void*)vs->state.tokens);

    vs->draw_vs = draw_create_vertex_shader(draw, &new_vs);

    /* Instead of duplicating and freeing the tokens, copy the pointer directly. */
    vs->state.tokens = new_vs.tokens;

    /* Init the VS output table for the rasterizer. */
    r300_init_vs_outputs(r300, vs);

    /* Make the last generic be WPOS. */
    vs->shader->outputs.wpos = vs->shader->outputs.generic[transform.last_generic + 1];
    vs->shader->outputs.generic[transform.last_generic + 1] = ATTR_UNUSED;
}
