/*
 * Copyright 2023 Igalia S.L.
 * SPDX-License-Identifier: MIT
 */

#include "compiler/nir/nir_builder.h"
#include "ir3_nir.h"

static bool
nir_lower_layer_id(nir_builder *b, nir_instr *instr, UNUSED void *cb_data)
{
  if (instr->type != nir_instr_type_intrinsic) {
    return false;
  }
  nir_intrinsic_instr *intr = nir_instr_as_intrinsic(instr);
  if (intr->intrinsic != nir_intrinsic_load_layer_id)
    return false;
  b->cursor = nir_before_instr(&intr->instr);

  nir_variable *layer = nir_find_variable_with_location(b->shader, nir_var_shader_in, VARYING_SLOT_LAYER);

  if (!layer) {
    layer = nir_variable_create(b->shader, nir_var_shader_in, glsl_int_type(), "layer");
    layer->data.location = VARYING_SLOT_LAYER;
    layer->data.driver_location = b->shader->num_inputs++;
  }

  nir_intrinsic_instr *load_input = nir_intrinsic_instr_create(b->shader, nir_intrinsic_load_input);
  nir_intrinsic_set_base(load_input, layer->data.driver_location);
  nir_intrinsic_set_component(load_input, 0);
  load_input->num_components = 1;
  load_input->src[0] = nir_src_for_ssa(nir_imm_int(b, 0));
  nir_intrinsic_set_dest_type(load_input, nir_type_int);
  nir_io_semantics semantics = {
    .location = VARYING_SLOT_LAYER,
    .num_slots = 1,
  };
  nir_intrinsic_set_io_semantics(load_input, semantics);
  nir_def_init(&load_input->instr, &load_input->def, 1, 32);
  nir_builder_instr_insert(b, &load_input->instr);
  nir_def_rewrite_uses(&intr->def, &load_input->def);
  return true;
}

bool ir3_nir_lower_layer_id(nir_shader *shader)
{
  assert(shader->info.stage == MESA_SHADER_FRAGMENT);
  return nir_shader_instructions_pass(shader, nir_lower_layer_id,
                nir_metadata_block_index | nir_metadata_dominance,
                NULL);
}