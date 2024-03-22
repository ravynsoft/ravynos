/*
 * Copyright 2009 Nicolai Hähnle <nhaehnle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <stdio.h>
#include "r300_tgsi_to_rc.h"

#include "compiler/radeon_compiler.h"

#include "tgsi/tgsi_info.h"
#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_util.h"

#include "util/compiler.h"

static unsigned translate_opcode(unsigned opcode)
{
    switch(opcode) {
        case TGSI_OPCODE_ARL: return RC_OPCODE_ARL;
        case TGSI_OPCODE_MOV: return RC_OPCODE_MOV;
        case TGSI_OPCODE_LIT: return RC_OPCODE_LIT;
        case TGSI_OPCODE_RCP: return RC_OPCODE_RCP;
        case TGSI_OPCODE_RSQ: return RC_OPCODE_RSQ;
        case TGSI_OPCODE_EXP: return RC_OPCODE_EXP;
        case TGSI_OPCODE_LOG: return RC_OPCODE_LOG;
        case TGSI_OPCODE_MUL: return RC_OPCODE_MUL;
        case TGSI_OPCODE_ADD: return RC_OPCODE_ADD;
        case TGSI_OPCODE_DP3: return RC_OPCODE_DP3;
        case TGSI_OPCODE_DP4: return RC_OPCODE_DP4;
        case TGSI_OPCODE_DST: return RC_OPCODE_DST;
        case TGSI_OPCODE_MIN: return RC_OPCODE_MIN;
        case TGSI_OPCODE_MAX: return RC_OPCODE_MAX;
        case TGSI_OPCODE_SLT: return RC_OPCODE_SLT;
        case TGSI_OPCODE_SGE: return RC_OPCODE_SGE;
        case TGSI_OPCODE_MAD: return RC_OPCODE_MAD;
        case TGSI_OPCODE_FRC: return RC_OPCODE_FRC;
        case TGSI_OPCODE_ROUND: return RC_OPCODE_ROUND;
        case TGSI_OPCODE_EX2: return RC_OPCODE_EX2;
        case TGSI_OPCODE_LG2: return RC_OPCODE_LG2;
        case TGSI_OPCODE_POW: return RC_OPCODE_POW;
        case TGSI_OPCODE_COS: return RC_OPCODE_COS;
        case TGSI_OPCODE_DDX: return RC_OPCODE_DDX;
        case TGSI_OPCODE_DDY: return RC_OPCODE_DDY;
        case TGSI_OPCODE_KILL: return RC_OPCODE_KILP;
        case TGSI_OPCODE_SEQ: return RC_OPCODE_SEQ;
        case TGSI_OPCODE_SGT: return RC_OPCODE_SGT;
        case TGSI_OPCODE_SIN: return RC_OPCODE_SIN;
        case TGSI_OPCODE_SLE: return RC_OPCODE_SLE;
        case TGSI_OPCODE_SNE: return RC_OPCODE_SNE;
        case TGSI_OPCODE_TEX: return RC_OPCODE_TEX;
        case TGSI_OPCODE_TXD: return RC_OPCODE_TXD;
        case TGSI_OPCODE_TXP: return RC_OPCODE_TXP;
        case TGSI_OPCODE_ARR: return RC_OPCODE_ARR;
        case TGSI_OPCODE_CMP: return RC_OPCODE_CMP;
        case TGSI_OPCODE_TXB: return RC_OPCODE_TXB;
        case TGSI_OPCODE_DP2: return RC_OPCODE_DP2;
        case TGSI_OPCODE_TXL: return RC_OPCODE_TXL;
        case TGSI_OPCODE_BRK: return RC_OPCODE_BRK;
        case TGSI_OPCODE_IF: return RC_OPCODE_IF;
        case TGSI_OPCODE_BGNLOOP: return RC_OPCODE_BGNLOOP;
        case TGSI_OPCODE_ELSE: return RC_OPCODE_ELSE;
        case TGSI_OPCODE_ENDIF: return RC_OPCODE_ENDIF;
        case TGSI_OPCODE_ENDLOOP: return RC_OPCODE_ENDLOOP;
        case TGSI_OPCODE_CONT: return RC_OPCODE_CONT;
        case TGSI_OPCODE_NOP: return RC_OPCODE_NOP;
        case TGSI_OPCODE_KILL_IF: return RC_OPCODE_KIL;
    }

    fprintf(stderr, "r300: Unknown TGSI/RC opcode: %s\n", tgsi_get_opcode_name(opcode));
    return RC_OPCODE_ILLEGAL_OPCODE;
}

static unsigned translate_saturate(unsigned saturate)
{
    return saturate ? RC_SATURATE_ZERO_ONE : RC_SATURATE_NONE;
}

static unsigned translate_register_file(unsigned file)
{
    switch(file) {
        case TGSI_FILE_CONSTANT: return RC_FILE_CONSTANT;
        case TGSI_FILE_IMMEDIATE: return RC_FILE_CONSTANT;
        case TGSI_FILE_INPUT: return RC_FILE_INPUT;
        case TGSI_FILE_OUTPUT: return RC_FILE_OUTPUT;
        default:
            fprintf(stderr, "Unhandled register file: %i\n", file);
            FALLTHROUGH;
        case TGSI_FILE_TEMPORARY: return RC_FILE_TEMPORARY;
        case TGSI_FILE_ADDRESS: return RC_FILE_ADDRESS;
    }
}

static int translate_register_index(
    struct tgsi_to_rc * ttr,
    unsigned file,
    int index)
{
    if (file == TGSI_FILE_IMMEDIATE)
        return ttr->immediate_offset + index;

    return index;
}

static void transform_dstreg(
    struct tgsi_to_rc * ttr,
    struct rc_dst_register * dst,
    struct tgsi_full_dst_register * src)
{
    dst->File = translate_register_file(src->Register.File);
    dst->Index = translate_register_index(ttr, src->Register.File, src->Register.Index);
    dst->WriteMask = src->Register.WriteMask;

    if (src->Register.Indirect) {
        ttr->error = true;
        fprintf(stderr, "r300: Relative addressing of destination operands "
                "is unsupported.\n");
    }
}

static void transform_srcreg(
    struct tgsi_to_rc * ttr,
    struct rc_src_register * dst,
    struct tgsi_full_src_register * src)
{
    dst->File = translate_register_file(src->Register.File);
    int index = translate_register_index(ttr, src->Register.File, src->Register.Index);
    /* Negative offsets to relative addressing should have been lowered in NIR */
    assert(index >= 0);
    /* Also check for overflow */
    if (index >= RC_REGISTER_MAX_INDEX) {
        ttr->error = true;
        fprintf(stderr, "r300: Register index too high.\n");
    }
    dst->Index = index;
    dst->RelAddr = src->Register.Indirect;
    dst->Swizzle = tgsi_util_get_full_src_register_swizzle(src, 0);
    dst->Swizzle |= tgsi_util_get_full_src_register_swizzle(src, 1) << 3;
    dst->Swizzle |= tgsi_util_get_full_src_register_swizzle(src, 2) << 6;
    dst->Swizzle |= tgsi_util_get_full_src_register_swizzle(src, 3) << 9;
    dst->Abs = src->Register.Absolute;
    dst->Negate = src->Register.Negate ? RC_MASK_XYZW : 0;
}

