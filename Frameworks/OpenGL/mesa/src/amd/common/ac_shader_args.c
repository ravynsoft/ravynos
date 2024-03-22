/*
 * Copyright 2019 Valve Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_shader_args.h"

#include "nir/nir_builder.h"

void ac_add_arg(struct ac_shader_args *info, enum ac_arg_regfile regfile, unsigned size,
                enum ac_arg_type type, struct ac_arg *arg)
{
   assert(info->arg_count < AC_MAX_ARGS);

   unsigned offset;
   if (regfile == AC_ARG_SGPR) {
      offset = info->num_sgprs_used;
      info->num_sgprs_used += size;
   } else {
      assert(regfile == AC_ARG_VGPR);
      offset = info->num_vgprs_used;
      info->num_vgprs_used += size;
   }

   info->args[info->arg_count].file = regfile;
   info->args[info->arg_count].offset = offset;
   info->args[info->arg_count].size = size;
   info->args[info->arg_count].type = type;

   if (arg) {
      arg->arg_index = info->arg_count;
      arg->used = true;
   }

   info->arg_count++;
}

void ac_add_return(struct ac_shader_args *info, enum ac_arg_regfile regfile)
{
   assert(info->return_count < AC_MAX_ARGS);

   if (regfile == AC_ARG_SGPR) {
      /* SGPRs must be inserted before VGPRs. */
      assert(info->num_vgprs_returned == 0);
      info->num_sgprs_returned++;;
   } else {
      assert(regfile == AC_ARG_VGPR);
      info->num_vgprs_returned++;
   }

   info->return_count++;
}

void ac_add_preserved(struct ac_shader_args *info, const struct ac_arg *arg)
{
   info->args[arg->arg_index].preserved = true;
}

void ac_compact_ps_vgpr_args(struct ac_shader_args *info, uint32_t spi_ps_input)
{
   /* LLVM optimizes away unused FS inputs and computes spi_ps_input_addr itself and then
    * communicates the results back via the ELF binary. Mirror what LLVM does by re-mapping the
    * VGPR arguments here.
    */
   unsigned vgpr_arg = 0;
   unsigned vgpr_reg = 0;

   for (unsigned i = 0; i < info->arg_count; i++) {
      if (info->args[i].file != AC_ARG_VGPR)
         continue;

      if (!(spi_ps_input & (1 << vgpr_arg))) {
         info->args[i].skip = true;
      } else {
         info->args[i].offset = vgpr_reg;
         vgpr_reg += info->args[i].size;
      }
      vgpr_arg++;
   }

   info->num_vgprs_used = vgpr_reg;
}
