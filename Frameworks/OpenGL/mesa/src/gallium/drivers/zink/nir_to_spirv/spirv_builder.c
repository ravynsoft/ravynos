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

#include "spirv_builder.h"

#include "util/macros.h"
#include "util/set.h"
#include "util/ralloc.h"
#include "util/u_bitcast.h"
#include "util/u_memory.h"
#include "util/half_float.h"
#include "util/hash_table.h"
#define XXH_INLINE_ALL
#include "util/xxhash.h"
#include "vk_util.h"

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

static bool
spirv_buffer_grow(struct spirv_buffer *b, void *mem_ctx, size_t needed)
{
   size_t new_room = MAX3(64, (b->room * 3) / 2, needed);

   uint32_t *new_words = reralloc_size(mem_ctx, b->words,
                                       new_room * sizeof(uint32_t));
   if (!new_words)
      return false;

   b->words = new_words;
   b->room = new_room;
   return true;
}

static inline bool
spirv_buffer_prepare(struct spirv_buffer *b, void *mem_ctx, size_t needed)
{
   needed += b->num_words;
   if (b->room >= b->num_words + needed)
      return true;

   return spirv_buffer_grow(b, mem_ctx, needed);
}

static inline uint32_t
spirv_buffer_emit_word(struct spirv_buffer *b, uint32_t word)
{
   assert(b->num_words < b->room);
   b->words[b->num_words] = word;
   return b->num_words++;
}

static int
spirv_buffer_emit_string(struct spirv_buffer *b, void *mem_ctx,
                         const char *str)
{
   int pos = 0;
   uint32_t word = 0;
   while (str[pos] != '\0') {
      word |= str[pos] << (8 * (pos % 4));
      if (++pos % 4 == 0) {
         spirv_buffer_prepare(b, mem_ctx, 1);
         spirv_buffer_emit_word(b, word);
         word = 0;
      }
   }

   spirv_buffer_prepare(b, mem_ctx, 1);
   spirv_buffer_emit_word(b, word);

   return 1 + pos / 4;
}

void
spirv_builder_emit_cap(struct spirv_builder *b, SpvCapability cap)
{
   if (!b->caps)
      b->caps = _mesa_set_create_u32_keys(b->mem_ctx);

   assert(b->caps);
   _mesa_set_add(b->caps, (void*)(uintptr_t)cap);
}

void
spirv_builder_emit_extension(struct spirv_builder *b, const char *name)
{
   size_t pos = b->extensions.num_words;
   spirv_buffer_prepare(&b->extensions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->extensions, SpvOpExtension);
   int len = spirv_buffer_emit_string(&b->extensions, b->mem_ctx, name);
   b->extensions.words[pos] |= (1 + len) << 16;
}

void
spirv_builder_emit_source(struct spirv_builder *b, SpvSourceLanguage lang,
                          uint32_t version)
{
   spirv_buffer_prepare(&b->debug_names, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->debug_names, SpvOpSource | (3 << 16));
   spirv_buffer_emit_word(&b->debug_names, lang);
   spirv_buffer_emit_word(&b->debug_names, version);
}

void
spirv_builder_emit_mem_model(struct spirv_builder *b,
                             SpvAddressingModel addr_model,
                             SpvMemoryModel mem_model)
{
   spirv_buffer_prepare(&b->memory_model, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->memory_model, SpvOpMemoryModel | (3 << 16));
   spirv_buffer_emit_word(&b->memory_model, addr_model);
   spirv_buffer_emit_word(&b->memory_model, mem_model);
}

void
spirv_builder_emit_entry_point(struct spirv_builder *b,
                               SpvExecutionModel exec_model, SpvId entry_point,
                               const char *name, const SpvId interfaces[],
                               size_t num_interfaces)
{
   size_t pos = b->entry_points.num_words;
   spirv_buffer_prepare(&b->entry_points, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->entry_points, SpvOpEntryPoint);
   spirv_buffer_emit_word(&b->entry_points, exec_model);
   spirv_buffer_emit_word(&b->entry_points, entry_point);
   int len = spirv_buffer_emit_string(&b->entry_points, b->mem_ctx, name);
   b->entry_points.words[pos] |= (3 + len + num_interfaces) << 16;
   spirv_buffer_prepare(&b->entry_points, b->mem_ctx, num_interfaces);
   for (int i = 0; i < num_interfaces; ++i)
      spirv_buffer_emit_word(&b->entry_points, interfaces[i]);
}

uint32_t
spirv_builder_emit_exec_mode_literal(struct spirv_builder *b, SpvId entry_point,
                                     SpvExecutionMode exec_mode, uint32_t param)
{
   spirv_buffer_prepare(&b->exec_modes, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->exec_modes, SpvOpExecutionMode | (4 << 16));
   spirv_buffer_emit_word(&b->exec_modes, entry_point);
   spirv_buffer_emit_word(&b->exec_modes, exec_mode);
   return spirv_buffer_emit_word(&b->exec_modes, param);
}

void
spirv_builder_emit_exec_mode_literal3(struct spirv_builder *b, SpvId entry_point,
                                     SpvExecutionMode exec_mode, uint32_t param[3])
{
   spirv_buffer_prepare(&b->exec_modes, b->mem_ctx, 6);
   spirv_buffer_emit_word(&b->exec_modes, SpvOpExecutionMode | (6 << 16));
   spirv_buffer_emit_word(&b->exec_modes, entry_point);
   spirv_buffer_emit_word(&b->exec_modes, exec_mode);
   for (unsigned i = 0; i < 3; i++)
      spirv_buffer_emit_word(&b->exec_modes, param[i]);
}

void
spirv_builder_emit_exec_mode_id3(struct spirv_builder *b, SpvId entry_point,
                                SpvExecutionMode exec_mode, SpvId param[3])
{
   spirv_buffer_prepare(&b->exec_modes, b->mem_ctx, 6);
   spirv_buffer_emit_word(&b->exec_modes, SpvOpExecutionModeId | (6 << 16));
   spirv_buffer_emit_word(&b->exec_modes, entry_point);
   spirv_buffer_emit_word(&b->exec_modes, exec_mode);
   for (unsigned i = 0; i < 3; i++)
      spirv_buffer_emit_word(&b->exec_modes, param[i]);
}

void
spirv_builder_emit_exec_mode(struct spirv_builder *b, SpvId entry_point,
                             SpvExecutionMode exec_mode)
{
   spirv_buffer_prepare(&b->exec_modes, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->exec_modes, SpvOpExecutionMode | (3 << 16));
   spirv_buffer_emit_word(&b->exec_modes, entry_point);
   spirv_buffer_emit_word(&b->exec_modes, exec_mode);
}

void
spirv_builder_emit_name(struct spirv_builder *b, SpvId target,
                        const char *name)
{
   size_t pos = b->debug_names.num_words;
   spirv_buffer_prepare(&b->debug_names, b->mem_ctx, 2);
   spirv_buffer_emit_word(&b->debug_names, SpvOpName);
   spirv_buffer_emit_word(&b->debug_names, target);
   int len = spirv_buffer_emit_string(&b->debug_names, b->mem_ctx, name);
   b->debug_names.words[pos] |= (2 + len) << 16;
}

