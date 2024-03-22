/*
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
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
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "nir/tgsi_to_nir.h"
#include "tgsi/tgsi_dump.h"
#include "tgsi/tgsi_text.h"

#include "ir3/instr-a3xx.h"
#include "ir3/ir3.h"
#include "ir3/ir3_compiler.h"
#include "ir3/ir3_gallium.h"
#include "ir3/ir3_nir.h"

#include "main/mtypes.h"

#include "compiler/glsl_types.h"
#include "compiler/glsl/gl_nir.h"
#include "compiler/glsl/glsl_to_nir.h"
#include "compiler/glsl/standalone.h"
#include "compiler/spirv/nir_spirv.h"

#include "pipe/p_context.h"

static void
dump_info(struct ir3_shader_variant *so, const char *str)
{
   uint32_t *bin;
   const char *type = ir3_shader_stage(so);
   bin = ir3_shader_assemble(so);
   printf("; %s: %s\n", type, str);
   ir3_shader_disasm(so, bin, stdout);
}

static void
insert_sorted(struct exec_list *var_list, nir_variable *new_var)
{
   nir_foreach_variable_in_list (var, var_list) {
      if (var->data.location > new_var->data.location) {
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
   nir_foreach_variable_with_modes_safe (var, nir, mode) {
      exec_node_remove(&var->node);
      insert_sorted(&new_list, var);
   }
   exec_list_append(&nir->variables, &new_list);
}

static void
fixup_varying_slots(nir_shader *nir, nir_variable_mode mode)
{
   nir_foreach_variable_with_modes (var, nir, mode) {
      if (var->data.location >= VARYING_SLOT_VAR0) {
         var->data.location += 9;
      } else if ((var->data.location >= VARYING_SLOT_TEX0) &&
                 (var->data.location <= VARYING_SLOT_TEX7)) {
         var->data.location += VARYING_SLOT_VAR0 - VARYING_SLOT_TEX0;
      }
   }
}

static struct ir3_compiler *compiler;

static nir_shader *
load_glsl(unsigned num_files, char *const *files, gl_shader_stage stage)
{
   static const struct standalone_options options = {
      .glsl_version = 310,
      .do_link = true,
      .lower_precision = true,
   };
   struct gl_shader_program *prog;
   const nir_shader_compiler_options *nir_options =
      ir3_get_compiler_options(compiler);
   static struct gl_context local_ctx;

   prog = standalone_compile_shader(&options, num_files, files, &local_ctx);
   if (!prog)
      errx(1, "couldn't parse `%s'", files[0]);

   nir_shader *nir = glsl_to_nir(&local_ctx.Const, prog, stage, nir_options);

   /* required NIR passes: */
   if (nir_options->lower_all_io_to_temps ||
       nir->info.stage == MESA_SHADER_VERTEX ||
       nir->info.stage == MESA_SHADER_GEOMETRY) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir), true, true);
   } else if (nir->info.stage == MESA_SHADER_FRAGMENT) {
      NIR_PASS_V(nir, nir_lower_io_to_temporaries,
                 nir_shader_get_entrypoint(nir), true, false);
   }

   NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);

   NIR_PASS_V(nir, nir_split_var_copies);
   NIR_PASS_V(nir, nir_lower_var_copies);
   nir_print_shader(nir, stdout);
   NIR_PASS_V(nir, gl_nir_lower_atomics, prog, true);
   NIR_PASS_V(nir, gl_nir_lower_buffers, prog);
   NIR_PASS_V(nir, nir_lower_atomics_to_ssbo, 0);
   nir_print_shader(nir, stdout);

   switch (stage) {
   case MESA_SHADER_VERTEX:
      nir_assign_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               ir3_glsl_type_size);

      /* Re-lower global vars, to deal with any dead VS inputs. */
      NIR_PASS_V(nir, nir_lower_global_vars_to_local);

      sort_varyings(nir, nir_var_shader_out);
      nir_assign_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               ir3_glsl_type_size);
      fixup_varying_slots(nir, nir_var_shader_out);
      break;
   case MESA_SHADER_FRAGMENT:
      sort_varyings(nir, nir_var_shader_in);
      nir_assign_var_locations(nir, nir_var_shader_in, &nir->num_inputs,
                               ir3_glsl_type_size);
      fixup_varying_slots(nir, nir_var_shader_in);
      nir_assign_var_locations(nir, nir_var_shader_out, &nir->num_outputs,
                               ir3_glsl_type_size);
      break;
   case MESA_SHADER_COMPUTE:
   case MESA_SHADER_KERNEL:
      break;
   default:
      errx(1, "unhandled shader stage: %d", stage);
   }

   nir_assign_var_locations(nir, nir_var_uniform, &nir->num_uniforms,
                            ir3_glsl_type_size);

   NIR_PASS_V(nir, nir_lower_system_values);
   NIR_PASS_V(nir, nir_lower_compute_system_values, NULL);

   NIR_PASS_V(nir, nir_lower_frexp);
   NIR_PASS_V(nir, nir_lower_io,
              nir_var_shader_in | nir_var_shader_out | nir_var_uniform,
              ir3_glsl_type_size, (nir_lower_io_options)0);
   NIR_PASS_V(nir, gl_nir_lower_samplers, prog);

   return nir;
}

