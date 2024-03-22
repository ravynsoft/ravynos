/*
 * Copyright © 2014-2017 Broadcom
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

#include <inttypes.h>
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/ralloc.h"
#include "util/hash_table.h"
#include "util/u_upload_mgr.h"
#include "tgsi/tgsi_dump.h"
#include "compiler/nir/nir.h"
#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_serialize.h"
#include "nir/tgsi_to_nir.h"
#include "compiler/v3d_compiler.h"
#include "v3d_context.h"
/* packets here are the same across V3D versions. */
#include "broadcom/cle/v3d_packet_v42_pack.h"

static struct v3d_compiled_shader *
v3d_get_compiled_shader(struct v3d_context *v3d,
                        struct v3d_key *key, size_t key_size,
                        struct v3d_uncompiled_shader *uncompiled);

static void
v3d_setup_shared_precompile_key(struct v3d_uncompiled_shader *uncompiled,
                                struct v3d_key *key);

static gl_varying_slot
v3d_get_slot_for_driver_location(nir_shader *s, uint32_t driver_location)
{
        nir_foreach_shader_out_variable(var, s) {
                if (var->data.driver_location == driver_location) {
                        return var->data.location;
                }

                /* For compact arrays, we have more than one location to
                 * check.
                 */
                if (var->data.compact) {
                        assert(glsl_type_is_array(var->type));
                        for (int i = 0; i < DIV_ROUND_UP(glsl_array_size(var->type), 4); i++) {
                                if ((var->data.driver_location + i) == driver_location) {
                                        return var->data.location;
                                }
                        }
                }
        }

        return -1;
}

/**
 * Precomputes the TRANSFORM_FEEDBACK_OUTPUT_DATA_SPEC array for the shader.
 *
 * A shader can have 16 of these specs, and each one of them can write up to
 * 16 dwords.  Since we allow a total of 64 transform feedback output
 * components (not 16 vectors), we have to group the writes of multiple
 * varyings together in a single data spec.
 */
static void
v3d_set_transform_feedback_outputs(struct v3d_uncompiled_shader *so,
                                   const struct pipe_stream_output_info *stream_output)
{
        if (!stream_output->num_outputs)
                return;

        struct v3d_varying_slot slots[PIPE_MAX_SO_OUTPUTS * 4];
        int slot_count = 0;

        for (int buffer = 0; buffer < PIPE_MAX_SO_BUFFERS; buffer++) {
                uint32_t buffer_offset = 0;
                uint32_t vpm_start = slot_count;

                for (int i = 0; i < stream_output->num_outputs; i++) {
                        const struct pipe_stream_output *output =
                                &stream_output->output[i];

                        if (output->output_buffer != buffer)
                                continue;

                        /* We assume that the SO outputs appear in increasing
                         * order in the buffer.
                         */
                        assert(output->dst_offset >= buffer_offset);

                        /* Pad any undefined slots in the output */
                        for (int j = buffer_offset; j < output->dst_offset; j++) {
                                slots[slot_count] =
                                        v3d_slot_from_slot_and_component(VARYING_SLOT_POS, 0);
                                slot_count++;
                                buffer_offset++;
                        }

                        /* Set the coordinate shader up to output the
                         * components of this varying.
                         */
                        for (int j = 0; j < output->num_components; j++) {
                                gl_varying_slot slot =
                                        v3d_get_slot_for_driver_location(so->base.ir.nir, output->register_index);

                                slots[slot_count] =
                                        v3d_slot_from_slot_and_component(slot,
                                                                         output->start_component + j);
                                slot_count++;
                                buffer_offset++;
                        }
                }

                uint32_t vpm_size = slot_count - vpm_start;
                if (!vpm_size)
                        continue;

                uint32_t vpm_start_offset = vpm_start + 6;

                while (vpm_size) {
                        uint32_t write_size = MIN2(vpm_size, 1 << 4);

                        struct V3D42_TRANSFORM_FEEDBACK_OUTPUT_DATA_SPEC unpacked = {
                                /* We need the offset from the coordinate shader's VPM
                                 * output block, which has the [X, Y, Z, W, Xs, Ys]
                                 * values at the start.
                                 */
                                .first_shaded_vertex_value_to_output = vpm_start_offset,
                                .number_of_consecutive_vertex_values_to_output_as_32_bit_values = write_size,
                                .output_buffer_to_write_to = buffer,
                        };

                        /* GFXH-1559 */
                        assert(unpacked.first_shaded_vertex_value_to_output != 8 ||
                               so->num_tf_specs != 0);

                        assert(so->num_tf_specs != ARRAY_SIZE(so->tf_specs));
                        V3D42_TRANSFORM_FEEDBACK_OUTPUT_DATA_SPEC_pack(NULL,
                                                                       (void *)&so->tf_specs[so->num_tf_specs],
                                                                       &unpacked);

                        /* If point size is being written by the shader, then
                         * all the VPM start offsets are shifted up by one.
                         * We won't know that until the variant is compiled,
                         * though.
                         */
                        unpacked.first_shaded_vertex_value_to_output++;

                        /* GFXH-1559 */
                        assert(unpacked.first_shaded_vertex_value_to_output != 8 ||
                               so->num_tf_specs != 0);

                        V3D42_TRANSFORM_FEEDBACK_OUTPUT_DATA_SPEC_pack(NULL,
                                                                       (void *)&so->tf_specs_psiz[so->num_tf_specs],
                                                                       &unpacked);
                        so->num_tf_specs++;
                        vpm_start_offset += write_size;
                        vpm_size -= write_size;
                }
                so->base.stream_output.stride[buffer] =
                        stream_output->stride[buffer];
        }

        so->num_tf_outputs = slot_count;
        so->tf_outputs = ralloc_array(so->base.ir.nir, struct v3d_varying_slot,
                                      slot_count);
        memcpy(so->tf_outputs, slots, sizeof(*slots) * slot_count);
}