static void
emit_decoration(struct spirv_builder *b, SpvId target,
                SpvDecoration decoration, const uint32_t extra_operands[],
                size_t num_extra_operands)
{
   int words = 3 + num_extra_operands;
   spirv_buffer_prepare(&b->decorations, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->decorations, SpvOpDecorate | (words << 16));
   spirv_buffer_emit_word(&b->decorations, target);
   spirv_buffer_emit_word(&b->decorations, decoration);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->decorations, extra_operands[i]);
}

void
spirv_builder_emit_decoration(struct spirv_builder *b, SpvId target,
                              SpvDecoration decoration)
{
   emit_decoration(b, target, decoration, NULL, 0);
}

void
spirv_builder_emit_rounding_mode(struct spirv_builder *b, SpvId target,
                                 SpvFPRoundingMode rounding)
{
   uint32_t args[] = { rounding };
   emit_decoration(b, target, SpvDecorationFPRoundingMode, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_input_attachment_index(struct spirv_builder *b, SpvId target, uint32_t id)
{
   uint32_t args[] = { id };
   emit_decoration(b, target, SpvDecorationInputAttachmentIndex, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_specid(struct spirv_builder *b, SpvId target, uint32_t id)
{
   uint32_t args[] = { id };
   emit_decoration(b, target, SpvDecorationSpecId, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_location(struct spirv_builder *b, SpvId target,
                            uint32_t location)
{
   uint32_t args[] = { location };
   emit_decoration(b, target, SpvDecorationLocation, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_component(struct spirv_builder *b, SpvId target,
                             uint32_t component)
{
   uint32_t args[] = { component };
   emit_decoration(b, target, SpvDecorationComponent, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_builtin(struct spirv_builder *b, SpvId target,
                           SpvBuiltIn builtin)
{
   uint32_t args[] = { builtin };
   emit_decoration(b, target, SpvDecorationBuiltIn, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_vertex(struct spirv_builder *b, uint32_t stream, bool multistream)
{
   unsigned words = 1;
   SpvOp op = SpvOpEmitVertex;
   if (multistream) {
      op = SpvOpEmitStreamVertex;
      words++;
   }
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, op | (words << 16));
   if (multistream)
      spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, stream));
}

void
spirv_builder_end_primitive(struct spirv_builder *b, uint32_t stream, bool multistream)
{
   unsigned words = 1;
   SpvOp op = SpvOpEndPrimitive;
   if (multistream || stream > 0) {
      op = SpvOpEndStreamPrimitive;
      words++;
   }
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, op | (words << 16));
   if (multistream || stream > 0)
      spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, stream));
}

void
spirv_builder_emit_descriptor_set(struct spirv_builder *b, SpvId target,
                                  uint32_t descriptor_set)
{
   uint32_t args[] = { descriptor_set };
   emit_decoration(b, target, SpvDecorationDescriptorSet, args,
                   ARRAY_SIZE(args));
}

void
spirv_builder_emit_binding(struct spirv_builder *b, SpvId target,
                           uint32_t binding)
{
   uint32_t args[] = { binding };
   emit_decoration(b, target, SpvDecorationBinding, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_array_stride(struct spirv_builder *b, SpvId target,
                                uint32_t stride)
{
   uint32_t args[] = { stride };
   emit_decoration(b, target, SpvDecorationArrayStride, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_offset(struct spirv_builder *b, SpvId target,
                          uint32_t offset)
{
   uint32_t args[] = { offset };
   emit_decoration(b, target, SpvDecorationOffset, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_xfb_buffer(struct spirv_builder *b, SpvId target,
                              uint32_t buffer)
{
   uint32_t args[] = { buffer };
   emit_decoration(b, target, SpvDecorationXfbBuffer, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_xfb_stride(struct spirv_builder *b, SpvId target,
                              uint32_t stride)
{
   uint32_t args[] = { stride };
   emit_decoration(b, target, SpvDecorationXfbStride, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_index(struct spirv_builder *b, SpvId target, int index)
{
   uint32_t args[] = { index };
   emit_decoration(b, target, SpvDecorationIndex, args, ARRAY_SIZE(args));
}

void
spirv_builder_emit_stream(struct spirv_builder *b, SpvId target, int stream)
{
   uint32_t args[] = { stream };
   emit_decoration(b, target, SpvDecorationStream, args, ARRAY_SIZE(args));
}

static void
emit_member_decoration(struct spirv_builder *b, SpvId target, uint32_t member,
                       SpvDecoration decoration, const uint32_t extra_operands[],
                       size_t num_extra_operands)
{
   int words = 4 + num_extra_operands;
   spirv_buffer_prepare(&b->decorations, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->decorations,
                          SpvOpMemberDecorate | (words << 16));
   spirv_buffer_emit_word(&b->decorations, target);
   spirv_buffer_emit_word(&b->decorations, member);
   spirv_buffer_emit_word(&b->decorations, decoration);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->decorations, extra_operands[i]);
}

void
spirv_builder_emit_member_offset(struct spirv_builder *b, SpvId target,
                          uint32_t member, uint32_t offset)
{
   uint32_t args[] = { offset };
   emit_member_decoration(b, target, member, SpvDecorationOffset,
                          args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_emit_undef(struct spirv_builder *b, SpvId result_type)
{
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->instructions, SpvOpUndef | (3 << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   return result;
}

void
spirv_builder_function(struct spirv_builder *b, SpvId result,
                       SpvId return_type,
                       SpvFunctionControlMask function_control,
                       SpvId function_type)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 5);
   spirv_buffer_emit_word(&b->instructions, SpvOpFunction | (5 << 16));
   spirv_buffer_emit_word(&b->instructions, return_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, function_control);
   spirv_buffer_emit_word(&b->instructions, function_type);
}

void
spirv_builder_function_end(struct spirv_builder *b)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, SpvOpFunctionEnd | (1 << 16));
}

SpvId
spirv_builder_function_call(struct spirv_builder *b, SpvId result_type,
                            SpvId function, const SpvId arguments[],
                            size_t num_arguments)
{
   SpvId result = spirv_builder_new_id(b);

   int words = 4 + num_arguments;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions,
                          SpvOpFunctionCall | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, function);

   for (int i = 0; i < num_arguments; ++i)
      spirv_buffer_emit_word(&b->instructions, arguments[i]);

   return result;
}


void
spirv_builder_label(struct spirv_builder *b, SpvId label)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 2);
   spirv_buffer_emit_word(&b->instructions, SpvOpLabel | (2 << 16));
   spirv_buffer_emit_word(&b->instructions, label);
}

void
spirv_builder_return(struct spirv_builder *b)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, SpvOpReturn | (1 << 16));
}

SpvId
spirv_builder_emit_load(struct spirv_builder *b, SpvId result_type,
                        SpvId pointer)
{
   return spirv_builder_emit_unop(b, SpvOpLoad, result_type, pointer);
}

SpvId
spirv_builder_emit_load_aligned(struct spirv_builder *b, SpvId result_type, SpvId pointer, unsigned alignment, bool coherent)
{
   if (coherent) {
      SpvId scope = spirv_builder_const_int(b, 32, SpvScopeDevice);
      return spirv_builder_emit_quadop(b, SpvOpLoad, result_type, pointer, SpvMemoryAccessAlignedMask | SpvMemoryAccessNonPrivatePointerMask | SpvMemoryAccessMakePointerVisibleMask, alignment, scope);
   } else {
      return spirv_builder_emit_triop(b, SpvOpLoad, result_type, pointer, SpvMemoryAccessAlignedMask, alignment);
   }
}

void
spirv_builder_emit_store(struct spirv_builder *b, SpvId pointer, SpvId object)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->instructions, SpvOpStore | (3 << 16));
   spirv_buffer_emit_word(&b->instructions, pointer);
   spirv_buffer_emit_word(&b->instructions, object);
}

void
spirv_builder_emit_store_aligned(struct spirv_builder *b, SpvId pointer, SpvId object, unsigned alignment, bool coherent)
{
   unsigned size = 5;
   SpvMemoryAccessMask mask = SpvMemoryAccessAlignedMask;

   if (coherent) {
      mask |= SpvMemoryAccessNonPrivatePointerMask | SpvMemoryAccessMakePointerAvailableMask;
      size++;
   }

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, size);
   spirv_buffer_emit_word(&b->instructions, SpvOpStore | (size << 16));
   spirv_buffer_emit_word(&b->instructions, pointer);
   spirv_buffer_emit_word(&b->instructions, object);
   spirv_buffer_emit_word(&b->instructions, mask);
   spirv_buffer_emit_word(&b->instructions, alignment);

   if (coherent) {
      SpvId scope = spirv_builder_const_int(b, 32, SpvScopeDevice);
      spirv_buffer_emit_word(&b->instructions, scope);
   }
}

void
spirv_builder_emit_atomic_store(struct spirv_builder *b, SpvId pointer, SpvScope scope,
                                SpvMemorySemanticsMask semantics, SpvId object)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 5);
   spirv_buffer_emit_word(&b->instructions, SpvOpAtomicStore | (5 << 16));
   spirv_buffer_emit_word(&b->instructions, pointer);
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, scope));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, semantics));
   spirv_buffer_emit_word(&b->instructions, object);
}

