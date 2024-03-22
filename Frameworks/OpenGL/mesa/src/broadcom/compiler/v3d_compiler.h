/*
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

#ifndef V3D_COMPILER_H
#define V3D_COMPILER_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util/blend.h"
#include "util/macros.h"
#include "common/v3d_debug.h"
#include "common/v3d_device_info.h"
#include "common/v3d_limits.h"
#include "compiler/nir/nir.h"
#include "util/list.h"
#include "util/u_math.h"

#include "qpu/qpu_instr.h"

/**
 * Maximum number of outstanding TMU operations we can queue for execution.
 *
 * This is mostly limited by the size of the TMU fifos. The Input and Config
 * fifos can stall, but we prefer that than injecting TMU flushes manually
 * in the driver, so we can ignore these, but we can't overflow the Output fifo,
 * which has 16 / threads per-thread entries, meaning that the maximum number
 * of outstanding LDTMUs we can ever have is 8, for a 2-way threaded shader.
 * This means that at most we can have 8 outstanding TMU loads, if each load
 * is just one component.
 *
 * NOTE: we could actually have a larger value here because TMU stores don't
 * consume any entries in the Output fifo (so we could have any number of
 * outstanding stores) and the driver keeps track of used Output fifo entries
 * and will flush if we ever needs more than 8, but since loads are much more
 * common than stores, it is probably not worth it.
 */
#define MAX_TMU_QUEUE_SIZE 8

/**
 * Maximum offset distance in bytes between two consecutive constant UBO loads
 * for the same UBO where we would favor updating the unifa address by emitting
 * dummy ldunifa instructions to avoid writing the unifa register.
 */
#define MAX_UNIFA_SKIP_DISTANCE 16

struct nir_builder;

struct v3d_fs_inputs {
        /**
         * Array of the meanings of the VPM inputs this shader needs.
         *
         * It doesn't include those that aren't part of the VPM, like
         * point/line coordinates.
         */
        struct v3d_varying_slot *input_slots;
        uint32_t num_inputs;
};

enum qfile {
        /** An unused source or destination register. */
        QFILE_NULL,

        /** A physical register, such as the W coordinate payload. */
        QFILE_REG,
        /** One of the registers for fixed function interactions. */
        QFILE_MAGIC,

        /**
         *  A virtual register, that will be allocated to actual accumulator
         * or physical registers later.
         */
        QFILE_TEMP,

        /**
         * Stores an immediate value in the index field that will be used
         * directly by qpu_load_imm().
         */
        QFILE_LOAD_IMM,

        /**
         * Stores an immediate value in the index field that can be turned
         * into a small immediate field by qpu_encode_small_immediate().
         */
        QFILE_SMALL_IMM,
};

/**
 * A reference to a QPU register or a virtual temp register.
 */
struct qreg {
        enum qfile file;
        uint32_t index;
};

static inline struct qreg vir_reg(enum qfile file, uint32_t index)
{
        return (struct qreg){file, index};
}

static inline struct qreg vir_magic_reg(uint32_t index)
{
        return (struct qreg){QFILE_MAGIC, index};
}

static inline struct qreg vir_nop_reg(void)
{
        return (struct qreg){QFILE_NULL, 0};
}

/**
 * A reference to an actual register at the QPU level, for register
 * allocation.
 */
struct qpu_reg {
        bool magic;
        bool smimm;
        int index;
};

struct qinst {
        /** Entry in qblock->instructions */
        struct list_head link;

        /**
         * The instruction being wrapped.  Its condition codes, pack flags,
         * signals, etc. will all be used, with just the register references
         * being replaced by the contents of qinst->dst and qinst->src[].
         */
        struct v3d_qpu_instr qpu;

        /* Pre-register-allocation references to src/dst registers */
        struct qreg dst;
        struct qreg src[3];
        bool is_last_thrsw;

        /* If the instruction reads a uniform (other than through src[i].file
         * == QFILE_UNIF), that uniform's index in c->uniform_contents.  ~0
         * otherwise.
         */
        int uniform;

        /* If this is a a TLB Z write */
        bool is_tlb_z_write;

        /* If this is a retiring TMU instruction (the last in a lookup sequence),
         * how many ldtmu instructions are required to read the results.
         */
        uint32_t ldtmu_count;

        /* Position of this instruction in the program. Filled in during
         * register allocation.
         */
        int32_t ip;
};

enum quniform_contents {
        /**
         * Indicates that a constant 32-bit value is copied from the program's
         * uniform contents.
         */
        QUNIFORM_CONSTANT,
        /**
         * Indicates that the program's uniform contents are used as an index
         * into the GL uniform storage.
         */
        QUNIFORM_UNIFORM,

        /** @{
         * Scaling factors from clip coordinates to relative to the viewport
         * center.
         *
         * This is used by the coordinate and vertex shaders to produce the
         * 32-bit entry consisting of 2 16-bit fields with 12.4 signed fixed
         * point offsets from the viewport ccenter.
         */
        QUNIFORM_VIEWPORT_X_SCALE,
        QUNIFORM_VIEWPORT_Y_SCALE,
        /** @} */

        QUNIFORM_VIEWPORT_Z_OFFSET,
        QUNIFORM_VIEWPORT_Z_SCALE,

        QUNIFORM_USER_CLIP_PLANE,

        /**
         * A reference to a V3D 3.x texture config parameter 0 uniform.
         *
         * This is a uniform implicitly loaded with a QPU_W_TMU* write, which
         * defines texture type, miplevels, and such.  It will be found as a
         * parameter to the first QOP_TEX_[STRB] instruction in a sequence.
         */
        QUNIFORM_TEXTURE_CONFIG_P0_0,
        QUNIFORM_TEXTURE_CONFIG_P0_1,
        QUNIFORM_TEXTURE_CONFIG_P0_2,
        QUNIFORM_TEXTURE_CONFIG_P0_3,
        QUNIFORM_TEXTURE_CONFIG_P0_4,
        QUNIFORM_TEXTURE_CONFIG_P0_5,
        QUNIFORM_TEXTURE_CONFIG_P0_6,
        QUNIFORM_TEXTURE_CONFIG_P0_7,
        QUNIFORM_TEXTURE_CONFIG_P0_8,
        QUNIFORM_TEXTURE_CONFIG_P0_9,
        QUNIFORM_TEXTURE_CONFIG_P0_10,
        QUNIFORM_TEXTURE_CONFIG_P0_11,
        QUNIFORM_TEXTURE_CONFIG_P0_12,
        QUNIFORM_TEXTURE_CONFIG_P0_13,
        QUNIFORM_TEXTURE_CONFIG_P0_14,
        QUNIFORM_TEXTURE_CONFIG_P0_15,
        QUNIFORM_TEXTURE_CONFIG_P0_16,
        QUNIFORM_TEXTURE_CONFIG_P0_17,
        QUNIFORM_TEXTURE_CONFIG_P0_18,
        QUNIFORM_TEXTURE_CONFIG_P0_19,
        QUNIFORM_TEXTURE_CONFIG_P0_20,
        QUNIFORM_TEXTURE_CONFIG_P0_21,
        QUNIFORM_TEXTURE_CONFIG_P0_22,
        QUNIFORM_TEXTURE_CONFIG_P0_23,
        QUNIFORM_TEXTURE_CONFIG_P0_24,
        QUNIFORM_TEXTURE_CONFIG_P0_25,
        QUNIFORM_TEXTURE_CONFIG_P0_26,
        QUNIFORM_TEXTURE_CONFIG_P0_27,
        QUNIFORM_TEXTURE_CONFIG_P0_28,
        QUNIFORM_TEXTURE_CONFIG_P0_29,
        QUNIFORM_TEXTURE_CONFIG_P0_30,
        QUNIFORM_TEXTURE_CONFIG_P0_31,
        QUNIFORM_TEXTURE_CONFIG_P0_32,

