/*
 * Copyright © 2015 Intel Corporation
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

#include "anv_nir.h"
#include "nir/nir_builder.h"
#include "compiler/brw_nir.h"
#include "util/mesa-sha1.h"
#include "util/set.h"

#include "vk_enum_to_str.h"

#include "genxml/genX_bits.h"

/* Sampler tables don't actually have a maximum size but we pick one just so
 * that we don't end up emitting too much state on-the-fly.
 */
#define MAX_SAMPLER_TABLE_SIZE 128
#define BINDLESS_OFFSET        255

#define sizeof_field(type, field) sizeof(((type *)0)->field)

enum binding_property {
   BINDING_PROPERTY_NORMAL   = BITFIELD_BIT(0),
   BINDING_PROPERTY_PUSHABLE = BITFIELD_BIT(1),
};

struct apply_pipeline_layout_state {
   const struct anv_physical_device *pdevice;

   const struct anv_pipeline_sets_layout *layout;
   nir_address_format desc_addr_format;
   nir_address_format ssbo_addr_format;
   nir_address_format ubo_addr_format;

   /* Place to flag lowered instructions so we don't lower them twice */
   struct set *lowered_instrs;

   bool uses_constants;
   bool has_dynamic_buffers;
   bool has_independent_sets;
   uint8_t constants_offset;
   struct {
      bool desc_buffer_used;
      uint8_t desc_offset;

      struct {
         uint8_t use_count;

         /* Binding table offset */
         uint8_t surface_offset;

         /* Sampler table offset */
         uint8_t sampler_offset;

         /* Properties of the binding */
         enum binding_property properties;

         /* For each binding is identified with a unique identifier for push
          * computation.
          */
         uint32_t push_block;
      } *binding;
   } set[MAX_SETS];
};

/* For a given binding, tells us how many binding table entries are needed per
 * element.
 */
static uint32_t
bti_multiplier(const struct apply_pipeline_layout_state *state,
               uint32_t set, uint32_t binding)
{
   const struct anv_descriptor_set_layout *set_layout =
      state->layout->set[set].layout;
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &set_layout->binding[binding];

   return bind_layout->max_plane_count == 0 ? 1 : bind_layout->max_plane_count;
}

static nir_address_format
addr_format_for_desc_type(VkDescriptorType desc_type,
                          struct apply_pipeline_layout_state *state)
{
   switch (desc_type) {
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      return state->ssbo_addr_format;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      return state->ubo_addr_format;

   case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
      return state->desc_addr_format;

   default:
      unreachable("Unsupported descriptor type");
   }
}

static void
add_binding(struct apply_pipeline_layout_state *state,
            uint32_t set, uint32_t binding)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   assert(set < state->layout->num_sets);
   assert(binding < state->layout->set[set].layout->binding_count);

   if (state->set[set].binding[binding].use_count < UINT8_MAX)
      state->set[set].binding[binding].use_count++;

   /* Only flag the descriptor buffer as used if there's actually data for
    * this binding.  This lets us be lazy and call this function constantly
    * without worrying about unnecessarily enabling the buffer.
    */
   if (bind_layout->descriptor_surface_stride)
      state->set[set].desc_buffer_used = true;

   if (bind_layout->dynamic_offset_index >= 0)
      state->has_dynamic_buffers = true;

   state->set[set].binding[binding].properties |= BINDING_PROPERTY_NORMAL;
}

const VkDescriptorBindingFlags non_pushable_binding_flags =
   VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
   VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
   VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

static void
add_binding_type(struct apply_pipeline_layout_state *state,
                 uint32_t set, uint32_t binding, VkDescriptorType type)
{
   add_binding(state, set, binding);

   if ((state->layout->set[set].layout->binding[binding].flags &
        non_pushable_binding_flags) == 0 &&
       (state->layout->set[set].layout->binding[binding].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
        state->layout->set[set].layout->binding[binding].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
        state->layout->set[set].layout->binding[binding].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK ||
        state->layout->set[set].layout->binding[binding].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) &&
       (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
        type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK))
      state->set[set].binding[binding].properties |= BINDING_PROPERTY_PUSHABLE;
}

static void
add_deref_src_binding(struct apply_pipeline_layout_state *state, nir_src src)
{
   nir_deref_instr *deref = nir_src_as_deref(src);
   nir_variable *var = nir_deref_instr_get_variable(deref);
   add_binding(state, var->data.descriptor_set, var->data.binding);
}

static void
add_tex_src_binding(struct apply_pipeline_layout_state *state,
                    nir_tex_instr *tex, nir_tex_src_type deref_src_type)
{
   int deref_src_idx = nir_tex_instr_src_index(tex, deref_src_type);
   if (deref_src_idx < 0)
      return;

   add_deref_src_binding(state, tex->src[deref_src_idx].src);
}

static bool
get_used_bindings(UNUSED nir_builder *_b, nir_instr *instr, void *_state)
{
   struct apply_pipeline_layout_state *state = _state;

   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_vulkan_resource_index:
         add_binding_type(state,
                          nir_intrinsic_desc_set(intrin),
                          nir_intrinsic_binding(intrin),
                          nir_intrinsic_desc_type(intrin));
         break;

      case nir_intrinsic_image_deref_load:
      case nir_intrinsic_image_deref_store:
      case nir_intrinsic_image_deref_atomic:
      case nir_intrinsic_image_deref_atomic_swap:
      case nir_intrinsic_image_deref_size:
      case nir_intrinsic_image_deref_samples:
      case nir_intrinsic_image_deref_load_param_intel:
      case nir_intrinsic_image_deref_load_raw_intel:
      case nir_intrinsic_image_deref_store_raw_intel:
      case nir_intrinsic_image_deref_sparse_load:
         add_deref_src_binding(state, intrin->src[0]);
         break;

      case nir_intrinsic_load_constant:
         state->uses_constants = true;
         break;

      default:
         break;
      }
      break;
   }
   case nir_instr_type_tex: {
      nir_tex_instr *tex = nir_instr_as_tex(instr);
      add_tex_src_binding(state, tex, nir_tex_src_texture_deref);
      add_tex_src_binding(state, tex, nir_tex_src_sampler_deref);
      break;
   }
   default:
      break;
   }

   return false;
}

static nir_intrinsic_instr *
find_descriptor_for_index_src(nir_src src,
                              struct apply_pipeline_layout_state *state)
{
   nir_intrinsic_instr *intrin = nir_src_as_intrinsic(src);

   while (intrin && intrin->intrinsic == nir_intrinsic_vulkan_resource_reindex)
      intrin = nir_src_as_intrinsic(intrin->src[0]);

   if (!intrin || intrin->intrinsic != nir_intrinsic_vulkan_resource_index)
      return NULL;

   return intrin;
}

static bool
descriptor_has_bti(nir_intrinsic_instr *intrin,
                   struct apply_pipeline_layout_state *state)
{
   assert(intrin->intrinsic == nir_intrinsic_vulkan_resource_index);

   uint32_t set = nir_intrinsic_desc_set(intrin);
   uint32_t binding = nir_intrinsic_binding(intrin);
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   uint32_t surface_index;
   if (bind_layout->data & ANV_DESCRIPTOR_INLINE_UNIFORM)
      surface_index = state->set[set].desc_offset;
   else
      surface_index = state->set[set].binding[binding].surface_offset;

   /* Only lower to a BTI message if we have a valid binding table index. */
   return surface_index < MAX_BINDING_TABLE_SIZE;
}

static nir_address_format
descriptor_address_format(nir_intrinsic_instr *intrin,
                          struct apply_pipeline_layout_state *state)
{
   assert(intrin->intrinsic == nir_intrinsic_vulkan_resource_index);

   return addr_format_for_desc_type(nir_intrinsic_desc_type(intrin), state);
}

static nir_intrinsic_instr *
nir_deref_find_descriptor(nir_deref_instr *deref,
                          struct apply_pipeline_layout_state *state)
{
   while (1) {
      /* Nothing we will use this on has a variable */
      assert(deref->deref_type != nir_deref_type_var);

      nir_deref_instr *parent = nir_src_as_deref(deref->parent);
      if (!parent)
         break;

      deref = parent;
   }
   assert(deref->deref_type == nir_deref_type_cast);

   nir_intrinsic_instr *intrin = nir_src_as_intrinsic(deref->parent);
   if (!intrin || intrin->intrinsic != nir_intrinsic_load_vulkan_descriptor)
      return NULL;

   return find_descriptor_for_index_src(intrin->src[0], state);
}

static nir_def *
build_load_descriptor_mem(nir_builder *b,
                          nir_def *desc_addr, unsigned desc_offset,
                          unsigned num_components, unsigned bit_size,
                          const struct apply_pipeline_layout_state *state)

{
   switch (state->desc_addr_format) {
   case nir_address_format_64bit_global_32bit_offset: {
      nir_def *base_addr =
         nir_pack_64_2x32(b, nir_trim_vector(b, desc_addr, 2));
      nir_def *offset32 =
         nir_iadd_imm(b, nir_channel(b, desc_addr, 3), desc_offset);

      return nir_load_global_constant_offset(b, num_components, bit_size,
                                             base_addr, offset32,
                                             .align_mul = 8,
                                             .align_offset = desc_offset % 8);
   }

   case nir_address_format_32bit_index_offset: {
      nir_def *surface_index = nir_channel(b, desc_addr, 0);
      nir_def *offset32 =
         nir_iadd_imm(b, nir_channel(b, desc_addr, 1), desc_offset);

      return nir_load_ubo(b, num_components, bit_size,
                          surface_index, offset32,
                          .align_mul = 8,
                          .align_offset = desc_offset % 8,
                          .range_base = 0,
                          .range = num_components * bit_size / 8);
   }

   default:
      unreachable("Unsupported address format");
   }
}

/* When using direct descriptor, we do not have a structure to read in memory
 * like anv_address_range_descriptor where all the fields match perfectly the
 * vec4 address format we need to generate for A64 messages. Instead we need
 * to build the vec4 from parsing the RENDER_SURFACE_STATE structure. Easy
 * enough for the surface address, lot less fun for the size where you have to
 * combine 3 fields scattered over multiple dwords, add one to the total and
 * do a check against the surface type to deal with the null descriptors.
 *
 * Fortunately we can reuse the Auxiliary surface adddress field to stash our
 * buffer size and just load a vec4.
 */