SpvId
spirv_builder_emit_access_chain(struct spirv_builder *b, SpvId result_type,
                                SpvId base, const SpvId indexes[],
                                size_t num_indexes)
{
   assert(base);
   assert(result_type);
   SpvId result = spirv_builder_new_id(b);

   int words = 4 + num_indexes;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpAccessChain | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, base);
   for (int i = 0; i < num_indexes; ++i) {
      assert(indexes[i]);
      spirv_buffer_emit_word(&b->instructions, indexes[i]);
   }
   return result;
}

void
spirv_builder_emit_interlock(struct spirv_builder *b, bool end)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, (end ? SpvOpEndInvocationInterlockEXT : SpvOpBeginInvocationInterlockEXT) | (1 << 16));
}


SpvId
spirv_builder_emit_unop_const(struct spirv_builder *b, SpvOp op, SpvId result_type, uint64_t operand)
{
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->instructions, op | (4 << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, operand));
   return result;
}

SpvId
spirv_builder_emit_unop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                        SpvId operand)
{
   struct spirv_buffer *buf = op == SpvOpSpecConstant ? &b->types_const_defs : &b->instructions;
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(buf, b->mem_ctx, 4);
   spirv_buffer_emit_word(buf, op | (4 << 16));
   spirv_buffer_emit_word(buf, result_type);
   spirv_buffer_emit_word(buf, result);
   spirv_buffer_emit_word(buf, operand);
   return result;
}

SpvId
spirv_builder_emit_binop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1)
{
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 5);
   spirv_buffer_emit_word(&b->instructions, op | (5 << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, operand0);
   spirv_buffer_emit_word(&b->instructions, operand1);
   return result;
}

SpvId
spirv_builder_emit_triop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2)
{
   struct spirv_buffer *buf = op == SpvOpSpecConstantOp ? &b->types_const_defs : &b->instructions;

   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(buf, b->mem_ctx, 6);
   spirv_buffer_emit_word(buf, op | (6 << 16));
   spirv_buffer_emit_word(buf, result_type);
   spirv_buffer_emit_word(buf, result);
   spirv_buffer_emit_word(buf, operand0);
   spirv_buffer_emit_word(buf, operand1);
   spirv_buffer_emit_word(buf, operand2);
   return result;
}

SpvId
spirv_builder_emit_quadop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2, SpvId operand3)
{
   struct spirv_buffer *buf = op == SpvOpSpecConstantOp ? &b->types_const_defs : &b->instructions;

   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(buf, b->mem_ctx, 7);
   spirv_buffer_emit_word(buf, op | (7 << 16));
   spirv_buffer_emit_word(buf, result_type);
   spirv_buffer_emit_word(buf, result);
   spirv_buffer_emit_word(buf, operand0);
   spirv_buffer_emit_word(buf, operand1);
   spirv_buffer_emit_word(buf, operand2);
   spirv_buffer_emit_word(buf, operand3);
   return result;
}

SpvId
spirv_builder_emit_hexop(struct spirv_builder *b, SpvOp op, SpvId result_type,
                         SpvId operand0, SpvId operand1, SpvId operand2, SpvId operand3,
                         SpvId operand4, SpvId operand5)
{
   struct spirv_buffer *buf = op == SpvOpSpecConstantOp ? &b->types_const_defs : &b->instructions;

   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(buf, b->mem_ctx, 9);
   spirv_buffer_emit_word(buf, op | (9 << 16));
   spirv_buffer_emit_word(buf, result_type);
   spirv_buffer_emit_word(buf, result);
   spirv_buffer_emit_word(buf, operand0);
   spirv_buffer_emit_word(buf, operand1);
   spirv_buffer_emit_word(buf, operand2);
   spirv_buffer_emit_word(buf, operand3);
   spirv_buffer_emit_word(buf, operand4);
   spirv_buffer_emit_word(buf, operand5);
   return result;
}

SpvId
spirv_builder_emit_composite_extract(struct spirv_builder *b, SpvId result_type,
                                     SpvId composite, const uint32_t indexes[],
                                     size_t num_indexes)
{
   SpvId result = spirv_builder_new_id(b);

   assert(num_indexes > 0);
   int words = 4 + num_indexes;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions,
                          SpvOpCompositeExtract | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, composite);
   for (int i = 0; i < num_indexes; ++i)
      spirv_buffer_emit_word(&b->instructions, indexes[i]);
   return result;
}