        /**
         * A reference to a V3D 3.x texture config parameter 1 uniform.
         *
         * This is a uniform implicitly loaded with a QPU_W_TMU* write, which
         * has the pointer to the indirect texture state.  Our data[] field
         * will have a packed p1 value, but the address field will be just
         * which texture unit's texture should be referenced.
         */
        QUNIFORM_TEXTURE_CONFIG_P1,

        /* A V3D 4.x texture config parameter.  The high 8 bits will be
         * which texture or sampler is being sampled, and the driver must
         * replace the address field with the appropriate address.
         */
        QUNIFORM_TMU_CONFIG_P0,
        QUNIFORM_TMU_CONFIG_P1,

        QUNIFORM_IMAGE_TMU_CONFIG_P0,

        QUNIFORM_TEXTURE_FIRST_LEVEL,

        QUNIFORM_TEXTURE_WIDTH,
        QUNIFORM_TEXTURE_HEIGHT,
        QUNIFORM_TEXTURE_DEPTH,
        QUNIFORM_TEXTURE_ARRAY_SIZE,
        QUNIFORM_TEXTURE_LEVELS,
        QUNIFORM_TEXTURE_SAMPLES,

        QUNIFORM_UBO_ADDR,

        QUNIFORM_TEXRECT_SCALE_X,
        QUNIFORM_TEXRECT_SCALE_Y,

        /* Returns the base offset of the SSBO given by the data value. */
        QUNIFORM_SSBO_OFFSET,

        /* Returns the size of the SSBO or UBO given by the data value. */
        QUNIFORM_GET_SSBO_SIZE,
        QUNIFORM_GET_UBO_SIZE,

        /* Sizes (in pixels) of a shader image given by the data value. */
        QUNIFORM_IMAGE_WIDTH,
        QUNIFORM_IMAGE_HEIGHT,
        QUNIFORM_IMAGE_DEPTH,
        QUNIFORM_IMAGE_ARRAY_SIZE,

        QUNIFORM_LINE_WIDTH,

        /* The line width sent to hardware. This includes the expanded width
         * when anti-aliasing is enabled.
         */
        QUNIFORM_AA_LINE_WIDTH,

        /* Number of workgroups passed to glDispatchCompute in the dimension
         * selected by the data value.
         */
        QUNIFORM_NUM_WORK_GROUPS,

        /* Base workgroup offset passed to vkCmdDispatchBase in the dimension
         * selected by the data value.
         */
        QUNIFORM_WORK_GROUP_BASE,

        /**
         * Returns the the offset of the scratch buffer for register spilling.
         */
        QUNIFORM_SPILL_OFFSET,
        QUNIFORM_SPILL_SIZE_PER_THREAD,

        /**
         * Returns the offset of the shared memory for compute shaders.
         *
         * This will be accessed using TMU general memory operations, so the
         * L2T cache will effectively be the shared memory area.
         */
        QUNIFORM_SHARED_OFFSET,

        /**
         * Returns the number of layers in the framebuffer.
         *
         * This is used to cap gl_Layer in geometry shaders to avoid
         * out-of-bounds accesses into the tile state during binning.
         */
        QUNIFORM_FB_LAYERS,

        /**
         * Current value of gl_ViewIndex for Multiview rendering.
         */
        QUNIFORM_VIEW_INDEX,

        /**
         * Inline uniform buffers
         */
         QUNIFORM_INLINE_UBO_0,
         QUNIFORM_INLINE_UBO_1,
         QUNIFORM_INLINE_UBO_2,
         QUNIFORM_INLINE_UBO_3,

        /**
         * Current value of DrawIndex for Multidraw
         */
        QUNIFORM_DRAW_ID,
};

static inline uint32_t v3d_unit_data_create(uint32_t unit, uint32_t value)
{
        assert(value < (1 << 24));
        return unit << 24 | value;
}

static inline uint32_t v3d_unit_data_get_unit(uint32_t data)
{
        return data >> 24;
}

static inline uint32_t v3d_unit_data_get_offset(uint32_t data)
{
        return data & 0xffffff;
}

struct v3d_varying_slot {
        uint8_t slot_and_component;
};

static inline struct v3d_varying_slot
v3d_slot_from_slot_and_component(uint8_t slot, uint8_t component)
{
        assert(slot < 255 / 4);
        return (struct v3d_varying_slot){ (slot << 2) + component };
}

static inline uint8_t v3d_slot_get_slot(struct v3d_varying_slot slot)
{
        return slot.slot_and_component >> 2;
}

static inline uint8_t v3d_slot_get_component(struct v3d_varying_slot slot)
{
        return slot.slot_and_component & 3;
}

struct v3d_key {
        struct {
                uint8_t swizzle[4];
        } tex[V3D_MAX_TEXTURE_SAMPLERS];
        struct {
                uint8_t return_size;
                uint8_t return_channels;
        } sampler[V3D_MAX_TEXTURE_SAMPLERS];

        uint8_t num_tex_used;
        uint8_t num_samplers_used;
        uint8_t ucp_enables;
        bool is_last_geometry_stage;
        bool robust_uniform_access;
        bool robust_storage_access;
        bool robust_image_access;
};

struct v3d_fs_key {
        struct v3d_key base;
        bool is_points;
        bool is_lines;
        bool line_smoothing;
        bool point_coord_upper_left;
        bool msaa;
        bool sample_alpha_to_coverage;
        bool sample_alpha_to_one;
        /* Mask of which color render targets are present. */
        uint8_t cbufs;
        uint8_t swap_color_rb;
        /* Mask of which render targets need to be written as 32-bit floats */
        uint8_t f32_color_rb;
        /* Masks of which render targets need to be written as ints/uints.
         * Used by gallium to work around lost information in TGSI.
         */
        uint8_t int_color_rb;
        uint8_t uint_color_rb;

