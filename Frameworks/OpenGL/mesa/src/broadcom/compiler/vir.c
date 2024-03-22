/*
 * Copyright © 2016-2017 Broadcom
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

#include "broadcom/common/v3d_device_info.h"
#include "v3d_compiler.h"
#include "compiler/nir/nir_schedule.h"
#include "compiler/nir/nir_builder.h"

int
vir_get_nsrc(struct qinst *inst)
{
        switch (inst->qpu.type) {
        case V3D_QPU_INSTR_TYPE_BRANCH:
                return 0;
        case V3D_QPU_INSTR_TYPE_ALU:
                if (inst->qpu.alu.add.op != V3D_QPU_A_NOP)
                        return v3d_qpu_add_op_num_src(inst->qpu.alu.add.op);
                else
                        return v3d_qpu_mul_op_num_src(inst->qpu.alu.mul.op);
        }

        return 0;
}

/**
 * Returns whether the instruction has any side effects that must be
 * preserved.
 */
bool
vir_has_side_effects(struct v3d_compile *c, struct qinst *inst)
{
        switch (inst->qpu.type) {
        case V3D_QPU_INSTR_TYPE_BRANCH:
                return true;
        case V3D_QPU_INSTR_TYPE_ALU:
                switch (inst->qpu.alu.add.op) {
                case V3D_QPU_A_SETREVF:
                case V3D_QPU_A_SETMSF:
                case V3D_QPU_A_VPMSETUP:
                case V3D_QPU_A_STVPMV:
                case V3D_QPU_A_STVPMD:
                case V3D_QPU_A_STVPMP:
                case V3D_QPU_A_VPMWT:
                case V3D_QPU_A_TMUWT:
                        return true;
                default:
                        break;
                }

                switch (inst->qpu.alu.mul.op) {
                case V3D_QPU_M_MULTOP:
                        return true;
                default:
                        break;
                }
        }

        if (inst->qpu.sig.ldtmu ||
            inst->qpu.sig.ldvary ||
            inst->qpu.sig.ldtlbu ||
            inst->qpu.sig.ldtlb ||
            inst->qpu.sig.wrtmuc ||
            inst->qpu.sig.thrsw) {
                return true;
        }

        /* ldunifa works like ldunif: it reads an element and advances the
         * pointer, so each read has a side effect (we don't care for ldunif
         * because we reconstruct the uniform stream buffer after compiling
         * with the surviving uniforms), so allowing DCE to remove
         * one would break follow-up loads. We could fix this by emitting a
         * unifa for each ldunifa, but each unifa requires 3 delay slots
         * before a ldunifa, so that would be quite expensive.
         */
        if (inst->qpu.sig.ldunifa || inst->qpu.sig.ldunifarf)
                return true;

        return false;
}

bool
vir_is_raw_mov(struct qinst *inst)
{
        if (inst->qpu.type != V3D_QPU_INSTR_TYPE_ALU ||
            (inst->qpu.alu.mul.op != V3D_QPU_M_FMOV &&
             inst->qpu.alu.mul.op != V3D_QPU_M_MOV)) {
                return false;
        }

        if (inst->qpu.alu.add.output_pack != V3D_QPU_PACK_NONE ||
            inst->qpu.alu.mul.output_pack != V3D_QPU_PACK_NONE) {
                return false;
        }

        if (inst->qpu.alu.add.a.unpack != V3D_QPU_UNPACK_NONE ||
            inst->qpu.alu.add.b.unpack != V3D_QPU_UNPACK_NONE ||
            inst->qpu.alu.mul.a.unpack != V3D_QPU_UNPACK_NONE ||
            inst->qpu.alu.mul.b.unpack != V3D_QPU_UNPACK_NONE) {
                return false;
        }

        if (inst->qpu.flags.ac != V3D_QPU_COND_NONE ||
            inst->qpu.flags.mc != V3D_QPU_COND_NONE)
                return false;

        return true;
}

bool
vir_is_add(struct qinst *inst)
{
        return (inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
                inst->qpu.alu.add.op != V3D_QPU_A_NOP);
}

bool
vir_is_mul(struct qinst *inst)
{
        return (inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
                inst->qpu.alu.mul.op != V3D_QPU_M_NOP);
}

bool
vir_is_tex(const struct v3d_device_info *devinfo, struct qinst *inst)
{
        if (inst->dst.file == QFILE_MAGIC)
                return v3d_qpu_magic_waddr_is_tmu(devinfo, inst->dst.index);

        if (inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU &&
            inst->qpu.alu.add.op == V3D_QPU_A_TMUWT) {
                return true;
        }

        return false;
}

bool
vir_writes_r4_implicitly(const struct v3d_device_info *devinfo,
                         struct qinst *inst)
{
        if (!devinfo->has_accumulators)
                return false;

        switch (inst->dst.file) {
        case QFILE_MAGIC:
                switch (inst->dst.index) {
                case V3D_QPU_WADDR_RECIP:
                case V3D_QPU_WADDR_RSQRT:
                case V3D_QPU_WADDR_EXP:
                case V3D_QPU_WADDR_LOG:
                case V3D_QPU_WADDR_SIN:
                        return true;
                }
                break;
        default:
                break;
        }

        return false;
}

void
vir_set_unpack(struct qinst *inst, int src,
               enum v3d_qpu_input_unpack unpack)
{
        assert(src == 0 || src == 1);

        if (vir_is_add(inst)) {
                if (src == 0)
                        inst->qpu.alu.add.a.unpack = unpack;
                else
                        inst->qpu.alu.add.b.unpack = unpack;
        } else {
                assert(vir_is_mul(inst));
                if (src == 0)
                        inst->qpu.alu.mul.a.unpack = unpack;
                else
                        inst->qpu.alu.mul.b.unpack = unpack;
        }
}

void
vir_set_pack(struct qinst *inst, enum v3d_qpu_output_pack pack)
{
        if (vir_is_add(inst)) {
                inst->qpu.alu.add.output_pack = pack;
        } else {
                assert(vir_is_mul(inst));
                inst->qpu.alu.mul.output_pack = pack;
        }
}

void
vir_set_cond(struct qinst *inst, enum v3d_qpu_cond cond)
{
        if (vir_is_add(inst)) {
                inst->qpu.flags.ac = cond;
        } else {
                assert(vir_is_mul(inst));
                inst->qpu.flags.mc = cond;
        }
}

enum v3d_qpu_cond
vir_get_cond(struct qinst *inst)
{
        assert(inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU);

        if (vir_is_add(inst))
                return inst->qpu.flags.ac;
        else if (vir_is_mul(inst))
                return inst->qpu.flags.mc;
        else /* NOP */
                return V3D_QPU_COND_NONE;
}

void
vir_set_pf(struct v3d_compile *c, struct qinst *inst, enum v3d_qpu_pf pf)
{
        c->flags_temp = -1;
        if (vir_is_add(inst)) {
                inst->qpu.flags.apf = pf;
        } else {
                assert(vir_is_mul(inst));
                inst->qpu.flags.mpf = pf;
        }
}

void
vir_set_uf(struct v3d_compile *c, struct qinst *inst, enum v3d_qpu_uf uf)
{
        c->flags_temp = -1;
        if (vir_is_add(inst)) {
                inst->qpu.flags.auf = uf;
        } else {
                assert(vir_is_mul(inst));
                inst->qpu.flags.muf = uf;
        }
}

#if 0
uint8_t
vir_channels_written(struct qinst *inst)
{
        if (vir_is_mul(inst)) {
                switch (inst->dst.pack) {
                case QPU_PACK_MUL_NOP:
                case QPU_PACK_MUL_8888:
                        return 0xf;
                case QPU_PACK_MUL_8A:
                        return 0x1;
                case QPU_PACK_MUL_8B:
                        return 0x2;
                case QPU_PACK_MUL_8C:
                        return 0x4;
                case QPU_PACK_MUL_8D:
                        return 0x8;
                }
        } else {
                switch (inst->dst.pack) {
                case QPU_PACK_A_NOP:
                case QPU_PACK_A_8888:
                case QPU_PACK_A_8888_SAT:
                case QPU_PACK_A_32_SAT:
                        return 0xf;
                case QPU_PACK_A_8A:
                case QPU_PACK_A_8A_SAT:
                        return 0x1;
                case QPU_PACK_A_8B:
                case QPU_PACK_A_8B_SAT:
                        return 0x2;
                case QPU_PACK_A_8C:
                case QPU_PACK_A_8C_SAT:
                        return 0x4;
                case QPU_PACK_A_8D:
                case QPU_PACK_A_8D_SAT:
                        return 0x8;
                case QPU_PACK_A_16A:
                case QPU_PACK_A_16A_SAT:
                        return 0x3;
                case QPU_PACK_A_16B:
                case QPU_PACK_A_16B_SAT:
                        return 0xc;
                }
        }
        unreachable("Bad pack field");
}
#endif

struct qreg
vir_get_temp(struct v3d_compile *c)
{
        struct qreg reg;

        reg.file = QFILE_TEMP;
        reg.index = c->num_temps++;

        if (c->num_temps > c->defs_array_size) {
                uint32_t old_size = c->defs_array_size;
                c->defs_array_size = MAX2(old_size * 2, 16);

                c->defs = reralloc(c, c->defs, struct qinst *,
                                   c->defs_array_size);
                memset(&c->defs[old_size], 0,
                       sizeof(c->defs[0]) * (c->defs_array_size - old_size));

                c->spillable = reralloc(c, c->spillable,
                                        BITSET_WORD,
                                        BITSET_WORDS(c->defs_array_size));
                for (int i = old_size; i < c->defs_array_size; i++)
                        BITSET_SET(c->spillable, i);
        }

        return reg;
}

struct qinst *
vir_add_inst(enum v3d_qpu_add_op op, struct qreg dst, struct qreg src0, struct qreg src1)
{
        struct qinst *inst = calloc(1, sizeof(*inst));

        inst->qpu = v3d_qpu_nop();
        inst->qpu.alu.add.op = op;

        inst->dst = dst;
        inst->src[0] = src0;
        inst->src[1] = src1;
        inst->uniform = ~0;

        inst->ip = -1;

        return inst;
}