static void transform_texture(struct rc_instruction * dst, struct tgsi_instruction_texture src,
                              uint32_t *shadowSamplers)
{
    switch(src.Texture) {
        case TGSI_TEXTURE_1D:
            dst->U.I.TexSrcTarget = RC_TEXTURE_1D;
            break;
        case TGSI_TEXTURE_2D:
            dst->U.I.TexSrcTarget = RC_TEXTURE_2D;
            break;
        case TGSI_TEXTURE_3D:
            dst->U.I.TexSrcTarget = RC_TEXTURE_3D;
            break;
        case TGSI_TEXTURE_CUBE:
            dst->U.I.TexSrcTarget = RC_TEXTURE_CUBE;
            break;
        case TGSI_TEXTURE_RECT:
            dst->U.I.TexSrcTarget = RC_TEXTURE_RECT;
            break;
        case TGSI_TEXTURE_SHADOW1D:
            dst->U.I.TexSrcTarget = RC_TEXTURE_1D;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
        case TGSI_TEXTURE_SHADOW2D:
            dst->U.I.TexSrcTarget = RC_TEXTURE_2D;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
        case TGSI_TEXTURE_SHADOWRECT:
            dst->U.I.TexSrcTarget = RC_TEXTURE_RECT;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
        case TGSI_TEXTURE_1D_ARRAY:
            dst->U.I.TexSrcTarget = RC_TEXTURE_1D_ARRAY;
            break;
        case TGSI_TEXTURE_2D_ARRAY:
            dst->U.I.TexSrcTarget = RC_TEXTURE_2D_ARRAY;
            break;
        case TGSI_TEXTURE_SHADOW1D_ARRAY:
            dst->U.I.TexSrcTarget = RC_TEXTURE_1D_ARRAY;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
        case TGSI_TEXTURE_SHADOW2D_ARRAY:
            dst->U.I.TexSrcTarget = RC_TEXTURE_2D_ARRAY;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
        case TGSI_TEXTURE_SHADOWCUBE:
            dst->U.I.TexSrcTarget = RC_TEXTURE_CUBE;
            dst->U.I.TexShadow = 1;
            *shadowSamplers |= 1U << dst->U.I.TexSrcUnit;
            break;
    }
    dst->U.I.TexSwizzle = RC_SWIZZLE_XYZW;
}