static int
type_size(const struct glsl_type *type, bool bindless)
{
        return glsl_count_attribute_slots(type, false);
}

static void
precompile_all_outputs(nir_shader *s,
                       struct v3d_varying_slot *outputs,
                       uint8_t *num_outputs)
{
        nir_foreach_shader_out_variable(var, s) {
                const int array_len = MAX2(glsl_get_length(var->type), 1);
                for (int j = 0; j < array_len; j++) {
                        const int slot = var->data.location + j;
                        const int num_components =
                                glsl_get_components(var->type);
                        for (int i = 0; i < num_components; i++) {
                                const int swiz = var->data.location_frac + i;
                                outputs[(*num_outputs)++] =
                                        v3d_slot_from_slot_and_component(slot,
                                                                         swiz);
                        }
                }
        }
}

/**
 * Precompiles a shader variant at shader state creation time if
 * V3D_DEBUG=precompile is set.  Used for shader-db
 * (https://gitlab.freedesktop.org/mesa/shader-db)
 */
static void
v3d_shader_precompile(struct v3d_context *v3d,
                      struct v3d_uncompiled_shader *so)
{
        nir_shader *s = so->base.ir.nir;

        if (s->info.stage == MESA_SHADER_FRAGMENT) {
                struct v3d_fs_key key = {
                };

                nir_foreach_shader_out_variable(var, s) {
                        if (var->data.location == FRAG_RESULT_COLOR) {
                                key.cbufs |= 1 << 0;
                        } else if (var->data.location >= FRAG_RESULT_DATA0) {
                                key.cbufs |= 1 << (var->data.location -
                                                   FRAG_RESULT_DATA0);
                        }
                }

                key.logicop_func = PIPE_LOGICOP_COPY;

                v3d_setup_shared_precompile_key(so, &key.base);
                v3d_get_compiled_shader(v3d, &key.base, sizeof(key), so);
        } else if (s->info.stage == MESA_SHADER_GEOMETRY) {
                struct v3d_gs_key key = {
                        .base.is_last_geometry_stage = true,
                };

                v3d_setup_shared_precompile_key(so, &key.base);

                precompile_all_outputs(s,
                                       key.used_outputs,
                                       &key.num_used_outputs);

                v3d_get_compiled_shader(v3d, &key.base, sizeof(key), so);

                /* Compile GS bin shader: only position (XXX: include TF) */
                key.is_coord = true;
                key.num_used_outputs = 0;
                for (int i = 0; i < 4; i++) {
                        key.used_outputs[key.num_used_outputs++] =
                                v3d_slot_from_slot_and_component(VARYING_SLOT_POS,
                                                                 i);
                }
                v3d_get_compiled_shader(v3d, &key.base, sizeof(key), so);
        } else {
                assert(s->info.stage == MESA_SHADER_VERTEX);
                struct v3d_vs_key key = {
                        /* Emit fixed function outputs */
                        .base.is_last_geometry_stage = true,
                };

                v3d_setup_shared_precompile_key(so, &key.base);

                precompile_all_outputs(s,
                                       key.used_outputs,
                                       &key.num_used_outputs);

                v3d_get_compiled_shader(v3d, &key.base, sizeof(key), so);

                /* Compile VS bin shader: only position (XXX: include TF) */
                key.is_coord = true;
                key.num_used_outputs = 0;
                for (int i = 0; i < 4; i++) {
                        key.used_outputs[key.num_used_outputs++] =
                                v3d_slot_from_slot_and_component(VARYING_SLOT_POS,
                                                                 i);
                }
                v3d_get_compiled_shader(v3d, &key.base, sizeof(key), so);
        }
}

static bool
lower_uniform_offset_to_bytes_cb(nir_builder *b, nir_intrinsic_instr *intr,
                                 void *_state)
{
        if (intr->intrinsic != nir_intrinsic_load_uniform)
                return false;

        b->cursor = nir_before_instr(&intr->instr);
        nir_intrinsic_set_base(intr, nir_intrinsic_base(intr) * 16);
        nir_src_rewrite(&intr->src[0], nir_ishl_imm(b, intr->src[0].ssa, 4));
        return true;
}

static bool
lower_textures_cb(nir_builder *b, nir_instr *instr, void *_state)
{
        if (instr->type != nir_instr_type_tex)
                return false;

        nir_tex_instr *tex = nir_instr_as_tex(instr);
        if (nir_tex_instr_need_sampler(tex))
                return false;

        /* Use the texture index as sampler index for the purposes of
         * lower_tex_packing, since in GL we currently make packing
         * decisions based on texture format.
         */
        tex->backend_flags = tex->texture_index;
        return true;
}