static nir_def *
build_optimized_load_render_surface_state_address(nir_builder *b,
                                                  nir_def *desc_addr,
                                                  struct apply_pipeline_layout_state *state)

{
   const struct intel_device_info *devinfo = &state->pdevice->info;

   nir_def *surface_addr =
      build_load_descriptor_mem(b, desc_addr,
                                RENDER_SURFACE_STATE_SurfaceBaseAddress_start(devinfo) / 8,
                                4, 32, state);
   nir_def *addr_ldw = nir_channel(b, surface_addr, 0);
   nir_def *addr_udw = nir_channel(b, surface_addr, 1);
   nir_def *length = nir_channel(b, surface_addr, 3);

   return nir_vec4(b, addr_ldw, addr_udw, length, nir_imm_int(b, 0));
}

/* When using direct descriptor, we do not have a structure to read in memory
 * like anv_address_range_descriptor where all the fields match perfectly the
 * vec4 address format we need to generate for A64 messages. Instead we need
 * to build the vec4 from parsing the RENDER_SURFACE_STATE structure. Easy
 * enough for the surface address, lot less fun for the size.
 */
static nir_def *
build_non_optimized_load_render_surface_state_address(nir_builder *b,
                                                      nir_def *desc_addr,
                                                      struct apply_pipeline_layout_state *state)

{
   const struct intel_device_info *devinfo = &state->pdevice->info;

   assert(((RENDER_SURFACE_STATE_SurfaceBaseAddress_start(devinfo) +
            RENDER_SURFACE_STATE_SurfaceBaseAddress_bits(devinfo) - 1) -
           RENDER_SURFACE_STATE_Width_start(devinfo)) / 8 <= 32);

   nir_def *surface_addr =
      build_load_descriptor_mem(b, desc_addr,
                                RENDER_SURFACE_STATE_SurfaceBaseAddress_start(devinfo) / 8,
                                DIV_ROUND_UP(RENDER_SURFACE_STATE_SurfaceBaseAddress_bits(devinfo), 32),
                                32, state);
   nir_def *addr_ldw = nir_channel(b, surface_addr, 0);
   nir_def *addr_udw = nir_channel(b, surface_addr, 1);

   /* Take all the RENDER_SURFACE_STATE fields from the beginning of the
    * structure up to the Depth field.
    */
   const uint32_t type_sizes_dwords =
      DIV_ROUND_UP(RENDER_SURFACE_STATE_Depth_start(devinfo) +
                   RENDER_SURFACE_STATE_Depth_bits(devinfo), 32);
   nir_def *type_sizes =
      build_load_descriptor_mem(b, desc_addr, 0, type_sizes_dwords, 32, state);

   const unsigned width_start = RENDER_SURFACE_STATE_Width_start(devinfo);
   /* SKL PRMs, Volume 2d: Command Reference: Structures, RENDER_SURFACE_STATE
    *
    *    Width:  "bits [6:0]   of the number of entries in the buffer - 1"
    *    Height: "bits [20:7]  of the number of entries in the buffer - 1"
    *    Depth:  "bits [31:21] of the number of entries in the buffer - 1"
    */
   const unsigned width_bits = 7;
   nir_def *width =
      nir_iand_imm(b,
                   nir_ishr_imm(b,
                                nir_channel(b, type_sizes, width_start / 32),
                                width_start % 32),
                   (1u << width_bits) - 1);

   const unsigned height_start = RENDER_SURFACE_STATE_Height_start(devinfo);
   const unsigned height_bits = RENDER_SURFACE_STATE_Height_bits(devinfo);
   nir_def *height =
      nir_iand_imm(b,
                   nir_ishr_imm(b,
                                nir_channel(b, type_sizes, height_start / 32),
                                height_start % 32),
                   (1u << height_bits) - 1);

   const unsigned depth_start = RENDER_SURFACE_STATE_Depth_start(devinfo);
   const unsigned depth_bits = RENDER_SURFACE_STATE_Depth_bits(devinfo);
   nir_def *depth =
      nir_iand_imm(b,
                   nir_ishr_imm(b,
                                nir_channel(b, type_sizes, depth_start / 32),
                                depth_start % 32),
                   (1u << depth_bits) - 1);

   nir_def *length = width;
   length = nir_ior(b, length, nir_ishl_imm(b, height, width_bits));
   length = nir_ior(b, length, nir_ishl_imm(b, depth, width_bits + height_bits));
   length = nir_iadd_imm(b, length, 1);

   /* Check the surface type, if it's SURFTYPE_NULL, set the length of the
    * buffer to 0.
    */
   const unsigned type_start = RENDER_SURFACE_STATE_SurfaceType_start(devinfo);
   const unsigned type_dw = type_start / 32;
   nir_def *type =
      nir_iand_imm(b,
                   nir_ishr_imm(b,
                                nir_channel(b, type_sizes, type_dw),
                                type_start % 32),
                   (1u << RENDER_SURFACE_STATE_SurfaceType_bits(devinfo)) - 1);

   length = nir_bcsel(b,
                      nir_ieq_imm(b, type, 7 /* SURFTYPE_NULL */),
                      nir_imm_int(b, 0), length);

   return nir_vec4(b, addr_ldw, addr_udw, length, nir_imm_int(b, 0));
}

static inline nir_def *
build_load_render_surface_state_address(nir_builder *b,
                                        nir_def *desc_addr,
                                        struct apply_pipeline_layout_state *state)
{
   if (state->pdevice->isl_dev.buffer_length_in_aux_addr)
      return build_optimized_load_render_surface_state_address(b, desc_addr, state);
   return build_non_optimized_load_render_surface_state_address(b, desc_addr, state);
}

/* Load the depth of a 3D storage image.
 *
 * Either by reading the indirect descriptor value, or reading the value from
 * RENDER_SURFACE_STATE.
 *
 * This is necessary for VK_EXT_image_sliced_view_of_3d.
 */
static nir_def *
build_load_storage_3d_image_depth(nir_builder *b,
                                  nir_def *desc_addr,
                                  nir_def *resinfo_depth,
                                  struct apply_pipeline_layout_state *state)

{
   const struct intel_device_info *devinfo = &state->pdevice->info;

   if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT) {
      return build_load_descriptor_mem(
         b, desc_addr,
         offsetof(struct anv_storage_image_descriptor, image_depth),
         1, 32, state);
   } else {
      nir_def *data = build_load_descriptor_mem(
         b, desc_addr,
         RENDER_SURFACE_STATE_RenderTargetViewExtent_start(devinfo) / 8,
         1, 32, state);
      nir_def *depth =
         nir_ushr_imm(
            b, data,
            RENDER_SURFACE_STATE_RenderTargetViewExtent_start(devinfo) % 32);
      depth = nir_iand_imm(
         b, depth,
         (1u << RENDER_SURFACE_STATE_RenderTargetViewExtent_bits(devinfo)) - 1);
      depth = nir_iadd_imm(b, depth, 1);

      /* Return the minimum between the RESINFO value and the
       * RENDER_SURFACE_STATE::RenderTargetViewExtent value.
       *
       * Both are expressed for the current view LOD, but in the case of a
       * SURFTYPE_NULL, RESINFO will return the right value, while the -1
       * value in RENDER_SURFACE_STATE should be ignored.
       */
      return nir_umin(b, resinfo_depth, depth);
   }
}
/** Build a Vulkan resource index
 *
 * A "resource index" is the term used by our SPIR-V parser and the relevant
 * NIR intrinsics for a reference into a descriptor set.  It acts much like a
 * deref in NIR except that it accesses opaque descriptors instead of memory.
 *
 * Coming out of SPIR-V, both the resource indices (in the form of
 * vulkan_resource_[re]index intrinsics) and the memory derefs (in the form
 * of nir_deref_instr) use the same vector component/bit size.  The meaning
 * of those values for memory derefs (nir_deref_instr) is given by the
 * nir_address_format associated with the descriptor type.  For resource
 * indices, it's an entirely internal to ANV encoding which describes, in some
 * sense, the address of the descriptor.  Thanks to the NIR/SPIR-V rules, it
 * must be packed into the same size SSA values as a memory address.  For this
 * reason, the actual encoding may depend both on the address format for
 * memory derefs and the descriptor address format.
 *
 * The load_vulkan_descriptor intrinsic exists to provide a transition point
 * between these two forms of derefs: descriptor and memory.
 */
static nir_def *
build_res_index(nir_builder *b,
                uint32_t set, uint32_t binding,
                nir_def *array_index,
                struct apply_pipeline_layout_state *state)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   uint32_t array_size = bind_layout->array_size;

   uint32_t set_idx;
   switch (state->desc_addr_format) {
   case nir_address_format_64bit_global_32bit_offset:
      /* Descriptor set buffer accesses will go through A64 messages, so the
       * index to get the descriptor set buffer address is located in the
       * anv_push_constants::desc_surface_offsets and it's indexed by the set
       * number.
       */
      set_idx = set;
      break;

   case nir_address_format_32bit_index_offset:
      /* Descriptor set buffer accesses will go through the binding table. The
       * offset is the entry in the binding table.
       */
      assert(state->set[set].desc_offset < MAX_BINDING_TABLE_SIZE);
      set_idx = state->set[set].desc_offset;
      break;

   default:
      unreachable("Unsupported address format");
   }

   assert(bind_layout->dynamic_offset_index < MAX_DYNAMIC_BUFFERS);
      nir_def *dynamic_offset_index;
      if (bind_layout->dynamic_offset_index >= 0) {
         if (state->has_independent_sets) {
            nir_def *dynamic_offset_start =
               nir_load_desc_set_dynamic_index_intel(b, nir_imm_int(b, set));
            dynamic_offset_index =
               nir_iadd_imm(b, dynamic_offset_start,
                            bind_layout->dynamic_offset_index);
         } else {
            dynamic_offset_index =
               nir_imm_int(b,
                           state->layout->set[set].dynamic_offset_start +
                           bind_layout->dynamic_offset_index);
         }
      } else {
         dynamic_offset_index = nir_imm_int(b, 0xff); /* No dynamic offset */
      }

   const uint32_t desc_bti = state->set[set].binding[binding].surface_offset;
   assert(bind_layout->descriptor_surface_stride % 8 == 0);
   const uint32_t desc_stride = bind_layout->descriptor_surface_stride / 8;

      nir_def *packed =
         nir_ior_imm(b,
                     dynamic_offset_index,
                     (desc_stride << 24) |
                     (desc_bti << 16)    |
                     (set_idx << 8));


   return nir_vec4(b, packed,
                      nir_imm_int(b, bind_layout->descriptor_surface_offset),
                      nir_imm_int(b, array_size - 1),
                      array_index);
}