static void transform_instruction(struct tgsi_to_rc * ttr, struct tgsi_full_instruction * src)
{
    struct rc_instruction * dst;
    int i;

    dst = rc_insert_new_instruction(ttr->compiler, ttr->compiler->Program.Instructions.Prev);
    dst->U.I.Opcode = translate_opcode(src->Instruction.Opcode);
    if (!ttr->compiler->is_r500 && dst->U.I.Opcode == RC_OPCODE_BGNLOOP && ttr->error == false) {
        ttr->error = true;
        fprintf(stderr, "r300: Dynamic loops are not supported on R3xx/R4xx.\n");
    }
    if (!ttr->compiler->is_r500 && dst->U.I.Opcode == RC_OPCODE_IF && ttr->error == false) {
        ttr->error = true;
        fprintf(stderr, "r300: Branches are not supported on R3xx/R4xx.\n");
    }

    dst->U.I.SaturateMode = translate_saturate(src->Instruction.Saturate);

    if (src->Instruction.NumDstRegs)
        transform_dstreg(ttr, &dst->U.I.DstReg, &src->Dst[0]);

    for(i = 0; i < src->Instruction.NumSrcRegs; ++i) {
        if (src->Src[i].Register.File == TGSI_FILE_SAMPLER)
            dst->U.I.TexSrcUnit = src->Src[i].Register.Index;
        else
            transform_srcreg(ttr, &dst->U.I.SrcReg[i], &src->Src[i]);
    }

    /* Texturing. */
    if (src->Instruction.Texture)
        transform_texture(dst, src->Texture,
                          &ttr->compiler->Program.ShadowSamplers);
}

static void handle_immediate(struct tgsi_to_rc * ttr,
                             struct tgsi_full_immediate * imm,
                             unsigned index)
{
    struct rc_constant constant;

    constant.Type = RC_CONSTANT_IMMEDIATE;
    constant.Size = 4;
    for (unsigned i = 0; i < 4; ++i)
        constant.u.Immediate[i] = imm->u[i].Float;
    rc_constants_add(&ttr->compiler->Program.Constants, &constant);
}

void r300_tgsi_to_rc(struct tgsi_to_rc * ttr,
                     const struct tgsi_token * tokens)
{
    struct tgsi_full_instruction *inst;
    struct tgsi_parse_context parser;
    unsigned imm_index = 0;
    int i;

    ttr->error = false;

    /* Allocate constants placeholders.
     *
     * Note: What if declared constants are not contiguous? */
    for(i = 0; i <= ttr->info->file_max[TGSI_FILE_CONSTANT]; ++i) {
        struct rc_constant constant;
        memset(&constant, 0, sizeof(constant));
        constant.Type = RC_CONSTANT_EXTERNAL;
        constant.Size = 4;
        constant.u.External = i;
        rc_constants_add(&ttr->compiler->Program.Constants, &constant);
    }

    ttr->immediate_offset = ttr->compiler->Program.Constants.Count;

    tgsi_parse_init(&parser, tokens);

    while (!tgsi_parse_end_of_tokens(&parser)) {
        tgsi_parse_token(&parser);

        switch (parser.FullToken.Token.Type) {
            case TGSI_TOKEN_TYPE_DECLARATION:
                break;
            case TGSI_TOKEN_TYPE_IMMEDIATE:
                handle_immediate(ttr, &parser.FullToken.FullImmediate, imm_index);
                imm_index++;
                break;
            case TGSI_TOKEN_TYPE_INSTRUCTION:
                inst = &parser.FullToken.FullInstruction;
                if (inst->Instruction.Opcode == TGSI_OPCODE_END) {
                    break;
                }

                transform_instruction(ttr, inst);
                break;
        }
    }

    tgsi_parse_free(&parser);

    rc_calculate_inputs_outputs(ttr->compiler);
}