static bool
v3d_nir_lower_uniform_offset_to_bytes(nir_shader *s)
{
        return nir_shader_intrinsics_pass(s, lower_uniform_offset_to_bytes_cb,
                                            nir_metadata_block_index |
                                            nir_metadata_dominance, NULL);
}

static bool
v3d_nir_lower_textures(nir_shader *s)
{
        return nir_shader_instructions_pass(s, lower_textures_cb,
                                            nir_metadata_block_index |
                                            nir_metadata_dominance, NULL);
}

static void *
v3d_uncompiled_shader_create(struct pipe_context *pctx,
                             enum pipe_shader_ir type, void *ir)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_uncompiled_shader *so = CALLOC_STRUCT(v3d_uncompiled_shader);
        if (!so)
                return NULL;

        so->program_id = v3d->next_uncompiled_program_id++;

        nir_shader *s;

        if (type == PIPE_SHADER_IR_NIR) {
                /* The backend takes ownership of the NIR shader on state
                 * creation.
                 */
                s = ir;
        } else {
                assert(type == PIPE_SHADER_IR_TGSI);

                if (V3D_DBG(TGSI)) {
                        fprintf(stderr, "prog %d TGSI:\n",
                                so->program_id);
                        tgsi_dump(ir, 0);
                        fprintf(stderr, "\n");
                }
                s = tgsi_to_nir(ir, pctx->screen, false);
        }

        if (s->info.stage != MESA_SHADER_VERTEX &&
            s->info.stage != MESA_SHADER_GEOMETRY) {
                NIR_PASS(_, s, nir_lower_io,
                         nir_var_shader_in | nir_var_shader_out,
                         type_size, (nir_lower_io_options)0);
        }

        NIR_PASS(_, s, nir_normalize_cubemap_coords);

        NIR_PASS(_, s, nir_lower_load_const_to_scalar);

        v3d_optimize_nir(NULL, s);

        NIR_PASS(_, s, nir_lower_var_copies);

        /* Get rid of split copies */
        v3d_optimize_nir(NULL, s);

        NIR_PASS(_, s, nir_remove_dead_variables, nir_var_function_temp, NULL);

        NIR_PASS(_, s, nir_lower_frexp);

        /* Since we can't expose PIPE_CAP_PACKED_UNIFORMS the state tracker
         * will produce uniform intrinsics with offsets in vec4 units but
         * our compiler expects to work in units of bytes.
         */
        NIR_PASS(_, s, v3d_nir_lower_uniform_offset_to_bytes);

        NIR_PASS(_, s, v3d_nir_lower_textures);

        /* Garbage collect dead instructions */
        nir_sweep(s);

        so->base.type = PIPE_SHADER_IR_NIR;
        so->base.ir.nir = s;

        /* Generate sha1 from NIR for caching */
        struct blob blob;
        blob_init(&blob);
        nir_serialize(&blob, s, true);
        assert(!blob.out_of_memory);
        _mesa_sha1_compute(blob.data, blob.size, so->sha1);
        blob_finish(&blob);

        if (V3D_DBG(NIR) || v3d_debug_flag_for_shader_stage(s->info.stage)) {
                fprintf(stderr, "%s prog %d NIR:\n",
                        gl_shader_stage_name(s->info.stage),
                        so->program_id);
                nir_print_shader(s, stderr);
                fprintf(stderr, "\n");
        }

        if (V3D_DBG(PRECOMPILE))
                v3d_shader_precompile(v3d, so);

        return so;
}

static void
v3d_shader_debug_output(const char *message, void *data)
{
        struct pipe_context *ctx = data;

        util_debug_message(&ctx->debug, SHADER_INFO, "%s", message);
}

static void *
v3d_shader_state_create(struct pipe_context *pctx,
                        const struct pipe_shader_state *cso)
{
        struct v3d_uncompiled_shader *so =
                v3d_uncompiled_shader_create(pctx,
                                             cso->type,
                                             (cso->type == PIPE_SHADER_IR_TGSI ?
                                              (void *)cso->tokens :
                                              cso->ir.nir));

        v3d_set_transform_feedback_outputs(so, &cso->stream_output);

        return so;
}

/* Key ued with the RAM cache */
struct v3d_cache_key {
        struct v3d_key *key;
        unsigned char sha1[20];
};

struct v3d_compiled_shader *
v3d_get_compiled_shader(struct v3d_context *v3d,
                        struct v3d_key *key,
                        size_t key_size,
                        struct v3d_uncompiled_shader *uncompiled)
{
        nir_shader *s = uncompiled->base.ir.nir;
        struct hash_table *ht = v3d->prog.cache[s->info.stage];
        struct v3d_cache_key cache_key;
        cache_key.key = key;
        memcpy(cache_key.sha1, uncompiled->sha1, sizeof(cache_key.sha1));
        struct hash_entry *entry = _mesa_hash_table_search(ht, &cache_key);
        if (entry)
                return entry->data;

        int variant_id =
                p_atomic_inc_return(&uncompiled->compiled_variant_count);