struct res_index_defs {
   nir_def *bti_idx;
   nir_def *set_idx;
   nir_def *dyn_offset_base;
   nir_def *desc_offset_base;
   nir_def *array_index;
   nir_def *desc_stride;
};

static struct res_index_defs
unpack_res_index(nir_builder *b, nir_def *index)
{
   struct res_index_defs defs;

   nir_def *packed = nir_channel(b, index, 0);
   defs.desc_stride =
      nir_imul_imm(b, nir_extract_u8(b, packed, nir_imm_int(b, 3)), 8);
   defs.bti_idx = nir_extract_u8(b, packed, nir_imm_int(b, 2));
   defs.set_idx = nir_extract_u8(b, packed, nir_imm_int(b, 1));
   defs.dyn_offset_base = nir_extract_u8(b, packed, nir_imm_int(b, 0));

   defs.desc_offset_base = nir_channel(b, index, 1);
   defs.array_index = nir_umin(b, nir_channel(b, index, 2),
                                  nir_channel(b, index, 3));

   return defs;
}

/** Whether a surface is accessed through the bindless surface state heap */
static bool
is_binding_bindless(unsigned set, unsigned binding, bool sampler,
                    const struct apply_pipeline_layout_state *state)
{
   /* Has binding table entry has been allocated for this binding? */
   if (sampler &&
       state->set[set].binding[binding].sampler_offset != BINDLESS_OFFSET)
      return false;
   if (!sampler &&
       state->set[set].binding[binding].surface_offset != BINDLESS_OFFSET)
      return false;

   return true;
}

/** Adjust a Vulkan resource index
 *
 * This is the equivalent of nir_deref_type_ptr_as_array for resource indices.
 * For array descriptors, it allows us to adjust the array index.  Thanks to
 * variable pointers, we cannot always fold this re-index operation into the
 * vulkan_resource_index intrinsic and we have to do it based on nothing but
 * the address format.
 */
static nir_def *
build_res_reindex(nir_builder *b, nir_def *orig, nir_def *delta)
{
   return nir_vec4(b, nir_channel(b, orig, 0),
                      nir_channel(b, orig, 1),
                      nir_channel(b, orig, 2),
                      nir_iadd(b, nir_channel(b, orig, 3), delta));
}

/** Get the address for a descriptor given its resource index
 *
 * Because of the re-indexing operations, we can't bounds check descriptor
 * array access until we have the final index.  That means we end up doing the
 * bounds check here, if needed.  See unpack_res_index() for more details.
 *
 * This function takes both a bind_layout and a desc_type which are used to
 * determine the descriptor stride for array descriptors.  The bind_layout is
 * optional for buffer descriptor types.
 */
static nir_def *
build_desc_addr_for_res_index(nir_builder *b,
                              const VkDescriptorType desc_type,
                              nir_def *index, nir_address_format addr_format,
                              struct apply_pipeline_layout_state *state)
{
   struct res_index_defs res = unpack_res_index(b, index);

   nir_def *desc_offset = res.desc_offset_base;
   if (desc_type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      /* Compute the actual descriptor offset.  For inline uniform blocks,
       * the array index is ignored as they are only allowed to be a single
       * descriptor (not an array) and there is no concept of a "stride".
       *
       */
      desc_offset =
         nir_iadd(b, desc_offset, nir_imul(b, res.array_index, res.desc_stride));
   }

   switch (addr_format) {
   case nir_address_format_64bit_global_32bit_offset:
   case nir_address_format_64bit_bounded_global: {
      switch (state->desc_addr_format) {
      case nir_address_format_64bit_global_32bit_offset: {
         nir_def *base_addr =
            nir_load_desc_set_address_intel(b, res.set_idx);
         return nir_vec4(b, nir_unpack_64_2x32_split_x(b, base_addr),
                            nir_unpack_64_2x32_split_y(b, base_addr),
                            nir_imm_int(b, UINT32_MAX),
                            desc_offset);
      }

      case nir_address_format_32bit_index_offset:
         return nir_vec2(b, res.set_idx, desc_offset);

      default:
         unreachable("Unhandled address format");
      }
   }

   case nir_address_format_32bit_index_offset:
      assert(desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK);
      assert(state->desc_addr_format == nir_address_format_32bit_index_offset);
      return nir_vec2(b, res.set_idx, desc_offset);

   default:
      unreachable("Unhandled address format");
   }
}

static nir_def *
build_desc_addr_for_binding(nir_builder *b,
                            unsigned set, unsigned binding,
                            nir_def *array_index,
                            const struct apply_pipeline_layout_state *state)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   switch (state->desc_addr_format) {
   case nir_address_format_64bit_global_32bit_offset:
   case nir_address_format_64bit_bounded_global: {
      nir_def *set_addr = nir_load_desc_set_address_intel(b, nir_imm_int(b, set));
      nir_def *desc_offset =
         nir_iadd_imm(b,
                      nir_imul_imm(b,
                                   array_index,
                                   bind_layout->descriptor_surface_stride),
                      bind_layout->descriptor_surface_offset);

      return nir_vec4(b, nir_unpack_64_2x32_split_x(b, set_addr),
                         nir_unpack_64_2x32_split_y(b, set_addr),
                         nir_imm_int(b, UINT32_MAX),
                         desc_offset);
   }

   case nir_address_format_32bit_index_offset:
      return nir_vec2(b,
                      nir_imm_int(b, state->set[set].desc_offset),
                      nir_iadd_imm(b,
                                   nir_imul_imm(b,
                                                array_index,
                                                bind_layout->descriptor_surface_stride),
                                   bind_layout->descriptor_surface_offset));

   default:
      unreachable("Unhandled address format");
   }
}

static unsigned
binding_descriptor_offset(const struct apply_pipeline_layout_state *state,
                          const struct anv_descriptor_set_binding_layout *bind_layout,
                          bool sampler)
{
   if (sampler &&
       state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT)
      return bind_layout->descriptor_sampler_offset;

   return bind_layout->descriptor_surface_offset;
}

static unsigned
binding_descriptor_stride(const struct apply_pipeline_layout_state *state,
                          const struct anv_descriptor_set_binding_layout *bind_layout,
                          bool sampler)
{
   if (sampler &&
       state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_DIRECT)
      return bind_layout->descriptor_sampler_stride;

   return bind_layout->descriptor_surface_stride;
}

static nir_def *
build_surface_index_for_binding(nir_builder *b,
                                unsigned set, unsigned binding,
                                nir_def *array_index,
                                unsigned plane,
                                bool non_uniform,
                                const struct apply_pipeline_layout_state *state)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];
   const unsigned descriptor_offset =
      binding_descriptor_offset(state, bind_layout, false /* sampler */);
   const unsigned descriptor_stride =
      binding_descriptor_stride(state, bind_layout, false /* sampler */);
   const bool is_bindless =
      is_binding_bindless(set, binding, false /* sampler */, state);

   nir_def *set_offset, *surface_index;
   if (is_bindless) {
      if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT) {
         set_offset = nir_imm_int(b, 0xdeaddead);

         nir_def *desc_addr =
            build_desc_addr_for_binding(b, set, binding, array_index, state);

         surface_index =
            build_load_descriptor_mem(b, desc_addr, 0, 1, 32, state);
      } else {
         set_offset =
            nir_load_push_constant(b, 1, 32, nir_imm_int(b, 0),
                                   .base = offsetof(struct anv_push_constants,
                                                    desc_surface_offsets[set]),
                                   .range = sizeof_field(struct anv_push_constants,
                                                         desc_surface_offsets[set]));

         /* With bindless indexes are offsets in the descriptor buffer */
         surface_index =
            nir_iadd_imm(b,
                         nir_imul_imm(b, array_index, descriptor_stride),
                         descriptor_offset);
         if (plane != 0) {
            assert(plane < bind_layout->max_plane_count);
            surface_index = nir_iadd_imm(b, surface_index,
                                         plane * (descriptor_stride /
                                                  bind_layout->max_plane_count));
         }

         assert(descriptor_offset % 64 == 0);
         assert(descriptor_stride % 64 == 0);
      }
   } else {
      /* Unused */
      set_offset = nir_imm_int(b, 0xdeaddead);

      unsigned bti_stride = bti_multiplier(state, set, binding);
      assert(bti_stride >= 1);

      /* For Ycbcr descriptors, add the plane offset */
      unsigned element_index = plane;

      /* With the binding table, it's an index in the table */
      surface_index =
         nir_iadd_imm(b, nir_imul_imm(b, array_index, bti_stride),
                         state->set[set].binding[binding].surface_offset + element_index);
      assert(state->set[set].binding[binding].surface_offset < MAX_BINDING_TABLE_SIZE);
   }

   return nir_resource_intel(b,
                             set_offset,
                             surface_index,
                             array_index,
                             .desc_set = set,
                             .binding = binding,
                             .resource_block_intel = state->set[set].binding[binding].push_block,
                             .resource_access_intel =
                                (is_bindless ? nir_resource_intel_bindless : 0) |
                                (non_uniform ? nir_resource_intel_non_uniform : 0) |
                                ((state->set[set].binding[binding].properties &
                                  BINDING_PROPERTY_PUSHABLE) ? nir_resource_intel_pushable : 0));
}