        /* Color format information per render target. Only set when logic
         * operations are enabled.
         */
        struct {
                enum pipe_format format;
                uint8_t swizzle[4];
        } color_fmt[V3D_MAX_DRAW_BUFFERS];

        enum pipe_logicop logicop_func;
        uint32_t point_sprite_mask;

        /* If the fragment shader reads gl_PrimitiveID then we have 2 scenarios:
         *
         * - If there is a geometry shader, then gl_PrimitiveID must be written
         *   by it and the fragment shader loads it as a regular explicit input
         *   varying. This is the only valid use case in GLES 3.1.
         *
         * - If there is not a geometry shader (allowed since GLES 3.2 and
         *   Vulkan 1.0), then gl_PrimitiveID must be implicitly written by
         *   hardware and is considered an implicit input varying in the
         *   fragment shader.
         */
        bool has_gs;
};

struct v3d_gs_key {
        struct v3d_key base;

        struct v3d_varying_slot used_outputs[V3D_MAX_FS_INPUTS];
        uint8_t num_used_outputs;

        bool is_coord;
        bool per_vertex_point_size;
};

struct v3d_vs_key {
        struct v3d_key base;

        struct v3d_varying_slot used_outputs[V3D_MAX_ANY_STAGE_INPUTS];
        uint8_t num_used_outputs;

        /* A bit-mask indicating if we need to swap the R/B channels for
         * vertex attributes. Since the hardware doesn't provide any
         * means to swizzle vertex attributes we need to do it in the shader.
         */
        uint32_t va_swap_rb_mask;

        bool is_coord;
        bool per_vertex_point_size;
        bool clamp_color;
};

/** A basic block of VIR instructions. */
struct qblock {
        struct list_head link;

        struct list_head instructions;

        struct set *predecessors;
        struct qblock *successors[2];

        int index;

        /* Instruction IPs for the first and last instruction of the block.
         * Set by qpu_schedule.c.
         */
        uint32_t start_qpu_ip;
        uint32_t end_qpu_ip;

        /* Instruction IP for the branch instruction of the block.  Set by
         * qpu_schedule.c.
         */
        uint32_t branch_qpu_ip;

        /** Offset within the uniform stream at the start of the block. */
        uint32_t start_uniform;
        /** Offset within the uniform stream of the branch instruction */
        uint32_t branch_uniform;

        /**
         * Has the terminating branch of this block already been emitted
         * by a break or continue?
         */
        bool branch_emitted;

        /** @{ used by v3d_vir_live_variables.c */
        BITSET_WORD *def;
        BITSET_WORD *defin;
        BITSET_WORD *defout;
        BITSET_WORD *use;
        BITSET_WORD *live_in;
        BITSET_WORD *live_out;
        int start_ip, end_ip;
        /** @} */
};

/** Which util/list.h add mode we should use when inserting an instruction. */
enum vir_cursor_mode {
        vir_cursor_add,
        vir_cursor_addtail,
};

/**
 * Tracking structure for where new instructions should be inserted.  Create
 * with one of the vir_after_inst()-style helper functions.
 *
 * This does not protect against removal of the block or instruction, so we
 * have an assert in instruction removal to try to catch it.
 */
struct vir_cursor {
        enum vir_cursor_mode mode;
        struct list_head *link;
};

static inline struct vir_cursor
vir_before_inst(struct qinst *inst)
{
        return (struct vir_cursor){ vir_cursor_addtail, &inst->link };
}

static inline struct vir_cursor
vir_after_inst(struct qinst *inst)
{
        return (struct vir_cursor){ vir_cursor_add, &inst->link };
}

static inline struct vir_cursor
vir_before_block(struct qblock *block)
{
        return (struct vir_cursor){ vir_cursor_add, &block->instructions };
}

static inline struct vir_cursor
vir_after_block(struct qblock *block)
{
        return (struct vir_cursor){ vir_cursor_addtail, &block->instructions };
}

enum v3d_compilation_result {
        V3D_COMPILATION_SUCCEEDED,
        V3D_COMPILATION_FAILED_REGISTER_ALLOCATION,
        V3D_COMPILATION_FAILED,
};

/**
 * Compiler state saved across compiler invocations, for any expensive global
 * setup.
 */
struct v3d_compiler {
        const struct v3d_device_info *devinfo;
        uint32_t max_inline_uniform_buffers;
        struct ra_regs *regs;
        struct ra_class *reg_class_any[3];
        struct ra_class *reg_class_r5[3];
        struct ra_class *reg_class_phys[3];
        struct ra_class *reg_class_phys_or_acc[3];
};

/**
 * This holds partially interpolated inputs as provided by hardware
 * (The Vp = A*(x - x0) + B*(y - y0) term), as well as the C coefficient
 * required to compute the final interpolated value.
 */
struct v3d_interp_input {
   struct qreg vp;
   struct qreg C;
   unsigned mode; /* interpolation mode */
};

struct v3d_ra_node_info {
        struct {
                uint32_t priority;
                uint8_t class_bits;
                bool is_program_end;
                bool unused;

                /* V3D 7.x */
                bool is_ldunif_dst;
        } *info;
        uint32_t alloc_count;
};

struct v3d_compile {
        const struct v3d_device_info *devinfo;
        nir_shader *s;
        nir_function_impl *impl;
        struct exec_list *cf_node_list;
        const struct v3d_compiler *compiler;

        void (*debug_output)(const char *msg,
                             void *debug_output_data);
        void *debug_output_data;

        /**
         * Mapping from nir_register * or nir_def * to array of struct
         * qreg for the values.
         */
        struct hash_table *def_ht;

        /* For each temp, the instruction generating its value. */
        struct qinst **defs;
        uint32_t defs_array_size;

        /* TMU pipelining tracking */
        struct {
                /* NIR registers that have been updated with a TMU operation
                 * that has not been flushed yet.
                 */
                struct set *outstanding_regs;

                uint32_t output_fifo_size;

                struct {
                        nir_def *def;
                        uint8_t num_components;
                        uint8_t component_mask;
                } flush[MAX_TMU_QUEUE_SIZE];
                uint32_t flush_count;
                uint32_t total_count;
        } tmu;

