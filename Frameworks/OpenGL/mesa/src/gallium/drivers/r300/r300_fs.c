/*
 * Copyright 2008 Corbin Simpson <MostAwesomeDude@gmail.com>
 *                Joakim Sindholt <opensource@zhasha.com>
 * Copyright 2009 Marek Olšák <maraeo@gmail.com>
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
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_ureg.h"

#include "r300_cb.h"
#include "r300_context.h"
#include "r300_emit.h"
#include "r300_screen.h"
#include "r300_fs.h"
#include "r300_reg.h"
#include "r300_texture.h"
#include "r300_tgsi_to_rc.h"

#include "compiler/radeon_compiler.h"

/* Convert info about FS input semantics to r300_shader_semantics. */
void r300_shader_read_fs_inputs(struct tgsi_shader_info* info,
                                struct r300_shader_semantics* fs_inputs)
{
    int i;
    unsigned index;

    r300_shader_semantics_reset(fs_inputs);

    for (i = 0; i < info->num_inputs; i++) {
        index = info->input_semantic_index[i];

        switch (info->input_semantic_name[i]) {
            case TGSI_SEMANTIC_COLOR:
                assert(index < ATTR_COLOR_COUNT);
                fs_inputs->color[index] = i;
                break;

            case TGSI_SEMANTIC_PCOORD:
                fs_inputs->pcoord = i;
                break;

            case TGSI_SEMANTIC_TEXCOORD:
                assert(index < ATTR_TEXCOORD_COUNT);
                fs_inputs->texcoord[index] = i;
                fs_inputs->num_texcoord++;
                break;

            case TGSI_SEMANTIC_GENERIC:
                assert(index < ATTR_GENERIC_COUNT);
                fs_inputs->generic[index] = i;
                fs_inputs->num_generic++;
                break;

            case TGSI_SEMANTIC_FOG:
                assert(index == 0);
                fs_inputs->fog = i;
                break;

            case TGSI_SEMANTIC_POSITION:
                assert(index == 0);
                fs_inputs->wpos = i;
                break;

            case TGSI_SEMANTIC_FACE:
                assert(index == 0);
                fs_inputs->face = i;
                break;

            default:
                fprintf(stderr, "r300: FP: Unknown input semantic: %i\n",
                        info->input_semantic_name[i]);
        }
    }
}

static void find_output_registers(struct r300_fragment_program_compiler * compiler,
                                  struct r300_fragment_shader_code *shader)
{
    unsigned i;

    /* Mark the outputs as not present initially */
    compiler->OutputColor[0] = shader->info.num_outputs;
    compiler->OutputColor[1] = shader->info.num_outputs;
    compiler->OutputColor[2] = shader->info.num_outputs;
    compiler->OutputColor[3] = shader->info.num_outputs;
    compiler->OutputDepth = shader->info.num_outputs;

    /* Now see where they really are. */
    for(i = 0; i < shader->info.num_outputs; ++i) {
        switch(shader->info.output_semantic_name[i]) {
            case TGSI_SEMANTIC_COLOR:
                compiler->OutputColor[shader->info.output_semantic_index[i]] = i;
                break;
            case TGSI_SEMANTIC_POSITION:
                compiler->OutputDepth = i;
                break;
        }
    }
}

static void allocate_hardware_inputs(
    struct r300_fragment_program_compiler * c,
    void (*allocate)(void * data, unsigned input, unsigned hwreg),
    void * mydata)
{
    struct r300_shader_semantics* inputs =
        (struct r300_shader_semantics*)c->UserData;
    int i, reg = 0;

    /* Allocate input registers. */
    for (i = 0; i < ATTR_COLOR_COUNT; i++) {
        if (inputs->color[i] != ATTR_UNUSED) {
            allocate(mydata, inputs->color[i], reg++);
        }
    }
    if (inputs->face != ATTR_UNUSED) {
        allocate(mydata, inputs->face, reg++);
    }
    for (i = 0; i < ATTR_GENERIC_COUNT; i++) {
        if (inputs->generic[i] != ATTR_UNUSED) {
            allocate(mydata, inputs->generic[i], reg++);
        }
    }
    for (i = 0; i < ATTR_TEXCOORD_COUNT; i++) {
        if (inputs->texcoord[i] != ATTR_UNUSED) {
            allocate(mydata, inputs->texcoord[i], reg++);
        }
    }
    if (inputs->pcoord != ATTR_UNUSED) {
        allocate(mydata, inputs->pcoord, reg++);
    }
    if (inputs->fog != ATTR_UNUSED) {
        allocate(mydata, inputs->fog, reg++);
    }
    if (inputs->wpos != ATTR_UNUSED) {
        allocate(mydata, inputs->wpos, reg++);
    }
}