static nir_def *
build_sampler_handle_for_binding(nir_builder *b,
                                 unsigned set, unsigned binding,
                                 nir_def *array_index,
                                 unsigned plane,
                                 bool non_uniform,
                                 const struct apply_pipeline_layout_state *state)
{
   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];
   const unsigned descriptor_offset =
      binding_descriptor_offset(state, bind_layout, true /* sampler */);
   const unsigned descriptor_stride =
      binding_descriptor_stride(state, bind_layout, true /* sampler */);
   const bool is_bindless =
      is_binding_bindless(set, binding, true /* sampler */, state);
   nir_def *set_offset, *sampler_index;

   if (is_bindless) {
      if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT) {
         set_offset = nir_imm_int(b, 0xdeaddead);

         nir_def *desc_addr =
            build_desc_addr_for_binding(b, set, binding, array_index, state);

         /* This is anv_sampled_image_descriptor, the sampler handle is always
          * in component 1.
          */
         nir_def *desc_data =
            build_load_descriptor_mem(b, desc_addr, 0, 2, 32, state);

         sampler_index = nir_channel(b, desc_data, 1);
      } else {
         set_offset =
            nir_load_push_constant(b, 1, 32, nir_imm_int(b, 0),
                                   .base = offsetof(struct anv_push_constants,
                                                    desc_sampler_offsets[set]),
                                   .range = sizeof_field(struct anv_push_constants,
                                                         desc_sampler_offsets[set]));

         uint32_t base_offset = descriptor_offset;

         /* The SAMPLER_STATE can only be located at a 64 byte in the combined
          * image/sampler case. Combined image/sampler is not supported to be
          * used with mutable descriptor types.
          */
         if (bind_layout->data & ANV_DESCRIPTOR_SURFACE_SAMPLER)
            base_offset += ANV_SURFACE_STATE_SIZE;

         if (plane != 0) {
            assert(plane < bind_layout->max_plane_count);
            base_offset += plane * (descriptor_stride /
                                    bind_layout->max_plane_count);
         }

         sampler_index =
            nir_iadd_imm(b,
                         nir_imul_imm(b, array_index, descriptor_stride),
                         base_offset);
      }
   } else {
      /* Unused */
      set_offset = nir_imm_int(b, 0xdeaddead);

      sampler_index =
         nir_iadd_imm(b, array_index,
                      state->set[set].binding[binding].sampler_offset + plane);
   }

   return nir_resource_intel(b, set_offset, sampler_index, array_index,
                             .desc_set = set,
                             .binding = binding,
                             .resource_access_intel =
                                (is_bindless ? nir_resource_intel_bindless : 0) |
                                (non_uniform ? nir_resource_intel_non_uniform : 0) |
                                nir_resource_intel_sampler);
}

static nir_def *
build_buffer_dynamic_offset_for_res_index(nir_builder *b,
                                          nir_def *dyn_offset_base,
                                          nir_def *array_index,
                                          struct apply_pipeline_layout_state *state)
{
   nir_def *dyn_offset_idx = nir_iadd(b, dyn_offset_base, array_index);

   nir_def *dyn_load =
      nir_load_push_constant(b, 1, 32, nir_imul_imm(b, dyn_offset_idx, 4),
                             .base = offsetof(struct anv_push_constants, dynamic_offsets),
                             .range = sizeof_field(struct anv_push_constants, dynamic_offsets));

   return nir_bcsel(b, nir_ieq_imm(b, dyn_offset_base, 0xff),
                       nir_imm_int(b, 0), dyn_load);
}

/** Convert a Vulkan resource index into a buffer address
 *
 * In some cases, this does a  memory load from the descriptor set and, in
 * others, it simply converts from one form to another.
 *
 * See build_res_index for details about each resource index format.
 */
static nir_def *
build_indirect_buffer_addr_for_res_index(nir_builder *b,
                                         const VkDescriptorType desc_type,
                                         nir_def *res_index,
                                         nir_address_format addr_format,
                                         struct apply_pipeline_layout_state *state)
{
   struct res_index_defs res = unpack_res_index(b, res_index);

   if (desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      assert(addr_format == state->desc_addr_format);
      return build_desc_addr_for_res_index(b, desc_type, res_index,
                                           addr_format, state);
   } else if (addr_format == nir_address_format_32bit_index_offset) {
      return nir_vec2(b, nir_iadd(b, res.bti_idx, res.array_index),
                         nir_imm_int(b, 0));
   }

   nir_def *desc_addr =
      build_desc_addr_for_res_index(b, desc_type, res_index,
                                    addr_format, state);

   nir_def *desc = build_load_descriptor_mem(b, desc_addr, 0, 4, 32, state);

   if (state->has_dynamic_buffers) {
      /* This shader has dynamic offsets and we have no way of knowing
       * (save from the dynamic offset base index) if this buffer has a
       * dynamic offset.
       */
      nir_def *dyn_offset_idx =
         nir_iadd(b, res.dyn_offset_base, res.array_index);

      nir_def *dyn_load =
         nir_load_push_constant(b, 1, 32, nir_imul_imm(b, dyn_offset_idx, 4),
                                .base = offsetof(struct anv_push_constants, dynamic_offsets),
                                .range = MAX_DYNAMIC_BUFFERS * 4);

      nir_def *dynamic_offset =
         nir_bcsel(b, nir_ieq_imm(b, res.dyn_offset_base, 0xff),
                      nir_imm_int(b, 0), dyn_load);

      /* The dynamic offset gets added to the base pointer so that we
       * have a sliding window range.
       */
      nir_def *base_ptr =
         nir_pack_64_2x32(b, nir_trim_vector(b, desc, 2));
      base_ptr = nir_iadd(b, base_ptr, nir_u2u64(b, dynamic_offset));
      desc = nir_vec4(b, nir_unpack_64_2x32_split_x(b, base_ptr),
                         nir_unpack_64_2x32_split_y(b, base_ptr),
                         nir_channel(b, desc, 2),
                         nir_channel(b, desc, 3));
   }

   /* The last element of the vec4 is always zero.
    *
    * See also struct anv_address_range_descriptor
    */
   return nir_vec4(b, nir_channel(b, desc, 0),
                      nir_channel(b, desc, 1),
                      nir_channel(b, desc, 2),
                      nir_imm_int(b, 0));
}

static nir_def *
build_direct_buffer_addr_for_res_index(nir_builder *b,
                                       const VkDescriptorType desc_type,
                                       nir_def *res_index,
                                       nir_address_format addr_format,
                                       struct apply_pipeline_layout_state *state)
{
   if (desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      assert(addr_format == state->desc_addr_format);
      return build_desc_addr_for_res_index(b, desc_type, res_index,
                                           addr_format, state);
   } else if (addr_format == nir_address_format_32bit_index_offset) {
      struct res_index_defs res = unpack_res_index(b, res_index);

      return nir_vec2(b, nir_iadd(b, res.desc_offset_base,
                                  nir_imul(b, res.array_index, res.desc_stride)),
                      nir_imm_int(b, 0));
   }

   nir_def *desc_addr =
      build_desc_addr_for_res_index(b, desc_type, res_index,
                                    addr_format, state);

   nir_def *addr =
      build_load_render_surface_state_address(b, desc_addr, state);

   if (state->has_dynamic_buffers) {
      struct res_index_defs res = unpack_res_index(b, res_index);

      /* This shader has dynamic offsets and we have no way of knowing (save
       * from the dynamic offset base index) if this buffer has a dynamic
       * offset.
       */
      nir_def *dynamic_offset =
         build_buffer_dynamic_offset_for_res_index(
            b, res.dyn_offset_base, res.array_index, state);

      /* The dynamic offset gets added to the base pointer so that we
       * have a sliding window range.
       */
      nir_def *base_ptr =
         nir_pack_64_2x32(b, nir_trim_vector(b, addr, 2));
      base_ptr = nir_iadd(b, base_ptr, nir_u2u64(b, dynamic_offset));
      addr = nir_vec4(b, nir_unpack_64_2x32_split_x(b, base_ptr),
                         nir_unpack_64_2x32_split_y(b, base_ptr),
                         nir_channel(b, addr, 2),
                         nir_channel(b, addr, 3));
   }

   /* The last element of the vec4 is always zero.
    *
    * See also struct anv_address_range_descriptor
    */
   return nir_vec4(b, nir_channel(b, addr, 0),
                      nir_channel(b, addr, 1),
                      nir_channel(b, addr, 2),
                      nir_imm_int(b, 0));
}

static nir_def *
build_buffer_addr_for_res_index(nir_builder *b,
                                const VkDescriptorType desc_type,
                                nir_def *res_index,
                                nir_address_format addr_format,
                                struct apply_pipeline_layout_state *state)
{
   if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT)
      return build_indirect_buffer_addr_for_res_index(b, desc_type, res_index, addr_format, state);
   else
      return build_direct_buffer_addr_for_res_index(b, desc_type, res_index, addr_format, state);
}

static nir_def *
build_buffer_addr_for_binding(nir_builder *b,
                              const VkDescriptorType desc_type,
                              unsigned set,
                              unsigned binding,
                              nir_def *res_index,
                              nir_address_format addr_format,
                              struct apply_pipeline_layout_state *state)
{
   if (addr_format != nir_address_format_32bit_index_offset)
      return build_buffer_addr_for_res_index(b, desc_type, res_index, addr_format, state);

   if (desc_type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
      const struct anv_descriptor_set_binding_layout *bind_layout =
         &state->layout->set[set].layout->binding[binding];
      return nir_vec2(b,
                      nir_imm_int(b, state->set[set].desc_offset),
                      nir_imm_int(b, bind_layout->descriptor_surface_offset));
   }

   struct res_index_defs res = unpack_res_index(b, res_index);

   return nir_vec2(b,
                   build_surface_index_for_binding(b, set, binding, res.array_index,
                                                   0 /* plane */,
                                                   false /* non_uniform */,
                                                   state),
                   nir_imm_int(b, 0));
}

/** Loads descriptor memory for a variable-based deref chain
 *
 * The deref chain has to terminate at a variable with a descriptor_set and
 * binding set.  This is used for images, textures, and samplers.
 */
static nir_def *
build_load_var_deref_surface_handle(nir_builder *b, nir_deref_instr *deref,
                                    bool non_uniform,
                                    bool *out_is_bindless,
                                    struct apply_pipeline_layout_state *state)
{
   nir_variable *var = nir_deref_instr_get_variable(deref);

