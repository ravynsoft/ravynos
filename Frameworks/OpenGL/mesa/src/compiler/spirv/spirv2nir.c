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

/*
 * A simple executable that opens a SPIR-V shader, converts it to NIR, and
 * dumps out the result.  This should be useful for testing the
 * spirv_to_nir code.
 */

#include "nir.h"
#include "nir_spirv.h"
#include "spirv.h"
#include "util/u_dynarray.h"
#include "vtn_private.h"

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

#define WORD_SIZE 4

struct {
   const char *name;
   gl_shader_stage stage;
} abbrev_stage_table[] = {
   { "vs",    MESA_SHADER_VERTEX },
   { "tcs",   MESA_SHADER_TESS_CTRL },
   { "tes",   MESA_SHADER_TESS_EVAL },
   { "gs",    MESA_SHADER_GEOMETRY },
   { "fs",    MESA_SHADER_FRAGMENT },
   { "cs",    MESA_SHADER_COMPUTE },
   { "cl",    MESA_SHADER_KERNEL },
   { "task",  MESA_SHADER_TASK },
   { "mesh",  MESA_SHADER_MESH },

   /* Keep previously used shader names working. */
   { "vertex",    MESA_SHADER_VERTEX },
   { "tess-ctrl", MESA_SHADER_TESS_CTRL },
   { "tess-eval", MESA_SHADER_TESS_EVAL },
   { "geometry",  MESA_SHADER_GEOMETRY },
   { "fragment",  MESA_SHADER_FRAGMENT },
   { "compute",   MESA_SHADER_COMPUTE },
   { "kernel",    MESA_SHADER_KERNEL },
};

static gl_shader_stage
abbrev_to_stage(const char *name)
{
   for (unsigned i = 0; i < ARRAY_SIZE(abbrev_stage_table); i++) {
      if (!strcasecmp(abbrev_stage_table[i].name, name))
         return abbrev_stage_table[i].stage;
   }
   return MESA_SHADER_NONE;
}

static const char *
stage_to_abbrev(gl_shader_stage stage)
{
   for (unsigned i = 0; i < ARRAY_SIZE(abbrev_stage_table); i++) {
      if (abbrev_stage_table[i].stage == stage)
         return abbrev_stage_table[i].name;
   }
   return "UNKNOWN";
}

static void
print_usage(char *exec_name, FILE *f)
{
   fprintf(f,
           "Usage: %s [options] file\n"
           "Options:\n"
           "  -h  --help              Print this help.\n"
           "  -s, --stage <stage>     Specify the shader stage.  Valid stages are:\n"
           "                          vs, tcs, tes, gs, fs, cs, cl (OpenCL-style compute),\n"
           "                          task and mesh.  Case insensitive.\n"
           "  -e, --entry <name>      Specify the entry-point name.\n"
           "  -g, --opengl            Use OpenGL environment instead of Vulkan for\n"
           "                          graphics stages.\n"
           "  --optimize              Run basic NIR optimizations in the result.\n"
           "\n"
           "Passing the stage and the entry-point name is optional unless there's\n"
           "ambiguity, in which case the program will print the entry-points\n"
           "available.",
           exec_name);
}

struct entry_point {
   const char *name;
   gl_shader_stage stage;
};

static struct entry_point
select_entry_point(void *mem_ctx, const uint32_t *words, size_t word_count,
                  struct entry_point args)
{
   /* Create a dummy vtn_builder to use with vtn_string_literal. */
   struct vtn_builder *b = rzalloc(mem_ctx, struct vtn_builder);

   struct util_dynarray candidates;
   util_dynarray_init(&candidates, mem_ctx);

   /* Skip header. */
   const uint32_t *w = words + 5;
   const uint32_t *end = words + word_count;

   bool seen_entry_point = false;
   while (w < end) {
      SpvOp opcode = w[0] & SpvOpCodeMask;
      unsigned count = w[0] >> SpvWordCountShift;
      assert(count >= 1 && w + count <= end);

      if (opcode == SpvOpEntryPoint) {
         seen_entry_point = true;

         unsigned name_words;
         const char *name = vtn_string_literal(b, &w[3], count - 3, &name_words);
         gl_shader_stage stage = vtn_stage_for_execution_model(w[1]);

         struct entry_point e = { name, stage };
         util_dynarray_append(&candidates, struct entry_point, e);
      } else if (seen_entry_point) {
         /* List of entry_points is over, we can break now. */
         break;
      }

      w += count;
   }

   struct entry_point r = {0};
   if (util_dynarray_num_elements(&candidates, struct entry_point) == 0) {
      fprintf(stderr, "ERROR: No entry-points available.\n");
      return r;
   }

   int matches = 0;
   util_dynarray_foreach(&candidates, struct entry_point, e) {
      if ((!args.name || !strcmp(args.name, e->name)) &&
          (args.stage == MESA_SHADER_NONE || args.stage == e->stage)) {
         if (matches == 0) {
            /* Save the first match we found. */
            r = *e;
         }
         matches++;
      }
   }