SpvId
spirv_builder_emit_composite_construct(struct spirv_builder *b,
                                       SpvId result_type,
                                       const SpvId constituents[],
                                       size_t num_constituents)
{
   SpvId result = spirv_builder_new_id(b);

   assert(num_constituents > 0);
   int words = 3 + num_constituents;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions,
                          SpvOpCompositeConstruct | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   for (int i = 0; i < num_constituents; ++i)
      spirv_buffer_emit_word(&b->instructions, constituents[i]);
   return result;
}

SpvId
spirv_builder_emit_vector_shuffle(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1, SpvId vector_2,
                                  const uint32_t components[],
                                  size_t num_components)
{
   SpvId result = spirv_builder_new_id(b);

   assert(num_components > 0);
   int words = 5 + num_components;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpVectorShuffle | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, vector_1);
   spirv_buffer_emit_word(&b->instructions, vector_2);
   for (int i = 0; i < num_components; ++i)
      spirv_buffer_emit_word(&b->instructions, components[i]);
   return result;
}

SpvId
spirv_builder_emit_vector_extract(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1,
                                  uint32_t component)
{
   SpvId result = spirv_builder_new_id(b);

   int words = 5;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpVectorExtractDynamic | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, vector_1);
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, component));
   return result;
}

SpvId
spirv_builder_emit_vector_insert(struct spirv_builder *b, SpvId result_type,
                                  SpvId vector_1,
                                  SpvId component,
                                  uint32_t index)
{
   SpvId result = spirv_builder_new_id(b);

   int words = 6;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpVectorInsertDynamic | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, vector_1);
   spirv_buffer_emit_word(&b->instructions, component);
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, index));
   return result;
}

void
spirv_builder_emit_branch(struct spirv_builder *b, SpvId label)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 2);
   spirv_buffer_emit_word(&b->instructions, SpvOpBranch | (2 << 16));
   spirv_buffer_emit_word(&b->instructions, label);
}

void
spirv_builder_emit_selection_merge(struct spirv_builder *b, SpvId merge_block,
                                   SpvSelectionControlMask selection_control)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->instructions, SpvOpSelectionMerge | (3 << 16));
   spirv_buffer_emit_word(&b->instructions, merge_block);
   spirv_buffer_emit_word(&b->instructions, selection_control);
}

void
spirv_builder_loop_merge(struct spirv_builder *b, SpvId merge_block,
                         SpvId cont_target, SpvLoopControlMask loop_control)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->instructions, SpvOpLoopMerge | (4 << 16));
   spirv_buffer_emit_word(&b->instructions, merge_block);
   spirv_buffer_emit_word(&b->instructions, cont_target);
   spirv_buffer_emit_word(&b->instructions, loop_control);
}

void
spirv_builder_emit_branch_conditional(struct spirv_builder *b, SpvId condition,
                                      SpvId true_label, SpvId false_label)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->instructions, SpvOpBranchConditional | (4 << 16));
   spirv_buffer_emit_word(&b->instructions, condition);
   spirv_buffer_emit_word(&b->instructions, true_label);
   spirv_buffer_emit_word(&b->instructions, false_label);
}

SpvId
spirv_builder_emit_phi(struct spirv_builder *b, SpvId result_type,
                       size_t num_vars, size_t *position)
{
   SpvId result = spirv_builder_new_id(b);

   assert(num_vars > 0);
   int words = 3 + 2 * num_vars;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpPhi | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   *position = b->instructions.num_words;
   for (int i = 0; i < 2 * num_vars; ++i)
      spirv_buffer_emit_word(&b->instructions, 0);
   return result;
}

void
spirv_builder_set_phi_operand(struct spirv_builder *b, size_t position,
                              size_t index, SpvId variable, SpvId parent)
{
   b->instructions.words[position + index * 2 + 0] = variable;
   b->instructions.words[position + index * 2 + 1] = parent;
}

void
spirv_builder_emit_kill(struct spirv_builder *b)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, SpvOpKill | (1 << 16));
}

void
spirv_builder_emit_terminate(struct spirv_builder *b)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, SpvOpTerminateInvocation | (1 << 16));
}

void
spirv_builder_emit_demote(struct spirv_builder *b)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 1);
   spirv_buffer_emit_word(&b->instructions, SpvOpDemoteToHelperInvocation | (1 << 16));
}

SpvId
spirv_is_helper_invocation(struct spirv_builder *b)
{
   SpvId result = spirv_builder_new_id(b);
   SpvId result_type = spirv_builder_type_bool(b);

   int words = 3;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpIsHelperInvocationEXT | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   return result;
}

SpvId
spirv_builder_emit_vote(struct spirv_builder *b, SpvOp op, SpvId src)
{
   return spirv_builder_emit_binop(b, op, spirv_builder_type_bool(b),
                                   spirv_builder_const_uint(b, 32, SpvScopeSubgroup), src);
}

static SpvId
sparse_wrap_result_type(struct spirv_builder *b, SpvId result_type)
{
   SpvId types[2];
   types[0] = spirv_builder_type_uint(b, 32);
   types[1] = result_type;
   return spirv_builder_type_struct(b, types, 2);
}

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
                                bool sparse)
{
   SpvId result = spirv_builder_new_id(b);

   int operands = 5;
   int opcode;
   if (sparse) {
      opcode = SpvOpImageSparseSampleImplicitLod;
      if (proj)
         opcode += SpvOpImageSparseSampleProjImplicitLod - SpvOpImageSparseSampleImplicitLod;
      if (lod || (dx && dy))
         opcode += SpvOpImageSparseSampleExplicitLod - SpvOpImageSparseSampleImplicitLod;
      if (dref) {
         opcode += SpvOpImageSparseSampleDrefImplicitLod - SpvOpImageSparseSampleImplicitLod;
         operands++;
      }
      result_type = sparse_wrap_result_type(b, result_type);
   } else {
      opcode = SpvOpImageSampleImplicitLod;
      if (proj)
         opcode += SpvOpImageSampleProjImplicitLod - SpvOpImageSampleImplicitLod;
      if (lod || (dx && dy))
         opcode += SpvOpImageSampleExplicitLod - SpvOpImageSampleImplicitLod;
      if (dref) {
         opcode += SpvOpImageSampleDrefImplicitLod - SpvOpImageSampleImplicitLod;
         operands++;
      }
   }

   SpvImageOperandsMask operand_mask = SpvImageOperandsMaskNone;
   SpvId extra_operands[6];
   int num_extra_operands = 1;
   if (bias) {
      extra_operands[num_extra_operands++] = bias;
      operand_mask |= SpvImageOperandsBiasMask;
   }
   if (lod) {
      extra_operands[num_extra_operands++] = lod;
      operand_mask |= SpvImageOperandsLodMask;
   } else if (dx && dy) {
      extra_operands[num_extra_operands++] = dx;
      extra_operands[num_extra_operands++] = dy;
      operand_mask |= SpvImageOperandsGradMask;
   }
   assert(!(const_offset && offset));
   if (const_offset) {
      extra_operands[num_extra_operands++] = const_offset;
      operand_mask |= SpvImageOperandsConstOffsetMask;
   } else if (offset) {
      extra_operands[num_extra_operands++] = offset;
      operand_mask |= SpvImageOperandsOffsetMask;
   }
   if (min_lod) {
      extra_operands[num_extra_operands++] = min_lod;
      operand_mask |= SpvImageOperandsMinLodMask;
   }

   /* finalize num_extra_operands / extra_operands */
   extra_operands[0] = operand_mask;

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, operands + num_extra_operands);
   spirv_buffer_emit_word(&b->instructions, opcode | ((operands + num_extra_operands) << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, sampled_image);
   spirv_buffer_emit_word(&b->instructions, coordinate);
   if (dref)
      spirv_buffer_emit_word(&b->instructions, dref);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->instructions, extra_operands[i]);
   return result;
}