        struct v3d_compiled_shader *shader = NULL;

#ifdef ENABLE_SHADER_CACHE
        shader = v3d_disk_cache_retrieve(v3d, key, uncompiled);
#endif
        if (!shader) {
                shader = rzalloc(NULL, struct v3d_compiled_shader);

                int program_id = uncompiled->program_id;
                uint64_t *qpu_insts;
                uint32_t shader_size;

                qpu_insts = v3d_compile(v3d->screen->compiler, key,
                                        &shader->prog_data.base, s,
                                        v3d_shader_debug_output,
                                        v3d,
                                        program_id, variant_id, &shader_size);

                /* qpu_insts being NULL can happen if the register allocation
                 * failed. At this point we can't really trigger an OpenGL API
                 * error, as the final compilation could happen on the draw
                 * call. So let's at least assert, so debug builds finish at
                 * this point.
                 */
                assert(qpu_insts);
                ralloc_steal(shader, shader->prog_data.base);

                if (shader_size) {
                        u_upload_data(v3d->state_uploader, 0, shader_size, 8,
                                      qpu_insts, &shader->offset, &shader->resource);
                }

#ifdef ENABLE_SHADER_CACHE
                v3d_disk_cache_store(v3d, key, uncompiled,
                                     shader, qpu_insts, shader_size);
#endif

                free(qpu_insts);
        }

        v3d_set_shader_uniform_dirty_flags(shader);

        if (ht) {
                struct v3d_cache_key *dup_cache_key =
                        ralloc_size(shader, sizeof(struct v3d_cache_key));
                dup_cache_key->key = ralloc_size(shader, key_size);
                memcpy(dup_cache_key->key, cache_key.key, key_size);
                memcpy(dup_cache_key->sha1, cache_key.sha1 ,sizeof(dup_cache_key->sha1));
                _mesa_hash_table_insert(ht, dup_cache_key, shader);
        }

        if (shader->prog_data.base->spill_size >
            v3d->prog.spill_size_per_thread) {
                /* The TIDX register we use for choosing the area to access
                 * for scratch space is: (core << 6) | (qpu << 2) | thread.
                 * Even at minimum threadcount in a particular shader, that
                 * means we still multiply by qpus by 4.
                 */
                int total_spill_size = (v3d->screen->devinfo.qpu_count * 4 *
                                        shader->prog_data.base->spill_size);

                v3d_bo_unreference(&v3d->prog.spill_bo);
                v3d->prog.spill_bo = v3d_bo_alloc(v3d->screen,
                                                  total_spill_size, "spill");
                v3d->prog.spill_size_per_thread =
                        shader->prog_data.base->spill_size;
        }

        return shader;
}

static void
v3d_free_compiled_shader(struct v3d_compiled_shader *shader)
{
        pipe_resource_reference(&shader->resource, NULL);
        ralloc_free(shader);
}

static void
v3d_setup_shared_key(struct v3d_context *v3d, struct v3d_key *key,
                     struct v3d_texture_stateobj *texstate)
{
        const struct v3d_device_info *devinfo = &v3d->screen->devinfo;

        key->num_tex_used = texstate->num_textures;
        key->num_samplers_used = texstate->num_textures;
        assert(key->num_tex_used == key->num_samplers_used);
        for (int i = 0; i < texstate->num_textures; i++) {
                struct pipe_sampler_view *sampler = texstate->textures[i];

                if (!sampler)
                        continue;

                key->sampler[i].return_size =
                        v3d_get_tex_return_size(devinfo, sampler->format);

                /* For 16-bit, we set up the sampler to always return 2
                 * channels (meaning no recompiles for most statechanges),
                 * while for 32 we actually scale the returns with channels.
                 */
                if (key->sampler[i].return_size == 16) {
                        key->sampler[i].return_channels = 2;
                } else {
                        key->sampler[i].return_channels = 4;
                }

                /* We let the sampler state handle the swizzle.
                 */
                key->tex[i].swizzle[0] = PIPE_SWIZZLE_X;
                key->tex[i].swizzle[1] = PIPE_SWIZZLE_Y;
                key->tex[i].swizzle[2] = PIPE_SWIZZLE_Z;
                key->tex[i].swizzle[3] = PIPE_SWIZZLE_W;
        }
}

static void
v3d_setup_shared_precompile_key(struct v3d_uncompiled_shader *uncompiled,
                                struct v3d_key *key)
{
        nir_shader *s = uncompiled->base.ir.nir;

        /* Note that below we access they key's texture and sampler fields
         * using the same index. On OpenGL they are the same (they are
         * combined)
         */
        key->num_tex_used = s->info.num_textures;
        key->num_samplers_used = s->info.num_textures;
        for (int i = 0; i < s->info.num_textures; i++) {
                key->sampler[i].return_size = 16;
                key->sampler[i].return_channels = 2;

                key->tex[i].swizzle[0] = PIPE_SWIZZLE_X;
                key->tex[i].swizzle[1] = PIPE_SWIZZLE_Y;
                key->tex[i].swizzle[2] = PIPE_SWIZZLE_Z;
                key->tex[i].swizzle[3] = PIPE_SWIZZLE_W;
        }
}