struct qinst *
vir_mul_inst(enum v3d_qpu_mul_op op, struct qreg dst, struct qreg src0, struct qreg src1)
{
        struct qinst *inst = calloc(1, sizeof(*inst));

        inst->qpu = v3d_qpu_nop();
        inst->qpu.alu.mul.op = op;

        inst->dst = dst;
        inst->src[0] = src0;
        inst->src[1] = src1;
        inst->uniform = ~0;

        inst->ip = -1;

        return inst;
}

struct qinst *
vir_branch_inst(struct v3d_compile *c, enum v3d_qpu_branch_cond cond)
{
        struct qinst *inst = calloc(1, sizeof(*inst));

        inst->qpu = v3d_qpu_nop();
        inst->qpu.type = V3D_QPU_INSTR_TYPE_BRANCH;
        inst->qpu.branch.cond = cond;
        inst->qpu.branch.msfign = V3D_QPU_MSFIGN_NONE;
        inst->qpu.branch.bdi = V3D_QPU_BRANCH_DEST_REL;
        inst->qpu.branch.ub = true;
        inst->qpu.branch.bdu = V3D_QPU_BRANCH_DEST_REL;

        inst->dst = vir_nop_reg();
        inst->uniform = vir_get_uniform_index(c, QUNIFORM_CONSTANT, 0);

        inst->ip = -1;

        return inst;
}

static void
vir_emit(struct v3d_compile *c, struct qinst *inst)
{
        inst->ip = -1;

        switch (c->cursor.mode) {
        case vir_cursor_add:
                list_add(&inst->link, c->cursor.link);
                break;
        case vir_cursor_addtail:
                list_addtail(&inst->link, c->cursor.link);
                break;
        }

        c->cursor = vir_after_inst(inst);
        c->live_intervals_valid = false;
}

/* Updates inst to write to a new temporary, emits it, and notes the def. */
struct qreg
vir_emit_def(struct v3d_compile *c, struct qinst *inst)
{
        assert(inst->dst.file == QFILE_NULL);

        /* If we're emitting an instruction that's a def, it had better be
         * writing a register.
         */
        if (inst->qpu.type == V3D_QPU_INSTR_TYPE_ALU) {
                assert(inst->qpu.alu.add.op == V3D_QPU_A_NOP ||
                       v3d_qpu_add_op_has_dst(inst->qpu.alu.add.op));
                assert(inst->qpu.alu.mul.op == V3D_QPU_M_NOP ||
                       v3d_qpu_mul_op_has_dst(inst->qpu.alu.mul.op));
        }

        inst->dst = vir_get_temp(c);

        if (inst->dst.file == QFILE_TEMP)
                c->defs[inst->dst.index] = inst;

        vir_emit(c, inst);

        return inst->dst;
}

struct qinst *
vir_emit_nondef(struct v3d_compile *c, struct qinst *inst)
{
        if (inst->dst.file == QFILE_TEMP)
                c->defs[inst->dst.index] = NULL;

        vir_emit(c, inst);

        return inst;
}

struct qblock *
vir_new_block(struct v3d_compile *c)
{
        struct qblock *block = rzalloc(c, struct qblock);

        list_inithead(&block->instructions);

        block->predecessors = _mesa_set_create(block,
                                               _mesa_hash_pointer,
                                               _mesa_key_pointer_equal);

        block->index = c->next_block_index++;

        return block;
}

void
vir_set_emit_block(struct v3d_compile *c, struct qblock *block)
{
        c->cur_block = block;
        c->cursor = vir_after_block(block);
        list_addtail(&block->link, &c->blocks);
}

struct qblock *
vir_entry_block(struct v3d_compile *c)
{
        return list_first_entry(&c->blocks, struct qblock, link);
}

struct qblock *
vir_exit_block(struct v3d_compile *c)
{
        return list_last_entry(&c->blocks, struct qblock, link);
}

void
vir_link_blocks(struct qblock *predecessor, struct qblock *successor)
{
        _mesa_set_add(successor->predecessors, predecessor);
        if (predecessor->successors[0]) {
                assert(!predecessor->successors[1]);
                predecessor->successors[1] = successor;
        } else {
                predecessor->successors[0] = successor;
        }
}

const struct v3d_compiler *
v3d_compiler_init(const struct v3d_device_info *devinfo,
                  uint32_t max_inline_uniform_buffers)
{
        struct v3d_compiler *compiler = rzalloc(NULL, struct v3d_compiler);
        if (!compiler)
                return NULL;

        compiler->devinfo = devinfo;
        compiler->max_inline_uniform_buffers = max_inline_uniform_buffers;

        if (!vir_init_reg_sets(compiler)) {
                ralloc_free(compiler);
                return NULL;
        }

        return compiler;
}

void
v3d_compiler_free(const struct v3d_compiler *compiler)
{
        ralloc_free((void *)compiler);
}

struct v3d_compiler_strategy {
        const char *name;
        uint32_t max_threads;
        uint32_t min_threads;
        bool disable_general_tmu_sched;
        bool disable_gcm;
        bool disable_loop_unrolling;
        bool disable_ubo_load_sorting;
        bool move_buffer_loads;
        bool disable_tmu_pipelining;
        uint32_t max_tmu_spills;
};

static struct v3d_compile *
vir_compile_init(const struct v3d_compiler *compiler,
                 struct v3d_key *key,
                 nir_shader *s,
                 void (*debug_output)(const char *msg,
                                      void *debug_output_data),
                 void *debug_output_data,
                 int program_id, int variant_id,
                 uint32_t compile_strategy_idx,
                 const struct v3d_compiler_strategy *strategy,
                 bool fallback_scheduler)
{
        struct v3d_compile *c = rzalloc(NULL, struct v3d_compile);

        c->compiler = compiler;
        c->devinfo = compiler->devinfo;
        c->key = key;
        c->program_id = program_id;
        c->variant_id = variant_id;
        c->compile_strategy_idx = compile_strategy_idx;
        c->threads = strategy->max_threads;
        c->debug_output = debug_output;
        c->debug_output_data = debug_output_data;
        c->compilation_result = V3D_COMPILATION_SUCCEEDED;
        c->min_threads_for_reg_alloc = strategy->min_threads;
        c->max_tmu_spills = strategy->max_tmu_spills;
        c->fallback_scheduler = fallback_scheduler;
        c->disable_general_tmu_sched = strategy->disable_general_tmu_sched;
        c->disable_tmu_pipelining = strategy->disable_tmu_pipelining;
        c->disable_constant_ubo_load_sorting = strategy->disable_ubo_load_sorting;
        c->move_buffer_loads = strategy->move_buffer_loads;
        c->disable_gcm = strategy->disable_gcm;
        c->disable_loop_unrolling = V3D_DBG(NO_LOOP_UNROLL)
                ? true : strategy->disable_loop_unrolling;


        s = nir_shader_clone(c, s);
        c->s = s;

        list_inithead(&c->blocks);
        vir_set_emit_block(c, vir_new_block(c));

        c->output_position_index = -1;
        c->output_sample_mask_index = -1;

        c->def_ht = _mesa_hash_table_create(c, _mesa_hash_pointer,
                                            _mesa_key_pointer_equal);

        c->tmu.outstanding_regs = _mesa_pointer_set_create(c);
        c->flags_temp = -1;

        return c;
}

static int
type_size_vec4(const struct glsl_type *type, bool bindless)
{
        return glsl_count_attribute_slots(type, false);
}

static enum nir_lower_tex_packing
lower_tex_packing_cb(const nir_tex_instr *tex, const void *data)
{
   struct v3d_compile *c = (struct v3d_compile *) data;

   int sampler_index = nir_tex_instr_need_sampler(tex) ?
      tex->sampler_index : tex->backend_flags;

   assert(sampler_index < c->key->num_samplers_used);
   return c->key->sampler[sampler_index].return_size == 16 ?
      nir_lower_tex_packing_16 : nir_lower_tex_packing_none;
}

static bool
v3d_nir_lower_null_pointers_cb(nir_builder *b,
                               nir_intrinsic_instr *intr,
                               void *_state)
{
        uint32_t buffer_src_idx;

        switch (intr->intrinsic) {
        case nir_intrinsic_load_ubo:
        case nir_intrinsic_load_ssbo:
                buffer_src_idx = 0;
                break;
        case nir_intrinsic_store_ssbo:
                buffer_src_idx = 1;
                break;
        default:
                return false;
        }

        /* If index if constant we are good */
        nir_src *src = &intr->src[buffer_src_idx];
        if (nir_src_is_const(*src))
                return false;

        /* Otherwise, see if it comes from a bcsel including a null pointer */
        if (src->ssa->parent_instr->type != nir_instr_type_alu)
                return false;

        nir_alu_instr *alu = nir_instr_as_alu(src->ssa->parent_instr);
        if (alu->op != nir_op_bcsel)
                return false;

        /* A null pointer is specified using block index 0xffffffff */
        int32_t null_src_idx = -1;
        for (int i = 1; i < 3; i++) {
                 /* FIXME: since we are running this before optimization maybe
                  * we need to also handle the case where we may have bcsel
                  * chain that we need to recurse?
                  */
                if (!nir_src_is_const(alu->src[i].src))
                        continue;
                if (nir_src_comp_as_uint(alu->src[i].src, 0) != 0xffffffff)
                        continue;

                /* One of the bcsel srcs is a null pointer reference */
                null_src_idx = i;
                break;
        }

        if (null_src_idx < 0)
                return false;

        assert(null_src_idx == 1 || null_src_idx == 2);
        int32_t copy_src_idx = null_src_idx == 1 ? 2 : 1;

        /* Rewrite the null pointer reference so we use the same buffer index
         * as the other bcsel branch. This will allow optimization to remove
         * the bcsel and we should then end up with a constant buffer index
         * like we need.
         */
        b->cursor = nir_before_instr(&alu->instr);
        nir_def *copy = nir_mov(b, alu->src[copy_src_idx].src.ssa);
        nir_src_rewrite(&alu->src[null_src_idx].src, copy);

        return true;
}

static bool
v3d_nir_lower_null_pointers(nir_shader *s)
{
        return nir_shader_intrinsics_pass(s, v3d_nir_lower_null_pointers_cb,
                                            nir_metadata_block_index |
                                            nir_metadata_dominance, NULL);
}