   if (matches != 1) {
      if (matches == 0) {
         fprintf(stderr, "No matching entry-point for arguments passed.\n");
      } else {
         fprintf(stderr, "Multiple entry-points available, select with --stage and/or --entry.\n");

         /* Discard whatever we found before. */
         r.name = NULL;
         r.stage = MESA_SHADER_NONE;
      }

      fprintf(stderr, "Entry-points available:\n");
      util_dynarray_foreach(&candidates, struct entry_point, e)
         fprintf(stderr, "  --entry e \"%s\" --stage %s\n", e->name, stage_to_abbrev(e->stage));
   }

   return r;
}

int main(int argc, char **argv)
{
   struct entry_point entry_point = {
      .name = NULL,
      .stage = MESA_SHADER_NONE,
   };
   int ch;
   bool optimize = false;
   enum nir_spirv_execution_environment env = NIR_SPIRV_VULKAN;

   static struct option long_options[] =
      {
         {"help",     no_argument,       0, 'h'},
         {"stage",    required_argument, 0, 's'},
         {"entry",    required_argument, 0, 'e'},
         {"opengl",   no_argument,       0, 'g'},
         {"optimize", no_argument,       0, 'O'},
         {0, 0,                          0, 0}
      };

   while ((ch = getopt_long(argc, argv, "hs:e:g", long_options, NULL)) != -1) {
      switch (ch) {
      case 'h':
         print_usage(argv[0], stdout);
         return 0;
      case 's': {
         gl_shader_stage s = abbrev_to_stage(optarg);
         if (s == MESA_SHADER_NONE) {
            fprintf(stderr, "Unknown stage \"%s\"\n", optarg);
            print_usage(argv[0], stderr);
            return 1;
         }
         entry_point.stage = s;
         break;
      }
      case 'e':
         entry_point.name = optarg;
         break;
      case 'g':
         env = NIR_SPIRV_OPENGL;
         break;
      case 'O':
         optimize = true;
         break;
      default:
         fprintf(stderr, "Unrecognized option \"%s\".\n", optarg);
         print_usage(argv[0], stderr);
         return 1;
      }
   }

   const char *filename = argv[optind];
   int fd = open(filename, O_RDONLY);
   if (fd < 0) {
      fprintf(stderr, "Failed to open %s\n", filename);
      return 1;
   }

   off_t len = lseek(fd, 0, SEEK_END);
   if (len % WORD_SIZE != 0) {
      fprintf(stderr, "File length isn't a multiple of the word size\n");
      fprintf(stderr, "Are you sure this is a valid SPIR-V shader?\n");
      close(fd);
      return 1;
   }

   size_t word_count = len / WORD_SIZE;

   const void *map = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
   if (map == MAP_FAILED) {
      fprintf(stderr, "Failed to mmap the file: errno=%d, %s\n",
              errno, strerror(errno));
      close(fd);
      return 1;
   }

   const uint32_t *words = map;
   if (words[0] != SpvMagicNumber) {
      fprintf(stderr, "ERROR: Not a SPIR-V file. First word was 0x%x, want 0x%x (SPIR-V magic).",
              words[0], SpvMagicNumber);
      return 1;
   }

   void *mem_ctx = ralloc_context(NULL);

   entry_point = select_entry_point(mem_ctx, map, word_count, entry_point);
   if (!entry_point.name)
      return 1;

   glsl_type_singleton_init_or_ref();

   struct nir_shader_compiler_options nir_opts = {0};

   struct spirv_to_nir_options spirv_opts = {
      .environment = env,
   };

   if (entry_point.stage == MESA_SHADER_KERNEL) {
      spirv_opts.environment = NIR_SPIRV_OPENCL;
      spirv_opts.caps.address = true;
      spirv_opts.caps.float64 = true;
      spirv_opts.caps.int8 = true;
      spirv_opts.caps.int16 = true;
      spirv_opts.caps.int64 = true;
      spirv_opts.caps.kernel = true;
   }

   nir_shader *nir = spirv_to_nir(map, word_count, NULL, 0,
                                  entry_point.stage, entry_point.name,
                                  &spirv_opts, &nir_opts);

   if (nir) {
      if (optimize) {
         bool progress;
         do {
            progress = false;

            #define OPT(pass, ...) ({                                  \
               bool this_progress = false;                             \
               NIR_PASS(this_progress, nir, pass, ##__VA_ARGS__);      \
               if (this_progress)                                      \
                  progress = true;                                     \
               this_progress;                                          \
            })

            OPT(nir_opt_dce);
            OPT(nir_opt_cse);
            OPT(nir_opt_dead_cf);
            OPT(nir_lower_vars_to_ssa);
            OPT(nir_copy_prop);
            OPT(nir_opt_deref);
            OPT(nir_opt_constant_folding);
            OPT(nir_opt_copy_prop_vars);
            OPT(nir_opt_dead_write_vars);
            OPT(nir_opt_combine_stores, nir_var_all);
            OPT(nir_remove_dead_variables, nir_var_function_temp, NULL);
            OPT(nir_opt_algebraic);
            OPT(nir_opt_if, 0);
            OPT(nir_opt_loop_unroll);

            #undef OPT
         } while (progress);
      }
      nir_print_shader(nir, stdout);
   } else {
      fprintf(stderr, "SPIRV to NIR compilation failed\n");
   }

   glsl_type_singleton_decref();

   ralloc_free(mem_ctx);

   return 0;
}