void r300_fragment_program_get_external_state(
    struct r300_context* r300,
    struct r300_fragment_program_external_state* state)
{
    struct r300_textures_state *texstate = r300->textures_state.state;
    unsigned i;

    state->alpha_to_one = r300->alpha_to_one && r300->msaa_enable;

    for (i = 0; i < texstate->sampler_state_count; i++) {
        struct r300_sampler_state *s = texstate->sampler_states[i];
        struct r300_sampler_view *v = texstate->sampler_views[i];
        struct r300_resource *t;

        if (!s || !v) {
            continue;
        }

        t = r300_resource(v->base.texture);

        if (s->state.compare_mode == PIPE_TEX_COMPARE_R_TO_TEXTURE) {
            state->unit[i].compare_mode_enabled = 1;

            /* Fortunately, no need to translate this. */
            state->unit[i].texture_compare_func = s->state.compare_func;
        }

        /* Pass texture swizzling to the compiler, some lowering passes need it. */
        if (state->unit[i].compare_mode_enabled) {
            state->unit[i].texture_swizzle =
                RC_MAKE_SWIZZLE(v->swizzle[0], v->swizzle[1],
                                v->swizzle[2], v->swizzle[3]);
        }

        /* XXX this should probably take into account STR, not just S. */
        if (t->tex.is_npot) {
            switch (s->state.wrap_s) {
            case PIPE_TEX_WRAP_REPEAT:
                state->unit[i].wrap_mode = RC_WRAP_REPEAT;
                break;

            case PIPE_TEX_WRAP_MIRROR_REPEAT:
                state->unit[i].wrap_mode = RC_WRAP_MIRRORED_REPEAT;
                break;

            case PIPE_TEX_WRAP_MIRROR_CLAMP:
            case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_EDGE:
            case PIPE_TEX_WRAP_MIRROR_CLAMP_TO_BORDER:
                state->unit[i].wrap_mode = RC_WRAP_MIRRORED_CLAMP;
                break;

            default:
                state->unit[i].wrap_mode = RC_WRAP_NONE;
            }

            if (t->b.target == PIPE_TEXTURE_3D)
                state->unit[i].clamp_and_scale_before_fetch = true;
        }
    }
}

static void r300_translate_fragment_shader(
    struct r300_context* r300,
    struct r300_fragment_shader_code* shader,
    const struct tgsi_token *tokens);

static void r300_dummy_fragment_shader(
    struct r300_context* r300,
    struct r300_fragment_shader_code* shader)
{
    struct pipe_shader_state state;
    struct ureg_program *ureg;
    struct ureg_dst out;
    struct ureg_src imm;

    /* Make a simple fragment shader which outputs (0, 0, 0, 1) */
    ureg = ureg_create(PIPE_SHADER_FRAGMENT);
    out = ureg_DECL_output(ureg, TGSI_SEMANTIC_COLOR, 0);
    imm = ureg_imm4f(ureg, 0, 0, 0, 1);

    ureg_MOV(ureg, out, imm);
    ureg_END(ureg);

    state.tokens = ureg_finalize(ureg);

    shader->dummy = true;
    r300_translate_fragment_shader(r300, shader, state.tokens);

    ureg_destroy(ureg);
}