SpvId
spirv_builder_emit_image(struct spirv_builder *b, SpvId result_type,
                         SpvId sampled_image)
{
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->instructions, SpvOpImage | (4 << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, sampled_image);
   return result;
}

SpvId
spirv_builder_emit_image_texel_pointer(struct spirv_builder *b,
                                       SpvId result_type,
                                       SpvId image,
                                       SpvId coordinate,
                                       SpvId sample)
{
   SpvId pointer_type = spirv_builder_type_pointer(b,
                                                   SpvStorageClassImage,
                                                   result_type);
   return spirv_builder_emit_triop(b, SpvOpImageTexelPointer, pointer_type, image, coordinate, sample);
}

SpvId
spirv_builder_emit_image_read(struct spirv_builder *b,
                              SpvId result_type,
                              SpvId image,
                              SpvId coordinate,
                              SpvId lod,
                              SpvId sample,
                              SpvId offset,
                              bool sparse)
{
   SpvId result = spirv_builder_new_id(b);

   SpvImageOperandsMask operand_mask = SpvImageOperandsMaskNone;
   SpvId extra_operands[5];
   int num_extra_operands = 1;
   if (sparse)
      result_type = sparse_wrap_result_type(b, result_type);
   if (lod) {
      extra_operands[num_extra_operands++] = lod;
      operand_mask |= SpvImageOperandsLodMask;
   }
   if (sample) {
      extra_operands[num_extra_operands++] = sample;
      operand_mask |= SpvImageOperandsSampleMask;
   }
   if (offset) {
      extra_operands[num_extra_operands++] = offset;
      operand_mask |= SpvImageOperandsOffsetMask;
   }
   /* finalize num_extra_operands / extra_operands */
   extra_operands[0] = operand_mask;

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 5 + num_extra_operands);
   spirv_buffer_emit_word(&b->instructions, (sparse ? SpvOpImageSparseRead : SpvOpImageRead) |
                          ((5 + num_extra_operands) << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, image);
   spirv_buffer_emit_word(&b->instructions, coordinate);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->instructions, extra_operands[i]);
   return result;
}

void
spirv_builder_emit_image_write(struct spirv_builder *b,
                               SpvId image,
                               SpvId coordinate,
                               SpvId texel,
                               SpvId lod,
                               SpvId sample,
                               SpvId offset)
{
   SpvImageOperandsMask operand_mask = SpvImageOperandsMaskNone;
   SpvId extra_operands[5];
   int num_extra_operands = 1;
   if (lod) {
      extra_operands[num_extra_operands++] = lod;
      operand_mask |= SpvImageOperandsLodMask;
   }
   if (sample) {
      extra_operands[num_extra_operands++] = sample;
      operand_mask |= SpvImageOperandsSampleMask;
   }
   if (offset) {
      extra_operands[num_extra_operands++] = offset;
      operand_mask |= SpvImageOperandsOffsetMask;
   }
   /* finalize num_extra_operands / extra_operands */
   extra_operands[0] = operand_mask;

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4 + num_extra_operands);
   spirv_buffer_emit_word(&b->instructions, SpvOpImageWrite |
                          ((4 + num_extra_operands) << 16));
   spirv_buffer_emit_word(&b->instructions, image);
   spirv_buffer_emit_word(&b->instructions, coordinate);
   spirv_buffer_emit_word(&b->instructions, texel);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->instructions, extra_operands[i]);
}

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
                               bool sparse)
{
   SpvId result = spirv_builder_new_id(b);
   SpvId op = sparse ? SpvOpImageSparseGather : SpvOpImageGather;

   SpvImageOperandsMask operand_mask = SpvImageOperandsMaskNone;
   SpvId extra_operands[4];
   int num_extra_operands = 1;
   if (lod) {
      extra_operands[num_extra_operands++] = lod;
      operand_mask |= SpvImageOperandsLodMask;
   }
   if (sample) {
      extra_operands[num_extra_operands++] = sample;
      operand_mask |= SpvImageOperandsSampleMask;
   }
   assert(!(const_offset && offset));
   if (const_offset) {
      extra_operands[num_extra_operands++] = const_offset;
      operand_mask |= SpvImageOperandsConstOffsetMask;
   } else if (offset) {
      extra_operands[num_extra_operands++] = offset;
      operand_mask |= SpvImageOperandsOffsetMask;
   }
   if (dref)
      op = sparse ? SpvOpImageSparseDrefGather : SpvOpImageDrefGather;
   if (sparse)
      result_type = sparse_wrap_result_type(b, result_type);
   /* finalize num_extra_operands / extra_operands */
   extra_operands[0] = operand_mask;

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 6 + num_extra_operands);
   spirv_buffer_emit_word(&b->instructions, op |
                          ((6 + num_extra_operands) << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, image);
   spirv_buffer_emit_word(&b->instructions, coordinate);
   if (dref)
      spirv_buffer_emit_word(&b->instructions, dref);
   else
      spirv_buffer_emit_word(&b->instructions, component);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->instructions, extra_operands[i]);
   return result;
}

SpvId
spirv_builder_emit_image_fetch(struct spirv_builder *b,
                               SpvId result_type,
                               SpvId image,
                               SpvId coordinate,
                               SpvId lod,
                               SpvId sample,
                               SpvId const_offset,
                               SpvId offset,
                               bool sparse)
{
   SpvId result = spirv_builder_new_id(b);

   SpvImageOperandsMask operand_mask = SpvImageOperandsMaskNone;
   SpvId extra_operands[4];
   int num_extra_operands = 1;
   if (lod) {
      extra_operands[num_extra_operands++] = lod;
      operand_mask |= SpvImageOperandsLodMask;
   }
   if (sample) {
      extra_operands[num_extra_operands++] = sample;
      operand_mask |= SpvImageOperandsSampleMask;
   }
   assert(!(const_offset && offset));
   if (const_offset) {
      extra_operands[num_extra_operands++] = const_offset;
      operand_mask |= SpvImageOperandsConstOffsetMask;
   } else if (offset) {
      extra_operands[num_extra_operands++] = offset;
      operand_mask |= SpvImageOperandsOffsetMask;
   }
   if (sparse)
      result_type = sparse_wrap_result_type(b, result_type);

   /* finalize num_extra_operands / extra_operands */
   extra_operands[0] = operand_mask;

   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 5 + num_extra_operands);
   spirv_buffer_emit_word(&b->instructions, (sparse ? SpvOpImageSparseFetch : SpvOpImageFetch) |
                          ((5 + num_extra_operands) << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, image);
   spirv_buffer_emit_word(&b->instructions, coordinate);
   for (int i = 0; i < num_extra_operands; ++i)
      spirv_buffer_emit_word(&b->instructions, extra_operands[i]);
   return result;
}