static int
read_file(const char *filename, void **ptr, size_t *size)
{
   int fd, ret;
   struct stat st;

   *ptr = MAP_FAILED;

   fd = open(filename, O_RDONLY);
   if (fd == -1) {
      warnx("couldn't open `%s'", filename);
      return 1;
   }

   ret = fstat(fd, &st);
   if (ret)
      errx(1, "couldn't stat `%s'", filename);

   *size = st.st_size;
   *ptr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
   if (*ptr == MAP_FAILED)
      errx(1, "couldn't map `%s'", filename);

   close(fd);

   return 0;
}

static void
debug_func(void *priv, enum nir_spirv_debug_level level, size_t spirv_offset,
           const char *message)
{
   //	printf("%s\n", message);
}

static nir_shader *
load_spirv(const char *filename, const char *entry, gl_shader_stage stage)
{
   const struct spirv_to_nir_options spirv_options = {
      /* these caps are just make-believe */
      .caps = {
         .draw_parameters = true,
         .float64 = true,
         .image_read_without_format = true,
         .image_write_without_format = true,
         .int64 = true,
         .variable_pointers = true,
      },
      .debug = {
         .func = debug_func,
      }
   };
   nir_shader *nir;
   void *buf;
   size_t size;

   read_file(filename, &buf, &size);

   nir = spirv_to_nir(buf, size / 4, NULL, 0, /* spec_entries */
                      stage, entry, &spirv_options,
                      ir3_get_compiler_options(compiler));

   const struct nir_lower_sysvals_to_varyings_options sysvals_to_varyings = {
      .frag_coord = true,
      .point_coord = true,
   };
   NIR_PASS_V(nir, nir_lower_sysvals_to_varyings, &sysvals_to_varyings);

   nir_print_shader(nir, stdout);

   return nir;
}

static const char *shortopts = "g:hv";

static const struct option longopts[] = {
   {"gpu",     required_argument, 0, 'g'},
   {"help",    no_argument,       0, 'h'},
   {"verbose", no_argument,       0, 'v'},
};

static void
print_usage(void)
{
   printf("Usage: ir3_compiler [OPTIONS]... <file.tgsi | file.spv entry_point "
          "| (file.vert | file.frag)*>\n");
   printf("    -g, --gpu GPU_ID - specify gpu-id (default 320)\n");
   printf("    -h, --help       - show this message\n");
   printf("    -v, --verbose    - verbose compiler/debug messages\n");
}