        /**
         * Inputs to the shader, arranged by TGSI declaration order.
         *
         * Not all fragment shader QFILE_VARY reads are present in this array.
         */
        struct qreg *inputs;
        /**
         * Partially interpolated inputs to the shader.
         */
        struct v3d_interp_input *interp;
        struct qreg *outputs;
        bool msaa_per_sample_output;
        struct qreg color_reads[V3D_MAX_DRAW_BUFFERS * V3D_MAX_SAMPLES * 4];
        struct qreg sample_colors[V3D_MAX_DRAW_BUFFERS * V3D_MAX_SAMPLES * 4];
        uint32_t inputs_array_size;
        uint32_t outputs_array_size;
        uint32_t uniforms_array_size;

        /* Booleans for whether the corresponding QFILE_VARY[i] is
         * flat-shaded.  This includes gl_FragColor flat-shading, which is
         * customized based on the shademodel_flat shader key.
         */
        uint32_t flat_shade_flags[BITSET_WORDS(V3D_MAX_FS_INPUTS)];

        uint32_t noperspective_flags[BITSET_WORDS(V3D_MAX_FS_INPUTS)];

        uint32_t centroid_flags[BITSET_WORDS(V3D_MAX_FS_INPUTS)];

        bool uses_center_w;
        bool writes_z;
        bool writes_z_from_fep;
        bool reads_z;
        bool uses_implicit_point_line_varyings;

        /* True if a fragment shader reads gl_PrimitiveID */
        bool fs_uses_primitive_id;

        /* Whether we are using the fallback scheduler. This will be set after
         * register allocation has failed once.
         */
        bool fallback_scheduler;

        /* Disable TMU pipelining. This may increase the chances of being able
         * to compile shaders with high register pressure that require to emit
         * TMU spills.
         */
        bool disable_tmu_pipelining;
        bool pipelined_any_tmu;

        /* Disable sorting of UBO loads with constant offset. This may
         * increase the chances of being able to compile shaders with high
         * register pressure.
         */
        bool disable_constant_ubo_load_sorting;
        bool sorted_any_ubo_loads;

        /* Moves UBO/SSBO loads right before their first user (nir_opt_move).
         * This can reduce register pressure.
         */
        bool move_buffer_loads;

        /* Emits ldunif for each new uniform, even if the uniform was already
         * emitted in the same block. Useful to compile shaders with high
         * register pressure or to disable the optimization during uniform
         * spills.
         */
        bool disable_ldunif_opt;

        /* Disables loop unrolling to reduce register pressure. */
        bool disable_loop_unrolling;
        bool unrolled_any_loops;

        /* Disables nir_opt_gcm to reduce register pressure. */
        bool disable_gcm;

        /* If calling nir_opt_gcm made any progress. Used to skip new rebuilds
         * if possible
         */
        bool gcm_progress;

        /* Disables scheduling of general TMU loads (and unfiltered image load).
         */
        bool disable_general_tmu_sched;
        bool has_general_tmu_load;

        /* Minimum number of threads we are willing to use to register allocate
         * a shader with the current compilation strategy. This only prevents
         * us from lowering the thread count to register allocate successfully,
         * which can be useful when we prefer doing other changes to the
         * compilation strategy before dropping thread count.
         */
        uint32_t min_threads_for_reg_alloc;

        /* Whether TMU spills are allowed. If this is disabled it may cause
         * register allocation to fail. We set this to favor other compilation
         * strategies that can reduce register pressure and hopefully reduce or
         * eliminate TMU spills in the shader.
         */
        uint32_t max_tmu_spills;

        uint32_t compile_strategy_idx;

        /* The UBO index and block used with the last unifa load, as well as the
         * current unifa offset *after* emitting that load. This is used to skip
         * unifa writes (and their 3 delay slot) when the next UBO load reads
         * right after the previous one in the same block.
         */
        struct qblock *current_unifa_block;
        int32_t current_unifa_index;
        uint32_t current_unifa_offset;
        bool current_unifa_is_ubo;

        /* State for whether we're executing on each channel currently.  0 if
         * yes, otherwise a block number + 1 that the channel jumped to.
         */
        struct qreg execute;
        bool in_control_flow;

        struct qreg line_x, point_x, point_y, primitive_id;

        /**
         * Instance ID, which comes in before the vertex attribute payload if
         * the shader record requests it.
         */
        struct qreg iid;

        /**
         * Base Instance ID, which comes in before the vertex attribute payload
         * (after Instance ID) if the shader record requests it.
         */
        struct qreg biid;

        /**
         * Vertex ID, which comes in before the vertex attribute payload
         * (after Base Instance) if the shader record requests it.
         */
        struct qreg vid;

        /* Fragment shader payload regs. */
        struct qreg payload_w, payload_w_centroid, payload_z;

        struct qreg cs_payload[2];
        struct qreg cs_shared_offset;
        int local_invocation_index_bits;

        /* If the shader uses subgroup functionality */
        bool has_subgroups;

        uint8_t vattr_sizes[V3D_MAX_VS_INPUTS / 4];
        uint32_t vpm_output_size;

        /* Size in bytes of registers that have been spilled. This is how much
         * space needs to be available in the spill BO per thread per QPU.
         */
        uint32_t spill_size;
        /* Shader-db stats */
        uint32_t spills, fills, loops;

        /* Whether we are in the process of spilling registers for
         * register allocation
         */
        bool spilling;

        /**
         * Register spilling's per-thread base address, shared between each
         * spill/fill's addressing calculations (also used for scratch
         * access).
         */
        struct qreg spill_base;

        /* Bit vector of which temps may be spilled */
        BITSET_WORD *spillable;

        /* Used during register allocation */
        int thread_index;
        struct v3d_ra_node_info nodes;
        struct ra_graph *g;

        /**
         * Array of the VARYING_SLOT_* of all FS QFILE_VARY reads.
         *
         * This includes those that aren't part of the VPM varyings, like
         * point/line coordinates.
         */
        struct v3d_varying_slot input_slots[V3D_MAX_FS_INPUTS];

        /**
         * An entry per outputs[] in the VS indicating what the VARYING_SLOT_*
         * of the output is.  Used to emit from the VS in the order that the
         * FS needs.
         */
        struct v3d_varying_slot *output_slots;

        struct pipe_shader_state *shader_state;
        struct v3d_key *key;
        struct v3d_fs_key *fs_key;
        struct v3d_gs_key *gs_key;
        struct v3d_vs_key *vs_key;

        /* Live ranges of temps. */
        int *temp_start, *temp_end;
        bool live_intervals_valid;

        uint32_t *uniform_data;
        enum quniform_contents *uniform_contents;
        uint32_t uniform_array_size;
        uint32_t num_uniforms;
        uint32_t output_position_index;
        nir_variable *output_color_var[V3D_MAX_DRAW_BUFFERS];
        uint32_t output_sample_mask_index;