static void
v3d_update_compiled_fs(struct v3d_context *v3d, uint8_t prim_mode)
{
        struct v3d_job *job = v3d->job;
        struct v3d_fs_key local_key;
        struct v3d_fs_key *key = &local_key;
        nir_shader *s = v3d->prog.bind_fs->base.ir.nir;

        if (!(v3d->dirty & (V3D_DIRTY_PRIM_MODE |
                            V3D_DIRTY_BLEND |
                            V3D_DIRTY_FRAMEBUFFER |
                            V3D_DIRTY_ZSA |
                            V3D_DIRTY_RASTERIZER |
                            V3D_DIRTY_SAMPLE_STATE |
                            V3D_DIRTY_FRAGTEX |
                            V3D_DIRTY_UNCOMPILED_FS))) {
                return;
        }

        memset(key, 0, sizeof(*key));
        v3d_setup_shared_key(v3d, &key->base, &v3d->tex[PIPE_SHADER_FRAGMENT]);
        key->base.ucp_enables = v3d->rasterizer->base.clip_plane_enable;
        key->is_points = (prim_mode == MESA_PRIM_POINTS);
        key->is_lines = (prim_mode >= MESA_PRIM_LINES &&
                         prim_mode <= MESA_PRIM_LINE_STRIP);
        key->line_smoothing = (key->is_lines &&
                               v3d_line_smoothing_enabled(v3d));
        key->has_gs = v3d->prog.bind_gs != NULL;
        if (v3d->blend->base.logicop_enable) {
                key->logicop_func = v3d->blend->base.logicop_func;
        } else {
                key->logicop_func = PIPE_LOGICOP_COPY;
        }
        if (job->msaa) {
                key->msaa = v3d->rasterizer->base.multisample;
                key->sample_alpha_to_coverage = v3d->blend->base.alpha_to_coverage;
                key->sample_alpha_to_one = v3d->blend->base.alpha_to_one;
        }

        key->swap_color_rb = v3d->swap_color_rb;

        for (int i = 0; i < v3d->framebuffer.nr_cbufs; i++) {
                struct pipe_surface *cbuf = v3d->framebuffer.cbufs[i];
                if (!cbuf)
                        continue;

                /* gl_FragColor's propagation to however many bound color
                 * buffers there are means that the shader compile needs to
                 * know what buffers are present.
                 */
                key->cbufs |= 1 << i;

                /* If logic operations are enabled then we might emit color
                 * reads and we need to know the color buffer format and
                 * swizzle for that.
                 */
                if (key->logicop_func != PIPE_LOGICOP_COPY) {
                        key->color_fmt[i].format = cbuf->format;
                        memcpy(key->color_fmt[i].swizzle,
                               v3d_get_format_swizzle(&v3d->screen->devinfo,
                                                       cbuf->format),
                               sizeof(key->color_fmt[i].swizzle));
                }

                const struct util_format_description *desc =
                        util_format_description(cbuf->format);

                if (desc->channel[0].type == UTIL_FORMAT_TYPE_FLOAT &&
                    desc->channel[0].size == 32) {
                        key->f32_color_rb |= 1 << i;
                }

                if (s->info.fs.untyped_color_outputs) {
                        if (util_format_is_pure_uint(cbuf->format))
                                key->uint_color_rb |= 1 << i;
                        else if (util_format_is_pure_sint(cbuf->format))
                                key->int_color_rb |= 1 << i;
                }
        }

        if (key->is_points) {
                key->point_sprite_mask =
                        v3d->rasterizer->base.sprite_coord_enable;
                /* this is handled by lower_wpos_pntc */
                key->point_coord_upper_left = false;
        }

        struct v3d_compiled_shader *old_fs = v3d->prog.fs;
        v3d->prog.fs = v3d_get_compiled_shader(v3d, &key->base, sizeof(*key),
                                               v3d->prog.bind_fs);
        if (v3d->prog.fs == old_fs)
                return;

        v3d->dirty |= V3D_DIRTY_COMPILED_FS;

        if (old_fs) {
                if (v3d->prog.fs->prog_data.fs->flat_shade_flags !=
                    old_fs->prog_data.fs->flat_shade_flags) {
                        v3d->dirty |= V3D_DIRTY_FLAT_SHADE_FLAGS;
                }

                if (v3d->prog.fs->prog_data.fs->noperspective_flags !=
                    old_fs->prog_data.fs->noperspective_flags) {
                        v3d->dirty |= V3D_DIRTY_NOPERSPECTIVE_FLAGS;
                }

                if (v3d->prog.fs->prog_data.fs->centroid_flags !=
                    old_fs->prog_data.fs->centroid_flags) {
                        v3d->dirty |= V3D_DIRTY_CENTROID_FLAGS;
                }
        }

        if (old_fs && memcmp(v3d->prog.fs->prog_data.fs->input_slots,
                             old_fs->prog_data.fs->input_slots,
                             sizeof(v3d->prog.fs->prog_data.fs->input_slots))) {
                v3d->dirty |= V3D_DIRTY_FS_INPUTS;
        }
}