static void r300_emit_fs_code_to_buffer(
    struct r300_context *r300,
    struct r300_fragment_shader_code *shader)
{
    struct rX00_fragment_program_code *generic_code = &shader->code;
    unsigned imm_count = shader->immediates_count;
    unsigned imm_first = shader->externals_count;
    unsigned imm_end = generic_code->constants.Count;
    struct rc_constant *constants = generic_code->constants.Constants;
    unsigned i;
    CB_LOCALS;

    if (r300->screen->caps.is_r500) {
        struct r500_fragment_program_code *code = &generic_code->code.r500;

        shader->cb_code_size = 19 +
                               ((code->inst_end + 1) * 6) +
                               imm_count * 7 +
                               code->int_constant_count * 2;

        NEW_CB(shader->cb_code, shader->cb_code_size);
        OUT_CB_REG(R500_US_CONFIG, R500_ZERO_TIMES_ANYTHING_EQUALS_ZERO);
        OUT_CB_REG(R500_US_PIXSIZE, code->max_temp_idx);
        OUT_CB_REG(R500_US_FC_CTRL, code->us_fc_ctrl);
        for(i = 0; i < code->int_constant_count; i++){
                OUT_CB_REG(R500_US_FC_INT_CONST_0 + (i * 4),
                                                code->int_constants[i]);
        }
        OUT_CB_REG(R500_US_CODE_RANGE,
                   R500_US_CODE_RANGE_ADDR(0) | R500_US_CODE_RANGE_SIZE(code->inst_end));
        OUT_CB_REG(R500_US_CODE_OFFSET, 0);
        OUT_CB_REG(R500_US_CODE_ADDR,
                   R500_US_CODE_START_ADDR(0) | R500_US_CODE_END_ADDR(code->inst_end));

        OUT_CB_REG(R500_GA_US_VECTOR_INDEX, R500_GA_US_VECTOR_INDEX_TYPE_INSTR);
        OUT_CB_ONE_REG(R500_GA_US_VECTOR_DATA, (code->inst_end + 1) * 6);
        for (i = 0; i <= code->inst_end; i++) {
            OUT_CB(code->inst[i].inst0);
            OUT_CB(code->inst[i].inst1);
            OUT_CB(code->inst[i].inst2);
            OUT_CB(code->inst[i].inst3);
            OUT_CB(code->inst[i].inst4);
            OUT_CB(code->inst[i].inst5);
        }

        /* Emit immediates. */
        if (imm_count) {
            for(i = imm_first; i < imm_end; ++i) {
                if (constants[i].Type == RC_CONSTANT_IMMEDIATE) {
                    const float *data = constants[i].u.Immediate;

                    OUT_CB_REG(R500_GA_US_VECTOR_INDEX,
                               R500_GA_US_VECTOR_INDEX_TYPE_CONST |
                               (i & R500_GA_US_VECTOR_INDEX_MASK));
                    OUT_CB_ONE_REG(R500_GA_US_VECTOR_DATA, 4);
                    OUT_CB_TABLE(data, 4);
                }
            }
        }
    } else { /* r300 */
        struct r300_fragment_program_code *code = &generic_code->code.r300;
        unsigned int alu_length = code->alu.length;
        unsigned int alu_iterations = ((alu_length - 1) / 64) + 1;
        unsigned int tex_length = code->tex.length;
        unsigned int tex_iterations =
            tex_length > 0 ? ((tex_length - 1) / 32) + 1 : 0;
        unsigned int iterations =
            alu_iterations > tex_iterations ? alu_iterations : tex_iterations;
        unsigned int bank = 0;

        shader->cb_code_size = 15 +
            /* R400_US_CODE_BANK */
            (r300->screen->caps.is_r400 ? 2 * (iterations + 1): 0) +
            /* R400_US_CODE_EXT */
            (r300->screen->caps.is_r400 ? 2 : 0) +
            /* R300_US_ALU_{RGB,ALPHA}_{INST,ADDR}_0, R400_US_ALU_EXT_ADDR_0 */
            (code->r390_mode ? (5 * alu_iterations) : 4) +
            /* R400_US_ALU_EXT_ADDR_[0-63] */
            (code->r390_mode ? (code->alu.length) : 0) +
            /* R300_US_ALU_{RGB,ALPHA}_{INST,ADDR}_0 */
            code->alu.length * 4 +
            /* R300_US_TEX_INST_0, R300_US_TEX_INST_[0-31] */
            (code->tex.length > 0 ? code->tex.length + tex_iterations : 0) +
            imm_count * 5;

        NEW_CB(shader->cb_code, shader->cb_code_size);

        OUT_CB_REG(R300_US_CONFIG, code->config);
        OUT_CB_REG(R300_US_PIXSIZE, code->pixsize);
        OUT_CB_REG(R300_US_CODE_OFFSET, code->code_offset);

        if (code->r390_mode) {
            OUT_CB_REG(R400_US_CODE_EXT, code->r400_code_offset_ext);
        } else if (r300->screen->caps.is_r400) {
            /* This register appears to affect shaders even if r390_mode is
             * disabled, so it needs to be set to 0 for shaders that
             * don't use r390_mode. */
            OUT_CB_REG(R400_US_CODE_EXT, 0);
        }

        OUT_CB_REG_SEQ(R300_US_CODE_ADDR_0, 4);
        OUT_CB_TABLE(code->code_addr, 4);

        do {
            unsigned int bank_alu_length = (alu_length < 64 ? alu_length : 64);
            unsigned int bank_alu_offset = bank * 64;
            unsigned int bank_tex_length = (tex_length < 32 ? tex_length : 32);
            unsigned int bank_tex_offset = bank * 32;

            if (r300->screen->caps.is_r400) {
                OUT_CB_REG(R400_US_CODE_BANK, code->r390_mode ?
                                (bank << R400_BANK_SHIFT) | R400_R390_MODE_ENABLE : 0);//2
            }

            if (bank_alu_length > 0) {
                OUT_CB_REG_SEQ(R300_US_ALU_RGB_INST_0, bank_alu_length);
                for (i = 0; i < bank_alu_length; i++)
                    OUT_CB(code->alu.inst[i + bank_alu_offset].rgb_inst);

                OUT_CB_REG_SEQ(R300_US_ALU_RGB_ADDR_0, bank_alu_length);
                for (i = 0; i < bank_alu_length; i++)
                    OUT_CB(code->alu.inst[i + bank_alu_offset].rgb_addr);

                OUT_CB_REG_SEQ(R300_US_ALU_ALPHA_INST_0, bank_alu_length);
                for (i = 0; i < bank_alu_length; i++)
                    OUT_CB(code->alu.inst[i + bank_alu_offset].alpha_inst);

                OUT_CB_REG_SEQ(R300_US_ALU_ALPHA_ADDR_0, bank_alu_length);
                for (i = 0; i < bank_alu_length; i++)
                    OUT_CB(code->alu.inst[i + bank_alu_offset].alpha_addr);

                if (code->r390_mode) {
                    OUT_CB_REG_SEQ(R400_US_ALU_EXT_ADDR_0, bank_alu_length);
                    for (i = 0; i < bank_alu_length; i++)
                        OUT_CB(code->alu.inst[i + bank_alu_offset].r400_ext_addr);
                }
            }

            if (bank_tex_length > 0) {
                OUT_CB_REG_SEQ(R300_US_TEX_INST_0, bank_tex_length);
                OUT_CB_TABLE(code->tex.inst + bank_tex_offset, bank_tex_length);
            }

            alu_length -= bank_alu_length;
            tex_length -= bank_tex_length;
            bank++;
        } while(code->r390_mode && (alu_length > 0 || tex_length > 0));

        /* R400_US_CODE_BANK needs to be reset to 0, otherwise some shaders
         * will be rendered incorrectly. */
        if (r300->screen->caps.is_r400) {
            OUT_CB_REG(R400_US_CODE_BANK,
                code->r390_mode ? R400_R390_MODE_ENABLE : 0);
        }

        /* Emit immediates. */
        if (imm_count) {
            for(i = imm_first; i < imm_end; ++i) {
                if (constants[i].Type == RC_CONSTANT_IMMEDIATE) {
                    const float *data = constants[i].u.Immediate;

                    OUT_CB_REG_SEQ(R300_PFS_PARAM_0_X + i * 16, 4);
                    OUT_CB(pack_float24(data[0]));
                    OUT_CB(pack_float24(data[1]));
                    OUT_CB(pack_float24(data[2]));
                    OUT_CB(pack_float24(data[3]));
                }
            }
        }
    }