static void
v3d_lower_nir(struct v3d_compile *c)
{
        struct nir_lower_tex_options tex_options = {
                .lower_txd = true,
                .lower_tg4_offsets = true,
                .lower_tg4_broadcom_swizzle = true,

                .lower_rect = false, /* XXX: Use this on V3D 3.x */
                .lower_txp = ~0,
                /* Apply swizzles to all samplers. */
                .swizzle_result = ~0,
                .lower_invalid_implicit_lod = true,
        };

        /* Lower the format swizzle and (for 32-bit returns)
         * ARB_texture_swizzle-style swizzle.
         */
        assert(c->key->num_tex_used <= ARRAY_SIZE(c->key->tex));
        for (int i = 0; i < c->key->num_tex_used; i++) {
                for (int j = 0; j < 4; j++)
                        tex_options.swizzles[i][j] = c->key->tex[i].swizzle[j];
        }

        tex_options.lower_tex_packing_cb = lower_tex_packing_cb;
        tex_options.lower_tex_packing_data = c;

        NIR_PASS(_, c->s, nir_lower_tex, &tex_options);
        NIR_PASS(_, c->s, nir_lower_system_values);

        if (c->s->info.zero_initialize_shared_memory &&
            c->s->info.shared_size > 0) {
                /* All our BOs allocate full pages, so the underlying allocation
                 * for shared memory will always be a multiple of 4KB. This
                 * ensures that we can do an exact number of full chunk_size
                 * writes to initialize the memory independently of the actual
                 * shared_size used by the shader, which is a requirement of
                 * the initialization pass.
                 */
                const unsigned chunk_size = 16; /* max single store size */
                NIR_PASS(_, c->s, nir_zero_initialize_shared_memory,
                         align(c->s->info.shared_size, chunk_size), chunk_size);
        }

        NIR_PASS(_, c->s, nir_lower_compute_system_values, NULL);

        NIR_PASS(_, c->s, nir_lower_vars_to_scratch,
                 nir_var_function_temp,
                 0,
                 glsl_get_natural_size_align_bytes);
        NIR_PASS(_, c->s, nir_lower_is_helper_invocation);
        NIR_PASS(_, c->s, v3d_nir_lower_scratch);
        NIR_PASS(_, c->s, v3d_nir_lower_null_pointers);
}

static void
v3d_set_prog_data_uniforms(struct v3d_compile *c,
                           struct v3d_prog_data *prog_data)
{
        int count = c->num_uniforms;
        struct v3d_uniform_list *ulist = &prog_data->uniforms;

        ulist->count = count;
        ulist->data = ralloc_array(prog_data, uint32_t, count);
        memcpy(ulist->data, c->uniform_data,
               count * sizeof(*ulist->data));
        ulist->contents = ralloc_array(prog_data, enum quniform_contents, count);
        memcpy(ulist->contents, c->uniform_contents,
               count * sizeof(*ulist->contents));
}

static void
v3d_vs_set_prog_data(struct v3d_compile *c,
                     struct v3d_vs_prog_data *prog_data)
{
        /* The vertex data gets format converted by the VPM so that
         * each attribute channel takes up a VPM column.  Precompute
         * the sizes for the shader record.
         */
        for (int i = 0; i < ARRAY_SIZE(prog_data->vattr_sizes); i++) {
                prog_data->vattr_sizes[i] = c->vattr_sizes[i];
                prog_data->vpm_input_size += c->vattr_sizes[i];
        }

        memset(prog_data->driver_location_map, -1,
               sizeof(prog_data->driver_location_map));

        nir_foreach_shader_in_variable(var, c->s) {
                prog_data->driver_location_map[var->data.location] =
                        var->data.driver_location;
        }

        prog_data->uses_vid = BITSET_TEST(c->s->info.system_values_read,
                                          SYSTEM_VALUE_VERTEX_ID) ||
                              BITSET_TEST(c->s->info.system_values_read,
                                          SYSTEM_VALUE_VERTEX_ID_ZERO_BASE);

        prog_data->uses_biid = BITSET_TEST(c->s->info.system_values_read,
                                           SYSTEM_VALUE_BASE_INSTANCE);

        prog_data->uses_iid = BITSET_TEST(c->s->info.system_values_read,
                                          SYSTEM_VALUE_INSTANCE_ID) ||
                              BITSET_TEST(c->s->info.system_values_read,
                                          SYSTEM_VALUE_INSTANCE_INDEX);

        if (prog_data->uses_vid)
                prog_data->vpm_input_size++;
        if (prog_data->uses_biid)
                prog_data->vpm_input_size++;
        if (prog_data->uses_iid)
                prog_data->vpm_input_size++;

        /* Input/output segment size are in sectors (8 rows of 32 bits per
         * channel).
         */
        prog_data->vpm_input_size = align(prog_data->vpm_input_size, 8) / 8;
        prog_data->vpm_output_size = align(c->vpm_output_size, 8) / 8;

        /* Set us up for shared input/output segments.  This is apparently
         * necessary for our VCM setup to avoid varying corruption.
         *
         * FIXME: initial testing on V3D 7.1 seems to work fine when using
         * separate segments. So we could try to reevaluate in the future, if
         * there is any advantage of using separate segments.
         */
        prog_data->separate_segments = false;
        prog_data->vpm_output_size = MAX2(prog_data->vpm_output_size,
                                          prog_data->vpm_input_size);
        prog_data->vpm_input_size = 0;

        /* Compute VCM cache size.  We set up our program to take up less than
         * half of the VPM, so that any set of bin and render programs won't
         * run out of space.  We need space for at least one input segment,
         * and then allocate the rest to output segments (one for the current
         * program, the rest to VCM).  The valid range of the VCM cache size
         * field is 1-4 16-vertex batches, but GFXH-1744 limits us to 2-4
         * batches.
         */
        assert(c->devinfo->vpm_size);
        int sector_size = V3D_CHANNELS * sizeof(uint32_t) * 8;
        int vpm_size_in_sectors = c->devinfo->vpm_size / sector_size;
        int half_vpm = vpm_size_in_sectors / 2;
        int vpm_output_sectors = half_vpm - prog_data->vpm_input_size;
        int vpm_output_batches = vpm_output_sectors / prog_data->vpm_output_size;
        assert(vpm_output_batches >= 2);
        prog_data->vcm_cache_size = CLAMP(vpm_output_batches - 1, 2, 4);
}

static void
v3d_gs_set_prog_data(struct v3d_compile *c,
                     struct v3d_gs_prog_data *prog_data)
{
        prog_data->num_inputs = c->num_inputs;
        memcpy(prog_data->input_slots, c->input_slots,
               c->num_inputs * sizeof(*c->input_slots));

        /* gl_PrimitiveIdIn is written by the GBG into the first word of the
         * VPM output header automatically and the shader will overwrite
         * it after reading it if necessary, so it doesn't add to the VPM
         * size requirements.
         */
        prog_data->uses_pid = BITSET_TEST(c->s->info.system_values_read,
                                          SYSTEM_VALUE_PRIMITIVE_ID);

        /* Output segment size is in sectors (8 rows of 32 bits per channel) */
        prog_data->vpm_output_size = align(c->vpm_output_size, 8) / 8;

        /* Compute SIMD dispatch width and update VPM output size accordingly
         * to ensure we can fit our program in memory. Available widths are
         * 16, 8, 4, 1.
         *
         * Notice that at draw time we will have to consider VPM memory
         * requirements from other stages and choose a smaller dispatch
         * width if needed to fit the program in VPM memory.
         */
        prog_data->simd_width = 16;
        while ((prog_data->simd_width > 1 && prog_data->vpm_output_size > 16) ||
               prog_data->simd_width == 2) {
                prog_data->simd_width >>= 1;
                prog_data->vpm_output_size =
                        align(prog_data->vpm_output_size, 2) / 2;
        }
        assert(prog_data->vpm_output_size <= 16);
        assert(prog_data->simd_width != 2);

        prog_data->out_prim_type = c->s->info.gs.output_primitive;
        prog_data->num_invocations = c->s->info.gs.invocations;

        prog_data->writes_psiz =
            c->s->info.outputs_written & (1 << VARYING_SLOT_PSIZ);
}

static void
v3d_set_fs_prog_data_inputs(struct v3d_compile *c,
                            struct v3d_fs_prog_data *prog_data)
{
        prog_data->num_inputs = c->num_inputs;
        memcpy(prog_data->input_slots, c->input_slots,
               c->num_inputs * sizeof(*c->input_slots));

        STATIC_ASSERT(ARRAY_SIZE(prog_data->flat_shade_flags) >
                      (V3D_MAX_FS_INPUTS - 1) / 24);
        for (int i = 0; i < V3D_MAX_FS_INPUTS; i++) {
                if (BITSET_TEST(c->flat_shade_flags, i))
                        prog_data->flat_shade_flags[i / 24] |= 1 << (i % 24);

                if (BITSET_TEST(c->noperspective_flags, i))
                        prog_data->noperspective_flags[i / 24] |= 1 << (i % 24);

                if (BITSET_TEST(c->centroid_flags, i))
                        prog_data->centroid_flags[i / 24] |= 1 << (i % 24);
        }
}

static void
v3d_fs_set_prog_data(struct v3d_compile *c,
                     struct v3d_fs_prog_data *prog_data)
{
        v3d_set_fs_prog_data_inputs(c, prog_data);
        prog_data->writes_z = c->writes_z;
        prog_data->writes_z_from_fep = c->writes_z_from_fep;
        prog_data->disable_ez = !c->s->info.fs.early_fragment_tests;
        prog_data->uses_center_w = c->uses_center_w;
        prog_data->uses_implicit_point_line_varyings =
                c->uses_implicit_point_line_varyings;
        prog_data->lock_scoreboard_on_first_thrsw =
                c->lock_scoreboard_on_first_thrsw;
        prog_data->force_per_sample_msaa = c->s->info.fs.uses_sample_shading;
        prog_data->uses_pid = c->fs_uses_primitive_id;
}

static void
v3d_cs_set_prog_data(struct v3d_compile *c,
                     struct v3d_compute_prog_data *prog_data)
{
        prog_data->shared_size = c->s->info.shared_size;

        prog_data->local_size[0] = c->s->info.workgroup_size[0];
        prog_data->local_size[1] = c->s->info.workgroup_size[1];
        prog_data->local_size[2] = c->s->info.workgroup_size[2];

        prog_data->has_subgroups = c->has_subgroups;
}