   const uint32_t set = var->data.descriptor_set;
   const uint32_t binding = var->data.binding;

   *out_is_bindless =
      is_binding_bindless(set, binding, false /* sampler */, state);

   nir_def *array_index;
   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);
      assert(nir_deref_instr_parent(deref)->deref_type == nir_deref_type_var);
      array_index = deref->arr.index.ssa;
   } else {
      array_index = nir_imm_int(b, 0);
   }

   return build_surface_index_for_binding(b, set, binding, array_index,
                                          0 /* plane */, non_uniform, state);
}

/** A recursive form of build_res_index()
 *
 * This recursively walks a resource [re]index chain and builds the resource
 * index.  It places the new code with the resource [re]index operation in the
 * hopes of better CSE.  This means the cursor is not where you left it when
 * this function returns.
 */
static nir_def *
build_res_index_for_chain(nir_builder *b, nir_intrinsic_instr *intrin,
                          nir_address_format addr_format,
                          uint32_t *set, uint32_t *binding,
                          struct apply_pipeline_layout_state *state)
{
   if (intrin->intrinsic == nir_intrinsic_vulkan_resource_index) {
      b->cursor = nir_before_instr(&intrin->instr);
      *set = nir_intrinsic_desc_set(intrin);
      *binding = nir_intrinsic_binding(intrin);
      return build_res_index(b, *set, *binding, intrin->src[0].ssa, state);
   } else {
      assert(intrin->intrinsic == nir_intrinsic_vulkan_resource_reindex);
      nir_intrinsic_instr *parent = nir_src_as_intrinsic(intrin->src[0]);
      nir_def *index =
         build_res_index_for_chain(b, parent, addr_format,
                                   set, binding, state);

      b->cursor = nir_before_instr(&intrin->instr);

      return build_res_reindex(b, index, intrin->src[1].ssa);
   }
}

/** Builds a buffer address for a given vulkan [re]index intrinsic
 *
 * The cursor is not where you left it when this function returns.
 */
static nir_def *
build_buffer_addr_for_idx_intrin(nir_builder *b,
                                 nir_intrinsic_instr *idx_intrin,
                                 nir_address_format addr_format,
                                 struct apply_pipeline_layout_state *state)
{
   uint32_t set = UINT32_MAX, binding = UINT32_MAX;
   nir_def *res_index =
      build_res_index_for_chain(b, idx_intrin, addr_format,
                                &set, &binding, state);

   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   return build_buffer_addr_for_binding(b, bind_layout->type,
                                        set, binding, res_index,
                                        addr_format, state);
}

/** Builds a buffer address for deref chain
 *
 * This assumes that you can chase the chain all the way back to the original
 * vulkan_resource_index intrinsic.
 *
 * The cursor is not where you left it when this function returns.
 */
static nir_def *
build_buffer_addr_for_deref(nir_builder *b, nir_deref_instr *deref,
                            nir_address_format addr_format,
                            struct apply_pipeline_layout_state *state)
{
   nir_deref_instr *parent = nir_deref_instr_parent(deref);
   if (parent) {
      nir_def *addr =
         build_buffer_addr_for_deref(b, parent, addr_format, state);

      b->cursor = nir_before_instr(&deref->instr);
      return nir_explicit_io_address_from_deref(b, deref, addr, addr_format);
   }

   nir_intrinsic_instr *load_desc = nir_src_as_intrinsic(deref->parent);
   assert(load_desc->intrinsic == nir_intrinsic_load_vulkan_descriptor);

   nir_intrinsic_instr *idx_intrin = nir_src_as_intrinsic(load_desc->src[0]);

   b->cursor = nir_before_instr(&deref->instr);

   return build_buffer_addr_for_idx_intrin(b, idx_intrin, addr_format, state);
}

static bool
try_lower_direct_buffer_intrinsic(nir_builder *b,
                                  nir_intrinsic_instr *intrin, bool is_atomic,
                                  struct apply_pipeline_layout_state *state)
{
   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);
   if (!nir_deref_mode_is_one_of(deref, nir_var_mem_ubo | nir_var_mem_ssbo))
      return false;

   nir_intrinsic_instr *desc = nir_deref_find_descriptor(deref, state);
   if (desc == NULL) {
      /* We should always be able to find the descriptor for UBO access. */
      assert(nir_deref_mode_is_one_of(deref, nir_var_mem_ssbo));
      return false;
   }

   const unsigned set = nir_intrinsic_desc_set(desc);
   const unsigned binding = nir_intrinsic_binding(desc);

   const struct anv_descriptor_set_binding_layout *bind_layout =
      &state->layout->set[set].layout->binding[binding];

   nir_address_format addr_format = descriptor_address_format(desc, state);

   /* Although we could lower non uniform binding table accesses with
    * nir_opt_non_uniform_access, we might as well use an A64 message and
    * avoid the loops inserted by that lowering pass.
    */
   if (nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM)
      return false;

   if (nir_deref_mode_is(deref, nir_var_mem_ssbo)) {
      /* 64-bit atomics only support A64 messages so we can't lower them to
       * the index+offset model.
       */
      if (is_atomic && intrin->def.bit_size == 64 &&
          !state->pdevice->info.has_lsc)
         return false;

      /* If we don't have a BTI for this binding and we're using indirect
       * descriptors, we'll use A64 messages. This is handled in the main
       * lowering path.
       */
      if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT &&
          !descriptor_has_bti(desc, state))
         return false;

      /* Rewrite to 32bit_index_offset whenever we can */
      addr_format = nir_address_format_32bit_index_offset;
   } else {
      assert(nir_deref_mode_is(deref, nir_var_mem_ubo));

      /* If we don't have a BTI for this binding and we're using indirect
       * descriptors, we'll use A64 messages. This is handled in the main
       * lowering path.
       *
       * We make an exception for uniform blocks which are built from the
       * descriptor set base address + offset. There is no indirect data to
       * fetch.
       */
      if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT &&
          bind_layout->type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK &&
          !descriptor_has_bti(desc, state))
         return false;

      /* If this is an inline uniform and the shader stage is bindless, we
       * can't switch to 32bit_index_offset.
       */
      if (bind_layout->type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK ||
          !brw_shader_stage_requires_bindless_resources(b->shader->info.stage))
         addr_format = nir_address_format_32bit_index_offset;
   }

   /* If a dynamic has not been assigned a binding table entry, we need to
    * bail here.
    */
   if ((bind_layout->type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
        bind_layout->type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) &&
       !descriptor_has_bti(desc, state))
      return false;

   nir_def *addr =
      build_buffer_addr_for_deref(b, deref, addr_format, state);

   b->cursor = nir_before_instr(&intrin->instr);
   nir_lower_explicit_io_instr(b, intrin, addr, addr_format);

   return true;
}

static bool
lower_load_accel_struct_desc(nir_builder *b,
                             nir_intrinsic_instr *load_desc,
                             struct apply_pipeline_layout_state *state)
{
   assert(load_desc->intrinsic == nir_intrinsic_load_vulkan_descriptor);

   nir_intrinsic_instr *idx_intrin = nir_src_as_intrinsic(load_desc->src[0]);

   /* It doesn't really matter what address format we choose as
    * everything will constant-fold nicely.  Choose one that uses the
    * actual descriptor buffer.
    */
   const nir_address_format addr_format =
      nir_address_format_64bit_bounded_global;

   uint32_t set = UINT32_MAX, binding = UINT32_MAX;
   nir_def *res_index =
      build_res_index_for_chain(b, idx_intrin, addr_format,
                                &set, &binding, state);

   b->cursor = nir_before_instr(&load_desc->instr);

   struct res_index_defs res = unpack_res_index(b, res_index);
   nir_def *desc_addr =
      build_desc_addr_for_binding(b, set, binding, res.array_index, state);

   /* Acceleration structure descriptors are always uint64_t */
   nir_def *desc = build_load_descriptor_mem(b, desc_addr, 0, 1, 64, state);

   assert(load_desc->def.bit_size == 64);
   assert(load_desc->def.num_components == 1);
   nir_def_rewrite_uses(&load_desc->def, desc);
   nir_instr_remove(&load_desc->instr);

   return true;
}

static bool
lower_direct_buffer_instr(nir_builder *b, nir_instr *instr, void *_state)
{
   struct apply_pipeline_layout_state *state = _state;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
   switch (intrin->intrinsic) {
   case nir_intrinsic_load_deref:
   case nir_intrinsic_store_deref:
      return try_lower_direct_buffer_intrinsic(b, intrin, false, state);

   case nir_intrinsic_deref_atomic:
   case nir_intrinsic_deref_atomic_swap:
      return try_lower_direct_buffer_intrinsic(b, intrin, true, state);

   case nir_intrinsic_get_ssbo_size: {
      /* The get_ssbo_size intrinsic always just takes a
       * index/reindex intrinsic.
       */
      nir_intrinsic_instr *idx_intrin =
         find_descriptor_for_index_src(intrin->src[0], state);
      if (idx_intrin == NULL)
         return false;

      /* We just checked that this is a BTI descriptor */
      const nir_address_format addr_format =
         nir_address_format_32bit_index_offset;

      b->cursor = nir_before_instr(&intrin->instr);

      uint32_t set = UINT32_MAX, binding = UINT32_MAX;
      nir_def *res_index =
         build_res_index_for_chain(b, idx_intrin, addr_format,
                                   &set, &binding, state);

      bool non_uniform = nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM;

      nir_def *surface_index =
         build_surface_index_for_binding(b, set, binding,
                                         nir_channel(b, res_index, 3),
                                         0 /* plane */,
                                         non_uniform,
                                         state);

      nir_src_rewrite(&intrin->src[0], surface_index);
      _mesa_set_add(state->lowered_instrs, intrin);
      return true;
   }

   case nir_intrinsic_load_vulkan_descriptor:
      if (nir_intrinsic_desc_type(intrin) ==
          VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR)
         return lower_load_accel_struct_desc(b, intrin, state);
      return false;

   default:
      return false;
   }
}