static void
v3d_update_compiled_gs(struct v3d_context *v3d, uint8_t prim_mode)
{
        struct v3d_gs_key local_key;
        struct v3d_gs_key *key = &local_key;

        if (!(v3d->dirty & (V3D_DIRTY_GEOMTEX |
                            V3D_DIRTY_RASTERIZER |
                            V3D_DIRTY_UNCOMPILED_GS |
                            V3D_DIRTY_PRIM_MODE |
                            V3D_DIRTY_FS_INPUTS))) {
                return;
        }

        if (!v3d->prog.bind_gs) {
                v3d->prog.gs = NULL;
                v3d->prog.gs_bin = NULL;
                return;
        }

        memset(key, 0, sizeof(*key));
        v3d_setup_shared_key(v3d, &key->base, &v3d->tex[PIPE_SHADER_GEOMETRY]);
        key->base.ucp_enables = v3d->rasterizer->base.clip_plane_enable;
        key->base.is_last_geometry_stage = true;
        key->num_used_outputs = v3d->prog.fs->prog_data.fs->num_inputs;
        STATIC_ASSERT(sizeof(key->used_outputs) ==
                      sizeof(v3d->prog.fs->prog_data.fs->input_slots));
        memcpy(key->used_outputs, v3d->prog.fs->prog_data.fs->input_slots,
               sizeof(key->used_outputs));

        key->per_vertex_point_size =
                (prim_mode == MESA_PRIM_POINTS &&
                 v3d->rasterizer->base.point_size_per_vertex);

        struct v3d_uncompiled_shader *uncompiled = v3d->prog.bind_gs;
        struct v3d_compiled_shader *gs =
                v3d_get_compiled_shader(v3d, &key->base, sizeof(*key),
                                        uncompiled);
        if (gs != v3d->prog.gs) {
                v3d->prog.gs = gs;
                v3d->dirty |= V3D_DIRTY_COMPILED_GS;
        }

        key->is_coord = true;

        /* The last bin-mode shader in the geometry pipeline only outputs
         * varyings used by transform feedback.
         */
        memcpy(key->used_outputs, uncompiled->tf_outputs,
               sizeof(*key->used_outputs) * uncompiled->num_tf_outputs);
        if (uncompiled->num_tf_outputs < key->num_used_outputs) {
                uint32_t size = sizeof(*key->used_outputs) *
                                (key->num_used_outputs -
                                 uncompiled->num_tf_outputs);
                memset(&key->used_outputs[uncompiled->num_tf_outputs],
                       0, size);
        }
        key->num_used_outputs = uncompiled->num_tf_outputs;

        struct v3d_compiled_shader *old_gs = v3d->prog.gs;
        struct v3d_compiled_shader *gs_bin =
                v3d_get_compiled_shader(v3d, &key->base, sizeof(*key),
                                        uncompiled);
        if (gs_bin != old_gs) {
                v3d->prog.gs_bin = gs_bin;
                v3d->dirty |= V3D_DIRTY_COMPILED_GS_BIN;
        }

        if (old_gs && memcmp(v3d->prog.gs->prog_data.gs->input_slots,
                             old_gs->prog_data.gs->input_slots,
                             sizeof(v3d->prog.gs->prog_data.gs->input_slots))) {
                v3d->dirty |= V3D_DIRTY_GS_INPUTS;
        }
}