static void
v3d_set_prog_data(struct v3d_compile *c,
                  struct v3d_prog_data *prog_data)
{
        prog_data->threads = c->threads;
        prog_data->single_seg = !c->last_thrsw;
        prog_data->spill_size = c->spill_size;
        prog_data->tmu_spills = c->spills;
        prog_data->tmu_fills = c->fills;
        prog_data->tmu_count = c->tmu.total_count;
        prog_data->qpu_read_stalls = c->qpu_inst_stalled_count;
        prog_data->compile_strategy_idx = c->compile_strategy_idx;
        prog_data->tmu_dirty_rcl = c->tmu_dirty_rcl;
        prog_data->has_control_barrier = c->s->info.uses_control_barrier;
        prog_data->has_global_address = c->has_global_address;

        v3d_set_prog_data_uniforms(c, prog_data);

        switch (c->s->info.stage) {
        case MESA_SHADER_VERTEX:
                v3d_vs_set_prog_data(c, (struct v3d_vs_prog_data *)prog_data);
                break;
        case MESA_SHADER_GEOMETRY:
                v3d_gs_set_prog_data(c, (struct v3d_gs_prog_data *)prog_data);
                break;
        case MESA_SHADER_FRAGMENT:
                v3d_fs_set_prog_data(c, (struct v3d_fs_prog_data *)prog_data);
                break;
        case MESA_SHADER_COMPUTE:
                v3d_cs_set_prog_data(c, (struct v3d_compute_prog_data *)prog_data);
                break;
        default:
                unreachable("unsupported shader stage");
        }
}

static uint64_t *
v3d_return_qpu_insts(struct v3d_compile *c, uint32_t *final_assembly_size)
{
        *final_assembly_size = c->qpu_inst_count * sizeof(uint64_t);

        uint64_t *qpu_insts = malloc(*final_assembly_size);
        if (!qpu_insts)
                return NULL;

        memcpy(qpu_insts, c->qpu_insts, *final_assembly_size);

        vir_compile_destroy(c);

        return qpu_insts;
}

static void
v3d_nir_lower_vs_early(struct v3d_compile *c)
{
        /* Split our I/O vars and dead code eliminate the unused
         * components.
         */
        NIR_PASS(_, c->s, nir_lower_io_to_scalar_early,
                 nir_var_shader_in | nir_var_shader_out);
        uint64_t used_outputs[4] = {0};
        for (int i = 0; i < c->vs_key->num_used_outputs; i++) {
                int slot = v3d_slot_get_slot(c->vs_key->used_outputs[i]);
                int comp = v3d_slot_get_component(c->vs_key->used_outputs[i]);
                used_outputs[comp] |= 1ull << slot;
        }
        NIR_PASS(_, c->s, nir_remove_unused_io_vars,
                 nir_var_shader_out, used_outputs, NULL); /* demotes to globals */
        NIR_PASS(_, c->s, nir_lower_global_vars_to_local);
        v3d_optimize_nir(c, c->s);
        NIR_PASS(_, c->s, nir_remove_dead_variables, nir_var_shader_in, NULL);

        /* This must go before nir_lower_io */
        if (c->vs_key->per_vertex_point_size)
                NIR_PASS(_, c->s, nir_lower_point_size, 1.0f, 0.0f);

        NIR_PASS(_, c->s, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
                 type_size_vec4,
                 (nir_lower_io_options)0);
        /* clean up nir_lower_io's deref_var remains and do a constant folding pass
         * on the code it generated.
         */
        NIR_PASS(_, c->s, nir_opt_dce);
        NIR_PASS(_, c->s, nir_opt_constant_folding);
}

static void
v3d_nir_lower_gs_early(struct v3d_compile *c)
{
        /* Split our I/O vars and dead code eliminate the unused
         * components.
         */
        NIR_PASS(_, c->s, nir_lower_io_to_scalar_early,
                 nir_var_shader_in | nir_var_shader_out);
        uint64_t used_outputs[4] = {0};
        for (int i = 0; i < c->gs_key->num_used_outputs; i++) {
                int slot = v3d_slot_get_slot(c->gs_key->used_outputs[i]);
                int comp = v3d_slot_get_component(c->gs_key->used_outputs[i]);
                used_outputs[comp] |= 1ull << slot;
        }
        NIR_PASS(_, c->s, nir_remove_unused_io_vars,
                 nir_var_shader_out, used_outputs, NULL); /* demotes to globals */
        NIR_PASS(_, c->s, nir_lower_global_vars_to_local);
        v3d_optimize_nir(c, c->s);
        NIR_PASS(_, c->s, nir_remove_dead_variables, nir_var_shader_in, NULL);

        /* This must go before nir_lower_io */
        if (c->gs_key->per_vertex_point_size)
                NIR_PASS(_, c->s, nir_lower_point_size, 1.0f, 0.0f);

        NIR_PASS(_, c->s, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
                 type_size_vec4,
                 (nir_lower_io_options)0);
        /* clean up nir_lower_io's deref_var remains and do a constant folding pass
         * on the code it generated.
         */
        NIR_PASS(_, c->s, nir_opt_dce);
        NIR_PASS(_, c->s, nir_opt_constant_folding);
}

static void
v3d_fixup_fs_output_types(struct v3d_compile *c)
{
        nir_foreach_shader_out_variable(var, c->s) {
                uint32_t mask = 0;

                switch (var->data.location) {
                case FRAG_RESULT_COLOR:
                        mask = ~0;
                        break;
                case FRAG_RESULT_DATA0:
                case FRAG_RESULT_DATA1:
                case FRAG_RESULT_DATA2:
                case FRAG_RESULT_DATA3:
                        mask = 1 << (var->data.location - FRAG_RESULT_DATA0);
                        break;
                }

                if (c->fs_key->int_color_rb & mask) {
                        var->type =
                                glsl_vector_type(GLSL_TYPE_INT,
                                                 glsl_get_components(var->type));
                } else if (c->fs_key->uint_color_rb & mask) {
                        var->type =
                                glsl_vector_type(GLSL_TYPE_UINT,
                                                 glsl_get_components(var->type));
                }
        }
}

static void
v3d_nir_lower_fs_early(struct v3d_compile *c)
{
        if (c->fs_key->int_color_rb || c->fs_key->uint_color_rb)
                v3d_fixup_fs_output_types(c);

        NIR_PASS(_, c->s, v3d_nir_lower_logic_ops, c);

        if (c->fs_key->line_smoothing) {
                NIR_PASS(_, c->s, v3d_nir_lower_line_smooth);
                NIR_PASS(_, c->s, nir_lower_global_vars_to_local);
                /* The lowering pass can introduce new sysval reads */
                nir_shader_gather_info(c->s, nir_shader_get_entrypoint(c->s));
        }
}

static void
v3d_nir_lower_gs_late(struct v3d_compile *c)
{
        if (c->key->ucp_enables) {
                NIR_PASS(_, c->s, nir_lower_clip_gs, c->key->ucp_enables,
                         false, NULL);
        }

        /* Note: GS output scalarizing must happen after nir_lower_clip_gs. */
        NIR_PASS_V(c->s, nir_lower_io_to_scalar, nir_var_shader_out, NULL, NULL);
}

static void
v3d_nir_lower_vs_late(struct v3d_compile *c)
{
        if (c->key->ucp_enables) {
                NIR_PASS(_, c->s, nir_lower_clip_vs, c->key->ucp_enables,
                         false, false, NULL);
                NIR_PASS_V(c->s, nir_lower_io_to_scalar,
                           nir_var_shader_out, NULL, NULL);
        }

        /* Note: VS output scalarizing must happen after nir_lower_clip_vs. */
        NIR_PASS_V(c->s, nir_lower_io_to_scalar, nir_var_shader_out, NULL, NULL);
}

static void
v3d_nir_lower_fs_late(struct v3d_compile *c)
{
        /* In OpenGL the fragment shader can't read gl_ClipDistance[], but
         * Vulkan allows it, in which case the SPIR-V compiler will declare
         * VARING_SLOT_CLIP_DIST0 as compact array variable. Pass true as
         * the last parameter to always operate with a compact array in both
         * OpenGL and Vulkan so we do't have to care about the API we
         * are using.
         */
        if (c->key->ucp_enables)
                NIR_PASS(_, c->s, nir_lower_clip_fs, c->key->ucp_enables, true);

        NIR_PASS_V(c->s, nir_lower_io_to_scalar, nir_var_shader_in, NULL, NULL);
}

static uint32_t
vir_get_max_temps(struct v3d_compile *c)
{
        int max_ip = 0;
        vir_for_each_inst_inorder(inst, c)
                max_ip++;

        uint32_t *pressure = rzalloc_array(NULL, uint32_t, max_ip);

        for (int t = 0; t < c->num_temps; t++) {
                for (int i = c->temp_start[t]; (i < c->temp_end[t] &&
                                                i < max_ip); i++) {
                        if (i > max_ip)
                                break;
                        pressure[i]++;
                }
        }

        uint32_t max_temps = 0;
        for (int i = 0; i < max_ip; i++)
                max_temps = MAX2(max_temps, pressure[i]);

        ralloc_free(pressure);

        return max_temps;
}

enum v3d_dependency_class {
        V3D_DEPENDENCY_CLASS_GS_VPM_OUTPUT_0
};

static bool
v3d_intrinsic_dependency_cb(nir_intrinsic_instr *intr,
                            nir_schedule_dependency *dep,
                            void *user_data)
{
        struct v3d_compile *c = user_data;

        switch (intr->intrinsic) {
        case nir_intrinsic_store_output:
                /* Writing to location 0 overwrites the value passed in for
                 * gl_PrimitiveID on geometry shaders
                 */
                if (c->s->info.stage != MESA_SHADER_GEOMETRY ||
                    nir_intrinsic_base(intr) != 0)
                        break;

                nir_const_value *const_value =
                        nir_src_as_const_value(intr->src[1]);

                if (const_value == NULL)
                        break;

                uint64_t offset =
                        nir_const_value_as_uint(*const_value,
                                                nir_src_bit_size(intr->src[1]));
                if (offset != 0)
                        break;

                dep->klass = V3D_DEPENDENCY_CLASS_GS_VPM_OUTPUT_0;
                dep->type = NIR_SCHEDULE_WRITE_DEPENDENCY;
                return true;

        case nir_intrinsic_load_primitive_id:
                if (c->s->info.stage != MESA_SHADER_GEOMETRY)
                        break;

                dep->klass = V3D_DEPENDENCY_CLASS_GS_VPM_OUTPUT_0;
                dep->type = NIR_SCHEDULE_READ_DEPENDENCY;
                return true;

        default:
                break;
        }

        return false;
}

