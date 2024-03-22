/*
 * Copyright 2018 Collabora Ltd.
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SPIRV_BUILDER_H
#define SPIRV_BUILDER_H

#include "compiler/spirv/spirv.h"
#include "compiler/spirv/GLSL.std.450.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct hash_table;
struct set;

struct spirv_buffer {
   uint32_t *words;
   size_t num_words, room;
};

struct spirv_builder {
   void *mem_ctx;

   struct set *caps;

   struct spirv_buffer extensions;
   struct spirv_buffer imports;
   struct spirv_buffer memory_model;
   struct spirv_buffer entry_points;
   struct spirv_buffer exec_modes;
   struct spirv_buffer debug_names;
   struct spirv_buffer decorations;

   struct spirv_buffer types_const_defs;
   struct spirv_buffer local_vars;
   struct hash_table *types;
   struct hash_table *consts;

   struct spirv_buffer instructions;
   SpvId prev_id;
   unsigned local_vars_begin;
};

static inline SpvId
spirv_builder_new_id(struct spirv_builder *b)
{
   return ++b->prev_id;
}

void
spirv_builder_emit_cap(struct spirv_builder *b, SpvCapability cap);

void
spirv_builder_emit_extension(struct spirv_builder *b, const char *ext);

void
spirv_builder_emit_source(struct spirv_builder *b, SpvSourceLanguage lang,
                          uint32_t version);

void
spirv_builder_emit_mem_model(struct spirv_builder *b,
                             SpvAddressingModel addr_model,
                             SpvMemoryModel mem_model);

void
spirv_builder_emit_name(struct spirv_builder *b, SpvId target,
                        const char *name);

void
spirv_builder_emit_decoration(struct spirv_builder *b, SpvId target,
                              SpvDecoration decoration);

void
spirv_builder_emit_rounding_mode(struct spirv_builder *b, SpvId target,
                                 SpvFPRoundingMode rounding);

void
spirv_builder_emit_input_attachment_index(struct spirv_builder *b, SpvId target, uint32_t id);

void
spirv_builder_emit_specid(struct spirv_builder *b, SpvId target, uint32_t id);

void
spirv_builder_emit_location(struct spirv_builder *b, SpvId target,
                            uint32_t location);

void
spirv_builder_emit_component(struct spirv_builder *b, SpvId target,
                             uint32_t component);

void
spirv_builder_emit_builtin(struct spirv_builder *b, SpvId target,
                           SpvBuiltIn builtin);

void
spirv_builder_emit_index(struct spirv_builder *b, SpvId target, int index);

void
spirv_builder_emit_stream(struct spirv_builder *b, SpvId target, int stream);

void
spirv_builder_emit_descriptor_set(struct spirv_builder *b, SpvId target,
                                  uint32_t descriptor_set);

void
spirv_builder_emit_binding(struct spirv_builder *b, SpvId target,
                           uint32_t binding);

void
spirv_builder_emit_array_stride(struct spirv_builder *b, SpvId target,
                                uint32_t stride);

void
spirv_builder_emit_offset(struct spirv_builder *b, SpvId target,
                          uint32_t offset);

void
spirv_builder_emit_xfb_buffer(struct spirv_builder *b, SpvId target,
                              uint32_t buffer);

void
spirv_builder_emit_xfb_stride(struct spirv_builder *b, SpvId target,
                              uint32_t stride);

void
spirv_builder_emit_member_offset(struct spirv_builder *b, SpvId target,
                                 uint32_t member, uint32_t offset);

void
spirv_builder_emit_entry_point(struct spirv_builder *b,
                               SpvExecutionModel exec_model, SpvId entry_point,
                               const char *name, const SpvId interfaces[],
                               size_t num_interfaces);
uint32_t
spirv_builder_emit_exec_mode_literal(struct spirv_builder *b, SpvId entry_point,
                                     SpvExecutionMode exec_mode, uint32_t param);
void
spirv_builder_emit_exec_mode_literal3(struct spirv_builder *b, SpvId entry_point,
                                     SpvExecutionMode exec_mode, uint32_t param[3]);
void
spirv_builder_emit_exec_mode_id3(struct spirv_builder *b, SpvId entry_point,
                                 SpvExecutionMode exec_mode, SpvId param[3]);
void
spirv_builder_emit_exec_mode(struct spirv_builder *b, SpvId entry_point,
                             SpvExecutionMode exec_mode);

void
spirv_builder_function(struct spirv_builder *b, SpvId result,
                       SpvId return_type,
                       SpvFunctionControlMask function_control,
                       SpvId function_type);

void
spirv_builder_function_end(struct spirv_builder *b);

void
spirv_builder_label(struct spirv_builder *b, SpvId label);

void
spirv_builder_return(struct spirv_builder *b);

SpvId
spirv_builder_emit_undef(struct spirv_builder *b, SpvId result_type);

SpvId
spirv_builder_emit_load(struct spirv_builder *b, SpvId result_type,
                        SpvId pointer);

SpvId
spirv_builder_emit_load_aligned(struct spirv_builder *b, SpvId result_type, SpvId pointer, unsigned alignment, bool coherent);
void
spirv_builder_emit_atomic_store(struct spirv_builder *b, SpvId pointer, SpvScope scope,
                                SpvMemorySemanticsMask semantics, SpvId object);

void
spirv_builder_emit_store(struct spirv_builder *b, SpvId pointer, SpvId object);
void
spirv_builder_emit_store_aligned(struct spirv_builder *b, SpvId pointer, SpvId object, unsigned alignment, bool coherent);

SpvId
spirv_builder_emit_access_chain(struct spirv_builder *b, SpvId result_type,
                                SpvId base, const SpvId indexes[],
                                size_t num_indexes);

void
spirv_builder_emit_interlock(struct spirv_builder *b, bool end);

SpvId
spirv_builder_emit_unop_const(struct spirv_builder *b, SpvOp op, SpvId result_type, uint64_t operand);

SpvId
spirv_builder_emit_unop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                        SpvId operand);

SpvId
spirv_builder_emit_binop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1);

SpvId
spirv_builder_emit_triop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2);

SpvId
spirv_builder_emit_quadop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2, SpvId operand3);

SpvId
spirv_builder_emit_hexop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2, SpvId operand3,
                         SpvId operand4, SpvId operand5);

SpvId
spirv_builder_emit_composite_extract(struct spirv_builder *b, SpvId result_type,
                                     SpvId composite, const uint32_t indexes[],
                                     size_t num_indexes);

SpvId
spirv_builder_emit_composite_construct(struct spirv_builder *b,
                                       SpvId result_type,
                                       const SpvId constituents[],
                                       size_t num_constituents);

SpvId
spirv_builder_emit_vector_shuffle(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1, SpvId vector_2,
                                  const uint32_t components[],
                                  size_t num_components);
SpvId
spirv_builder_emit_vector_extract(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1,
                                  uint32_t component);
SpvId
spirv_builder_emit_vector_insert(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1,
                                  SpvId component,
                                  uint32_t index);
void
spirv_builder_emit_branch(struct spirv_builder *b, SpvId label);

void
spirv_builder_emit_selection_merge(struct spirv_builder *b, SpvId merge_block,
                                   SpvSelectionControlMask selection_control);

void
spirv_builder_loop_merge(struct spirv_builder *b, SpvId merge_block,
                         SpvId cont_target, SpvLoopControlMask loop_control);

void
spirv_builder_emit_branch_conditional(struct spirv_builder *b, SpvId condition,
                                      SpvId true_label, SpvId false_label);

SpvId
spirv_builder_emit_phi(struct spirv_builder *b, SpvId result_type,
                       size_t num_vars, size_t *position);

void
spirv_builder_set_phi_operand(struct spirv_builder *b, size_t position,
                              size_t index, SpvId variable, SpvId parent);

void
spirv_builder_emit_kill(struct spirv_builder *b);

void
spirv_builder_emit_terminate(struct spirv_builder *b);

void
spirv_builder_emit_demote(struct spirv_builder *b);

SpvId
spirv_is_helper_invocation(struct spirv_builder *b);

SpvId
spirv_builder_emit_vote(struct spirv_builder *b, SpvOp op, SpvId src);

SpvId
spirv_builder_emit_image_sample(struct spirv_builder *b,
                                SpvId result_type,
                                SpvId sampled_image,
                                SpvId coordinate,
                                bool proj,
                                SpvId lod,
                                SpvId bias,
                                SpvId dref,
                                SpvId dx,
                                SpvId dy,
                                SpvId const_offset,
                                SpvId offset,
                                SpvId min_lod,
                                bool sparse);

SpvId
spirv_builder_emit_image(struct spirv_builder *b, SpvId result_type,
                         SpvId sampled_image);

SpvId
spirv_builder_emit_image_texel_pointer(struct spirv_builder *b,
                                       SpvId result_type,
                                       SpvId image,
                                       SpvId coordinate,
                                       SpvId sample);

SpvId
spirv_builder_emit_image_read(struct spirv_builder *b,
                              SpvId result_type,
                              SpvId image,
                              SpvId coordinate,
                              SpvId lod,
                              SpvId sample,
                              SpvId offset,
                              bool sparse);

void
spirv_builder_emit_image_write(struct spirv_builder *b,
                               SpvId image,
                               SpvId coordinate,
                               SpvId texel,
                               SpvId lod,
                               SpvId sample,
                               SpvId offset);

SpvId
spirv_builder_emit_image_fetch(struct spirv_builder *b,
                               SpvId result_type,
                               SpvId image,
                               SpvId coordinate,
                               SpvId lod,
                               SpvId sample,
                               SpvId const_offset,
                               SpvId offset,
                               bool sparse);
SpvId
spirv_builder_emit_image_gather(struct spirv_builder *b,
                               SpvId result_type,
                               SpvId image,
                               SpvId coordinate,
                               SpvId component,
                               SpvId lod,
                               SpvId sample,
                               SpvId const_offset,
                               SpvId offset,
                               SpvId dref,
                               bool sparse);

SpvId
spirv_builder_emit_image_query_size(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image,
                                    SpvId lod);

SpvId
spirv_builder_emit_image_query_levels(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image);

SpvId
spirv_builder_emit_image_query_lod(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image,
                                    SpvId coords);

SpvId
spirv_builder_emit_ext_inst(struct spirv_builder *b, SpvId result_type,
                            SpvId set, uint32_t instruction,
                            const SpvId args[], size_t num_args);

SpvId
spirv_builder_type_void(struct spirv_builder *b);

SpvId
spirv_builder_type_bool(struct spirv_builder *b);

SpvId
spirv_builder_type_int(struct spirv_builder *b, unsigned width);

SpvId
spirv_builder_type_uint(struct spirv_builder *b, unsigned width);

SpvId
spirv_builder_type_float(struct spirv_builder *b, unsigned width);

SpvId
spirv_builder_type_image(struct spirv_builder *b, SpvId sampled_type,
                         SpvDim dim, bool depth, bool arrayed, bool ms,
                         unsigned sampled, SpvImageFormat image_format);

SpvId
spirv_builder_type_sampled_image(struct spirv_builder *b, SpvId image_type);
SpvId
spirv_builder_type_sampler(struct spirv_builder *b);
SpvId
spirv_builder_emit_sampled_image(struct spirv_builder *b, SpvId result_type, SpvId image, SpvId sampler);

SpvId
spirv_builder_type_pointer(struct spirv_builder *b,
                           SpvStorageClass storage_class, SpvId type);

SpvId
spirv_builder_type_vector(struct spirv_builder *b, SpvId component_type,
                          unsigned component_count);

SpvId
spirv_builder_type_matrix(struct spirv_builder *b, SpvId component_type,
                          unsigned component_count);

SpvId
spirv_builder_type_runtime_array(struct spirv_builder *b, SpvId component_type);

SpvId
spirv_builder_type_array(struct spirv_builder *b, SpvId component_type,
                         SpvId length);

SpvId
spirv_builder_type_struct(struct spirv_builder *b, const SpvId member_types[],
                          size_t num_member_types);

SpvId
spirv_builder_type_function(struct spirv_builder *b, SpvId return_type,
                            const SpvId parameter_types[],
                            size_t num_parameter_types);

SpvId
spirv_builder_function_call(struct spirv_builder *b, SpvId result_type,
                            SpvId function, const SpvId arguments[],
                            size_t num_arguments);

SpvId
spirv_builder_const_bool(struct spirv_builder *b, bool val);

SpvId
spirv_builder_const_int(struct spirv_builder *b, int width, int64_t val);

SpvId
spirv_builder_const_uint(struct spirv_builder *b, int width, uint64_t val);

SpvId
spirv_builder_spec_const_uint(struct spirv_builder *b, int width);

SpvId
spirv_builder_const_float(struct spirv_builder *b, int width, double val);

SpvId
spirv_builder_const_composite(struct spirv_builder *b, SpvId result_type,
                              const SpvId constituents[],
                              size_t num_constituents);

SpvId
spirv_builder_spec_const_composite(struct spirv_builder *b, SpvId result_type,
                                   const SpvId constituents[],
                                   size_t num_constituents);

SpvId
spirv_builder_emit_var(struct spirv_builder *b, SpvId type,
                       SpvStorageClass storage_class);

void
spirv_builder_emit_memory_barrier(struct spirv_builder *b, SpvScope scope, SpvMemorySemanticsMask semantics);

void
spirv_builder_emit_control_barrier(struct spirv_builder *b, SpvScope scope, SpvScope mem_scope, SpvMemorySemanticsMask semantics);

SpvId
spirv_builder_import(struct spirv_builder *b, const char *name);

size_t
spirv_builder_get_num_words(struct spirv_builder *b);

size_t
spirv_builder_get_words(struct spirv_builder *b, uint32_t *words,
                        size_t num_words, uint32_t spirv_version,
                        uint32_t *tcs_vertices_out_word);

void
spirv_builder_emit_vertex(struct spirv_builder *b, uint32_t stream, bool multistream);
void
spirv_builder_end_primitive(struct spirv_builder *b, uint32_t stream, bool multistream);
void
spirv_builder_begin_local_vars(struct spirv_builder *b);
#endif