int
main(int argc, char **argv)
{
   int ret = 0, opt;
   char *filenames[2];
   int num_files = 0;
   unsigned stage = 0;
   struct ir3_shader_key key = {};
   unsigned gpu_id = 320;
   const char *info;
   const char *spirv_entry = NULL;
   void *ptr;
   bool from_tgsi = false;
   size_t size;

   while ((opt = getopt_long_only(argc, argv, shortopts, longopts, NULL)) !=
          -1) {
      switch (opt) {
      case 'g':
         gpu_id = strtol(optarg, NULL, 0);
         break;
      case 'v':
         ir3_shader_debug |= IR3_DBG_OPTMSGS | IR3_DBG_DISASM;
         break;
      default:
         printf("unrecognized arg: %c\n", opt);
         FALLTHROUGH;
      case 'h':
         print_usage();
         return 0;
      }
   }

   if (optind >= argc) {
      fprintf(stderr, "no file specified!\n");
      print_usage();
      return 0;
   }

   unsigned n = optind;
   while (n < argc) {
      char *filename = argv[n];
      char *ext = strrchr(filename, '.');

      if (strcmp(ext, ".tgsi") == 0) {
         if (num_files != 0)
            errx(1, "in TGSI mode, only a single file may be specified");
         from_tgsi = true;
      } else if (strcmp(ext, ".spv") == 0) {
         if (num_files != 0)
            errx(1, "in SPIR-V mode, only a single file may be specified");
         stage = MESA_SHADER_COMPUTE;
         filenames[num_files++] = filename;
         n++;
         if (n == argc)
            errx(1, "in SPIR-V mode, an entry point must be specified");
         spirv_entry = argv[n];
         n++;
      } else if (strcmp(ext, ".comp") == 0) {
         if (from_tgsi || spirv_entry)
            errx(1, "cannot mix GLSL/TGSI/SPIRV");
         if (num_files >= ARRAY_SIZE(filenames))
            errx(1, "too many GLSL files");
         stage = MESA_SHADER_COMPUTE;
      } else if (strcmp(ext, ".frag") == 0) {
         if (from_tgsi || spirv_entry)
            errx(1, "cannot mix GLSL/TGSI/SPIRV");
         if (num_files >= ARRAY_SIZE(filenames))
            errx(1, "too many GLSL files");
         stage = MESA_SHADER_FRAGMENT;
      } else if (strcmp(ext, ".vert") == 0) {
         if (from_tgsi)
            errx(1, "cannot mix GLSL and TGSI");
         if (num_files >= ARRAY_SIZE(filenames))
            errx(1, "too many GLSL files");
         stage = MESA_SHADER_VERTEX;
      } else {
         print_usage();
         return -1;
      }

      filenames[num_files++] = filename;

      n++;
   }

   nir_shader *nir;

   struct fd_dev_id dev_id = {
         .gpu_id = gpu_id,
   };
   compiler = ir3_compiler_create(NULL, &dev_id, fd_dev_info_raw(&dev_id),
                                  &(struct ir3_compiler_options) {});

   if (from_tgsi) {
      struct tgsi_token toks[65536];
      const nir_shader_compiler_options *nir_options =
         ir3_get_compiler_options(compiler);

      ret = read_file(filenames[0], &ptr, &size);
      if (ret) {
         print_usage();
         return ret;
      }

      if (ir3_shader_debug & IR3_DBG_OPTMSGS)
         printf("%s\n", (char *)ptr);

      if (!tgsi_text_translate(ptr, toks, ARRAY_SIZE(toks)))
         errx(1, "could not parse `%s'", filenames[0]);

      if (ir3_shader_debug & IR3_DBG_OPTMSGS)
         tgsi_dump(toks, 0);

      nir = tgsi_to_nir_noscreen(toks, nir_options);
      NIR_PASS_V(nir, nir_lower_global_vars_to_local);
   } else if (spirv_entry) {
      nir = load_spirv(filenames[0], spirv_entry, stage);

      NIR_PASS_V(nir, nir_lower_io, nir_var_shader_in | nir_var_shader_out,
                 ir3_glsl_type_size, (nir_lower_io_options)0);

      /* TODO do this somewhere else */
      nir_lower_int64(nir);
      nir_lower_system_values(nir);
      nir_lower_compute_system_values(nir, NULL);
   } else if (num_files > 0) {
      nir = load_glsl(num_files, filenames, stage);
   } else {
      print_usage();
      return -1;
   }

   ir3_nir_lower_io_to_temporaries(nir);
   ir3_finalize_nir(compiler, nir);

   struct ir3_shader *shader = rzalloc_size(NULL, sizeof(*shader));
   shader->compiler = compiler;
   shader->type = stage;
   shader->nir = nir;

   ir3_nir_post_finalize(shader);

   struct ir3_shader_variant *v = rzalloc_size(shader, sizeof(*v));
   v->type = shader->type;
   v->compiler = compiler;
   v->key = key;
   v->const_state = rzalloc_size(v, sizeof(*v->const_state));

   shader->variants = v;
   shader->variant_count = 1;

   ir3_nir_lower_variant(v, nir);

   info = "NIR compiler";
   ret = ir3_compile_shader_nir(compiler, shader, v);
   if (ret) {
      fprintf(stderr, "compiler failed!\n");
      return ret;
   }
   dump_info(v, info);

   return 0;
}