static unsigned
v3d_instr_delay_cb(nir_instr *instr, void *data)
{
   struct v3d_compile *c = (struct v3d_compile *) data;

   switch (instr->type) {
   case nir_instr_type_undef:
   case nir_instr_type_load_const:
   case nir_instr_type_alu:
   case nir_instr_type_deref:
   case nir_instr_type_jump:
   case nir_instr_type_parallel_copy:
   case nir_instr_type_call:
   case nir_instr_type_phi:
      return 1;

   /* We should not use very large delays for TMU instructions. Typically,
    * thread switches will be sufficient to hide all or most of the latency,
    * so we typically only need a little bit of extra room. If we over-estimate
    * the latency here we may end up unnecessarily delaying the critical path in
    * the shader, which would have a negative effect in performance, so here
    * we are trying to strike a balance based on empirical testing.
    */
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
      if (!c->disable_general_tmu_sched) {
         switch (intr->intrinsic) {
         case nir_intrinsic_decl_reg:
         case nir_intrinsic_load_reg:
         case nir_intrinsic_store_reg:
            return 0;
         case nir_intrinsic_load_ssbo:
         case nir_intrinsic_load_scratch:
         case nir_intrinsic_load_shared:
         case nir_intrinsic_image_load:
            return 3;
         case nir_intrinsic_load_ubo:
            if (nir_src_is_divergent(intr->src[1]))
               return 3;
            FALLTHROUGH;
         default:
            return 1;
         }
      } else {
         switch (intr->intrinsic) {
         case nir_intrinsic_decl_reg:
         case nir_intrinsic_load_reg:
         case nir_intrinsic_store_reg:
            return 0;
         default:
            return 1;
         }
      }
      break;
   }

   case nir_instr_type_tex:
      return 5;
   }

   return 0;
}

static bool
should_split_wrmask(const nir_instr *instr, const void *data)
{
        nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
        switch (intr->intrinsic) {
        case nir_intrinsic_store_ssbo:
        case nir_intrinsic_store_shared:
        case nir_intrinsic_store_global:
        case nir_intrinsic_store_scratch:
                return true;
        default:
                return false;
        }
}

static nir_intrinsic_instr *
nir_instr_as_constant_ubo_load(nir_instr *inst)
{
        if (inst->type != nir_instr_type_intrinsic)
                return NULL;

        nir_intrinsic_instr *intr = nir_instr_as_intrinsic(inst);
        if (intr->intrinsic != nir_intrinsic_load_ubo)
                return NULL;

        assert(nir_src_is_const(intr->src[0]));
        if (!nir_src_is_const(intr->src[1]))
                return NULL;

        return intr;
}

static bool
v3d_nir_sort_constant_ubo_load(nir_block *block, nir_intrinsic_instr *ref)
{
        bool progress = false;

        nir_instr *ref_inst = &ref->instr;
        uint32_t ref_offset = nir_src_as_uint(ref->src[1]);
        uint32_t ref_index = nir_src_as_uint(ref->src[0]);

        /* Go through all instructions after ref searching for constant UBO
         * loads for the same UBO index.
         */
        bool seq_break = false;
        nir_instr *inst = &ref->instr;
        nir_instr *next_inst = NULL;
        while (true) {
                inst = next_inst ? next_inst : nir_instr_next(inst);
                if (!inst)
                        break;

                next_inst = NULL;

                if (inst->type != nir_instr_type_intrinsic)
                        continue;

                nir_intrinsic_instr *intr = nir_instr_as_intrinsic(inst);
                if (intr->intrinsic != nir_intrinsic_load_ubo)
                        continue;

                /* We only produce unifa sequences for non-divergent loads */
                if (nir_src_is_divergent(intr->src[1]))
                        continue;

                /* If there are any UBO loads that are not constant or that
                 * use a different UBO index in between the reference load and
                 * any other constant load for the same index, they would break
                 * the unifa sequence. We will flag that so we can then move
                 * all constant UBO loads for the reference index before these
                 * and not just the ones that are not ordered to avoid breaking
                 * the sequence and reduce unifa writes.
                 */
                if (!nir_src_is_const(intr->src[1])) {
                        seq_break = true;
                        continue;
                }
                uint32_t offset = nir_src_as_uint(intr->src[1]);

                assert(nir_src_is_const(intr->src[0]));
                uint32_t index = nir_src_as_uint(intr->src[0]);
                if (index != ref_index) {
                       seq_break = true;
                       continue;
                }

                /* Only move loads with an offset that is close enough to the
                 * reference offset, since otherwise we would not be able to
                 * skip the unifa write for them. See ntq_emit_load_ubo_unifa.
                 */
                if (abs((int)(ref_offset - offset)) > MAX_UNIFA_SKIP_DISTANCE)
                        continue;

                /* We will move this load if its offset is smaller than ref's
                 * (in which case we will move it before ref) or if the offset
                 * is larger than ref's but there are sequence breakers in
                 * in between (in which case we will move it after ref and
                 * before the sequence breakers).
                 */
                if (!seq_break && offset >= ref_offset)
                        continue;

                /* Find where exactly we want to move this load:
                 *
                 * If we are moving it before ref, we want to check any other
                 * UBO loads we placed before ref and make sure we insert this
                 * one properly ordered with them. Likewise, if we are moving
                 * it after ref.
                 */
                nir_instr *pos = ref_inst;
                nir_instr *tmp = pos;
                do {
                        if (offset < ref_offset)
                                tmp = nir_instr_prev(tmp);
                        else
                                tmp = nir_instr_next(tmp);

                        if (!tmp || tmp == inst)
                                break;

                        /* Ignore non-unifa UBO loads */
                        if (tmp->type != nir_instr_type_intrinsic)
                                continue;

                        nir_intrinsic_instr *tmp_intr =
                                nir_instr_as_intrinsic(tmp);
                        if (tmp_intr->intrinsic != nir_intrinsic_load_ubo)
                                continue;

                        if (nir_src_is_divergent(tmp_intr->src[1]))
                                continue;

                        /* Stop if we find a unifa UBO load that breaks the
                         * sequence.
                         */
                        if (!nir_src_is_const(tmp_intr->src[1]))
                                break;

                        if (nir_src_as_uint(tmp_intr->src[0]) != index)
                                break;

                        uint32_t tmp_offset = nir_src_as_uint(tmp_intr->src[1]);
                        if (offset < ref_offset) {
                                if (tmp_offset < offset ||
                                    tmp_offset >= ref_offset) {
                                        break;
                                } else {
                                        pos = tmp;
                                }
                        } else {
                                if (tmp_offset > offset ||
                                    tmp_offset <= ref_offset) {
                                        break;
                                } else {
                                        pos = tmp;
                                }
                        }
                } while (true);

                /* We can't move the UBO load before the instruction that
                 * defines its constant offset. If that instruction is placed
                 * in between the new location (pos) and the current location
                 * of this load, we will have to move that instruction too.
                 *
                 * We don't care about the UBO index definition because that
                 * is optimized to be reused by all UBO loads for the same
                 * index and therefore is certain to be defined before the
                 * first UBO load that uses it.
                 */
                nir_instr *offset_inst = NULL;
                tmp = inst;
                while ((tmp = nir_instr_prev(tmp)) != NULL) {
                        if (pos == tmp) {
                                /* We reached the target location without
                                 * finding the instruction that defines the
                                 * offset, so that instruction must be before
                                 * the new position and we don't have to fix it.
                                 */
                                break;
                        }
                        if (intr->src[1].ssa->parent_instr == tmp) {
                                offset_inst = tmp;
                                break;
                        }
                }

                if (offset_inst) {
                        exec_node_remove(&offset_inst->node);
                        exec_node_insert_node_before(&pos->node,
                                                     &offset_inst->node);
                }

                /* Since we are moving the instruction before its current
                 * location, grab its successor before the move so that
                 * we can continue the next iteration of the main loop from
                 * that instruction.
                 */
                next_inst = nir_instr_next(inst);

                /* Move this load to the selected location */
                exec_node_remove(&inst->node);
                if (offset < ref_offset)
                        exec_node_insert_node_before(&pos->node, &inst->node);
                else
                        exec_node_insert_after(&pos->node, &inst->node);

                progress = true;
        }

        return progress;
}

static bool
v3d_nir_sort_constant_ubo_loads_block(struct v3d_compile *c,
                                      nir_block *block)
{
        bool progress = false;
        bool local_progress;
        do {
                local_progress = false;
                nir_foreach_instr_safe(inst, block) {
                        nir_intrinsic_instr *intr =
                                nir_instr_as_constant_ubo_load(inst);
                        if (intr) {
                                local_progress |=
                                        v3d_nir_sort_constant_ubo_load(block, intr);
                        }
                }
                progress |= local_progress;
        } while (local_progress);

        return progress;
}

/**
 * Sorts constant UBO loads in each block by offset to maximize chances of
 * skipping unifa writes when converting to VIR. This can increase register
 * pressure.
 */
static bool
v3d_nir_sort_constant_ubo_loads(nir_shader *s, struct v3d_compile *c)
{
        nir_foreach_function_impl(impl, s) {
                nir_foreach_block(block, impl) {
                        c->sorted_any_ubo_loads |=
                                v3d_nir_sort_constant_ubo_loads_block(c, block);
                }
                nir_metadata_preserve(impl,
                                      nir_metadata_block_index |
                                      nir_metadata_dominance);
        }
        return c->sorted_any_ubo_loads;
}

static void
lower_load_num_subgroups(struct v3d_compile *c,
                         nir_builder *b,
                         nir_intrinsic_instr *intr)
{
        assert(c->s->info.stage == MESA_SHADER_COMPUTE);
        assert(intr->intrinsic == nir_intrinsic_load_num_subgroups);

        b->cursor = nir_after_instr(&intr->instr);
        uint32_t num_subgroups =
                DIV_ROUND_UP(c->s->info.workgroup_size[0] *
                             c->s->info.workgroup_size[1] *
                             c->s->info.workgroup_size[2], V3D_CHANNELS);
        nir_def *result = nir_imm_int(b, num_subgroups);
        nir_def_rewrite_uses(&intr->def, result);
        nir_instr_remove(&intr->instr);
}

