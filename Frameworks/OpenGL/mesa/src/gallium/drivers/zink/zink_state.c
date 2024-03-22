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

#include "zink_state.h"

#include "zink_context.h"
#include "zink_format.h"
#include "zink_program.h"
#include "zink_screen.h"

#include "compiler/shader_enums.h"
#include "util/u_dual_blend.h"
#include "util/u_memory.h"
#include "util/u_helpers.h"
#include "vulkan/util/vk_format.h"

#include <math.h>

static void *
zink_create_vertex_elements_state(struct pipe_context *pctx,
                                  unsigned num_elements,
                                  const struct pipe_vertex_element *elements)
{
   struct zink_screen *screen = zink_screen(pctx->screen);
   unsigned int i;
   struct zink_vertex_elements_state *ves = CALLOC_STRUCT(zink_vertex_elements_state);
   if (!ves)
      return NULL;
   ves->hw_state.hash = _mesa_hash_pointer(ves);

   int buffer_map[PIPE_MAX_ATTRIBS];
   for (int i = 0; i < ARRAY_SIZE(buffer_map); ++i)
      buffer_map[i] = -1;

   int num_bindings = 0;
   unsigned num_decomposed = 0;
   uint32_t size8 = 0;
   uint32_t size16 = 0;
   uint32_t size32 = 0;
   uint16_t strides[PIPE_MAX_ATTRIBS];
   for (i = 0; i < num_elements; ++i) {
      const struct pipe_vertex_element *elem = elements + i;

      int binding = elem->vertex_buffer_index;
      if (buffer_map[binding] < 0) {
         ves->hw_state.binding_map[num_bindings] = binding;
         buffer_map[binding] = num_bindings++;
      }
      binding = buffer_map[binding];

      ves->bindings[binding].binding = binding;
      ves->bindings[binding].inputRate = elem->instance_divisor ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

      assert(!elem->instance_divisor || zink_screen(pctx->screen)->info.have_EXT_vertex_attribute_divisor);
      if (elem->instance_divisor > screen->info.vdiv_props.maxVertexAttribDivisor)
         debug_printf("zink: clamping instance divisor %u to %u\n", elem->instance_divisor, screen->info.vdiv_props.maxVertexAttribDivisor);
      ves->divisor[binding] = MIN2(elem->instance_divisor, screen->info.vdiv_props.maxVertexAttribDivisor);

      VkFormat format;
      if (screen->format_props[elem->src_format].bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
         format = zink_get_format(screen, elem->src_format);
      else {
         enum pipe_format new_format = zink_decompose_vertex_format(elem->src_format);
         assert(new_format);
         num_decomposed++;
         assert(screen->format_props[new_format].bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);
         if (util_format_get_blocksize(new_format) == 4)
            size32 |= BITFIELD_BIT(i);
         else if (util_format_get_blocksize(new_format) == 2)
            size16 |= BITFIELD_BIT(i);
         else
            size8 |= BITFIELD_BIT(i);
         format = zink_get_format(screen, new_format);
         unsigned size;
         if (i < 8)
            size = 1;
         else if (i < 16)
            size = 2;
         else
            size = 4;
         if (util_format_get_nr_components(elem->src_format) == 4) {
            ves->decomposed_attrs |= BITFIELD_BIT(i);
            ves->decomposed_attrs_size = size;
         } else {
            ves->decomposed_attrs_without_w |= BITFIELD_BIT(i);
            ves->decomposed_attrs_without_w_size = size;
         }
         ves->has_decomposed_attrs = true;
      }

      if (screen->info.have_EXT_vertex_input_dynamic_state) {
         ves->hw_state.dynattribs[i].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
         ves->hw_state.dynattribs[i].binding = binding;
         ves->hw_state.dynattribs[i].location = i;
         ves->hw_state.dynattribs[i].format = format;
         strides[binding] = elem->src_stride;
         assert(ves->hw_state.dynattribs[i].format != VK_FORMAT_UNDEFINED);
         ves->hw_state.dynattribs[i].offset = elem->src_offset;
      } else {
         ves->hw_state.attribs[i].binding = binding;
         ves->hw_state.attribs[i].location = i;
         ves->hw_state.attribs[i].format = format;
         ves->hw_state.b.strides[binding] = elem->src_stride;
         assert(ves->hw_state.attribs[i].format != VK_FORMAT_UNDEFINED);
         ves->hw_state.attribs[i].offset = elem->src_offset;
         ves->min_stride[binding] = MAX2(ves->min_stride[binding], elem->src_offset + vk_format_get_blocksize(format));
      }
   }
   assert(num_decomposed + num_elements <= PIPE_MAX_ATTRIBS);
   u_foreach_bit(i, ves->decomposed_attrs | ves->decomposed_attrs_without_w) {
      const struct pipe_vertex_element *elem = elements + i;
      const struct util_format_description *desc = util_format_description(elem->src_format);
      unsigned size = 1;
      if (size32 & BITFIELD_BIT(i))
         size = 4;
      else if (size16 & BITFIELD_BIT(i))
         size = 2;
      else
         assert(size8 & BITFIELD_BIT(i));
      for (unsigned j = 1; j < desc->nr_channels; j++) {
         if (screen->info.have_EXT_vertex_input_dynamic_state) {
            memcpy(&ves->hw_state.dynattribs[num_elements], &ves->hw_state.dynattribs[i], sizeof(VkVertexInputAttributeDescription2EXT));
            ves->hw_state.dynattribs[num_elements].location = num_elements;
            ves->hw_state.dynattribs[num_elements].offset += j * size;
         } else {
            memcpy(&ves->hw_state.attribs[num_elements], &ves->hw_state.attribs[i], sizeof(VkVertexInputAttributeDescription));
            ves->hw_state.attribs[num_elements].location = num_elements;
            ves->hw_state.attribs[num_elements].offset += j * size;
         }
         num_elements++;
      }
   }
   ves->hw_state.num_bindings = num_bindings;
   ves->hw_state.num_attribs = num_elements;
   if (screen->info.have_EXT_vertex_input_dynamic_state) {
      for (int i = 0; i < num_bindings; ++i) {
         ves->hw_state.dynbindings[i].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
         ves->hw_state.dynbindings[i].binding = ves->bindings[i].binding;
         ves->hw_state.dynbindings[i].inputRate = ves->bindings[i].inputRate;
         ves->hw_state.dynbindings[i].stride = strides[i];
         if (ves->divisor[i])
            ves->hw_state.dynbindings[i].divisor = ves->divisor[i];
         else
            ves->hw_state.dynbindings[i].divisor = 1;
      }
   } else {
      for (int i = 0; i < num_bindings; ++i) {
         ves->hw_state.b.bindings[i].binding = ves->bindings[i].binding;
         ves->hw_state.b.bindings[i].inputRate = ves->bindings[i].inputRate;
         if (ves->divisor[i]) {
            ves->hw_state.b.divisors[ves->hw_state.b.divisors_present].divisor = ves->divisor[i];
            ves->hw_state.b.divisors[ves->hw_state.b.divisors_present].binding = ves->bindings[i].binding;
            ves->hw_state.b.divisors_present++;
         }
      }
   }
   return ves;
}

static void
zink_bind_vertex_elements_state(struct pipe_context *pctx,
                                void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_gfx_pipeline_state *state = &ctx->gfx_pipeline_state;
   zink_flush_dgc_if_enabled(ctx);
   ctx->element_state = cso;
   if (cso) {
      if (state->element_state != &ctx->element_state->hw_state) {
         ctx->vertex_state_changed = !zink_screen(pctx->screen)->info.have_EXT_vertex_input_dynamic_state;
         ctx->vertex_buffers_dirty = ctx->element_state->hw_state.num_bindings > 0;
      }
      state->element_state = &ctx->element_state->hw_state;
      if (zink_screen(pctx->screen)->optimal_keys)
         return;
      const struct zink_vs_key *vs = zink_get_vs_key(ctx);
      uint32_t decomposed_attrs = 0, decomposed_attrs_without_w = 0;
      switch (vs->size) {
      case 1:
         decomposed_attrs = vs->u8.decomposed_attrs;
         decomposed_attrs_without_w = vs->u8.decomposed_attrs_without_w;
         break;
      case 2:
         decomposed_attrs = vs->u16.decomposed_attrs;
         decomposed_attrs_without_w = vs->u16.decomposed_attrs_without_w;
         break;
      case 4:
         decomposed_attrs = vs->u16.decomposed_attrs;
         decomposed_attrs_without_w = vs->u16.decomposed_attrs_without_w;
         break;
      }
      if (ctx->element_state->decomposed_attrs != decomposed_attrs ||
          ctx->element_state->decomposed_attrs_without_w != decomposed_attrs_without_w) {
         unsigned size = MAX2(ctx->element_state->decomposed_attrs_size, ctx->element_state->decomposed_attrs_without_w_size);
         struct zink_shader_key *key = (struct zink_shader_key *)zink_set_vs_key(ctx);
         key->size -= 2 * key->key.vs.size;
         switch (size) {
         case 1:
            key->key.vs.u8.decomposed_attrs = ctx->element_state->decomposed_attrs;
            key->key.vs.u8.decomposed_attrs_without_w = ctx->element_state->decomposed_attrs_without_w;
            break;
         case 2:
            key->key.vs.u16.decomposed_attrs = ctx->element_state->decomposed_attrs;
            key->key.vs.u16.decomposed_attrs_without_w = ctx->element_state->decomposed_attrs_without_w;
            break;
         case 4:
            key->key.vs.u32.decomposed_attrs = ctx->element_state->decomposed_attrs;
            key->key.vs.u32.decomposed_attrs_without_w = ctx->element_state->decomposed_attrs_without_w;
            break;
         default: break;
         }
         key->key.vs.size = size;
         key->size += 2 * size;
      }
   } else {
     state->element_state = NULL;
     ctx->vertex_buffers_dirty = false;
   }
}

static void
zink_delete_vertex_elements_state(struct pipe_context *pctx,
                                  void *ves)
{
   FREE(ves);
}

static VkBlendFactor
blend_factor(enum pipe_blendfactor factor)
{
   switch (factor) {
   case PIPE_BLENDFACTOR_ONE: return VK_BLEND_FACTOR_ONE;
   case PIPE_BLENDFACTOR_SRC_COLOR: return VK_BLEND_FACTOR_SRC_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA: return VK_BLEND_FACTOR_SRC_ALPHA;
   case PIPE_BLENDFACTOR_DST_ALPHA: return VK_BLEND_FACTOR_DST_ALPHA;
   case PIPE_BLENDFACTOR_DST_COLOR: return VK_BLEND_FACTOR_DST_COLOR;
   case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
   case PIPE_BLENDFACTOR_CONST_COLOR: return VK_BLEND_FACTOR_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_CONST_ALPHA: return VK_BLEND_FACTOR_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_SRC1_COLOR: return VK_BLEND_FACTOR_SRC1_COLOR;
   case PIPE_BLENDFACTOR_SRC1_ALPHA: return VK_BLEND_FACTOR_SRC1_ALPHA;

   case PIPE_BLENDFACTOR_ZERO: return VK_BLEND_FACTOR_ZERO;

   case PIPE_BLENDFACTOR_INV_SRC_COLOR:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_ALPHA:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
   case PIPE_BLENDFACTOR_INV_DST_COLOR:
      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;

   case PIPE_BLENDFACTOR_INV_CONST_COLOR:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
   case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
      return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
   case PIPE_BLENDFACTOR_INV_SRC1_COLOR:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
   case PIPE_BLENDFACTOR_INV_SRC1_ALPHA:
      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
   }
   unreachable("unexpected blend factor");
}


static VkBlendOp
blend_op(enum pipe_blend_func func)
{
   switch (func) {
   case PIPE_BLEND_ADD: return VK_BLEND_OP_ADD;
   case PIPE_BLEND_SUBTRACT: return VK_BLEND_OP_SUBTRACT;
   case PIPE_BLEND_REVERSE_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
   case PIPE_BLEND_MIN: return VK_BLEND_OP_MIN;
   case PIPE_BLEND_MAX: return VK_BLEND_OP_MAX;
   }
   unreachable("unexpected blend function");
}

static VkLogicOp
logic_op(enum pipe_logicop func)
{
   switch (func) {
   case PIPE_LOGICOP_CLEAR: return VK_LOGIC_OP_CLEAR;
   case PIPE_LOGICOP_NOR: return VK_LOGIC_OP_NOR;
   case PIPE_LOGICOP_AND_INVERTED: return VK_LOGIC_OP_AND_INVERTED;
   case PIPE_LOGICOP_COPY_INVERTED: return VK_LOGIC_OP_COPY_INVERTED;
   case PIPE_LOGICOP_AND_REVERSE: return VK_LOGIC_OP_AND_REVERSE;
   case PIPE_LOGICOP_INVERT: return VK_LOGIC_OP_INVERT;
   case PIPE_LOGICOP_XOR: return VK_LOGIC_OP_XOR;
   case PIPE_LOGICOP_NAND: return VK_LOGIC_OP_NAND;
   case PIPE_LOGICOP_AND: return VK_LOGIC_OP_AND;
   case PIPE_LOGICOP_EQUIV: return VK_LOGIC_OP_EQUIVALENT;
   case PIPE_LOGICOP_NOOP: return VK_LOGIC_OP_NO_OP;
   case PIPE_LOGICOP_OR_INVERTED: return VK_LOGIC_OP_OR_INVERTED;
   case PIPE_LOGICOP_COPY: return VK_LOGIC_OP_COPY;
   case PIPE_LOGICOP_OR_REVERSE: return VK_LOGIC_OP_OR_REVERSE;
   case PIPE_LOGICOP_OR: return VK_LOGIC_OP_OR;
   case PIPE_LOGICOP_SET: return VK_LOGIC_OP_SET;
   }
   unreachable("unexpected logicop function");
}

/* from iris */
static enum pipe_blendfactor
fix_blendfactor(enum pipe_blendfactor f, bool alpha_to_one)
{
   if (alpha_to_one) {
      if (f == PIPE_BLENDFACTOR_SRC1_ALPHA)
         return PIPE_BLENDFACTOR_ONE;

      if (f == PIPE_BLENDFACTOR_INV_SRC1_ALPHA)
         return PIPE_BLENDFACTOR_ZERO;
   }

   return f;
}

static void *
zink_create_blend_state(struct pipe_context *pctx,
                        const struct pipe_blend_state *blend_state)
{
   struct zink_blend_state *cso = CALLOC_STRUCT(zink_blend_state);
   if (!cso)
      return NULL;
   cso->hash = _mesa_hash_pointer(cso);

   if (blend_state->logicop_enable) {
      cso->logicop_enable = VK_TRUE;
      cso->logicop_func = logic_op(blend_state->logicop_func);
   }

   /* TODO: figure out what to do with dither (nothing is probably "OK" for now,
    *       as dithering is undefined in GL
    */

   /* TODO: these are multisampling-state, and should be set there instead of
    *       here, as that's closer tied to the update-frequency
    */
   cso->alpha_to_coverage = blend_state->alpha_to_coverage;
   cso->alpha_to_one = blend_state->alpha_to_one;
   cso->num_rts = blend_state->max_rt + 1;

   for (int i = 0; i < blend_state->max_rt + 1; ++i) {
      const struct pipe_rt_blend_state *rt = blend_state->rt;
      if (blend_state->independent_blend_enable)
         rt = blend_state->rt + i;

      VkPipelineColorBlendAttachmentState att = {0};

      if (rt->blend_enable) {
         att.blendEnable = VK_TRUE;
         att.srcColorBlendFactor = blend_factor(fix_blendfactor(rt->rgb_src_factor, cso->alpha_to_one));
         att.dstColorBlendFactor = blend_factor(fix_blendfactor(rt->rgb_dst_factor, cso->alpha_to_one));
         att.colorBlendOp = blend_op(rt->rgb_func);
         att.srcAlphaBlendFactor = blend_factor(fix_blendfactor(rt->alpha_src_factor, cso->alpha_to_one));
         att.dstAlphaBlendFactor = blend_factor(fix_blendfactor(rt->alpha_dst_factor, cso->alpha_to_one));
         att.alphaBlendOp = blend_op(rt->alpha_func);
      }

      if (rt->colormask & PIPE_MASK_R)
         att.colorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
      if (rt->colormask & PIPE_MASK_G)
         att.colorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
      if (rt->colormask & PIPE_MASK_B)
         att.colorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
      if (rt->colormask & PIPE_MASK_A)
         att.colorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

      cso->wrmask |= (rt->colormask << i);
      if (rt->blend_enable)
         cso->enables |= BITFIELD_BIT(i);

      cso->attachments[i] = att;

      cso->ds3.enables[i] = att.blendEnable;
      cso->ds3.eq[i].alphaBlendOp = att.alphaBlendOp;
      cso->ds3.eq[i].dstAlphaBlendFactor = att.dstAlphaBlendFactor;
      cso->ds3.eq[i].srcAlphaBlendFactor = att.srcAlphaBlendFactor;
      cso->ds3.eq[i].colorBlendOp = att.colorBlendOp;
      cso->ds3.eq[i].dstColorBlendFactor = att.dstColorBlendFactor;
      cso->ds3.eq[i].srcColorBlendFactor = att.srcColorBlendFactor;
      cso->ds3.wrmask[i] = att.colorWriteMask;
   }
   cso->dual_src_blend = util_blend_state_is_dual(blend_state, 0);

   return cso;
}

static void
zink_bind_blend_state(struct pipe_context *pctx, void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_gfx_pipeline_state* state = &zink_context(pctx)->gfx_pipeline_state;
   zink_flush_dgc_if_enabled(ctx);
   struct zink_blend_state *blend = cso;
   struct zink_blend_state *old_blend = state->blend_state;

   if (state->blend_state != cso) {
      state->blend_state = cso;
      if (!screen->have_full_ds3) {
         state->blend_id = blend ? blend->hash : 0;
         state->dirty = true;
      }
      bool force_dual_color_blend = screen->driconf.dual_color_blend_by_location &&
                                    blend && blend->dual_src_blend && state->blend_state->attachments[0].blendEnable;
      if (force_dual_color_blend != zink_get_fs_base_key(ctx)->force_dual_color_blend)
         zink_set_fs_base_key(ctx)->force_dual_color_blend = force_dual_color_blend;
      ctx->blend_state_changed = true;

      if (cso && screen->have_full_ds3) {
#define STATE_CHECK(NAME, FLAG) \
   if ((!old_blend || old_blend->NAME != blend->NAME)) \
      ctx->ds3_states |= BITFIELD_BIT(ZINK_DS3_BLEND_##FLAG)

         STATE_CHECK(alpha_to_coverage, A2C);
         if (screen->info.dynamic_state3_feats.extendedDynamicState3AlphaToOneEnable) {
            STATE_CHECK(alpha_to_one, A21);
         }
         STATE_CHECK(enables, ON);
         STATE_CHECK(wrmask, WRITE);
         if (old_blend && blend->num_rts == old_blend->num_rts) {
            if (memcmp(blend->ds3.eq, old_blend->ds3.eq, blend->num_rts * sizeof(blend->ds3.eq[0])))
               ctx->ds3_states |= BITFIELD_BIT(ZINK_DS3_BLEND_EQ);
         } else {
            ctx->ds3_states |= BITFIELD_BIT(ZINK_DS3_BLEND_EQ);
         }
         STATE_CHECK(logicop_enable, LOGIC_ON);
         STATE_CHECK(logicop_func, LOGIC);

#undef STATE_CHECK
      }

   }
}

static void
zink_delete_blend_state(struct pipe_context *pctx, void *blend_state)
{
   FREE(blend_state);
}

static VkCompareOp
compare_op(enum pipe_compare_func func)
{
   switch (func) {
   case PIPE_FUNC_NEVER: return VK_COMPARE_OP_NEVER;
   case PIPE_FUNC_LESS: return VK_COMPARE_OP_LESS;
   case PIPE_FUNC_EQUAL: return VK_COMPARE_OP_EQUAL;
   case PIPE_FUNC_LEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
   case PIPE_FUNC_GREATER: return VK_COMPARE_OP_GREATER;
   case PIPE_FUNC_NOTEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
   case PIPE_FUNC_GEQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
   case PIPE_FUNC_ALWAYS: return VK_COMPARE_OP_ALWAYS;
   }
   unreachable("unexpected func");
}

static VkStencilOp
stencil_op(enum pipe_stencil_op op)
{
   switch (op) {
   case PIPE_STENCIL_OP_KEEP: return VK_STENCIL_OP_KEEP;
   case PIPE_STENCIL_OP_ZERO: return VK_STENCIL_OP_ZERO;
   case PIPE_STENCIL_OP_REPLACE: return VK_STENCIL_OP_REPLACE;
   case PIPE_STENCIL_OP_INCR: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
   case PIPE_STENCIL_OP_DECR: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
   case PIPE_STENCIL_OP_INCR_WRAP: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
   case PIPE_STENCIL_OP_DECR_WRAP: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
   case PIPE_STENCIL_OP_INVERT: return VK_STENCIL_OP_INVERT;
   }
   unreachable("unexpected op");
}

static VkStencilOpState
stencil_op_state(const struct pipe_stencil_state *src)
{
   VkStencilOpState ret;
   ret.failOp = stencil_op(src->fail_op);
   ret.passOp = stencil_op(src->zpass_op);
   ret.depthFailOp = stencil_op(src->zfail_op);
   ret.compareOp = compare_op(src->func);
   ret.compareMask = src->valuemask;
   ret.writeMask = src->writemask;
   ret.reference = 0; // not used: we'll use a dynamic state for this
   return ret;
}

static void *
zink_create_depth_stencil_alpha_state(struct pipe_context *pctx,
                                      const struct pipe_depth_stencil_alpha_state *depth_stencil_alpha)
{
   struct zink_depth_stencil_alpha_state *cso = CALLOC_STRUCT(zink_depth_stencil_alpha_state);
   if (!cso)
      return NULL;

   cso->base = *depth_stencil_alpha;

   if (depth_stencil_alpha->depth_enabled) {
      cso->hw_state.depth_test = VK_TRUE;
      cso->hw_state.depth_compare_op = compare_op(depth_stencil_alpha->depth_func);
   }

   if (depth_stencil_alpha->depth_bounds_test) {
      cso->hw_state.depth_bounds_test = VK_TRUE;
      cso->hw_state.min_depth_bounds = depth_stencil_alpha->depth_bounds_min;
      cso->hw_state.max_depth_bounds = depth_stencil_alpha->depth_bounds_max;
   }

   if (depth_stencil_alpha->stencil[0].enabled) {
      cso->hw_state.stencil_test = VK_TRUE;
      cso->hw_state.stencil_front = stencil_op_state(depth_stencil_alpha->stencil);
   }

   if (depth_stencil_alpha->stencil[1].enabled)
      cso->hw_state.stencil_back = stencil_op_state(depth_stencil_alpha->stencil + 1);
   else
      cso->hw_state.stencil_back = cso->hw_state.stencil_front;

   cso->hw_state.depth_write = depth_stencil_alpha->depth_writemask;

   return cso;
}

static void
zink_bind_depth_stencil_alpha_state(struct pipe_context *pctx, void *cso)
{
   struct zink_context *ctx = zink_context(pctx);

   zink_flush_dgc_if_enabled(ctx);
   ctx->dsa_state = cso;

   if (cso) {
      struct zink_gfx_pipeline_state *state = &ctx->gfx_pipeline_state;
      if (state->dyn_state1.depth_stencil_alpha_state != &ctx->dsa_state->hw_state) {
         state->dyn_state1.depth_stencil_alpha_state = &ctx->dsa_state->hw_state;
         state->dirty |= !zink_screen(pctx->screen)->info.have_EXT_extended_dynamic_state;
         ctx->dsa_state_changed = true;
      }
   }
   if (!ctx->track_renderpasses && !ctx->blitting)
      ctx->rp_tc_info_updated = true;
}

static void
zink_delete_depth_stencil_alpha_state(struct pipe_context *pctx,
                                      void *depth_stencil_alpha)
{
   FREE(depth_stencil_alpha);
}

static float
round_to_granularity(float value, float granularity)
{
   return roundf(value / granularity) * granularity;
}

static float
line_width(float width, float granularity, const float range[2])
{
   assert(granularity >= 0);
   assert(range[0] <= range[1]);

   if (granularity > 0)
      width = round_to_granularity(width, granularity);

   return CLAMP(width, range[0], range[1]);
}

static void *
zink_create_rasterizer_state(struct pipe_context *pctx,
                             const struct pipe_rasterizer_state *rs_state)
{
   struct zink_screen *screen = zink_screen(pctx->screen);

   struct zink_rasterizer_state *state = CALLOC_STRUCT(zink_rasterizer_state);
   if (!state)
      return NULL;

   state->base = *rs_state;
   state->base.line_stipple_factor++;

   state->hw_state.line_stipple_enable =
      rs_state->line_stipple_enable &&
      !screen->driver_workarounds.no_linestipple;

   assert(rs_state->depth_clip_far == rs_state->depth_clip_near);
   state->hw_state.depth_clip = rs_state->depth_clip_near;
   state->hw_state.depth_clamp = rs_state->depth_clamp;
   state->hw_state.pv_last = !rs_state->flatshade_first;
   state->hw_state.clip_halfz = rs_state->clip_halfz;

   assert(rs_state->fill_front <= PIPE_POLYGON_MODE_POINT);
   if (rs_state->fill_back != rs_state->fill_front)
      debug_printf("BUG: vulkan doesn't support different front and back fill modes\n");

   if (rs_state->fill_front == PIPE_POLYGON_MODE_POINT &&
       screen->driver_workarounds.no_hw_gl_point) {
      state->hw_state.polygon_mode = VK_POLYGON_MODE_FILL;
      state->cull_mode = VK_CULL_MODE_NONE;
   } else {
      state->hw_state.polygon_mode = rs_state->fill_front; // same values
      state->cull_mode = rs_state->cull_face; // same bits
   }

   state->front_face = rs_state->front_ccw ?
                       VK_FRONT_FACE_COUNTER_CLOCKWISE :
                       VK_FRONT_FACE_CLOCKWISE;

   state->hw_state.line_mode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
   if (rs_state->line_rectangular) {
      if (rs_state->line_smooth &&
          !screen->driver_workarounds.no_linesmooth)
         state->hw_state.line_mode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;
      else
         state->hw_state.line_mode = VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
   } else {
      state->hw_state.line_mode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
   }
   state->dynamic_line_mode = state->hw_state.line_mode;
   switch (state->hw_state.line_mode) {
   case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT:
      if (!screen->info.line_rast_feats.rectangularLines)
         state->dynamic_line_mode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
      break;
   case VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT:
      if (!screen->info.line_rast_feats.smoothLines)
         state->dynamic_line_mode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
      break;
   case VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT:
      if (!screen->info.line_rast_feats.bresenhamLines)
         state->dynamic_line_mode = VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
      break;
   default: break;
   }

   if (!rs_state->line_stipple_enable) {
      state->base.line_stipple_factor = 1;
      state->base.line_stipple_pattern = UINT16_MAX;
   }

   state->offset_fill = util_get_offset(rs_state, rs_state->fill_front);
   state->offset_units = rs_state->offset_units;
   if (!rs_state->offset_units_unscaled)
      state->offset_units *= 2;
   state->offset_clamp = rs_state->offset_clamp;
   state->offset_scale = rs_state->offset_scale;

   state->line_width = line_width(rs_state->line_width,
                                  screen->info.props.limits.lineWidthGranularity,
                                  screen->info.props.limits.lineWidthRange);

   return state;
}

static void
zink_bind_rasterizer_state(struct pipe_context *pctx, void *cso)
{
   struct zink_context *ctx = zink_context(pctx);
   struct zink_screen *screen = zink_screen(pctx->screen);
   struct zink_rasterizer_state *prev_state = ctx->rast_state;
   bool point_quad_rasterization = ctx->rast_state ? ctx->rast_state->base.point_quad_rasterization : false;
   bool scissor = ctx->rast_state ? ctx->rast_state->base.scissor : false;
   bool pv_last = ctx->rast_state ? ctx->rast_state->hw_state.pv_last : false;
   bool force_persample_interp = ctx->gfx_pipeline_state.force_persample_interp;
   bool clip_halfz = ctx->rast_state ? ctx->rast_state->hw_state.clip_halfz : false;
   bool rasterizer_discard = ctx->rast_state ? ctx->rast_state->base.rasterizer_discard : false;
   bool half_pixel_center = ctx->rast_state ? ctx->rast_state->base.half_pixel_center : true;
   float line_width = ctx->rast_state ? ctx->rast_state->base.line_width : 1.0;
   zink_flush_dgc_if_enabled(ctx);
   ctx->rast_state = cso;

   if (ctx->rast_state) {
      if (screen->info.have_EXT_provoking_vertex &&
          pv_last != ctx->rast_state->hw_state.pv_last &&
          /* without this prop, change in pv mode requires new rp */
          !screen->info.pv_props.provokingVertexModePerPipeline)
         zink_batch_no_rp(ctx);
      memcpy(&ctx->gfx_pipeline_state.dyn_state3, &ctx->rast_state->hw_state, sizeof(struct zink_rasterizer_hw_state));

      ctx->gfx_pipeline_state.dirty |= !zink_screen(pctx->screen)->info.have_EXT_extended_dynamic_state3;
      ctx->rast_state_changed = true;

      if (clip_halfz != ctx->rast_state->base.clip_halfz) {
         if (screen->info.have_EXT_depth_clip_control)
            ctx->gfx_pipeline_state.dirty = true;
         else
            zink_set_last_vertex_key(ctx)->clip_halfz = ctx->rast_state->base.clip_halfz;
         ctx->vp_state_changed = true;
      }

      if (screen->info.have_EXT_extended_dynamic_state3) {
#define STATE_CHECK(NAME, FLAG) \
   if (cso && (!prev_state || prev_state->NAME != ctx->rast_state->NAME)) \
      ctx->ds3_states |= BITFIELD_BIT(ZINK_DS3_RAST_##FLAG)

         if (!screen->driver_workarounds.no_linestipple) {
            if (ctx->rast_state->base.line_stipple_enable) {
               STATE_CHECK(base.line_stipple_factor, STIPPLE);
               STATE_CHECK(base.line_stipple_pattern, STIPPLE);
            } else {
               ctx->ds3_states &= ~BITFIELD_BIT(ZINK_DS3_RAST_STIPPLE);
            }
            if (screen->info.dynamic_state3_feats.extendedDynamicState3LineStippleEnable) {
               STATE_CHECK(hw_state.line_stipple_enable, STIPPLE_ON);
            }
         }
         STATE_CHECK(hw_state.depth_clip, CLIP);
         STATE_CHECK(hw_state.depth_clamp, CLAMP);
         STATE_CHECK(hw_state.polygon_mode, POLYGON);
         STATE_CHECK(hw_state.clip_halfz, HALFZ);
         STATE_CHECK(hw_state.pv_last, PV);
         STATE_CHECK(dynamic_line_mode, LINE);

#undef STATE_CHECK
      }

      if (fabs(ctx->rast_state->base.line_width - line_width) > FLT_EPSILON)
         ctx->line_width_changed = true;

      bool lower_gl_point = screen->driver_workarounds.no_hw_gl_point;
      lower_gl_point &= ctx->rast_state->base.fill_front == PIPE_POLYGON_MODE_POINT;
      if (zink_get_gs_key(ctx)->lower_gl_point != lower_gl_point)
         zink_set_gs_key(ctx)->lower_gl_point = lower_gl_point;

      if (ctx->gfx_pipeline_state.dyn_state1.front_face != ctx->rast_state->front_face) {
         ctx->gfx_pipeline_state.dyn_state1.front_face = ctx->rast_state->front_face;
         ctx->gfx_pipeline_state.dirty |= !zink_screen(pctx->screen)->info.have_EXT_extended_dynamic_state;
      }
      if (ctx->gfx_pipeline_state.dyn_state1.cull_mode != ctx->rast_state->cull_mode) {
         ctx->gfx_pipeline_state.dyn_state1.cull_mode = ctx->rast_state->cull_mode;
         ctx->gfx_pipeline_state.dirty |= !zink_screen(pctx->screen)->info.have_EXT_extended_dynamic_state;
      }
      if (!ctx->primitives_generated_active)
         zink_set_rasterizer_discard(ctx, false);
      else if (rasterizer_discard != ctx->rast_state->base.rasterizer_discard)
         zink_set_null_fs(ctx);

      if (ctx->rast_state->base.point_quad_rasterization ||
          ctx->rast_state->base.point_quad_rasterization != point_quad_rasterization)
         zink_set_fs_point_coord_key(ctx);
      if (ctx->rast_state->base.scissor != scissor)
         ctx->scissor_changed = true;

      if (ctx->rast_state->base.force_persample_interp != force_persample_interp) {
         zink_set_fs_base_key(ctx)->force_persample_interp = ctx->rast_state->base.force_persample_interp;
         ctx->gfx_pipeline_state.dirty = true;
      }
      ctx->gfx_pipeline_state.force_persample_interp = ctx->rast_state->base.force_persample_interp;

      if (ctx->rast_state->base.half_pixel_center != half_pixel_center)
         ctx->vp_state_changed = true;

      if (!screen->optimal_keys)
         zink_update_gs_key_rectangular_line(ctx);
   }
}

static void
zink_delete_rasterizer_state(struct pipe_context *pctx, void *rs_state)
{
   FREE(rs_state);
}

struct pipe_vertex_state *
zink_create_vertex_state(struct pipe_screen *pscreen,
                          struct pipe_vertex_buffer *buffer,
                          const struct pipe_vertex_element *elements,
                          unsigned num_elements,
                          struct pipe_resource *indexbuf,
                          uint32_t full_velem_mask)
{
   struct zink_vertex_state *zstate = CALLOC_STRUCT(zink_vertex_state);
   if (!zstate) {
      mesa_loge("ZINK: failed to allocate zstate!");
      return NULL;
   }

   util_init_pipe_vertex_state(pscreen, buffer, elements, num_elements, indexbuf, full_velem_mask,
                               &zstate->b);

   /* Initialize the vertex element state in state->element.
    * Do it by creating a vertex element state object and copying it there.
    */
   struct zink_context ctx;
   ctx.base.screen = pscreen;
   struct zink_vertex_elements_state *elems = zink_create_vertex_elements_state(&ctx.base, num_elements, elements);
   zstate->velems = *elems;
   zink_delete_vertex_elements_state(&ctx.base, elems);

   return &zstate->b;
}

void
zink_vertex_state_destroy(struct pipe_screen *pscreen, struct pipe_vertex_state *vstate)
{
   pipe_vertex_buffer_unreference(&vstate->input.vbuffer);
   pipe_resource_reference(&vstate->input.indexbuf, NULL);
   FREE(vstate);
}

struct pipe_vertex_state *
zink_cache_create_vertex_state(struct pipe_screen *pscreen,
                               struct pipe_vertex_buffer *buffer,
                               const struct pipe_vertex_element *elements,
                               unsigned num_elements,
                               struct pipe_resource *indexbuf,
                               uint32_t full_velem_mask)
{
   struct zink_screen *screen = zink_screen(pscreen);

   return util_vertex_state_cache_get(pscreen, buffer, elements, num_elements, indexbuf,
                                      full_velem_mask, &screen->vertex_state_cache);
}

void
zink_cache_vertex_state_destroy(struct pipe_screen *pscreen, struct pipe_vertex_state *vstate)
{
   struct zink_screen *screen = zink_screen(pscreen);

   util_vertex_state_destroy(pscreen, &screen->vertex_state_cache, vstate);
}

void
zink_context_state_init(struct pipe_context *pctx)
{
   pctx->create_vertex_elements_state = zink_create_vertex_elements_state;
   pctx->bind_vertex_elements_state = zink_bind_vertex_elements_state;
   pctx->delete_vertex_elements_state = zink_delete_vertex_elements_state;

   pctx->create_blend_state = zink_create_blend_state;
   pctx->bind_blend_state = zink_bind_blend_state;
   pctx->delete_blend_state = zink_delete_blend_state;

   pctx->create_depth_stencil_alpha_state = zink_create_depth_stencil_alpha_state;
   pctx->bind_depth_stencil_alpha_state = zink_bind_depth_stencil_alpha_state;
   pctx->delete_depth_stencil_alpha_state = zink_delete_depth_stencil_alpha_state;

   pctx->create_rasterizer_state = zink_create_rasterizer_state;
   pctx->bind_rasterizer_state = zink_bind_rasterizer_state;
   pctx->delete_rasterizer_state = zink_delete_rasterizer_state;
}