        struct qreg undef;
        uint32_t num_temps;
        /* Number of temps in the program right before we spill a new temp. We
         * use this to know which temps existed before a spill and which were
         * added with the spill itself.
         */
        uint32_t spill_start_num_temps;

        struct vir_cursor cursor;
        struct list_head blocks;
        int next_block_index;
        struct qblock *cur_block;
        struct qblock *loop_cont_block;
        struct qblock *loop_break_block;
        /**
         * Which temp, if any, do we currently have in the flags?
         * This is set when processing a comparison instruction, and
         * reset to -1 by anything else that touches the flags.
         */
        int32_t flags_temp;
        enum v3d_qpu_cond flags_cond;

        uint64_t *qpu_insts;
        uint32_t qpu_inst_count;
        uint32_t qpu_inst_size;
        uint32_t qpu_inst_stalled_count;
        uint32_t nop_count;

        /* For the FS, the number of varying inputs not counting the
         * point/line varyings payload
         */
        uint32_t num_inputs;

        uint32_t program_id;
        uint32_t variant_id;

        /* Set to compile program in in 1x, 2x, or 4x threaded mode, where
         * SIG_THREAD_SWITCH is used to hide texturing latency at the cost of
         * limiting ourselves to the part of the physical reg space.
         *
         * On V3D 3.x, 2x or 4x divide the physical reg space by 2x or 4x.  On
         * V3D 4.x, all shaders are 2x threaded, and 4x only divides the
         * physical reg space in half.
         */
        uint8_t threads;
        struct qinst *last_thrsw;
        bool last_thrsw_at_top_level;

        bool emitted_tlb_load;
        bool lock_scoreboard_on_first_thrsw;

        enum v3d_compilation_result compilation_result;

        bool tmu_dirty_rcl;
        bool has_global_address;

        /* If we have processed a discard/terminate instruction. This may
         * cause some lanes to be inactive even during uniform control
         * flow.
         */
        bool emitted_discard;
};

struct v3d_uniform_list {
        enum quniform_contents *contents;
        uint32_t *data;
        uint32_t count;
};

struct v3d_prog_data {
        struct v3d_uniform_list uniforms;

        uint32_t spill_size;
        uint32_t tmu_spills;
        uint32_t tmu_fills;
        uint32_t tmu_count;

        uint32_t qpu_read_stalls;

        uint8_t compile_strategy_idx;

        uint8_t threads;

        /* For threads > 1, whether the program should be dispatched in the
         * after-final-THRSW state.
         */
        bool single_seg;

        bool tmu_dirty_rcl;

        bool has_control_barrier;

        bool has_global_address;
};

struct v3d_vs_prog_data {
        struct v3d_prog_data base;

        bool uses_iid, uses_biid, uses_vid;

        /* Number of components read from each vertex attribute. */
        uint8_t vattr_sizes[V3D_MAX_VS_INPUTS / 4];

        /* Total number of components read, for the shader state record. */
        uint32_t vpm_input_size;

        /* Total number of components written, for the shader state record. */
        uint32_t vpm_output_size;

        /* Set if there should be separate VPM segments for input and output.
         * If unset, vpm_input_size will be 0.
         */
        bool separate_segments;

        /* Value to be programmed in VCM_CACHE_SIZE. */
        uint8_t vcm_cache_size;

        /* Maps the nir->data.location to its
         * nir->data.driver_location. In general we are using the
         * driver location as index (like vattr_sizes above), so this
         * map is useful when what we have is the location
         *
         * Returns -1 if the location is not used
         */
        int32_t driver_location_map[V3D_MAX_VS_INPUTS];
};

struct v3d_gs_prog_data {
        struct v3d_prog_data base;

        /* Whether the program reads gl_PrimitiveIDIn */
        bool uses_pid;

        /* Number of components read from each input varying. */
        uint8_t input_sizes[V3D_MAX_GS_INPUTS / 4];

        /* Number of inputs */
        uint8_t num_inputs;
        struct v3d_varying_slot input_slots[V3D_MAX_GS_INPUTS];

        /* Total number of components written, for the shader state record. */
        uint32_t vpm_output_size;

        /* Maximum SIMD dispatch width to not exceed VPM output size limits
         * in the geometry shader. Notice that the final dispatch width has to
         * be decided at draw time and could be lower based on the VPM pressure
         * added by other shader stages.
         */
        uint8_t simd_width;

        /* Output primitive type */
        uint8_t out_prim_type;

        /* Number of GS invocations */
        uint8_t num_invocations;

        bool writes_psiz;
};

struct v3d_fs_prog_data {
        struct v3d_prog_data base;

        /* Whether the program reads gl_PrimitiveID */
        bool uses_pid;

        struct v3d_varying_slot input_slots[V3D_MAX_FS_INPUTS];

        /* Array of flat shade flags.
         *
         * Each entry is only 24 bits (high 8 bits 0), to match the hardware
         * packet layout.
         */
        uint32_t flat_shade_flags[((V3D_MAX_FS_INPUTS - 1) / 24) + 1];

        uint32_t noperspective_flags[((V3D_MAX_FS_INPUTS - 1) / 24) + 1];

        uint32_t centroid_flags[((V3D_MAX_FS_INPUTS - 1) / 24) + 1];

        uint8_t num_inputs;
        bool writes_z;
        bool writes_z_from_fep;
        bool disable_ez;
        bool uses_center_w;
        bool uses_implicit_point_line_varyings;
        bool lock_scoreboard_on_first_thrsw;

        /* If the fragment shader does anything that requires to force
         * per-sample MSAA, such as reading gl_SampleID.
         */
        bool force_per_sample_msaa;
};

struct v3d_compute_prog_data {
        struct v3d_prog_data base;
        /* Size in bytes of the workgroup's shared space. */
        uint32_t shared_size;
        uint16_t local_size[3];
        /* If the shader uses subgroup functionality */
        bool has_subgroups;
};

struct vpm_config {
   uint32_t As;
   uint32_t Vc;
   uint32_t Gs;
   uint32_t Gd;
   uint32_t Gv;
   uint32_t Ve;
   uint32_t gs_width;
};

bool
v3d_compute_vpm_config(struct v3d_device_info *devinfo,
                       struct v3d_vs_prog_data *vs_bin,
                       struct v3d_vs_prog_data *vs,
                       struct v3d_gs_prog_data *gs_bin,
                       struct v3d_gs_prog_data *gs,
                       struct vpm_config *vpm_cfg_bin,
                       struct vpm_config *vpm_cfg);
void
v3d_pack_unnormalized_coordinates(struct v3d_device_info *devinfo,
                                  uint32_t *p1_packed,
                                  bool unnormalized_coordinates);