static void
v3d_update_compiled_vs(struct v3d_context *v3d, uint8_t prim_mode)
{
        struct v3d_vs_key local_key;
        struct v3d_vs_key *key = &local_key;

        if (!(v3d->dirty & (V3D_DIRTY_VERTTEX |
                            V3D_DIRTY_VTXSTATE |
                            V3D_DIRTY_UNCOMPILED_VS |
                            (v3d->prog.bind_gs ? 0 : V3D_DIRTY_RASTERIZER) |
                            (v3d->prog.bind_gs ? 0 : V3D_DIRTY_PRIM_MODE) |
                            (v3d->prog.bind_gs ? V3D_DIRTY_GS_INPUTS :
                                                 V3D_DIRTY_FS_INPUTS)))) {
                return;
        }

        memset(key, 0, sizeof(*key));
        v3d_setup_shared_key(v3d, &key->base, &v3d->tex[PIPE_SHADER_VERTEX]);
        key->base.ucp_enables = v3d->rasterizer->base.clip_plane_enable;
        key->base.is_last_geometry_stage = !v3d->prog.bind_gs;

        if (!v3d->prog.bind_gs) {
            key->num_used_outputs = v3d->prog.fs->prog_data.fs->num_inputs;
            STATIC_ASSERT(sizeof(key->used_outputs) ==
                          sizeof(v3d->prog.fs->prog_data.fs->input_slots));
            memcpy(key->used_outputs, v3d->prog.fs->prog_data.fs->input_slots,
                   sizeof(key->used_outputs));
        } else {
            key->num_used_outputs = v3d->prog.gs->prog_data.gs->num_inputs;
            STATIC_ASSERT(sizeof(key->used_outputs) ==
                          sizeof(v3d->prog.gs->prog_data.gs->input_slots));
            memcpy(key->used_outputs, v3d->prog.gs->prog_data.gs->input_slots,
                   sizeof(key->used_outputs));
        }

        key->per_vertex_point_size =
                (prim_mode == MESA_PRIM_POINTS &&
                 v3d->rasterizer->base.point_size_per_vertex);

        nir_shader *s = v3d->prog.bind_vs->base.ir.nir;
        uint64_t inputs_read = s->info.inputs_read;
        assert(util_bitcount(inputs_read) <= v3d->vtx->num_elements);

        while (inputs_read) {
                int location = u_bit_scan64(&inputs_read);
                nir_variable *var =
                        nir_find_variable_with_location(s, nir_var_shader_in, location);
                assert (var != NULL);
                int driver_location = var->data.driver_location;
                switch (v3d->vtx->pipe[driver_location].src_format) {
                case PIPE_FORMAT_B8G8R8A8_UNORM:
                case PIPE_FORMAT_B10G10R10A2_UNORM:
                case PIPE_FORMAT_B10G10R10A2_SNORM:
                case PIPE_FORMAT_B10G10R10A2_USCALED:
                case PIPE_FORMAT_B10G10R10A2_SSCALED:
                        key->va_swap_rb_mask |= 1 << location;
                        break;
                default:
                        break;
                }
        }

        struct v3d_uncompiled_shader *shader_state = v3d->prog.bind_vs;
        struct v3d_compiled_shader *vs =
                v3d_get_compiled_shader(v3d, &key->base, sizeof(*key),
                                        shader_state);
        if (vs != v3d->prog.vs) {
                v3d->prog.vs = vs;
                v3d->dirty |= V3D_DIRTY_COMPILED_VS;
        }

        key->is_coord = true;

        /* Coord shaders only output varyings used by transform feedback,
         * unless they are linked to other shaders in the geometry side
         * of the pipeline, since in that case any of the output varyings
         * could be required in later geometry stages to compute
         * gl_Position or TF outputs.
         */
        if (!v3d->prog.bind_gs) {
                memcpy(key->used_outputs, shader_state->tf_outputs,
                       sizeof(*key->used_outputs) *
                       shader_state->num_tf_outputs);
                if (shader_state->num_tf_outputs < key->num_used_outputs) {
                        uint32_t tail_bytes =
                                sizeof(*key->used_outputs) *
                                (key->num_used_outputs -
                                 shader_state->num_tf_outputs);
                        memset(&key->used_outputs[shader_state->num_tf_outputs],
                               0, tail_bytes);
                }
                key->num_used_outputs = shader_state->num_tf_outputs;
        } else {
                key->num_used_outputs = v3d->prog.gs_bin->prog_data.gs->num_inputs;
                STATIC_ASSERT(sizeof(key->used_outputs) ==
                              sizeof(v3d->prog.gs_bin->prog_data.gs->input_slots));
                memcpy(key->used_outputs, v3d->prog.gs_bin->prog_data.gs->input_slots,
                       sizeof(key->used_outputs));
        }

        struct v3d_compiled_shader *cs =
                v3d_get_compiled_shader(v3d, &key->base, sizeof(*key),
                                        shader_state);
        if (cs != v3d->prog.cs) {
                v3d->prog.cs = cs;
                v3d->dirty |= V3D_DIRTY_COMPILED_CS;
        }
}

void
v3d_update_compiled_shaders(struct v3d_context *v3d, uint8_t prim_mode)
{
        v3d_update_compiled_fs(v3d, prim_mode);
        v3d_update_compiled_gs(v3d, prim_mode);
        v3d_update_compiled_vs(v3d, prim_mode);
}

void
v3d_update_compiled_cs(struct v3d_context *v3d)
{
        struct v3d_key local_key;
        struct v3d_key *key = &local_key;

        if (!(v3d->dirty & (V3D_DIRTY_UNCOMPILED_CS |
                            V3D_DIRTY_COMPTEX))) {
                return;
        }

        memset(key, 0, sizeof(*key));
        v3d_setup_shared_key(v3d, key, &v3d->tex[PIPE_SHADER_COMPUTE]);

        struct v3d_compiled_shader *cs =
                v3d_get_compiled_shader(v3d, key, sizeof(*key),
                                        v3d->prog.bind_compute);
        if (cs != v3d->prog.compute) {
                v3d->prog.compute = cs;
                v3d->dirty |= V3D_DIRTY_COMPILED_CS; /* XXX */
        }
}

static inline uint32_t
cache_hash(const void *_key, uint32_t key_size)
{
        const struct v3d_cache_key *key = (struct v3d_cache_key *) _key;

        struct mesa_sha1 ctx;
        unsigned char sha1[20];
        _mesa_sha1_init(&ctx);
        _mesa_sha1_update(&ctx, key->key, key_size);
        _mesa_sha1_update(&ctx, key->sha1, 20);
        _mesa_sha1_final(&ctx, sha1);
        return _mesa_hash_data(sha1, 20);
}

static inline bool
cache_compare(const void *_key1, const void *_key2, uint32_t key_size)
{
        const struct v3d_cache_key *key1 = (struct v3d_cache_key *) _key1;
        const struct v3d_cache_key *key2 = (struct v3d_cache_key *) _key2;

        if (memcmp(key1->key, key2->key, key_size) != 0)
            return false;

        return memcmp(key1->sha1, key2->sha1, 20) == 0;
}

static uint32_t
fs_cache_hash(const void *key)
{
        return cache_hash(key, sizeof(struct v3d_fs_key));
}

static uint32_t
gs_cache_hash(const void *key)
{
        return cache_hash(key, sizeof(struct v3d_gs_key));
}