static bool
lower_subgroup_intrinsics(struct v3d_compile *c,
                          nir_block *block, nir_builder *b)
{
        bool progress = false;
        nir_foreach_instr_safe(inst, block) {
                if (inst->type != nir_instr_type_intrinsic)
                        continue;;

                nir_intrinsic_instr *intr =
                        nir_instr_as_intrinsic(inst);
                if (!intr)
                        continue;

                switch (intr->intrinsic) {
                case nir_intrinsic_load_num_subgroups:
                        lower_load_num_subgroups(c, b, intr);
                        progress = true;
                        FALLTHROUGH;
                case nir_intrinsic_load_subgroup_id:
                case nir_intrinsic_load_subgroup_size:
                case nir_intrinsic_load_subgroup_invocation:
                case nir_intrinsic_elect:
                        c->has_subgroups = true;
                        break;
                default:
                        break;
                }
        }

        return progress;
}

static bool
v3d_nir_lower_subgroup_intrinsics(nir_shader *s, struct v3d_compile *c)
{
        bool progress = false;
        nir_foreach_function_impl(impl, s) {
                nir_builder b = nir_builder_create(impl);

                nir_foreach_block(block, impl)
                        progress |= lower_subgroup_intrinsics(c, block, &b);

                nir_metadata_preserve(impl,
                                      nir_metadata_block_index |
                                      nir_metadata_dominance);
        }
        return progress;
}

static void
v3d_attempt_compile(struct v3d_compile *c)
{
        switch (c->s->info.stage) {
        case MESA_SHADER_VERTEX:
                c->vs_key = (struct v3d_vs_key *) c->key;
                break;
        case MESA_SHADER_GEOMETRY:
                c->gs_key = (struct v3d_gs_key *) c->key;
                break;
        case MESA_SHADER_FRAGMENT:
                c->fs_key = (struct v3d_fs_key *) c->key;
                break;
        case MESA_SHADER_COMPUTE:
                break;
        default:
                unreachable("unsupported shader stage");
        }

        switch (c->s->info.stage) {
        case MESA_SHADER_VERTEX:
                v3d_nir_lower_vs_early(c);
                break;
        case MESA_SHADER_GEOMETRY:
                v3d_nir_lower_gs_early(c);
                break;
        case MESA_SHADER_FRAGMENT:
                v3d_nir_lower_fs_early(c);
                break;
        default:
                break;
        }

        v3d_lower_nir(c);

        switch (c->s->info.stage) {
        case MESA_SHADER_VERTEX:
                v3d_nir_lower_vs_late(c);
                break;
        case MESA_SHADER_GEOMETRY:
                v3d_nir_lower_gs_late(c);
                break;
        case MESA_SHADER_FRAGMENT:
                v3d_nir_lower_fs_late(c);
                break;
        default:
                break;
        }

        NIR_PASS(_, c->s, v3d_nir_lower_io, c);
        NIR_PASS(_, c->s, v3d_nir_lower_txf_ms);
        NIR_PASS(_, c->s, v3d_nir_lower_image_load_store, c);

        NIR_PASS(_, c->s, nir_opt_idiv_const, 8);
        nir_lower_idiv_options idiv_options = {
                .allow_fp16 = true,
        };
        NIR_PASS(_, c->s, nir_lower_idiv, &idiv_options);
        NIR_PASS(_, c->s, nir_lower_alu);

        if (c->key->robust_uniform_access || c->key->robust_storage_access ||
            c->key->robust_image_access) {
                /* nir_lower_robust_access assumes constant buffer
                 * indices on ubo/ssbo intrinsics so run copy propagation and
                 * constant folding passes before we run the lowering to warrant
                 * this. We also want to run the lowering before v3d_optimize to
                 * clean-up redundant get_buffer_size calls produced in the pass.
                 */
                NIR_PASS(_, c->s, nir_copy_prop);
                NIR_PASS(_, c->s, nir_opt_constant_folding);

                nir_lower_robust_access_options opts = {
                   .lower_image = c->key->robust_image_access,
                   .lower_ssbo = c->key->robust_storage_access,
                   .lower_ubo = c->key->robust_uniform_access,
                };

                NIR_PASS(_, c->s, nir_lower_robust_access, &opts);
        }

        NIR_PASS(_, c->s, nir_lower_wrmasks, should_split_wrmask, c->s);

        NIR_PASS(_, c->s, v3d_nir_lower_load_store_bitsize);

        NIR_PASS(_, c->s, v3d_nir_lower_subgroup_intrinsics, c);

        v3d_optimize_nir(c, c->s);

        /* Do late algebraic optimization to turn add(a, neg(b)) back into
         * subs, then the mandatory cleanup after algebraic.  Note that it may
         * produce fnegs, and if so then we need to keep running to squash
         * fneg(fneg(a)).
         */
        bool more_late_algebraic = true;
        while (more_late_algebraic) {
                more_late_algebraic = false;
                NIR_PASS(more_late_algebraic, c->s, nir_opt_algebraic_late);
                NIR_PASS(_, c->s, nir_opt_constant_folding);
                NIR_PASS(_, c->s, nir_copy_prop);
                NIR_PASS(_, c->s, nir_opt_dce);
                NIR_PASS(_, c->s, nir_opt_cse);
        }

        NIR_PASS(_, c->s, nir_lower_bool_to_int32);
        NIR_PASS(_, c->s, nir_convert_to_lcssa, true, true);
        NIR_PASS_V(c->s, nir_divergence_analysis);
        NIR_PASS(_, c->s, nir_convert_from_ssa, true);

        struct nir_schedule_options schedule_options = {
                /* Schedule for about half our register space, to enable more
                 * shaders to hit 4 threads.
                 */
                .threshold = c->threads == 4 ? 24 : 48,

                /* Vertex shaders share the same memory for inputs and outputs,
                 * fragment and geometry shaders do not.
                 */
                .stages_with_shared_io_memory =
                (((1 << MESA_ALL_SHADER_STAGES) - 1) &
                 ~((1 << MESA_SHADER_FRAGMENT) |
                   (1 << MESA_SHADER_GEOMETRY))),

                .fallback = c->fallback_scheduler,

                .intrinsic_cb = v3d_intrinsic_dependency_cb,
                .intrinsic_cb_data = c,

                .instr_delay_cb = v3d_instr_delay_cb,
                .instr_delay_cb_data = c,
        };
        NIR_PASS_V(c->s, nir_schedule, &schedule_options);

        if (!c->disable_constant_ubo_load_sorting)
                NIR_PASS(_, c->s, v3d_nir_sort_constant_ubo_loads, c);

        const nir_move_options buffer_opts = c->move_buffer_loads ?
                (nir_move_load_ubo | nir_move_load_ssbo) : 0;
        NIR_PASS(_, c->s, nir_opt_move, nir_move_load_uniform |
                                        nir_move_const_undef |
                                        buffer_opts);

        NIR_PASS_V(c->s, nir_trivialize_registers);

        v3d_nir_to_vir(c);
}

uint32_t
v3d_prog_data_size(gl_shader_stage stage)
{
        static const int prog_data_size[] = {
                [MESA_SHADER_VERTEX] = sizeof(struct v3d_vs_prog_data),
                [MESA_SHADER_GEOMETRY] = sizeof(struct v3d_gs_prog_data),
                [MESA_SHADER_FRAGMENT] = sizeof(struct v3d_fs_prog_data),
                [MESA_SHADER_COMPUTE] = sizeof(struct v3d_compute_prog_data),
        };

        assert(stage >= 0 &&
               stage < ARRAY_SIZE(prog_data_size) &&
               prog_data_size[stage]);

        return prog_data_size[stage];
}

int v3d_shaderdb_dump(struct v3d_compile *c,
		      char **shaderdb_str)
{
        if (c == NULL || c->compilation_result != V3D_COMPILATION_SUCCEEDED)
                return -1;

        return asprintf(shaderdb_str,
                        "%s shader: %d inst, %d threads, %d loops, "
                        "%d uniforms, %d max-temps, %d:%d spills:fills, "
                        "%d sfu-stalls, %d inst-and-stalls, %d nops",
                        vir_get_stage_name(c),
                        c->qpu_inst_count,
                        c->threads,
                        c->loops,
                        c->num_uniforms,
                        vir_get_max_temps(c),
                        c->spills,
                        c->fills,
                        c->qpu_inst_stalled_count,
                        c->qpu_inst_count + c->qpu_inst_stalled_count,
                        c->nop_count);
}

/* This is a list of incremental changes to the compilation strategy
 * that will be used to try to compile the shader successfully. The
 * default strategy is to enable all optimizations which will have
 * the highest register pressure but is expected to produce most
 * optimal code. Following strategies incrementally disable specific
 * optimizations that are known to contribute to register pressure
 * in order to be able to compile the shader successfully while meeting
 * thread count requirements.
 *
 * V3D 4.1+ has a min thread count of 2, but we can use 1 here to also
 * cover previous hardware as well (meaning that we are not limiting
 * register allocation to any particular thread count). This is fine
 * because v3d_nir_to_vir will cap this to the actual minimum.
 */
static const struct v3d_compiler_strategy strategies[] = {
        /*0*/  { "default",                        4, 4, false, false, false, false, false, false,  0 },
        /*1*/  { "disable general TMU sched",      4, 4, true,  false, false, false, false, false,  0 },
        /*2*/  { "disable gcm",                    4, 4, true,  true,  false, false, false, false,  0 },
        /*3*/  { "disable loop unrolling",         4, 4, true,  true,  true,  false, false, false,  0 },
        /*4*/  { "disable UBO load sorting",       4, 4, true,  true,  true,  true,  false, false,  0 },
        /*5*/  { "disable TMU pipelining",         4, 4, true,  true,  true,  true,  false, true,   0 },
        /*6*/  { "lower thread count",             2, 1, false, false, false, false, false, false, -1 },
        /*7*/  { "disable general TMU sched (2t)", 2, 1, true,  false, false, false, false, false, -1 },
        /*8*/  { "disable gcm (2t)",               2, 1, true,  true,  false, false, false, false, -1 },
        /*9*/  { "disable loop unrolling (2t)",    2, 1, true,  true,  true,  false, false, false, -1 },
        /*10*/ { "Move buffer loads (2t)",         2, 1, true,  true,  true,  true,  true,  false, -1 },
        /*11*/ { "disable TMU pipelining (2t)",    2, 1, true,  true,  true,  true,  true,  true,  -1 },
        /*12*/ { "fallback scheduler",             2, 1, true,  true,  true,  true,  true,  true,  -1 }
};