static bool
lower_res_index_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                          struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *index =
      build_res_index(b, nir_intrinsic_desc_set(intrin),
                         nir_intrinsic_binding(intrin),
                         intrin->src[0].ssa,
                         state);

   assert(intrin->def.bit_size == index->bit_size);
   assert(intrin->def.num_components == index->num_components);
   nir_def_rewrite_uses(&intrin->def, index);
   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
lower_res_reindex_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                            struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *index =
      build_res_reindex(b, intrin->src[0].ssa,
                           intrin->src[1].ssa);

   assert(intrin->def.bit_size == index->bit_size);
   assert(intrin->def.num_components == index->num_components);
   nir_def_rewrite_uses(&intrin->def, index);
   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
lower_load_vulkan_descriptor(nir_builder *b, nir_intrinsic_instr *intrin,
                             struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_before_instr(&intrin->instr);

   const VkDescriptorType desc_type = nir_intrinsic_desc_type(intrin);
   nir_address_format addr_format = addr_format_for_desc_type(desc_type, state);

   nir_def *desc =
      build_buffer_addr_for_res_index(b,
                                      desc_type, intrin->src[0].ssa,
                                      addr_format, state);

   assert(intrin->def.bit_size == desc->bit_size);
   assert(intrin->def.num_components == desc->num_components);
   nir_def_rewrite_uses(&intrin->def, desc);
   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
lower_get_ssbo_size(nir_builder *b, nir_intrinsic_instr *intrin,
                    struct apply_pipeline_layout_state *state)
{
   if (_mesa_set_search(state->lowered_instrs, intrin))
      return false;

   b->cursor = nir_before_instr(&intrin->instr);

   const nir_address_format addr_format =
      nir_address_format_64bit_bounded_global;

   nir_def *desc_addr =
      nir_build_addr_iadd_imm(
         b,
         build_desc_addr_for_res_index(b,
                                       VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                       intrin->src[0].ssa,
                                       addr_format, state),
         addr_format,
         nir_var_mem_ssbo,
         state->pdevice->isl_dev.ss.size);

   nir_def *desc_range;
   if (state->layout->type == ANV_PIPELINE_DESCRIPTOR_SET_LAYOUT_TYPE_INDIRECT) {
      /* Load the anv_address_range_descriptor */
      desc_range =
         build_load_descriptor_mem(b, desc_addr, 0, 4, 32, state);
   } else {
      /* Build a vec4 similar to anv_address_range_descriptor using the
       * RENDER_SURFACE_STATE.
       */
      desc_range =
         build_load_render_surface_state_address(b, desc_addr, state);
   }

   nir_def *size = nir_channel(b, desc_range, 2);
   nir_def_rewrite_uses(&intrin->def, size);
   nir_instr_remove(&intrin->instr);

   return true;
}

static bool
lower_image_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                      struct apply_pipeline_layout_state *state)
{
   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);

   b->cursor = nir_before_instr(&intrin->instr);

   bool non_uniform = nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM;
   bool is_bindless;
   nir_def *handle =
      build_load_var_deref_surface_handle(b, deref, non_uniform,
                                          &is_bindless, state);
   nir_rewrite_image_intrinsic(intrin, handle, is_bindless);

   return true;
}

static bool
lower_image_size_intrinsic(nir_builder *b, nir_intrinsic_instr *intrin,
                           struct apply_pipeline_layout_state *state)
{
   if (nir_intrinsic_image_dim(intrin) != GLSL_SAMPLER_DIM_3D)
      return lower_image_intrinsic(b, intrin, state);

   nir_deref_instr *deref = nir_src_as_deref(intrin->src[0]);

   b->cursor = nir_before_instr(&intrin->instr);

   bool non_uniform = nir_intrinsic_access(intrin) & ACCESS_NON_UNIFORM;
   bool is_bindless;
   nir_def *handle =
      build_load_var_deref_surface_handle(b, deref, non_uniform,
                                          &is_bindless, state);
   nir_rewrite_image_intrinsic(intrin, handle, is_bindless);

   nir_variable *var = nir_deref_instr_get_variable(deref);
   const uint32_t set = var->data.descriptor_set;
   const uint32_t binding = var->data.binding;

   nir_def *array_index;
   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);
      assert(nir_deref_instr_parent(deref)->deref_type == nir_deref_type_var);
      array_index = deref->arr.index.ssa;
   } else {
      array_index = nir_imm_int(b, 0);
   }

   nir_def *desc_addr = build_desc_addr_for_binding(
      b, set, binding, array_index, state);

   b->cursor = nir_after_instr(&intrin->instr);

   nir_def *image_depth =
      build_load_storage_3d_image_depth(b, desc_addr,
                                        nir_channel(b, &intrin->def, 2),
                                        state);

   nir_def *comps[4] = {};
   for (unsigned c = 0; c < intrin->def.num_components; c++)
      comps[c] = c == 2 ? image_depth : nir_channel(b, &intrin->def, c);

   nir_def *vec = nir_vec(b, comps, intrin->def.num_components);
   nir_def_rewrite_uses_after(&intrin->def, vec, vec->parent_instr);

   return true;
}

static bool
lower_load_constant(nir_builder *b, nir_intrinsic_instr *intrin,
                    struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   /* Any constant-offset load_constant instructions should have been removed
    * by constant folding.
    */
   assert(!nir_src_is_const(intrin->src[0]));
   nir_def *offset = nir_iadd_imm(b, intrin->src[0].ssa,
                                      nir_intrinsic_base(intrin));

   unsigned load_size = intrin->def.num_components *
                        intrin->def.bit_size / 8;
   unsigned load_align = intrin->def.bit_size / 8;

   assert(load_size < b->shader->constant_data_size);
   unsigned max_offset = b->shader->constant_data_size - load_size;
   offset = nir_umin(b, offset, nir_imm_int(b, max_offset));

   nir_def *const_data_addr = nir_pack_64_2x32_split(b,
      nir_iadd(b,
         nir_load_reloc_const_intel(b, BRW_SHADER_RELOC_CONST_DATA_ADDR_LOW),
         offset),
      nir_load_reloc_const_intel(b, BRW_SHADER_RELOC_CONST_DATA_ADDR_HIGH));

   nir_def *data =
      nir_load_global_constant(b, const_data_addr,
                               load_align,
                               intrin->def.num_components,
                               intrin->def.bit_size);

   nir_def_rewrite_uses(&intrin->def, data);

   return true;
}

static bool
lower_base_workgroup_id(nir_builder *b, nir_intrinsic_instr *intrin,
                        struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *base_workgroup_id =
      nir_load_push_constant(b, 3, 32, nir_imm_int(b, 0),
                             .base = offsetof(struct anv_push_constants, cs.base_work_group_id),
                             .range = sizeof_field(struct anv_push_constants, cs.base_work_group_id));
   nir_def_rewrite_uses(&intrin->def, base_workgroup_id);

   return true;
}

static void
lower_tex_deref(nir_builder *b, nir_tex_instr *tex,
                nir_tex_src_type deref_src_type,
                unsigned base_index, unsigned plane,
                struct apply_pipeline_layout_state *state)
{
   int deref_src_idx = nir_tex_instr_src_index(tex, deref_src_type);
   if (deref_src_idx < 0)
      return;

   nir_deref_instr *deref = nir_src_as_deref(tex->src[deref_src_idx].src);
   nir_variable *var = nir_deref_instr_get_variable(deref);

   const bool is_sampler = deref_src_type == nir_tex_src_sampler_deref;
   const unsigned set = var->data.descriptor_set;
   const unsigned binding = var->data.binding;
   const bool bindless = is_binding_bindless(set, binding, is_sampler, state);

   nir_def *array_index = NULL;
   if (deref->deref_type != nir_deref_type_var) {
      assert(deref->deref_type == nir_deref_type_array);

      array_index = deref->arr.index.ssa;
   } else {
      array_index = nir_imm_int(b, 0);
   }

   nir_tex_src_type offset_src_type;
   nir_def *index;
   if (deref_src_type == nir_tex_src_texture_deref) {
      index = build_surface_index_for_binding(b, set, binding, array_index,
                                              plane,
                                              tex->texture_non_uniform,
                                              state);
      offset_src_type = bindless ?
                        nir_tex_src_texture_handle :
                        nir_tex_src_texture_offset;
   } else {
      assert(deref_src_type == nir_tex_src_sampler_deref);

      index = build_sampler_handle_for_binding(b, set, binding, array_index,
                                               plane,
                                               tex->sampler_non_uniform,
                                               state);
      offset_src_type = bindless ?
                        nir_tex_src_sampler_handle :
                        nir_tex_src_sampler_offset;
   }

   nir_src_rewrite(&tex->src[deref_src_idx].src, index);
   tex->src[deref_src_idx].src_type = offset_src_type;
}

static uint32_t
tex_instr_get_and_remove_plane_src(nir_tex_instr *tex)
{
   int plane_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_plane);
   if (plane_src_idx < 0)
      return 0;

   unsigned plane = nir_src_as_uint(tex->src[plane_src_idx].src);

   nir_tex_instr_remove_src(tex, plane_src_idx);

   return plane;
}

static nir_def *
build_def_array_select(nir_builder *b, nir_def **srcs, nir_def *idx,
                       unsigned start, unsigned end)
{
   if (start == end - 1) {
      return srcs[start];
   } else {
      unsigned mid = start + (end - start) / 2;
      return nir_bcsel(b, nir_ilt_imm(b, idx, mid),
                       build_def_array_select(b, srcs, idx, start, mid),
                       build_def_array_select(b, srcs, idx, mid, end));
   }
}

static bool
lower_tex(nir_builder *b, nir_tex_instr *tex,
          struct apply_pipeline_layout_state *state)
{
   unsigned plane = tex_instr_get_and_remove_plane_src(tex);

   b->cursor = nir_before_instr(&tex->instr);

   lower_tex_deref(b, tex, nir_tex_src_texture_deref,
                   tex->texture_index, plane, state);
   lower_tex_deref(b, tex, nir_tex_src_sampler_deref,
                   tex->sampler_index, plane, state);

   /* The whole lot will be embedded in the offset/handle source */
   tex->texture_index = 0;
   tex->sampler_index = 0;

   return true;
}

