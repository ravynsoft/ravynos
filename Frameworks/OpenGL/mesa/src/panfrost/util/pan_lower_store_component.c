/*
 * Copyright (C) 2020-2022 Collabora Ltd.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors (Collabora):
 *      Alyssa Rosenzweig <alyssa.rosenzweig@collabora.com>
 */

#include "compiler/nir/nir_builder.h"
#include "pan_ir.h"

/*
 * If the shader packs multiple varyings into the same location with different
 * location_frac, we'll need to lower to a single varying store that collects
 * all of the channels together. This is because the varying instruction on
 * Midgard and Bifrost is slot-based, writing out an entire vec4 slot at a time.
 */
static bool
lower_store_component(nir_builder *b, nir_intrinsic_instr *intr, void *data)
{
   if (intr->intrinsic != nir_intrinsic_store_output)
      return false;

   struct hash_table_u64 *slots = data;
   unsigned component = nir_intrinsic_component(intr);
   nir_src *slot_src = nir_get_io_offset_src(intr);
   uint64_t slot = nir_src_as_uint(*slot_src) + nir_intrinsic_base(intr);

   nir_intrinsic_instr *prev = _mesa_hash_table_u64_search(slots, slot);
   unsigned mask = (prev ? nir_intrinsic_write_mask(prev) : 0);

   nir_def *value = intr->src[0].ssa;
   b->cursor = nir_before_instr(&intr->instr);

   nir_def *undef = nir_undef(b, 1, value->bit_size);
   nir_def *channels[4] = {undef, undef, undef, undef};

   /* Copy old */
   u_foreach_bit(i, mask) {
      assert(prev != NULL);
      nir_def *prev_ssa = prev->src[0].ssa;
      channels[i] = nir_channel(b, prev_ssa, i);
   }

   /* Copy new */
   unsigned new_mask = nir_intrinsic_write_mask(intr);
   mask |= (new_mask << component);

   u_foreach_bit(i, new_mask) {
      assert(component + i < 4);
      channels[component + i] = nir_channel(b, value, i);
   }

   intr->num_components = util_last_bit(mask);
   nir_src_rewrite(&intr->src[0], nir_vec(b, channels, intr->num_components));

   nir_intrinsic_set_component(intr, 0);
   nir_intrinsic_set_write_mask(intr, mask);

   if (prev) {
      _mesa_hash_table_u64_remove(slots, slot);
      nir_instr_remove(&prev->instr);
   }

   _mesa_hash_table_u64_insert(slots, slot, intr);
   return false;
}

bool
pan_nir_lower_store_component(nir_shader *s)
{
   assert(s->info.stage == MESA_SHADER_VERTEX);

   struct hash_table_u64 *stores = _mesa_hash_table_u64_create(NULL);
   bool progress = nir_shader_intrinsics_pass(
      s, lower_store_component,
      nir_metadata_block_index | nir_metadata_dominance, stores);
   _mesa_hash_table_u64_destroy(stores);
   return progress;
}