static uint32_t
vs_cache_hash(const void *key)
{
        return cache_hash(key, sizeof(struct v3d_vs_key));
}

static uint32_t
cs_cache_hash(const void *key)
{
        return cache_hash(key, sizeof(struct v3d_key));
}

static bool
fs_cache_compare(const void *key1, const void *key2)
{
        return cache_compare(key1, key2, sizeof(struct v3d_fs_key));
}

static bool
gs_cache_compare(const void *key1, const void *key2)
{
        return cache_compare(key1, key2, sizeof(struct v3d_gs_key));
}

static bool
vs_cache_compare(const void *key1, const void *key2)
{
        return cache_compare(key1, key2, sizeof(struct v3d_vs_key));
}

static bool
cs_cache_compare(const void *key1, const void *key2)
{
        return cache_compare(key1, key2, sizeof(struct v3d_key));
}

static void
v3d_shader_state_delete(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        struct v3d_uncompiled_shader *so = hwcso;
        nir_shader *s = so->base.ir.nir;

        hash_table_foreach(v3d->prog.cache[s->info.stage], entry) {
                const struct v3d_cache_key *cache_key = entry->key;
                struct v3d_compiled_shader *shader = entry->data;

                if (memcmp(cache_key->sha1, so->sha1, 20) != 0)
                        continue;

                if (v3d->prog.fs == shader)
                        v3d->prog.fs = NULL;
                if (v3d->prog.vs == shader)
                        v3d->prog.vs = NULL;
                if (v3d->prog.cs == shader)
                        v3d->prog.cs = NULL;
                if (v3d->prog.compute == shader)
                        v3d->prog.compute = NULL;

                _mesa_hash_table_remove(v3d->prog.cache[s->info.stage], entry);
                v3d_free_compiled_shader(shader);
        }

        ralloc_free(so->base.ir.nir);
        free(so);
}

static void
v3d_fp_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->prog.bind_fs = hwcso;
        v3d->dirty |= V3D_DIRTY_UNCOMPILED_FS;
}

static void
v3d_gp_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->prog.bind_gs = hwcso;
        v3d->dirty |= V3D_DIRTY_UNCOMPILED_GS;
}

static void
v3d_vp_state_bind(struct pipe_context *pctx, void *hwcso)
{
        struct v3d_context *v3d = v3d_context(pctx);
        v3d->prog.bind_vs = hwcso;
        v3d->dirty |= V3D_DIRTY_UNCOMPILED_VS;
}

static void
v3d_compute_state_bind(struct pipe_context *pctx, void *state)
{
        struct v3d_context *v3d = v3d_context(pctx);

        v3d->prog.bind_compute = state;
        v3d->dirty |= V3D_DIRTY_UNCOMPILED_CS;
}

static void *
v3d_create_compute_state(struct pipe_context *pctx,
                         const struct pipe_compute_state *cso)
{
        return v3d_uncompiled_shader_create(pctx, cso->ir_type,
                                            (void *)cso->prog);
}

void
v3d_program_init(struct pipe_context *pctx)
{
        struct v3d_context *v3d = v3d_context(pctx);

        pctx->create_vs_state = v3d_shader_state_create;
        pctx->delete_vs_state = v3d_shader_state_delete;

        pctx->create_gs_state = v3d_shader_state_create;
        pctx->delete_gs_state = v3d_shader_state_delete;

        pctx->create_fs_state = v3d_shader_state_create;
        pctx->delete_fs_state = v3d_shader_state_delete;

        pctx->bind_fs_state = v3d_fp_state_bind;
        pctx->bind_gs_state = v3d_gp_state_bind;
        pctx->bind_vs_state = v3d_vp_state_bind;

        if (v3d->screen->has_csd) {
                pctx->create_compute_state = v3d_create_compute_state;
                pctx->delete_compute_state = v3d_shader_state_delete;
                pctx->bind_compute_state = v3d_compute_state_bind;
        }

        v3d->prog.cache[MESA_SHADER_VERTEX] =
                _mesa_hash_table_create(pctx, vs_cache_hash, vs_cache_compare);
        v3d->prog.cache[MESA_SHADER_GEOMETRY] =
                _mesa_hash_table_create(pctx, gs_cache_hash, gs_cache_compare);
        v3d->prog.cache[MESA_SHADER_FRAGMENT] =
                _mesa_hash_table_create(pctx, fs_cache_hash, fs_cache_compare);
        v3d->prog.cache[MESA_SHADER_COMPUTE] =
                _mesa_hash_table_create(pctx, cs_cache_hash, cs_cache_compare);
}

void
v3d_program_fini(struct pipe_context *pctx)
{
        struct v3d_context *v3d = v3d_context(pctx);

        for (int i = 0; i < MESA_SHADER_STAGES; i++) {
                struct hash_table *cache = v3d->prog.cache[i];
                if (!cache)
                        continue;

                hash_table_foreach(cache, entry) {
                        struct v3d_compiled_shader *shader = entry->data;
                        v3d_free_compiled_shader(shader);
                        _mesa_hash_table_remove(cache, entry);
                }
        }

        v3d_bo_unreference(&v3d->prog.spill_bo);
}