    OUT_CB_REG(R300_FG_DEPTH_SRC, shader->fg_depth_src);
    OUT_CB_REG(R300_US_W_FMT, shader->us_out_w);
    END_CB;
}

static void r300_translate_fragment_shader(
    struct r300_context* r300,
    struct r300_fragment_shader_code* shader,
    const struct tgsi_token *tokens)
{
    struct r300_fragment_program_compiler compiler;
    struct tgsi_to_rc ttr;
    int wpos, face;
    unsigned i;

    tgsi_scan_shader(tokens, &shader->info);
    r300_shader_read_fs_inputs(&shader->info, &shader->inputs);

    wpos = shader->inputs.wpos;
    face = shader->inputs.face;

    /* Setup the compiler. */
    memset(&compiler, 0, sizeof(compiler));
    rc_init(&compiler.Base, &r300->fs_regalloc_state);
    DBG_ON(r300, DBG_FP) ? compiler.Base.Debug |= RC_DBG_LOG : 0;

    compiler.code = &shader->code;
    compiler.state = shader->compare_state;
    if (!shader->dummy)
        compiler.Base.debug = &r300->context.debug;
    compiler.Base.is_r500 = r300->screen->caps.is_r500;
    compiler.Base.is_r400 = r300->screen->caps.is_r400;
    compiler.Base.disable_optimizations = DBG_ON(r300, DBG_NO_OPT);
    compiler.Base.has_half_swizzles = true;
    compiler.Base.has_presub = true;
    compiler.Base.has_omod = true;
    compiler.Base.max_temp_regs =
        compiler.Base.is_r500 ? 128 : (compiler.Base.is_r400 ? 64 : 32);
    compiler.Base.max_constants = compiler.Base.is_r500 ? 256 : 32;
    compiler.Base.max_alu_insts =
        (compiler.Base.is_r500 || compiler.Base.is_r400) ? 512 : 64;
    compiler.Base.max_tex_insts =
        (compiler.Base.is_r500 || compiler.Base.is_r400) ? 512 : 32;
    compiler.AllocateHwInputs = &allocate_hardware_inputs;
    compiler.UserData = &shader->inputs;

    find_output_registers(&compiler, shader);

    shader->write_all =
          shader->info.properties[TGSI_PROPERTY_FS_COLOR0_WRITES_ALL_CBUFS];

    if (compiler.Base.Debug & RC_DBG_LOG) {
        DBG(r300, DBG_FP, "r300: Initial fragment program\n");
        tgsi_dump(tokens, 0);
    }

    /* Translate TGSI to our internal representation */
    ttr.compiler = &compiler.Base;
    ttr.info = &shader->info;

    r300_tgsi_to_rc(&ttr, tokens);

    if (ttr.error) {
        fprintf(stderr, "r300 FP: Cannot translate a shader. "
                "Using a dummy shader instead.\n");
        r300_dummy_fragment_shader(r300, shader);
        return;
    }

    if (!r300->screen->caps.is_r500 ||
        compiler.Base.Program.Constants.Count > 200) {
        compiler.Base.remove_unused_constants = true;
    }

    /**
     * Transform the program to support WPOS.
     *
     * Introduce a small fragment at the start of the program that will be
     * the only code that directly reads the WPOS input.
     * All other code pieces that reference that input will be rewritten
     * to read from a newly allocated temporary. */
    if (wpos != ATTR_UNUSED) {
        /* Moving the input to some other reg is not really necessary. */
        rc_transform_fragment_wpos(&compiler.Base, wpos, wpos, true);
    }

    if (face != ATTR_UNUSED) {
        rc_transform_fragment_face(&compiler.Base, face);
    }

    /* Invoke the compiler */
    r3xx_compile_fragment_program(&compiler);

    if (compiler.Base.Error) {
        fprintf(stderr, "r300 FP: Compiler Error:\n%sUsing a dummy shader"
                " instead.\n", compiler.Base.ErrorMsg);

        if (shader->dummy) {
            fprintf(stderr, "r300 FP: Cannot compile the dummy shader! "
                    "Giving up...\n");
            abort();
        }

        free(compiler.code->constants.Constants);
        rc_destroy(&compiler.Base);
        r300_dummy_fragment_shader(r300, shader);
        return;
    }

    /* Shaders with zero instructions are invalid,
     * use the dummy shader instead. */
    if (shader->code.code.r500.inst_end == -1) {
        rc_destroy(&compiler.Base);
        r300_dummy_fragment_shader(r300, shader);
        return;
    }

    /* Initialize numbers of constants for each type. */
    shader->externals_count = 0;
    for (i = 0;
         i < shader->code.constants.Count &&
         shader->code.constants.Constants[i].Type == RC_CONSTANT_EXTERNAL; i++) {
        shader->externals_count = i+1;
    }
    shader->immediates_count = 0;
    shader->rc_state_count = 0;

    for (i = shader->externals_count; i < shader->code.constants.Count; i++) {
        switch (shader->code.constants.Constants[i].Type) {
            case RC_CONSTANT_IMMEDIATE:
                ++shader->immediates_count;
                break;
            case RC_CONSTANT_STATE:
                ++shader->rc_state_count;
                break;
            default:
                assert(0);
        }
    }

    /* Setup shader depth output. */
    if (shader->code.writes_depth) {
        shader->fg_depth_src = R300_FG_DEPTH_SRC_SHADER;
        shader->us_out_w = R300_W_FMT_W24 | R300_W_SRC_US;
    } else {
        shader->fg_depth_src = R300_FG_DEPTH_SRC_SCAN;
        shader->us_out_w = R300_W_FMT_W0 | R300_W_SRC_US;
    }

    /* And, finally... */
    rc_destroy(&compiler.Base);

    /* Build the command buffer. */
    r300_emit_fs_code_to_buffer(r300, shader);
}