static bool
lower_ray_query_globals(nir_builder *b, nir_intrinsic_instr *intrin,
                        struct apply_pipeline_layout_state *state)
{
   b->cursor = nir_instr_remove(&intrin->instr);

   nir_def *rq_globals =
      nir_load_push_constant(b, 1, 64, nir_imm_int(b, 0),
                             .base = offsetof(struct anv_push_constants, ray_query_globals),
                             .range = sizeof_field(struct anv_push_constants, ray_query_globals));
   nir_def_rewrite_uses(&intrin->def, rq_globals);

   return true;
}

static bool
apply_pipeline_layout(nir_builder *b, nir_instr *instr, void *_state)
{
   struct apply_pipeline_layout_state *state = _state;

   switch (instr->type) {
   case nir_instr_type_intrinsic: {
      nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
      switch (intrin->intrinsic) {
      case nir_intrinsic_vulkan_resource_index:
         return lower_res_index_intrinsic(b, intrin, state);
      case nir_intrinsic_vulkan_resource_reindex:
         return lower_res_reindex_intrinsic(b, intrin, state);
      case nir_intrinsic_load_vulkan_descriptor:
         return lower_load_vulkan_descriptor(b, intrin, state);
      case nir_intrinsic_get_ssbo_size:
         return lower_get_ssbo_size(b, intrin, state);
      case nir_intrinsic_image_deref_load:
      case nir_intrinsic_image_deref_store:
      case nir_intrinsic_image_deref_atomic:
      case nir_intrinsic_image_deref_atomic_swap:
      case nir_intrinsic_image_deref_samples:
      case nir_intrinsic_image_deref_load_param_intel:
      case nir_intrinsic_image_deref_load_raw_intel:
      case nir_intrinsic_image_deref_store_raw_intel:
      case nir_intrinsic_image_deref_sparse_load:
         return lower_image_intrinsic(b, intrin, state);
      case nir_intrinsic_image_deref_size:
         return lower_image_size_intrinsic(b, intrin, state);
      case nir_intrinsic_load_constant:
         return lower_load_constant(b, intrin, state);
      case nir_intrinsic_load_base_workgroup_id:
         return lower_base_workgroup_id(b, intrin, state);
      case nir_intrinsic_load_ray_query_global_intel:
         return lower_ray_query_globals(b, intrin, state);
      default:
         return false;
      }
      break;
   }
   case nir_instr_type_tex:
      return lower_tex(b, nir_instr_as_tex(instr), state);
   default:
      return false;
   }
}

struct binding_info {
   uint32_t binding;
   uint8_t set;
   uint16_t score;
};

static int
compare_binding_infos(const void *_a, const void *_b)
{
   const struct binding_info *a = _a, *b = _b;
   if (a->score != b->score)
      return b->score - a->score;

   if (a->set != b->set)
      return a->set - b->set;

   return a->binding - b->binding;
}

#ifndef NDEBUG
static void
anv_validate_pipeline_layout(const struct anv_pipeline_sets_layout *layout,
                             nir_shader *shader)
{
   nir_foreach_function_impl(impl, shader) {
      nir_foreach_block(block, impl) {
         nir_foreach_instr(instr, block) {
            if (instr->type != nir_instr_type_intrinsic)
               continue;

            nir_intrinsic_instr *intrin = nir_instr_as_intrinsic(instr);
            if (intrin->intrinsic != nir_intrinsic_vulkan_resource_index)
               continue;

            unsigned set = nir_intrinsic_desc_set(intrin);
            assert(layout->set[set].layout);
         }
      }
   }
}
#endif

static bool
binding_is_promotable_to_push(const struct anv_descriptor_set_binding_layout *bind_layout)
{
   return (bind_layout->flags & non_pushable_binding_flags) == 0;
}

static void
add_null_bti_entry(struct anv_pipeline_bind_map *map)
{
   map->surface_to_descriptor[map->surface_count++] =
      (struct anv_pipeline_binding) {
         .set = ANV_DESCRIPTOR_SET_NULL,
   };
   assert(map->surface_count <= MAX_BINDING_TABLE_SIZE);
}

static void
add_bti_entry(struct anv_pipeline_bind_map *map,
              uint32_t set,
              uint32_t binding,
              uint32_t element,
              uint32_t plane,
              const struct anv_descriptor_set_binding_layout *bind_layout)
{
   map->surface_to_descriptor[map->surface_count++] =
      (struct anv_pipeline_binding) {
         .set = set,
         .binding = binding,
         .index = bind_layout->descriptor_index + element,
         .set_offset = bind_layout->descriptor_surface_offset +
                       element * bind_layout->descriptor_surface_stride +
                       plane * bind_layout->descriptor_data_surface_size,
         .plane = plane,
   };
   assert(map->surface_count <= MAX_BINDING_TABLE_SIZE);
}

static void
add_dynamic_bti_entry(struct anv_pipeline_bind_map *map,
                      uint32_t set,
                      uint32_t binding,
                      uint32_t element,
                      const struct anv_pipeline_sets_layout *layout,
                      const struct anv_descriptor_set_binding_layout *bind_layout)
{
   map->surface_to_descriptor[map->surface_count++] =
      (struct anv_pipeline_binding) {
         .set = set,
         .binding = binding,
         .index = bind_layout->descriptor_index + element,
         .set_offset = bind_layout->descriptor_surface_offset +
                       element * bind_layout->descriptor_surface_stride,
         .dynamic_offset_index = bind_layout->dynamic_offset_index + element,
   };
   assert(map->surface_count <= MAX_BINDING_TABLE_SIZE);
}

static void
add_sampler_entry(struct anv_pipeline_bind_map *map,
                  uint32_t set,
                  uint32_t binding,
                  uint32_t element,
                  uint32_t plane,
                  const struct anv_pipeline_sets_layout *layout,
                  const struct anv_descriptor_set_binding_layout *bind_layout)
{
   assert((bind_layout->descriptor_index + element) < layout->set[set].layout->descriptor_count);
   map->sampler_to_descriptor[map->sampler_count++] =
      (struct anv_pipeline_binding) {
         .set = set,
         .binding = binding,
         .index = bind_layout->descriptor_index + element,
         .plane = plane,
   };
}

static void
add_push_entry(struct anv_pipeline_push_map *push_map,
               uint32_t set,
               uint32_t binding,
               uint32_t element,
               const struct anv_pipeline_sets_layout *layout,
               const struct anv_descriptor_set_binding_layout *bind_layout)
{
   push_map->block_to_descriptor[push_map->block_count++] =
      (struct anv_pipeline_binding) {
         .set = set,
         .binding = binding,
         .index = bind_layout->descriptor_index + element,
         .dynamic_offset_index = bind_layout->dynamic_offset_index + element,
   };
}

static bool
binding_should_use_surface_binding_table(const struct apply_pipeline_layout_state *state,
                                         const struct anv_descriptor_set_binding_layout *binding)
{
   if ((binding->data & ANV_DESCRIPTOR_BTI_SURFACE_STATE) == 0)
      return false;

   if (state->pdevice->always_use_bindless &&
       (binding->data & ANV_DESCRIPTOR_SURFACE))
      return false;

   return true;
}

static bool
binding_should_use_sampler_binding_table(const struct apply_pipeline_layout_state *state,
                                         const struct anv_descriptor_set_binding_layout *binding)
{
   if ((binding->data & ANV_DESCRIPTOR_BTI_SAMPLER_STATE) == 0)
      return false;

   if (state->pdevice->always_use_bindless &&
       (binding->data & ANV_DESCRIPTOR_SAMPLER))
      return false;

   return true;
}