/**
 * If a particular optimization didn't make any progress during a compile
 * attempt disabling it alone won't allow us to compile the shader successfully,
 * since we'll end up with the same code. Detect these scenarios so we can
 * avoid wasting time with useless compiles. We should also consider if the
 * gy changes other aspects of the compilation process though, like
 * spilling, and not skip it in that case.
 */
static bool
skip_compile_strategy(struct v3d_compile *c, uint32_t idx)
{
   /* We decide if we can skip a strategy based on the optimizations that
    * were active in the previous strategy, so we should only be calling this
    * for strategies after the first.
    */
   assert(idx > 0);

   /* Don't skip a strategy that changes spilling behavior */
   if (strategies[idx].max_tmu_spills !=
       strategies[idx - 1].max_tmu_spills) {
           return false;
   }

   switch (idx) {
   /* General TMU sched.: skip if we didn't emit any TMU loads */
   case 1:
   case 7:
           return !c->has_general_tmu_load;
   /* Global code motion: skip if nir_opt_gcm didn't make any progress */
   case 2:
   case 8:
           return !c->gcm_progress;
   /* Loop unrolling: skip if we didn't unroll any loops */
   case 3:
   case 9:
           return !c->unrolled_any_loops;
   /* UBO load sorting: skip if we didn't sort any loads */
   case 4:
           return !c->sorted_any_ubo_loads;
   /* Move buffer loads: we assume any shader with difficult RA
    * most likely has UBO / SSBO loads so we never try to skip.
    * For now, we only try this for 2-thread compiles since it
    * is expected to impact instruction counts and latency.
    */
   case 10:
          assert(c->threads < 4);
          return false;
   /* TMU pipelining: skip if we didn't pipeline any TMU ops */
   case 5:
   case 11:
           return !c->pipelined_any_tmu;
   /* Lower thread count: skip if we already tried less that 4 threads */
   case 6:
          return c->threads < 4;
   default:
           return false;
   };
}

static inline void
set_best_compile(struct v3d_compile **best, struct v3d_compile *c)
{
   if (*best)
      vir_compile_destroy(*best);
   *best = c;
}

uint64_t *v3d_compile(const struct v3d_compiler *compiler,
                      struct v3d_key *key,
                      struct v3d_prog_data **out_prog_data,
                      nir_shader *s,
                      void (*debug_output)(const char *msg,
                                           void *debug_output_data),
                      void *debug_output_data,
                      int program_id, int variant_id,
                      uint32_t *final_assembly_size)
{
        struct v3d_compile *c = NULL;

        uint32_t best_spill_fill_count = UINT32_MAX;
        struct v3d_compile *best_c = NULL;
        for (int32_t strat = 0; strat < ARRAY_SIZE(strategies); strat++) {
                /* Fallback strategy */
                if (strat > 0) {
                        assert(c);
                        if (skip_compile_strategy(c, strat))
                                continue;

                        char *debug_msg;
                        int ret = asprintf(&debug_msg,
                                           "Falling back to strategy '%s' "
                                           "for %s prog %d/%d",
                                           strategies[strat].name,
                                           vir_get_stage_name(c),
                                           c->program_id, c->variant_id);

                        if (ret >= 0) {
                                if (V3D_DBG(PERF))
                                        fprintf(stderr, "%s\n", debug_msg);

                                c->debug_output(debug_msg, c->debug_output_data);
                                free(debug_msg);
                        }

                        if (c != best_c)
                                vir_compile_destroy(c);
                }

                c = vir_compile_init(compiler, key, s,
                                     debug_output, debug_output_data,
                                     program_id, variant_id,
                                     strat, &strategies[strat],
                                     strat == ARRAY_SIZE(strategies) - 1);

                v3d_attempt_compile(c);

                /* Broken shader or driver bug */
                if (c->compilation_result == V3D_COMPILATION_FAILED)
                        break;

                /* If we compiled without spills, choose this.
                 * Otherwise if this is a 4-thread compile, choose this (these
                 * have a very low cap on the allowed TMU spills so we assume
                 * it will be better than a 2-thread compile without spills).
                 * Otherwise, keep going while tracking the strategy with the
                 * lowest spill count.
                 */
                if (c->compilation_result == V3D_COMPILATION_SUCCEEDED) {
                        if (c->spills == 0 ||
                            strategies[strat].min_threads == 4 ||
                            V3D_DBG(OPT_COMPILE_TIME)) {
                                set_best_compile(&best_c, c);
                                break;
                        } else if (c->spills + c->fills <
                                   best_spill_fill_count) {
                                set_best_compile(&best_c, c);
                                best_spill_fill_count = c->spills + c->fills;
                        }

                        if (V3D_DBG(PERF)) {
                                char *debug_msg;
                                int ret = asprintf(&debug_msg,
                                                   "Compiled %s prog %d/%d with %d "
                                                   "spills and %d fills. Will try "
                                                   "more strategies.",
                                                   vir_get_stage_name(c),
                                                   c->program_id, c->variant_id,
                                                   c->spills, c->fills);
                                if (ret >= 0) {
                                        fprintf(stderr, "%s\n", debug_msg);
                                        c->debug_output(debug_msg, c->debug_output_data);
                                        free(debug_msg);
                                }
                        }
                }

                /* Only try next streategy if we failed to register allocate
                 * or we had to spill.
                 */
                assert(c->compilation_result ==
                       V3D_COMPILATION_FAILED_REGISTER_ALLOCATION ||
                       c->spills > 0);
        }

        /* If the best strategy was not the last, choose that */
        if (best_c && c != best_c)
                set_best_compile(&c, best_c);

        if (V3D_DBG(PERF) &&
            c->compilation_result !=
            V3D_COMPILATION_FAILED_REGISTER_ALLOCATION &&
            c->spills > 0) {
                char *debug_msg;
                int ret = asprintf(&debug_msg,
                                   "Compiled %s prog %d/%d with %d "
                                   "spills and %d fills",
                                   vir_get_stage_name(c),
                                   c->program_id, c->variant_id,
                                   c->spills, c->fills);
                fprintf(stderr, "%s\n", debug_msg);

                if (ret >= 0) {
                        c->debug_output(debug_msg, c->debug_output_data);
                        free(debug_msg);
                }
        }

        if (c->compilation_result != V3D_COMPILATION_SUCCEEDED) {
                fprintf(stderr, "Failed to compile %s prog %d/%d "
                        "with any strategy.\n",
                        vir_get_stage_name(c), c->program_id, c->variant_id);

                vir_compile_destroy(c);
                return NULL;
        }

        struct v3d_prog_data *prog_data;

        prog_data = rzalloc_size(NULL, v3d_prog_data_size(c->s->info.stage));

        v3d_set_prog_data(c, prog_data);

        *out_prog_data = prog_data;

        char *shaderdb;
        int ret = v3d_shaderdb_dump(c, &shaderdb);
        if (ret >= 0) {
                if (V3D_DBG(SHADERDB))
                        fprintf(stderr, "SHADER-DB-%s - %s\n", s->info.name, shaderdb);

                c->debug_output(shaderdb, c->debug_output_data);
                free(shaderdb);
        }

       return v3d_return_qpu_insts(c, final_assembly_size);
}

void
vir_remove_instruction(struct v3d_compile *c, struct qinst *qinst)
{
        if (qinst->dst.file == QFILE_TEMP)
                c->defs[qinst->dst.index] = NULL;

        assert(&qinst->link != c->cursor.link);

        list_del(&qinst->link);
        free(qinst);

        c->live_intervals_valid = false;
}

struct qreg
vir_follow_movs(struct v3d_compile *c, struct qreg reg)
{
        /* XXX
        int pack = reg.pack;

        while (reg.file == QFILE_TEMP &&
               c->defs[reg.index] &&
               (c->defs[reg.index]->op == QOP_MOV ||
                c->defs[reg.index]->op == QOP_FMOV) &&
               !c->defs[reg.index]->dst.pack &&
               !c->defs[reg.index]->src[0].pack) {
                reg = c->defs[reg.index]->src[0];
        }

        reg.pack = pack;
        */
        return reg;
}

void
vir_compile_destroy(struct v3d_compile *c)
{
        /* Defuse the assert that we aren't removing the cursor's instruction.
         */
        c->cursor.link = NULL;

        vir_for_each_block(block, c) {
                while (!list_is_empty(&block->instructions)) {
                        struct qinst *qinst =
                                list_first_entry(&block->instructions,
                                                 struct qinst, link);
                        vir_remove_instruction(c, qinst);
                }
        }

        ralloc_free(c);
}

uint32_t
vir_get_uniform_index(struct v3d_compile *c,
                      enum quniform_contents contents,
                      uint32_t data)
{
        for (int i = 0; i < c->num_uniforms; i++) {
                if (c->uniform_contents[i] == contents &&
                    c->uniform_data[i] == data) {
                        return i;
                }
        }

        uint32_t uniform = c->num_uniforms++;

        if (uniform >= c->uniform_array_size) {
                c->uniform_array_size = MAX2(MAX2(16, uniform + 1),
                                             c->uniform_array_size * 2);

                c->uniform_data = reralloc(c, c->uniform_data,
                                           uint32_t,
                                           c->uniform_array_size);
                c->uniform_contents = reralloc(c, c->uniform_contents,
                                               enum quniform_contents,
                                               c->uniform_array_size);
        }

        c->uniform_contents[uniform] = contents;
        c->uniform_data[uniform] = data;

        return uniform;
}

/* Looks back into the current block to find the ldunif that wrote the uniform
 * at the requested index. If it finds it, it returns true and writes the
 * destination register of the ldunif instruction to 'unif'.
 *
 * This can impact register pressure and end up leading to worse code, so we
 * limit the number of instructions we are willing to look back through to
 * strike a good balance.
 */