static inline bool
vir_has_uniform(struct qinst *inst)
{
        return inst->uniform != ~0;
}

const struct v3d_compiler *v3d_compiler_init(const struct v3d_device_info *devinfo,
                                             uint32_t max_inline_uniform_buffers);
void v3d_compiler_free(const struct v3d_compiler *compiler);
void v3d_optimize_nir(struct v3d_compile *c, struct nir_shader *s);

uint64_t *v3d_compile(const struct v3d_compiler *compiler,
                      struct v3d_key *key,
                      struct v3d_prog_data **prog_data,
                      nir_shader *s,
                      void (*debug_output)(const char *msg,
                                           void *debug_output_data),
                      void *debug_output_data,
                      int program_id, int variant_id,
                      uint32_t *final_assembly_size);

uint32_t v3d_prog_data_size(gl_shader_stage stage);
void v3d_nir_to_vir(struct v3d_compile *c);

void vir_compile_destroy(struct v3d_compile *c);
const char *vir_get_stage_name(struct v3d_compile *c);
struct qblock *vir_new_block(struct v3d_compile *c);
void vir_set_emit_block(struct v3d_compile *c, struct qblock *block);
void vir_link_blocks(struct qblock *predecessor, struct qblock *successor);
struct qblock *vir_entry_block(struct v3d_compile *c);
struct qblock *vir_exit_block(struct v3d_compile *c);
struct qinst *vir_add_inst(enum v3d_qpu_add_op op, struct qreg dst,
                           struct qreg src0, struct qreg src1);
struct qinst *vir_mul_inst(enum v3d_qpu_mul_op op, struct qreg dst,
                           struct qreg src0, struct qreg src1);
struct qinst *vir_branch_inst(struct v3d_compile *c,
                              enum v3d_qpu_branch_cond cond);
void vir_remove_instruction(struct v3d_compile *c, struct qinst *qinst);
uint32_t vir_get_uniform_index(struct v3d_compile *c,
                               enum quniform_contents contents,
                               uint32_t data);
struct qreg vir_uniform(struct v3d_compile *c,
                        enum quniform_contents contents,
                        uint32_t data);
void vir_schedule_instructions(struct v3d_compile *c);
void v3d_setup_spill_base(struct v3d_compile *c);
struct v3d_qpu_instr v3d_qpu_nop(void);

struct qreg vir_emit_def(struct v3d_compile *c, struct qinst *inst);
struct qinst *vir_emit_nondef(struct v3d_compile *c, struct qinst *inst);
void vir_set_cond(struct qinst *inst, enum v3d_qpu_cond cond);
enum v3d_qpu_cond vir_get_cond(struct qinst *inst);
void vir_set_pf(struct v3d_compile *c, struct qinst *inst, enum v3d_qpu_pf pf);
void vir_set_uf(struct v3d_compile *c, struct qinst *inst, enum v3d_qpu_uf uf);
void vir_set_unpack(struct qinst *inst, int src,
                    enum v3d_qpu_input_unpack unpack);
void vir_set_pack(struct qinst *inst, enum v3d_qpu_output_pack pack);

struct qreg vir_get_temp(struct v3d_compile *c);
void vir_calculate_live_intervals(struct v3d_compile *c);
int vir_get_nsrc(struct qinst *inst);
bool vir_has_side_effects(struct v3d_compile *c, struct qinst *inst);
bool vir_get_add_op(struct qinst *inst, enum v3d_qpu_add_op *op);
bool vir_get_mul_op(struct qinst *inst, enum v3d_qpu_mul_op *op);
bool vir_is_raw_mov(struct qinst *inst);
bool vir_is_tex(const struct v3d_device_info *devinfo, struct qinst *inst);
bool vir_is_add(struct qinst *inst);
bool vir_is_mul(struct qinst *inst);
bool vir_writes_r4_implicitly(const struct v3d_device_info *devinfo, struct qinst *inst);
struct qreg vir_follow_movs(struct v3d_compile *c, struct qreg reg);
uint8_t vir_channels_written(struct qinst *inst);
struct qreg ntq_get_src(struct v3d_compile *c, nir_src src, int i);
void ntq_store_def(struct v3d_compile *c, nir_def *def, int chan,
                   struct qreg result);
bool ntq_tmu_fifo_overflow(struct v3d_compile *c, uint32_t components);
void ntq_add_pending_tmu_flush(struct v3d_compile *c, nir_def *def,
                               uint32_t component_mask);
void ntq_flush_tmu(struct v3d_compile *c);
void vir_emit_thrsw(struct v3d_compile *c);

void vir_dump(struct v3d_compile *c);
void vir_dump_inst(struct v3d_compile *c, struct qinst *inst);
void vir_dump_uniform(enum quniform_contents contents, uint32_t data);

void vir_validate(struct v3d_compile *c);

void vir_optimize(struct v3d_compile *c);
bool vir_opt_algebraic(struct v3d_compile *c);
bool vir_opt_constant_folding(struct v3d_compile *c);
bool vir_opt_copy_propagate(struct v3d_compile *c);
bool vir_opt_dead_code(struct v3d_compile *c);
bool vir_opt_peephole_sf(struct v3d_compile *c);
bool vir_opt_redundant_flags(struct v3d_compile *c);
bool vir_opt_small_immediates(struct v3d_compile *c);
bool vir_opt_vpm(struct v3d_compile *c);
bool vir_opt_constant_alu(struct v3d_compile *c);
bool v3d_nir_lower_io(nir_shader *s, struct v3d_compile *c);
bool v3d_nir_lower_line_smooth(nir_shader *shader);
bool v3d_nir_lower_logic_ops(nir_shader *s, struct v3d_compile *c);
bool v3d_nir_lower_scratch(nir_shader *s);
bool v3d_nir_lower_txf_ms(nir_shader *s);
bool v3d_nir_lower_image_load_store(nir_shader *s, struct v3d_compile *c);
bool v3d_nir_lower_load_store_bitsize(nir_shader *s);

void v3d_vir_emit_tex(struct v3d_compile *c, nir_tex_instr *instr);
void v3d_vir_emit_image_load_store(struct v3d_compile *c,
                                   nir_intrinsic_instr *instr);

void v3d_vir_to_qpu(struct v3d_compile *c, struct qpu_reg *temp_registers);
uint32_t v3d_qpu_schedule_instructions(struct v3d_compile *c);
void qpu_validate(struct v3d_compile *c);
struct qpu_reg *v3d_register_allocate(struct v3d_compile *c);
bool vir_init_reg_sets(struct v3d_compiler *compiler);

int v3d_shaderdb_dump(struct v3d_compile *c, char **shaderdb_str);

bool v3d_gl_format_is_return_32(enum pipe_format format);