void
anv_nir_apply_pipeline_layout(nir_shader *shader,
                              const struct anv_physical_device *pdevice,
                              enum brw_robustness_flags robust_flags,
                              bool independent_sets,
                              const struct anv_pipeline_sets_layout *layout,
                              struct anv_pipeline_bind_map *map,
                              struct anv_pipeline_push_map *push_map,
                              void *push_map_mem_ctx)
{
   void *mem_ctx = ralloc_context(NULL);

#ifndef NDEBUG
   /* We should not have have any reference to a descriptor set that is not
    * given through the pipeline layout (layout->set[set].layout = NULL).
    */
   anv_validate_pipeline_layout(layout, shader);
#endif

   const bool bindless_stage =
      brw_shader_stage_requires_bindless_resources(shader->info.stage);
   struct apply_pipeline_layout_state state = {
      .pdevice = pdevice,
      .layout = layout,
      .desc_addr_format = bindless_stage ?
                          nir_address_format_64bit_global_32bit_offset :
                          nir_address_format_32bit_index_offset,
      .ssbo_addr_format = anv_nir_ssbo_addr_format(pdevice, robust_flags),
      .ubo_addr_format = anv_nir_ubo_addr_format(pdevice, robust_flags),
      .lowered_instrs = _mesa_pointer_set_create(mem_ctx),
      .has_independent_sets = independent_sets,
   };

   /* Compute the amount of push block items required. */
   unsigned push_block_count = 0;
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (!layout->set[s].layout)
         continue;

      const unsigned count = layout->set[s].layout->binding_count;
      state.set[s].binding = rzalloc_array_size(mem_ctx, sizeof(state.set[s].binding[0]), count);

      const struct anv_descriptor_set_layout *set_layout = layout->set[s].layout;
      for (unsigned b = 0; b < set_layout->binding_count; b++) {
         if (set_layout->binding[b].type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
            push_block_count += set_layout->binding[b].array_size;
      }
   }

   /* Find all use sets/bindings */
   nir_shader_instructions_pass(shader, get_used_bindings,
                                nir_metadata_all, &state);

   /* Assign a BTI to each used descriptor set */
   for (unsigned s = 0; s < layout->num_sets; s++) {
      if (state.desc_addr_format != nir_address_format_32bit_index_offset) {
         state.set[s].desc_offset = BINDLESS_OFFSET;
      } else if (state.set[s].desc_buffer_used) {
         map->surface_to_descriptor[map->surface_count] =
            (struct anv_pipeline_binding) {
               .set = ANV_DESCRIPTOR_SET_DESCRIPTORS,
               .binding = UINT32_MAX,
               .index = s,
            };
         state.set[s].desc_offset = map->surface_count++;
      }
   }

   /* Assign a block index for each surface */
   push_map->block_to_descriptor =
      rzalloc_array(push_map_mem_ctx, struct anv_pipeline_binding,
                    map->surface_count + push_block_count);

   memcpy(push_map->block_to_descriptor,
          map->surface_to_descriptor,
          sizeof(push_map->block_to_descriptor[0]) * map->surface_count);
   push_map->block_count = map->surface_count;

   /* Count used bindings and add push blocks for promotion to push
    * constants
    */
   unsigned used_binding_count = 0;
   for (uint32_t set = 0; set < layout->num_sets; set++) {
      struct anv_descriptor_set_layout *set_layout = layout->set[set].layout;
      if (!set_layout)
         continue;

      for (unsigned b = 0; b < set_layout->binding_count; b++) {
         if (state.set[set].binding[b].use_count == 0)
            continue;

         used_binding_count++;

         const struct anv_descriptor_set_binding_layout *bind_layout =
            &set_layout->binding[b];
         if (!binding_is_promotable_to_push(bind_layout))
            continue;

         if (bind_layout->type != VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            state.set[set].binding[b].push_block = push_map->block_count;
            for (unsigned i = 0; i < bind_layout->array_size; i++)
               add_push_entry(push_map, set, b, i, layout, bind_layout);
         } else {
            state.set[set].binding[b].push_block = state.set[set].desc_offset;
         }
      }
   }

   struct binding_info *infos =
      rzalloc_array(mem_ctx, struct binding_info, used_binding_count);
   used_binding_count = 0;
   for (uint32_t set = 0; set < layout->num_sets; set++) {
      const struct anv_descriptor_set_layout *set_layout = layout->set[set].layout;
      if (!set_layout)
         continue;

      for (unsigned b = 0; b < set_layout->binding_count; b++) {
         if (state.set[set].binding[b].use_count == 0)
            continue;

         const struct anv_descriptor_set_binding_layout *binding =
               &layout->set[set].layout->binding[b];

         /* Do a fixed-point calculation to generate a score based on the
          * number of uses and the binding array size.  We shift by 7 instead
          * of 8 because we're going to use the top bit below to make
          * everything which does not support bindless super higher priority
          * than things which do.
          */
         uint16_t score = ((uint16_t)state.set[set].binding[b].use_count << 7) /
                          binding->array_size;

         /* If the descriptor type doesn't support bindless then put it at the
          * beginning so we guarantee it gets a slot.
          */
         if (!anv_descriptor_supports_bindless(pdevice, binding, true) ||
             !anv_descriptor_supports_bindless(pdevice, binding, false))
            score |= 1 << 15;

         infos[used_binding_count++] = (struct binding_info) {
            .set = set,
            .binding = b,
            .score = score,
         };
      }
   }

   /* Order the binding infos based on score with highest scores first.  If
    * scores are equal we then order by set and binding.
    */
   qsort(infos, used_binding_count, sizeof(struct binding_info),
         compare_binding_infos);

   for (unsigned i = 0; i < used_binding_count; i++) {
      unsigned set = infos[i].set, b = infos[i].binding;
      assert(layout->set[set].layout);
      const struct anv_descriptor_set_binding_layout *binding =
            &layout->set[set].layout->binding[b];

      const uint32_t array_size = binding->array_size;

      if (binding->dynamic_offset_index >= 0)
         state.has_dynamic_buffers = true;

      const unsigned array_multiplier = bti_multiplier(&state, set, b);
      assert(array_multiplier >= 1);

      /* Assume bindless by default */
      state.set[set].binding[b].surface_offset = BINDLESS_OFFSET;
      state.set[set].binding[b].sampler_offset = BINDLESS_OFFSET;

      if (binding_should_use_surface_binding_table(&state, binding)) {
         if (map->surface_count + array_size * array_multiplier > MAX_BINDING_TABLE_SIZE ||
             anv_descriptor_requires_bindless(pdevice, binding, false) ||
             brw_shader_stage_requires_bindless_resources(shader->info.stage)) {
            /* If this descriptor doesn't fit in the binding table or if it
             * requires bindless for some reason, flag it as bindless.
             */
            assert(anv_descriptor_supports_bindless(pdevice, binding, false));
         } else {
            state.set[set].binding[b].surface_offset = map->surface_count;
            if (binding->dynamic_offset_index < 0) {
               struct anv_sampler **samplers = binding->immutable_samplers;
               uint8_t max_planes = bti_multiplier(&state, set, b);
               for (unsigned i = 0; i < binding->array_size; i++) {
                  uint8_t planes = samplers ? samplers[i]->n_planes : 1;
                  for (uint8_t p = 0; p < max_planes; p++) {
                     if (p < planes) {
                        add_bti_entry(map, set, b, i, p, binding);
                     } else {
                        add_null_bti_entry(map);
                     }
                  }
               }
            } else {
               for (unsigned i = 0; i < binding->array_size; i++)
                  add_dynamic_bti_entry(map, set, b, i, layout, binding);
            }
         }
         assert(map->surface_count <= MAX_BINDING_TABLE_SIZE);
      }

      if (binding_should_use_sampler_binding_table(&state, binding)) {
         if (map->sampler_count + array_size * array_multiplier > MAX_SAMPLER_TABLE_SIZE ||
             anv_descriptor_requires_bindless(pdevice, binding, true) ||
             brw_shader_stage_requires_bindless_resources(shader->info.stage)) {
            /* If this descriptor doesn't fit in the binding table or if it
             * requires bindless for some reason, flag it as bindless.
             *
             * We also make large sampler arrays bindless because we can avoid
             * using indirect sends thanks to bindless samplers being packed
             * less tightly than the sampler table.
             */
            assert(anv_descriptor_supports_bindless(pdevice, binding, true));
         } else {
            state.set[set].binding[b].sampler_offset = map->sampler_count;
            uint8_t max_planes = bti_multiplier(&state, set, b);
            for (unsigned i = 0; i < binding->array_size; i++) {
               for (uint8_t p = 0; p < max_planes; p++) {
                  add_sampler_entry(map, set, b, i, p, layout, binding);
               }
            }
         }
      }

      if (binding->data & ANV_DESCRIPTOR_INLINE_UNIFORM) {
         state.set[set].binding[b].surface_offset = state.set[set].desc_offset;
      }

#if 0
      fprintf(stderr, "set=%u binding=%u surface_offset=0x%08x require_bindless=%u type=%s\n",
              set, b,
              state.set[set].binding[b].surface_offset,
              anv_descriptor_requires_bindless(pdevice, binding, false),
              vk_DescriptorType_to_str(binding->type));
#endif
   }

   /* Before we do the normal lowering, we look for any SSBO operations
    * that we can lower to the BTI model and lower them up-front.  The BTI
    * model can perform better than the A64 model for a couple reasons:
    *
    *  1. 48-bit address calculations are potentially expensive and using
    *     the BTI model lets us simply compute 32-bit offsets and the
    *     hardware adds the 64-bit surface base address.
    *
    *  2. The BTI messages, because they use surface states, do bounds
    *     checking for us.  With the A64 model, we have to do our own
    *     bounds checking and this means wider pointers and extra
    *     calculations and branching in the shader.
    *
    * The solution to both of these is to convert things to the BTI model
    * opportunistically.  The reason why we need to do this as a pre-pass
    * is for two reasons:
    *
    *  1. The BTI model requires nir_address_format_32bit_index_offset
    *     pointers which are not the same type as the pointers needed for
    *     the A64 model.  Because all our derefs are set up for the A64
    *     model (in case we have variable pointers), we have to crawl all
    *     the way back to the vulkan_resource_index intrinsic and build a
    *     completely fresh index+offset calculation.
    *
    *  2. Because the variable-pointers-capable lowering that we do as part
    *     of apply_pipeline_layout_block is destructive (It really has to
    *     be to handle variable pointers properly), we've lost the deref
    *     information by the time we get to the load/store/atomic
    *     intrinsics in that pass.
    */
   nir_shader_instructions_pass(shader, lower_direct_buffer_instr,
                                nir_metadata_block_index |
                                nir_metadata_dominance,
                                &state);

   /* We just got rid of all the direct access.  Delete it so it's not in the
    * way when we do our indirect lowering.
    */
   nir_opt_dce(shader);

   nir_shader_instructions_pass(shader, apply_pipeline_layout,
                                nir_metadata_block_index |
                                nir_metadata_dominance,
                                &state);

   ralloc_free(mem_ctx);

   if (brw_shader_stage_is_bindless(shader->info.stage)) {
      assert(map->surface_count == 0);
      assert(map->sampler_count == 0);
   }

#if 0
   fprintf(stderr, "bti:\n");
   for (unsigned i = 0; i < map->surface_count; i++) {
      fprintf(stderr, "  %03i: set=%03u binding=%06i index=%u plane=%u set_offset=0x%08x dyn_offset=0x%08x\n", i,
              map->surface_to_descriptor[i].set,
              map->surface_to_descriptor[i].binding,
              map->surface_to_descriptor[i].index,
              map->surface_to_descriptor[i].plane,
              map->surface_to_descriptor[i].set_offset,
              map->surface_to_descriptor[i].dynamic_offset_index);
   }
   fprintf(stderr, "sti:\n");
   for (unsigned i = 0; i < map->sampler_count; i++) {
      fprintf(stderr, "  %03i: set=%03u binding=%06i index=%u plane=%u\n", i,
              map->sampler_to_descriptor[i].set,
              map->sampler_to_descriptor[i].binding,
              map->sampler_to_descriptor[i].index,
              map->sampler_to_descriptor[i].plane);
   }
#endif

   /* Now that we're done computing the surface and sampler portions of the
    * bind map, hash them.  This lets us quickly determine if the actual
    * mapping has changed and not just a no-op pipeline change.
    */
   _mesa_sha1_compute(map->surface_to_descriptor,
                      map->surface_count * sizeof(struct anv_pipeline_binding),
                      map->surface_sha1);
   _mesa_sha1_compute(map->sampler_to_descriptor,
                      map->sampler_count * sizeof(struct anv_pipeline_binding),
                      map->sampler_sha1);
}