static bool
try_opt_ldunif(struct v3d_compile *c, uint32_t index, struct qreg *unif)
{
        uint32_t count = 20;
        struct qinst *prev_inst = NULL;
        assert(c->cur_block);

#ifdef DEBUG
        /* We can only reuse a uniform if it was emitted in the same block,
         * so callers must make sure the current instruction is being emitted
         * in the current block.
         */
        bool found = false;
        vir_for_each_inst(inst, c->cur_block) {
                if (&inst->link == c->cursor.link) {
                        found = true;
                        break;
                }
        }

        assert(found || &c->cur_block->instructions == c->cursor.link);
#endif

        list_for_each_entry_from_rev(struct qinst, inst, c->cursor.link->prev,
                                     &c->cur_block->instructions, link) {
                if ((inst->qpu.sig.ldunif || inst->qpu.sig.ldunifrf) &&
                    inst->uniform == index) {
                        prev_inst = inst;
                        break;
                }

                if (--count == 0)
                        break;
        }

        if (!prev_inst)
                return false;

        /* Only reuse the ldunif result if it was written to a temp register,
         * otherwise there may be special restrictions (for example, ldunif
         * may write directly to unifa, which is a write-only register).
         */
        if (prev_inst->dst.file != QFILE_TEMP)
                return false;

        list_for_each_entry_from(struct qinst, inst, prev_inst->link.next,
                                 &c->cur_block->instructions, link) {
                if (inst->dst.file == prev_inst->dst.file &&
                    inst->dst.index == prev_inst->dst.index) {
                        return false;
                }
        }

        *unif = prev_inst->dst;
        return true;
}

struct qreg
vir_uniform(struct v3d_compile *c,
            enum quniform_contents contents,
            uint32_t data)
{
        const int num_uniforms = c->num_uniforms;
        const int index = vir_get_uniform_index(c, contents, data);

        /* If this is not the first time we see this uniform try to reuse the
         * result of the last ldunif that loaded it.
         */
        const bool is_new_uniform = num_uniforms != c->num_uniforms;
        if (!is_new_uniform && !c->disable_ldunif_opt) {
                struct qreg ldunif_dst;
                if (try_opt_ldunif(c, index, &ldunif_dst))
                        return ldunif_dst;
        }

        struct qinst *inst = vir_NOP(c);
        inst->qpu.sig.ldunif = true;
        inst->uniform = index;
        inst->dst = vir_get_temp(c);
        c->defs[inst->dst.index] = inst;
        return inst->dst;
}

#define OPTPASS(func)                                                   \
        do {                                                            \
                bool stage_progress = func(c);                          \
                if (stage_progress) {                                   \
                        progress = true;                                \
                        if (print_opt_debug) {                          \
                                fprintf(stderr,                         \
                                        "VIR opt pass %2d: %s progress\n", \
                                        pass, #func);                   \
                        }                                               \
                        /*XXX vir_validate(c);*/                        \
                }                                                       \
        } while (0)

void
vir_optimize(struct v3d_compile *c)
{
        bool print_opt_debug = false;
        int pass = 1;

        while (true) {
                bool progress = false;

                OPTPASS(vir_opt_copy_propagate);
                OPTPASS(vir_opt_redundant_flags);
                OPTPASS(vir_opt_dead_code);
                OPTPASS(vir_opt_small_immediates);
                OPTPASS(vir_opt_constant_alu);

                if (!progress)
                        break;

                pass++;
        }
}

const char *
vir_get_stage_name(struct v3d_compile *c)
{
        if (c->vs_key && c->vs_key->is_coord)
                return "MESA_SHADER_VERTEX_BIN";
        else if (c->gs_key && c->gs_key->is_coord)
                return "MESA_SHADER_GEOMETRY_BIN";
        else
                return gl_shader_stage_name(c->s->info.stage);
}

static inline uint32_t
compute_vpm_size_in_sectors(const struct v3d_device_info *devinfo)
{
   assert(devinfo->vpm_size > 0);
   const uint32_t sector_size = V3D_CHANNELS * sizeof(uint32_t) * 8;
   return devinfo->vpm_size / sector_size;
}

/* Computes various parameters affecting VPM memory configuration for programs
 * involving geometry shaders to ensure the program fits in memory and honors
 * requirements described in section "VPM usage" of the programming manual.
 */
static bool
compute_vpm_config_gs(struct v3d_device_info *devinfo,
                      struct v3d_vs_prog_data *vs,
                      struct v3d_gs_prog_data *gs,
                      struct vpm_config *vpm_cfg_out)
{
   const uint32_t A = vs->separate_segments ? 1 : 0;
   const uint32_t Ad = vs->vpm_input_size;
   const uint32_t Vd = vs->vpm_output_size;

   const uint32_t vpm_size = compute_vpm_size_in_sectors(devinfo);

   /* Try to fit program into our VPM memory budget by adjusting
    * configurable parameters iteratively. We do this in two phases:
    * the first phase tries to fit the program into the total available
    * VPM memory. If we succeed at that, then the second phase attempts
    * to fit the program into half of that budget so we can run bin and
    * render programs in parallel.
    */
   struct vpm_config vpm_cfg[2];
   struct vpm_config *final_vpm_cfg = NULL;
   uint32_t phase = 0;

   vpm_cfg[phase].As = 1;
   vpm_cfg[phase].Gs = 1;
   vpm_cfg[phase].Gd = gs->vpm_output_size;
   vpm_cfg[phase].gs_width = gs->simd_width;

   /* While there is a requirement that Vc >= [Vn / 16], this is
    * always the case when tessellation is not present because in that
    * case Vn can only be 6 at most (when input primitive is triangles
    * with adjacency).
    *
    * We always choose Vc=2. We can't go lower than this due to GFXH-1744,
    * and Broadcom has not found it worth it to increase it beyond this
    * in general. Increasing Vc also increases VPM memory pressure which
    * can turn up being detrimental for performance in some scenarios.
    */
   vpm_cfg[phase].Vc = 2;

   /* Gv is a constraint on the hardware to not exceed the
    * specified number of vertex segments per GS batch. If adding a
    * new primitive to a GS batch would result in a range of more
    * than Gv vertex segments being referenced by the batch, then
    * the hardware will flush the batch and start a new one. This
    * means that we can choose any value we want, we just need to
    * be aware that larger values improve GS batch utilization
    * at the expense of more VPM memory pressure (which can affect
    * other performance aspects, such as GS dispatch width).
    * We start with the largest value, and will reduce it if we
    * find that total memory pressure is too high.
    */
   vpm_cfg[phase].Gv = 3;
   do {
      /* When GS is present in absence of TES, then we need to satisfy
       * that Ve >= Gv. We go with the smallest value of Ve to avoid
       * increasing memory pressure.
       */
      vpm_cfg[phase].Ve = vpm_cfg[phase].Gv;

      uint32_t vpm_sectors =
         A * vpm_cfg[phase].As * Ad +
         (vpm_cfg[phase].Vc + vpm_cfg[phase].Ve) * Vd +
         vpm_cfg[phase].Gs * vpm_cfg[phase].Gd;

      /* Ideally we want to use no more than half of the available
       * memory so we can execute a bin and render program in parallel
       * without stalls. If we achieved that then we are done.
       */
      if (vpm_sectors <= vpm_size / 2) {
         final_vpm_cfg = &vpm_cfg[phase];
         break;
      }

      /* At the very least, we should not allocate more than the
       * total available VPM memory. If we have a configuration that
       * succeeds at this we save it and continue to see if we can
       * meet the half-memory-use criteria too.
       */
      if (phase == 0 && vpm_sectors <= vpm_size) {
         vpm_cfg[1] = vpm_cfg[0];
         phase = 1;
      }

      /* Try lowering Gv */
      if (vpm_cfg[phase].Gv > 0) {
         vpm_cfg[phase].Gv--;
         continue;
      }

      /* Try lowering GS dispatch width */
      if (vpm_cfg[phase].gs_width > 1) {
         do {
            vpm_cfg[phase].gs_width >>= 1;
            vpm_cfg[phase].Gd = align(vpm_cfg[phase].Gd, 2) / 2;
         } while (vpm_cfg[phase].gs_width == 2);

         /* Reset Gv to max after dropping dispatch width */
         vpm_cfg[phase].Gv = 3;
         continue;
      }

      /* We ran out of options to reduce memory pressure. If we
       * are at phase 1 we have at least a valid configuration, so we
       * we use that.
       */
      if (phase == 1)
         final_vpm_cfg = &vpm_cfg[0];
      break;
   } while (true);

   if (!final_vpm_cfg)
      return false;

   assert(final_vpm_cfg);
   assert(final_vpm_cfg->Gd <= 16);
   assert(final_vpm_cfg->Gv < 4);
   assert(final_vpm_cfg->Ve < 4);
   assert(final_vpm_cfg->Vc >= 2 && final_vpm_cfg->Vc <= 4);
   assert(final_vpm_cfg->gs_width == 1 ||
          final_vpm_cfg->gs_width == 4 ||
          final_vpm_cfg->gs_width == 8 ||
          final_vpm_cfg->gs_width == 16);

   *vpm_cfg_out = *final_vpm_cfg;
   return true;
}

bool
v3d_compute_vpm_config(struct v3d_device_info *devinfo,
                       struct v3d_vs_prog_data *vs_bin,
                       struct v3d_vs_prog_data *vs,
                       struct v3d_gs_prog_data *gs_bin,
                       struct v3d_gs_prog_data *gs,
                       struct vpm_config *vpm_cfg_bin,
                       struct vpm_config *vpm_cfg)
{
   assert(vs && vs_bin);
   assert((gs != NULL) == (gs_bin != NULL));

   if (!gs) {
      vpm_cfg_bin->As = 1;
      vpm_cfg_bin->Ve = 0;
      vpm_cfg_bin->Vc = vs_bin->vcm_cache_size;

      vpm_cfg->As = 1;
      vpm_cfg->Ve = 0;
      vpm_cfg->Vc = vs->vcm_cache_size;
   } else {
      if (!compute_vpm_config_gs(devinfo, vs_bin, gs_bin, vpm_cfg_bin))
         return false;

      if (!compute_vpm_config_gs(devinfo, vs, gs, vpm_cfg))
         return false;
   }

   return true;
}