uint32_t
v3d_get_op_for_atomic_add(nir_intrinsic_instr *instr, unsigned src);

static inline bool
quniform_contents_is_texture_p0(enum quniform_contents contents)
{
        return (contents >= QUNIFORM_TEXTURE_CONFIG_P0_0 &&
                contents < (QUNIFORM_TEXTURE_CONFIG_P0_0 +
                            V3D_MAX_TEXTURE_SAMPLERS));
}

static inline bool
vir_in_nonuniform_control_flow(struct v3d_compile *c)
{
        return c->execute.file != QFILE_NULL;
}

static inline struct qreg
vir_uniform_ui(struct v3d_compile *c, uint32_t ui)
{
        return vir_uniform(c, QUNIFORM_CONSTANT, ui);
}

static inline struct qreg
vir_uniform_f(struct v3d_compile *c, float f)
{
        return vir_uniform(c, QUNIFORM_CONSTANT, fui(f));
}

#define VIR_ALU0(name, vir_inst, op)                                     \
static inline struct qreg                                                \
vir_##name(struct v3d_compile *c)                                        \
{                                                                        \
        return vir_emit_def(c, vir_inst(op, c->undef,                    \
                                        c->undef, c->undef));            \
}                                                                        \
static inline struct qinst *                                             \
vir_##name##_dest(struct v3d_compile *c, struct qreg dest)               \
{                                                                        \
        return vir_emit_nondef(c, vir_inst(op, dest,                     \
                                           c->undef, c->undef));         \
}

#define VIR_ALU1(name, vir_inst, op)                                     \
static inline struct qreg                                                \
vir_##name(struct v3d_compile *c, struct qreg a)                         \
{                                                                        \
        return vir_emit_def(c, vir_inst(op, c->undef,                    \
                                        a, c->undef));                   \
}                                                                        \
static inline struct qinst *                                             \
vir_##name##_dest(struct v3d_compile *c, struct qreg dest,               \
                  struct qreg a)                                         \
{                                                                        \
        return vir_emit_nondef(c, vir_inst(op, dest, a,          \
                                           c->undef));                   \
}

#define VIR_ALU2(name, vir_inst, op)                                       \
static inline struct qreg                                                \
vir_##name(struct v3d_compile *c, struct qreg a, struct qreg b)          \
{                                                                        \
        return vir_emit_def(c, vir_inst(op, c->undef, a, b));    \
}                                                                        \
static inline struct qinst *                                             \
vir_##name##_dest(struct v3d_compile *c, struct qreg dest,               \
                  struct qreg a, struct qreg b)                          \
{                                                                        \
        return vir_emit_nondef(c, vir_inst(op, dest, a, b));     \
}

#define VIR_NODST_0(name, vir_inst, op)                                 \
static inline struct qinst *                                            \
vir_##name(struct v3d_compile *c)                                       \
{                                                                       \
        return vir_emit_nondef(c, vir_inst(op, c->undef,                \
                                           c->undef, c->undef));        \
}

#define VIR_NODST_1(name, vir_inst, op)                                               \
static inline struct qinst *                                            \
vir_##name(struct v3d_compile *c, struct qreg a)                        \
{                                                                       \
        return vir_emit_nondef(c, vir_inst(op, c->undef,        \
                                           a, c->undef));               \
}

#define VIR_NODST_2(name, vir_inst, op)                                               \
static inline struct qinst *                                            \
vir_##name(struct v3d_compile *c, struct qreg a, struct qreg b)         \
{                                                                       \
        return vir_emit_nondef(c, vir_inst(op, c->undef,                \
                                           a, b));                      \
}

#define VIR_SFU(name)                                                      \
static inline struct qreg                                                \
vir_##name(struct v3d_compile *c, struct qreg a)                         \
{                                                                       \
        return vir_emit_def(c, vir_add_inst(V3D_QPU_A_##name,           \
                                            c->undef,                   \
                                            a, c->undef));              \
}                                                                        \
static inline struct qinst *                                             \
vir_##name##_dest(struct v3d_compile *c, struct qreg dest,               \
                  struct qreg a)                                         \
{                                                                        \
        return vir_emit_nondef(c, vir_add_inst(V3D_QPU_A_##name,        \
                                               dest,                    \
                                               a, c->undef));           \
}

#define VIR_A_ALU2(name) VIR_ALU2(name, vir_add_inst, V3D_QPU_A_##name)
#define VIR_M_ALU2(name) VIR_ALU2(name, vir_mul_inst, V3D_QPU_M_##name)
#define VIR_A_ALU1(name) VIR_ALU1(name, vir_add_inst, V3D_QPU_A_##name)
#define VIR_M_ALU1(name) VIR_ALU1(name, vir_mul_inst, V3D_QPU_M_##name)
#define VIR_A_ALU0(name) VIR_ALU0(name, vir_add_inst, V3D_QPU_A_##name)
#define VIR_M_ALU0(name) VIR_ALU0(name, vir_mul_inst, V3D_QPU_M_##name)
#define VIR_A_NODST_2(name) VIR_NODST_2(name, vir_add_inst, V3D_QPU_A_##name)
#define VIR_M_NODST_2(name) VIR_NODST_2(name, vir_mul_inst, V3D_QPU_M_##name)
#define VIR_A_NODST_1(name) VIR_NODST_1(name, vir_add_inst, V3D_QPU_A_##name)
#define VIR_M_NODST_1(name) VIR_NODST_1(name, vir_mul_inst, V3D_QPU_M_##name)
#define VIR_A_NODST_0(name) VIR_NODST_0(name, vir_add_inst, V3D_QPU_A_##name)

VIR_A_ALU2(FADD)
VIR_A_ALU2(VFPACK)
VIR_A_ALU2(FSUB)
VIR_A_ALU2(FMIN)
VIR_A_ALU2(FMAX)

VIR_A_ALU2(ADD)
VIR_A_ALU2(SUB)
VIR_A_ALU2(SHL)
VIR_A_ALU2(SHR)
VIR_A_ALU2(ASR)
VIR_A_ALU2(ROR)
VIR_A_ALU2(MIN)
VIR_A_ALU2(MAX)
VIR_A_ALU2(UMIN)
VIR_A_ALU2(UMAX)
VIR_A_ALU2(AND)
VIR_A_ALU2(OR)
VIR_A_ALU2(XOR)
VIR_A_ALU2(VADD)
VIR_A_ALU2(VSUB)
VIR_A_NODST_2(STVPMV)
VIR_A_NODST_2(STVPMD)
VIR_A_ALU1(NOT)
VIR_A_ALU1(NEG)
VIR_A_ALU1(FLAPUSH)
VIR_A_ALU1(FLBPUSH)
VIR_A_ALU1(FLPOP)
VIR_A_ALU0(FLAFIRST)
VIR_A_ALU0(FLNAFIRST)
VIR_A_ALU1(SETMSF)
VIR_A_ALU1(SETREVF)
VIR_A_ALU0(TIDX)
VIR_A_ALU0(EIDX)
VIR_A_ALU1(LDVPMV_IN)
VIR_A_ALU1(LDVPMV_OUT)
VIR_A_ALU1(LDVPMD_IN)
VIR_A_ALU1(LDVPMD_OUT)
VIR_A_ALU2(LDVPMG_IN)
VIR_A_ALU2(LDVPMG_OUT)
VIR_A_ALU0(TMUWT)