SpvId
spirv_builder_emit_image_query_size(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image,
                                    SpvId lod)
{
   int opcode = SpvOpImageQuerySize;
   int words = 4;
   if (lod) {
      words++;
      opcode = SpvOpImageQuerySizeLod;
   }

   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, opcode | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, image);

   if (lod)
      spirv_buffer_emit_word(&b->instructions, lod);

   return result;
}

SpvId
spirv_builder_emit_image_query_levels(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image)
{
   return spirv_builder_emit_unop(b, SpvOpImageQueryLevels, result_type, image);
}

SpvId
spirv_builder_emit_image_query_lod(struct spirv_builder *b,
                                    SpvId result_type,
                                    SpvId image,
                                    SpvId coords)
{
   int opcode = SpvOpImageQueryLod;
   int words = 5;

   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, opcode | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, image);
   spirv_buffer_emit_word(&b->instructions, coords);

   return result;
}

SpvId
spirv_builder_emit_ext_inst(struct spirv_builder *b, SpvId result_type,
                            SpvId set, uint32_t instruction,
                            const SpvId *args, size_t num_args)
{
   SpvId result = spirv_builder_new_id(b);

   int words = 5 + num_args;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions, SpvOpExtInst | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   spirv_buffer_emit_word(&b->instructions, set);
   spirv_buffer_emit_word(&b->instructions, instruction);
   for (int i = 0; i < num_args; ++i)
      spirv_buffer_emit_word(&b->instructions, args[i]);
   return result;
}

struct spirv_type {
   SpvOp op;
   uint32_t args[8];
   size_t num_args;

   SpvId type;
};

static uint32_t
non_aggregate_type_hash(const void *arg)
{
   const struct spirv_type *type = arg;

   uint32_t hash = 0;
   hash = XXH32(&type->op, sizeof(type->op), hash);
   hash = XXH32(type->args, sizeof(uint32_t) * type->num_args, hash);
   return hash;
}

static bool
non_aggregate_type_equals(const void *a, const void *b)
{
   const struct spirv_type *ta = a, *tb = b;

   if (ta->op != tb->op)
      return false;

   assert(ta->num_args == tb->num_args);
   return memcmp(ta->args, tb->args, sizeof(uint32_t) * ta->num_args) == 0;
}

static SpvId
get_type_def(struct spirv_builder *b, SpvOp op, const uint32_t args[],
             size_t num_args)
{
   /* According to the SPIR-V specification:
    *
    *   "Two different type <id>s form, by definition, two different types. It
    *    is valid to declare multiple aggregate type <id>s having the same
    *    opcode and operands. This is to allow multiple instances of aggregate
    *    types with the same structure to be decorated differently. (Different
    *    decorations are not required; two different aggregate type <id>s are
    *    allowed to have identical declarations and decorations, and will still
    *    be two different types.) Non-aggregate types are different: It is
    *    invalid to declare multiple type <id>s for the same scalar, vector, or
    *    matrix type. That is, non-aggregate type declarations must all have
    *    different opcodes or operands. (Note that non-aggregate types cannot
    *    be decorated in ways that affect their type.)"
    *
    *  ..so, we need to prevent the same non-aggregate type to be re-defined
    *  with a new <id>. We do this by putting the definitions in a hash-map, so
    *  we can easily look up and reuse them.
    */

   struct spirv_type key;
   assert(num_args <= ARRAY_SIZE(key.args));
   key.op = op;
   memcpy(&key.args, args, sizeof(uint32_t) * num_args);
   key.num_args = num_args;

   struct hash_entry *entry;
   if (b->types) {
      entry = _mesa_hash_table_search(b->types, &key);
      if (entry)
         return ((struct spirv_type *)entry->data)->type;
   } else {
      b->types = _mesa_hash_table_create(b->mem_ctx,
                                         non_aggregate_type_hash,
                                         non_aggregate_type_equals);
      assert(b->types);
   }

   struct spirv_type *type = rzalloc(b->mem_ctx, struct spirv_type);
   if (!type)
      return 0;

   type->op = op;
   memcpy(&type->args, args, sizeof(uint32_t) * num_args);
   type->num_args = num_args;

   type->type = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, 2 + num_args);
   spirv_buffer_emit_word(&b->types_const_defs, op | ((2 + num_args) << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type->type);
   for (int i = 0; i < num_args; ++i)
      spirv_buffer_emit_word(&b->types_const_defs, args[i]);

   entry = _mesa_hash_table_insert(b->types, type, type);
   assert(entry);

   return ((struct spirv_type *)entry->data)->type;
}

SpvId
spirv_builder_type_void(struct spirv_builder *b)
{
   return get_type_def(b, SpvOpTypeVoid, NULL, 0);
}

SpvId
spirv_builder_type_bool(struct spirv_builder *b)
{
   return get_type_def(b, SpvOpTypeBool, NULL, 0);
}