bool r300_pick_fragment_shader(struct r300_context *r300,
                               struct r300_fragment_shader* fs,
                               struct r300_fragment_program_external_state *state)
{
    struct r300_fragment_shader_code* ptr;

    if (!fs->first) {
        /* Build the fragment shader for the first time. */
        fs->first = fs->shader = CALLOC_STRUCT(r300_fragment_shader_code);

        memcpy(&fs->shader->compare_state, state, sizeof(*state));
        r300_translate_fragment_shader(r300, fs->shader, fs->state.tokens);
        return true;

    } else {
        /* Check if the currently-bound shader has been compiled
         * with the texture-compare state we need. */
        if (memcmp(&fs->shader->compare_state, state, sizeof(*state)) != 0) {
            /* Search for the right shader. */
            ptr = fs->first;
            while (ptr) {
                if (memcmp(&ptr->compare_state, state, sizeof(*state)) == 0) {
                    if (fs->shader != ptr) {
                        fs->shader = ptr;
                        return true;
                    }
                    /* The currently-bound one is OK. */
                    return false;
                }
                ptr = ptr->next;
            }

            /* Not found, gotta compile a new one. */
            ptr = CALLOC_STRUCT(r300_fragment_shader_code);
            ptr->next = fs->first;
            fs->first = fs->shader = ptr;

            memcpy(&ptr->compare_state, state, sizeof(*state));
            r300_translate_fragment_shader(r300, ptr, fs->state.tokens);
            return true;
        }
    }

    return false;
}
