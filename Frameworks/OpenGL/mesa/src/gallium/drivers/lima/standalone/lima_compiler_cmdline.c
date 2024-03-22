/*
 * Copyright (c) 2017 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

#include "main/mtypes.h"

#include "compiler/glsl_types.h"
#include "compiler/glsl/standalone.h"
#include "compiler/glsl/glsl_to_nir.h"
#include "compiler/glsl/gl_nir.h"

#include "lima_context.h"
#include "lima_program.h"
#include "ir/lima_ir.h"
#include "standalone/glsl.h"

static void
print_usage(void)
{
   printf("Usage: lima_compiler [OPTIONS]... FILE\n");
   printf("    --help            - show this message\n");
}

static void
insert_sorted(struct exec_list *var_list, nir_variable *new_var)
{
   nir_foreach_variable_in_list(var, var_list) {
      if (var->data.location > new_var->data.location &&
          new_var->data.location >= 0) {
         exec_node_insert_node_before(&var->node, &new_var->node);
         return;
      }
   }
   exec_list_push_tail(var_list, &new_var->node);
}

static void
sort_varyings(nir_shader *nir, nir_variable_mode mode)
{
   struct exec_list new_list;
   exec_list_make_empty(&new_list);
   nir_foreach_variable_with_modes_safe(var, nir, mode) {
      exec_node_remove(&var->node);
      insert_sorted(&new_list, var);
   }
   exec_list_append(&nir->variables, &new_list);
}

static void
fixup_varying_slots(nir_shader *nir, nir_variable_mode mode)
{
   nir_foreach_variable_with_modes(var, nir, mode) {
      if (var->data.location >= VARYING_SLOT_VAR0) {
         var->data.location += 9;
      } else if ((var->data.location >= VARYING_SLOT_TEX0) &&
                 (var->data.location <= VARYING_SLOT_TEX7)) {
         var->data.location += VARYING_SLOT_VAR0 - VARYING_SLOT_TEX0;
      }
   }
}

static nir_shader *
load_glsl(unsigned num_files, char* const* files, gl_shader_stage stage)
{
   static const struct standalone_options options = {
      .glsl_version = 110,
      .do_link = false,
   };
   unsigned shader = 0;
   switch (stage) {
   case MESA_SHADER_FRAGMENT:
      shader = PIPE_SHADER_FRAGMENT;
      break;
   case MESA_SHADER_VERTEX:
      shader = PIPE_SHADER_VERTEX;
      break;
   default:
      unreachable("bad stage");
   }
   struct gl_shader_program *prog;
   const nir_shader_compiler_options *nir_options =
      lima_program_get_compiler_options(shader);
   static struct gl_context local_ctx;

   prog = standalone_compile_shader(&options, num_files, files, &local_ctx);
   if (!prog)
      errx(1, "couldn't parse `%s'", files[0]);

   lima_do_glsl_optimizations(prog->_LinkedShaders[stage]->ir);

   nir_shader *nir = glsl_to_nir(&local_ctx.Const, prog, stage, nir_options);

   /* required NIR passes: */
   if (nir_options->lower_all_io_to_temps ||
       nir->info.stage == MESA_SHADER_VERTEX ||
       nir->info.stage == MESA_SHADER_GEOMETRY) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir),
                 true, true);
   } else if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir),
                 true, false);
   }

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   nir_print_shader(nir, stdout);
   NIR_PASS_V(nir, gl_nir_lower_atomics, prog, true);
   NIR_PASS_V(nir, nir_lower_atomics_to_ssbo, 0);
   nir_print_shader(nir, stdout);

   switch (stage) {
   case MESA_SHADER_VERTEX:
      nir_assign_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               st_glsl_type_size);

      /* Re-lower global vars, to deal with any dead VS inputs. */
      NIR_PASS_V(nir, nir_lower_global_vars_to_local);

      sort_varyings(nir, nir_var_shader_out);
      nir_assign_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               st_glsl_type_size);
      fixup_varying_slots(nir, nir_var_shader_out);
      break;
   case MESA_SHADER_FRAGMENT:
      sort_varyings(nir, nir_var_shader_in);
      nir_assign_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               st_glsl_type_size);
      fixup_varying_slots(nir, nir_var_shader_in);
      nir_assign_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               st_glsl_type_size);
      break;
   default:
      errx(1, "unhandled shader stage: %d", stage);
   }

   nir_assign_var_locations(nir, nir_var_uniform,
                            &nir->num_uniforms,
                            st_glsl_type_size);

   NIR_PASS_V(nir, nir_lower_system_values);
   NIR_PASS_V(nir, nir_lower_frexp);
   NIR_PASS_V(nir, gl_nir_lower_samplers, prog);

   return nir;
}

int
main(int argc, char **argv)
{
   int n;

   lima_debug = LIMA_DEBUG_GP | LIMA_DEBUG_PP;

   if (argc < 2) {
      print_usage();
      return 1;
   }

   for (n = 1; n < argc; n++) {
      if (!strcmp(argv[n], "--help")) {
         print_usage();
         return 1;
      }

      break;
   }

   char *filename[10] = {0};
   filename[0] = argv[n];

   char *ext = rindex(filename[0], '.');
   unsigned stage = 0;

   if (!strcmp(ext, ".frag")) {
      stage = MESA_SHADER_FRAGMENT;
   }
   else if (!strcmp(ext, ".vert")) {
      stage = MESA_SHADER_VERTEX;
   }
   else {
      print_usage();
      return -1;
   }

   struct nir_lower_tex_options tex_options = {
      .lower_txp = ~0u,
      .lower_invalid_implicit_lod = true,
   };

   nir_shader *nir = load_glsl(1, filename, stage);

   switch (stage) {
   case MESA_SHADER_VERTEX:
      lima_program_optimize_vs_nir(nir);

      nir_print_shader(nir, stdout);

      struct lima_vs_compiled_shader *vs = ralloc(nir, struct lima_vs_compiled_shader);
      gpir_compile_nir(vs, nir, NULL);
      break;
   case MESA_SHADER_FRAGMENT:
      lima_program_optimize_fs_nir(nir, &tex_options);

      nir_print_shader(nir, stdout);

      struct lima_fs_compiled_shader *so = rzalloc(NULL, struct lima_fs_compiled_shader);
      struct ra_regs *ra = ppir_regalloc_init(NULL);
      ppir_compile_nir(so, nir, ra, NULL);
      break;
   default:
      errx(1, "unhandled shader stage: %d", stage);
   }

   ralloc_free(nir);
   return 0;
}