SpvId
spirv_builder_type_int(struct spirv_builder *b, unsigned width)
{
   uint32_t args[] = { width, 1 };
   if (width == 8)
      spirv_builder_emit_cap(b, SpvCapabilityInt8);
   else if (width == 16)
      spirv_builder_emit_cap(b, SpvCapabilityInt16);
   else if (width == 64)
      spirv_builder_emit_cap(b, SpvCapabilityInt64);
   return get_type_def(b, SpvOpTypeInt, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_uint(struct spirv_builder *b, unsigned width)
{
   uint32_t args[] = { width, 0 };
   if (width == 8)
      spirv_builder_emit_cap(b, SpvCapabilityInt8);
   else if (width == 16)
      spirv_builder_emit_cap(b, SpvCapabilityInt16);
   else if (width == 64)
      spirv_builder_emit_cap(b, SpvCapabilityInt64);
   return get_type_def(b, SpvOpTypeInt, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_float(struct spirv_builder *b, unsigned width)
{
   uint32_t args[] = { width };
   if (width == 16)
      spirv_builder_emit_cap(b, SpvCapabilityFloat16);
   else if (width == 64)
      spirv_builder_emit_cap(b, SpvCapabilityFloat64);
   return get_type_def(b, SpvOpTypeFloat, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_image(struct spirv_builder *b, SpvId sampled_type,
                         SpvDim dim, bool depth, bool arrayed, bool ms,
                         unsigned sampled, SpvImageFormat image_format)
{
   assert(sampled < 3);
   uint32_t args[] = {
      sampled_type, dim, depth ? 1 : 0, arrayed ? 1 : 0, ms ? 1 : 0, sampled,
      image_format
   };
   if (sampled == 2 && ms && dim != SpvDimSubpassData)
      spirv_builder_emit_cap(b, SpvCapabilityStorageImageMultisample);
   return get_type_def(b, SpvOpTypeImage, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_emit_sampled_image(struct spirv_builder *b, SpvId result_type, SpvId image, SpvId sampler)
{
   return spirv_builder_emit_binop(b, SpvOpSampledImage, result_type, image, sampler);
}

SpvId
spirv_builder_type_sampled_image(struct spirv_builder *b, SpvId image_type)
{
   uint32_t args[] = { image_type };
   return get_type_def(b, SpvOpTypeSampledImage, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_sampler(struct spirv_builder *b)
{
   uint32_t args[1] = {0};
   return get_type_def(b, SpvOpTypeSampler, args, 0);
}

SpvId
spirv_builder_type_pointer(struct spirv_builder *b,
                           SpvStorageClass storage_class, SpvId type)
{
   uint32_t args[] = { storage_class, type };
   return get_type_def(b, SpvOpTypePointer, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_vector(struct spirv_builder *b, SpvId component_type,
                          unsigned component_count)
{
   assert(component_count > 1);
   uint32_t args[] = { component_type, component_count };
   return get_type_def(b, SpvOpTypeVector, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_matrix(struct spirv_builder *b, SpvId component_type,
                          unsigned component_count)
{
   assert(component_count > 1);
   uint32_t args[] = { component_type, component_count };
   return get_type_def(b, SpvOpTypeMatrix, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_type_runtime_array(struct spirv_builder *b, SpvId component_type)
{
   SpvId type = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->types_const_defs, SpvOpTypeRuntimeArray | (3 << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type);
   spirv_buffer_emit_word(&b->types_const_defs, component_type);
   return type;
}

SpvId
spirv_builder_type_array(struct spirv_builder *b, SpvId component_type,
                         SpvId length)
{
   SpvId type = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->types_const_defs, SpvOpTypeArray | (4 << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type);
   spirv_buffer_emit_word(&b->types_const_defs, component_type);
   spirv_buffer_emit_word(&b->types_const_defs, length);
   return type;
}

SpvId
spirv_builder_type_struct(struct spirv_builder *b, const SpvId member_types[],
                          size_t num_member_types)
{
   int words = 2 + num_member_types;
   SpvId type = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->types_const_defs, SpvOpTypeStruct | (words << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type);
   for (int i = 0; i < num_member_types; ++i)
      spirv_buffer_emit_word(&b->types_const_defs, member_types[i]);
   return type;
}

SpvId
spirv_builder_type_function(struct spirv_builder *b, SpvId return_type,
                            const SpvId parameter_types[],
                            size_t num_parameter_types)
{
   int words = 3 + num_parameter_types;
   SpvId type = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->types_const_defs, SpvOpTypeFunction | (words << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type);
   spirv_buffer_emit_word(&b->types_const_defs, return_type);
   for (int i = 0; i < num_parameter_types; ++i)
      spirv_buffer_emit_word(&b->types_const_defs, parameter_types[i]);
   return type;
}

struct spirv_const {
   SpvOp op, type;
   uint32_t args[8];
   size_t num_args;

   SpvId result;
};

static uint32_t
const_hash(const void *arg)
{
   const struct spirv_const *key = arg;

   uint32_t hash = 0;
   hash = XXH32(&key->op, sizeof(key->op), hash);
   hash = XXH32(&key->type, sizeof(key->type), hash);
   hash = XXH32(key->args, sizeof(uint32_t) * key->num_args, hash);
   return hash;
}

static bool
const_equals(const void *a, const void *b)
{
   const struct spirv_const *ca = a, *cb = b;

   if (ca->op != cb->op ||
       ca->type != cb->type)
      return false;

   assert(ca->num_args == cb->num_args);
   return memcmp(ca->args, cb->args, sizeof(uint32_t) * ca->num_args) == 0;
}

static SpvId
get_const_def(struct spirv_builder *b, SpvOp op, SpvId type,
              const uint32_t args[], size_t num_args)
{
   struct spirv_const key;
   assert(num_args <= ARRAY_SIZE(key.args));
   key.op = op;
   key.type = type;
   memcpy(&key.args, args, sizeof(uint32_t) * num_args);
   key.num_args = num_args;

   struct hash_entry *entry;
   if (b->consts) {
      entry = _mesa_hash_table_search(b->consts, &key);
      if (entry)
         return ((struct spirv_const *)entry->data)->result;
   } else {
      b->consts = _mesa_hash_table_create(b->mem_ctx, const_hash,
                                          const_equals);
      assert(b->consts);
   }

   struct spirv_const *cnst = rzalloc(b->mem_ctx, struct spirv_const);
   if (!cnst)
      return 0;

   cnst->op = op;
   cnst->type = type;
   memcpy(&cnst->args, args, sizeof(uint32_t) * num_args);
   cnst->num_args = num_args;

   cnst->result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, 3 + num_args);
   spirv_buffer_emit_word(&b->types_const_defs, op | ((3 + num_args) << 16));
   spirv_buffer_emit_word(&b->types_const_defs, type);
   spirv_buffer_emit_word(&b->types_const_defs, cnst->result);
   for (int i = 0; i < num_args; ++i)
      spirv_buffer_emit_word(&b->types_const_defs, args[i]);

   entry = _mesa_hash_table_insert(b->consts, cnst, cnst);
   assert(entry);

   return ((struct spirv_const *)entry->data)->result;
}

static SpvId
emit_constant_32(struct spirv_builder *b, SpvId type, uint32_t val)
{
   uint32_t args[] = { val };
   return get_const_def(b, SpvOpConstant, type, args, ARRAY_SIZE(args));
}

static SpvId
emit_constant_64(struct spirv_builder *b, SpvId type, uint64_t val)
{
   uint32_t args[] = { val & UINT32_MAX, val >> 32 };
   return get_const_def(b, SpvOpConstant, type, args, ARRAY_SIZE(args));
}

SpvId
spirv_builder_const_bool(struct spirv_builder *b, bool val)
{
   return get_const_def(b, val ? SpvOpConstantTrue : SpvOpConstantFalse,
                        spirv_builder_type_bool(b), NULL, 0);
}

SpvId
spirv_builder_const_int(struct spirv_builder *b, int width, int64_t val)
{
   assert(width >= 8);
   SpvId type = spirv_builder_type_int(b, width);
   if (width <= 32)
      return emit_constant_32(b, type, val);
   else
      return emit_constant_64(b, type, val);
}

SpvId
spirv_builder_const_uint(struct spirv_builder *b, int width, uint64_t val)
{
   assert(width >= 8);
   if (width == 8)
      spirv_builder_emit_cap(b, SpvCapabilityInt8);
   else if (width == 16)
      spirv_builder_emit_cap(b, SpvCapabilityInt16);
   else if (width == 64)
      spirv_builder_emit_cap(b, SpvCapabilityInt64);
   SpvId type = spirv_builder_type_uint(b, width);
   if (width <= 32)
      return emit_constant_32(b, type, val);
   else
      return emit_constant_64(b, type, val);
}

SpvId
spirv_builder_spec_const_uint(struct spirv_builder *b, int width)
{
   assert(width <= 32);
   SpvId const_type = spirv_builder_type_uint(b, width);
   SpvId result = spirv_builder_new_id(b);
   spirv_buffer_prepare(&b->types_const_defs, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->types_const_defs, SpvOpSpecConstant | (4 << 16));
   spirv_buffer_emit_word(&b->types_const_defs, const_type);
   spirv_buffer_emit_word(&b->types_const_defs, result);
   /* this is the default value for spec constants;
    * if any users need a different default, add a param to pass for it
    */
   spirv_buffer_emit_word(&b->types_const_defs, 1);
   return result;
}

SpvId
spirv_builder_const_float(struct spirv_builder *b, int width, double val)
{
   assert(width >= 16);
   SpvId type = spirv_builder_type_float(b, width);
   if (width == 16) {
      spirv_builder_emit_cap(b, SpvCapabilityFloat16);
      return emit_constant_32(b, type, _mesa_float_to_half(val));
   } else if (width == 32)
      return emit_constant_32(b, type, u_bitcast_f2u(val));
   else if (width == 64) {
      spirv_builder_emit_cap(b, SpvCapabilityFloat64);
      return emit_constant_64(b, type, u_bitcast_d2u(val));
   }

   unreachable("unhandled float-width");
}

SpvId
spirv_builder_const_composite(struct spirv_builder *b, SpvId result_type,
                              const SpvId constituents[],
                              size_t num_constituents)
{
   return get_const_def(b, SpvOpConstantComposite, result_type,
                        (const uint32_t *)constituents,
                        num_constituents);
}

SpvId
spirv_builder_spec_const_composite(struct spirv_builder *b, SpvId result_type,
                                   const SpvId constituents[],
                                   size_t num_constituents)
{
   SpvId result = spirv_builder_new_id(b);

   assert(num_constituents > 0);
   int words = 3 + num_constituents;
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, words);
   spirv_buffer_emit_word(&b->instructions,
                          SpvOpSpecConstantComposite | (words << 16));
   spirv_buffer_emit_word(&b->instructions, result_type);
   spirv_buffer_emit_word(&b->instructions, result);
   for (int i = 0; i < num_constituents; ++i)
      spirv_buffer_emit_word(&b->instructions, constituents[i]);
   return result;
}

SpvId
spirv_builder_emit_var(struct spirv_builder *b, SpvId type,
                       SpvStorageClass storage_class)
{
   assert(storage_class != SpvStorageClassGeneric);
   struct spirv_buffer *buf = storage_class != SpvStorageClassFunction ?
                              &b->types_const_defs : &b->local_vars;

   SpvId ret = spirv_builder_new_id(b);
   spirv_buffer_prepare(buf, b->mem_ctx, 4);
   spirv_buffer_emit_word(buf, SpvOpVariable | (4 << 16));
   spirv_buffer_emit_word(buf, type);
   spirv_buffer_emit_word(buf, ret);
   spirv_buffer_emit_word(buf, storage_class);
   return ret;
}

void
spirv_builder_emit_memory_barrier(struct spirv_builder *b, SpvScope scope, SpvMemorySemanticsMask semantics)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 3);
   spirv_buffer_emit_word(&b->instructions, SpvOpMemoryBarrier | (3 << 16));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, scope));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, semantics));
}

void
spirv_builder_emit_control_barrier(struct spirv_builder *b, SpvScope scope, SpvScope mem_scope, SpvMemorySemanticsMask semantics)
{
   spirv_buffer_prepare(&b->instructions, b->mem_ctx, 4);
   spirv_buffer_emit_word(&b->instructions, SpvOpControlBarrier | (4 << 16));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, scope));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, mem_scope));
   spirv_buffer_emit_word(&b->instructions, spirv_builder_const_uint(b, 32, semantics));
}

SpvId
spirv_builder_import(struct spirv_builder *b, const char *name)
{
   SpvId result = spirv_builder_new_id(b);
   size_t pos = b->imports.num_words;
   spirv_buffer_prepare(&b->imports, b->mem_ctx, 2);
   spirv_buffer_emit_word(&b->imports, SpvOpExtInstImport);
   spirv_buffer_emit_word(&b->imports, result);
   int len = spirv_buffer_emit_string(&b->imports, b->mem_ctx, name);
   b->imports.words[pos] |= (2 + len) << 16;
   return result;
}

size_t
spirv_builder_get_num_words(struct spirv_builder *b)
{
   const size_t header_size = 5;
   const size_t caps_size = b->caps ? b->caps->entries * 2 : 0;
   return header_size + caps_size +
          b->extensions.num_words +
          b->imports.num_words +
          b->memory_model.num_words +
          b->entry_points.num_words +
          b->exec_modes.num_words +
          b->debug_names.num_words +
          b->decorations.num_words +
          b->types_const_defs.num_words +
          b->local_vars.num_words +
          b->instructions.num_words;
}

size_t
spirv_builder_get_words(struct spirv_builder *b, uint32_t *words,
                        size_t num_words, uint32_t spirv_version,
                        uint32_t *tcs_vertices_out_word)
{
   assert(num_words >= spirv_builder_get_num_words(b));

   size_t written  = 0;
   words[written++] = SpvMagicNumber;
   words[written++] = spirv_version;
   words[written++] = 0;
   words[written++] = b->prev_id + 1;
   words[written++] = 0;

   if (b->caps) {
      set_foreach(b->caps, entry) {
         words[written++] = SpvOpCapability | (2 << 16);
         words[written++] = (uintptr_t)entry->key;
      }
   }

   const struct spirv_buffer *buffers[] = {
      &b->extensions,
      &b->imports,
      &b->memory_model,
      &b->entry_points,
      &b->exec_modes,
      &b->debug_names,
      &b->decorations,
      &b->types_const_defs,
   };

   for (int i = 0; i < ARRAY_SIZE(buffers); ++i) {
      const struct spirv_buffer *buffer = buffers[i];

      if (buffer == &b->exec_modes && *tcs_vertices_out_word > 0)
         *tcs_vertices_out_word += written;

      memcpy(words + written, buffer->words,
             buffer->num_words * sizeof(uint32_t));
      written += buffer->num_words;
   }
   typed_memcpy(&words[written], b->instructions.words, b->local_vars_begin);
   written += b->local_vars_begin;
   typed_memcpy(&words[written], b->local_vars.words, b->local_vars.num_words);
   written += b->local_vars.num_words;
   typed_memcpy(&words[written], &b->instructions.words[b->local_vars_begin], (b->instructions.num_words - b->local_vars_begin));
   written += b->instructions.num_words - b->local_vars_begin;

   assert(written == spirv_builder_get_num_words(b));
   return written;
}

void
spirv_builder_begin_local_vars(struct spirv_builder *b)
{
   b->local_vars_begin = b->instructions.num_words;
}
