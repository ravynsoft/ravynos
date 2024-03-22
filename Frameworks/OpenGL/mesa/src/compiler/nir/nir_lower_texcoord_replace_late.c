/*
 * Copyright 2022 Alyssa Rosenzweig
 * Copyright 2020 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nir.h"
#include "nir_builder.h"
#include "nir_deref.h"

struct opts {
   unsigned coord_replace;
   bool point_coord_is_sysval;
};

static nir_def *
nir_channel_or_undef(nir_builder *b, nir_def *def, signed int channel)
{
   if (channel >= 0 && channel < def->num_components)
      return nir_channel(b, def, channel);
   else
      return nir_undef(b, def->bit_size, 1);
}

static bool
pass(nir_builder *b, nir_instr *instr, void *data)
{
   struct opts *opts = data;

   if (instr->type != nir_instr_type_intrinsic)
      return false;

   nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
   if (intr->intrinsic != nir_intrinsic_load_interpolated_input &&
       intr->intrinsic != nir_intrinsic_load_input)
      return false;

   nir_src *offset = nir_get_io_offset_src(intr);
   assert(nir_src_is_const(*offset) && "no indirects supported");

   nir_io_semantics sem = nir_intrinsic_io_semantics(intr);
   unsigned location = sem.location + nir_src_as_uint(*offset);
   signed component = nir_intrinsic_component(intr);

   if (location < VARYING_SLOT_TEX0 || location > VARYING_SLOT_TEX7)
      return false;

   if (!(opts->coord_replace & BITFIELD_BIT(location - VARYING_SLOT_TEX0)))
      return false;

   b->cursor = nir_before_instr(instr);
   nir_def *channels[4] = {
      NULL, NULL,
      nir_imm_float(b, 0.0),
      nir_imm_float(b, 1.0)
   };

   if (opts->point_coord_is_sysval) {
      nir_def *pntc = nir_load_point_coord(b);

      b->cursor = nir_after_instr(instr);
      channels[0] = nir_channel(b, pntc, 0);
      channels[1] = nir_channel(b, pntc, 1);
   } else {
      sem.location = VARYING_SLOT_PNTC;
      nir_src_rewrite(offset, nir_imm_int(b, 0));
      nir_intrinsic_set_io_semantics(intr, sem);
      nir_def *raw = &intr->def;

      b->cursor = nir_after_instr(instr);
      channels[0] = nir_channel_or_undef(b, raw, 0 - component);
      channels[1] = nir_channel_or_undef(b, raw, 1 - component);
   }

   nir_def *res = nir_vec(b, &channels[component], intr->num_components);
   nir_def_rewrite_uses_after(&intr->def, res,
                              res->parent_instr);
   return true;
}

void
nir_lower_texcoord_replace_late(nir_shader *s, unsigned coord_replace,
                                bool point_coord_is_sysval)
{
   assert(s->info.stage == MESA_SHADER_FRAGMENT);
   assert(coord_replace != 0);

   uint64_t replace_mask = (((uint64_t)coord_replace) << VARYING_SLOT_TEX0);

   /* If no relevant texcoords are read, there's nothing to do */
   if (!(s->info.inputs_read & replace_mask))
      return;

   /* Otherwise, we're going to replace these texcoord reads with a PNTC read */
   s->info.inputs_read &= ~(((uint64_t)coord_replace) << VARYING_SLOT_TEX0);

   if (!point_coord_is_sysval)
      s->info.inputs_read |= BITFIELD64_BIT(VARYING_SLOT_PNTC);

   nir_shader_instructions_pass(s, pass,
                                nir_metadata_block_index | nir_metadata_dominance,
                                &(struct opts){
                                   .coord_replace = coord_replace,
                                   .point_coord_is_sysval = point_coord_is_sysval,
                                });
}