VIR_A_ALU0(IID)
VIR_A_ALU0(FXCD)
VIR_A_ALU0(XCD)
VIR_A_ALU0(FYCD)
VIR_A_ALU0(YCD)
VIR_A_ALU0(MSF)
VIR_A_ALU0(REVF)
VIR_A_ALU0(BARRIERID)
VIR_A_ALU0(SAMPID)
VIR_A_NODST_1(VPMSETUP)
VIR_A_NODST_0(VPMWT)
VIR_A_ALU2(FCMP)
VIR_A_ALU2(VFMAX)

VIR_A_ALU1(FROUND)
VIR_A_ALU1(FTOIN)
VIR_A_ALU1(FTRUNC)
VIR_A_ALU1(FTOIZ)
VIR_A_ALU1(FFLOOR)
VIR_A_ALU1(FTOUZ)
VIR_A_ALU1(FCEIL)
VIR_A_ALU1(FTOC)

VIR_A_ALU1(FDX)
VIR_A_ALU1(FDY)

VIR_A_ALU1(ITOF)
VIR_A_ALU1(CLZ)
VIR_A_ALU1(UTOF)

VIR_M_ALU2(UMUL24)
VIR_M_ALU2(FMUL)
VIR_M_ALU2(SMUL24)
VIR_M_NODST_2(MULTOP)

VIR_M_ALU1(MOV)
VIR_M_ALU1(FMOV)

VIR_SFU(RECIP)
VIR_SFU(RSQRT)
VIR_SFU(EXP)
VIR_SFU(LOG)
VIR_SFU(SIN)
VIR_SFU(RSQRT2)

VIR_A_ALU2(VPACK)
VIR_A_ALU2(V8PACK)
VIR_A_ALU2(V10PACK)
VIR_A_ALU2(V11FPACK)

VIR_M_ALU1(FTOUNORM16)
VIR_M_ALU1(FTOSNORM16)

VIR_M_ALU1(VFTOUNORM8)
VIR_M_ALU1(VFTOSNORM8)

VIR_M_ALU1(VFTOUNORM10LO)
VIR_M_ALU1(VFTOUNORM10HI)

static inline struct qinst *
vir_MOV_cond(struct v3d_compile *c, enum v3d_qpu_cond cond,
             struct qreg dest, struct qreg src)
{
        struct qinst *mov = vir_MOV_dest(c, dest, src);
        vir_set_cond(mov, cond);
        return mov;
}

static inline struct qreg
vir_SEL(struct v3d_compile *c, enum v3d_qpu_cond cond,
        struct qreg src0, struct qreg src1)
{
        struct qreg t = vir_get_temp(c);
        vir_MOV_dest(c, t, src1);
        vir_MOV_cond(c, cond, t, src0);
        return t;
}

static inline struct qinst *
vir_NOP(struct v3d_compile *c)
{
        return vir_emit_nondef(c, vir_add_inst(V3D_QPU_A_NOP,
                                               c->undef, c->undef, c->undef));
}

static inline struct qreg
vir_LDTMU(struct v3d_compile *c)
{
        struct qinst *ldtmu = vir_add_inst(V3D_QPU_A_NOP, c->undef,
                                           c->undef, c->undef);
        ldtmu->qpu.sig.ldtmu = true;

        return vir_emit_def(c, ldtmu);
}

static inline struct qreg
vir_UMUL(struct v3d_compile *c, struct qreg src0, struct qreg src1)
{
        vir_MULTOP(c, src0, src1);
        return vir_UMUL24(c, src0, src1);
}

static inline struct qreg
vir_TLBU_COLOR_READ(struct v3d_compile *c, uint32_t config)
{
        assert((config & 0xffffff00) == 0xffffff00);

        struct qinst *ldtlb = vir_add_inst(V3D_QPU_A_NOP, c->undef,
                                           c->undef, c->undef);
        ldtlb->qpu.sig.ldtlbu = true;
        ldtlb->uniform = vir_get_uniform_index(c, QUNIFORM_CONSTANT, config);
        return vir_emit_def(c, ldtlb);
}

static inline struct qreg
vir_TLB_COLOR_READ(struct v3d_compile *c)
{
        struct qinst *ldtlb = vir_add_inst(V3D_QPU_A_NOP, c->undef,
                                           c->undef, c->undef);
        ldtlb->qpu.sig.ldtlb = true;
        return vir_emit_def(c, ldtlb);
}

static inline struct qinst *
vir_BRANCH(struct v3d_compile *c, enum v3d_qpu_branch_cond cond)
{
        /* The actual uniform_data value will be set at scheduling time */
        return vir_emit_nondef(c, vir_branch_inst(c, cond));
}

#define vir_for_each_block(block, c)                                    \
        list_for_each_entry(struct qblock, block, &c->blocks, link)

#define vir_for_each_block_rev(block, c)                                \
        list_for_each_entry_rev(struct qblock, block, &c->blocks, link)

/* Loop over the non-NULL members of the successors array. */
#define vir_for_each_successor(succ, block)                             \
        for (struct qblock *succ = block->successors[0];                \
             succ != NULL;                                              \
             succ = (succ == block->successors[1] ? NULL :              \
                     block->successors[1]))

#define vir_for_each_inst(inst, block)                                  \
        list_for_each_entry(struct qinst, inst, &block->instructions, link)

#define vir_for_each_inst_rev(inst, block)                                  \
        list_for_each_entry_rev(struct qinst, inst, &block->instructions, link)

#define vir_for_each_inst_safe(inst, block)                             \
        list_for_each_entry_safe(struct qinst, inst, &block->instructions, link)

#define vir_for_each_inst_inorder(inst, c)                              \
        vir_for_each_block(_block, c)                                   \
                vir_for_each_inst(inst, _block)

#define vir_for_each_inst_inorder_safe(inst, c)                         \
        vir_for_each_block(_block, c)                                   \
                vir_for_each_inst_safe(inst, _block)

#endif /* V3D_COMPILER_H */
