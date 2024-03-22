/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "compiler/shader_enums.h"
#include "nir/nir.h"
#include "rogue.h"
#include "util/macros.h"
#include "util/os_file.h"
#include "util/ralloc.h"
#include "util/u_dynarray.h"

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Number of hex columns to print before starting a new line. */
#define ARRAY_PRINT_COLS 16

/**
 * \file vk_compiler.c
 *
 * \brief Rogue offline Vulkan shader compiler.
 */

static const struct option cmdline_opts[] = {
   /* Arguments. */
   { "stage", required_argument, NULL, 's' },
   { "file", required_argument, NULL, 'f' },
   { "entry", required_argument, NULL, 'e' },

   /* Options. */
   { "help", no_argument, NULL, 'h' },
   { "out", required_argument, NULL, 'o' },

   { NULL, 0, NULL, 0 },
};

typedef struct compiler_opts {
   gl_shader_stage stage;
   char *file;
   char *entry;
   char *out_file;
} compiler_opts;

static void usage(const char *argv0)
{
   /* clang-format off */
   printf("Rogue offline Vulkan shader compiler.\n");
   printf("Usage: %s -s <stage> -f <file> [-e <entry>] [-o <file>] [-h]\n", argv0);
   printf("\n");

   printf("Required arguments:\n");
   printf("\t-s, --stage <stage> Shader stage (supported options: frag, vert).\n");
   printf("\t-f, --file <file>   Shader SPIR-V filename.\n");
   printf("\n");

   printf("Options:\n");
   printf("\t-h, --help          Prints this help message.\n");
   printf("\t-e, --entry <entry> Overrides the shader entry-point name (default: 'main').\n");
   printf("\t-o, --out <file>    Overrides the output filename (default: 'out.bin').\n");
   printf("\n");
   /* clang-format on */
}

static bool parse_cmdline(int argc, char *argv[], struct compiler_opts *opts)
{
   int opt;
   int longindex;

   while (
      (opt = getopt_long(argc, argv, "hs:f:e:o:", cmdline_opts, &longindex)) !=
      -1) {
      switch (opt) {
      case 'e':
         if (opts->entry)
            continue;

         opts->entry = optarg;
         break;

      case 'f':
         if (opts->file)
            continue;

         opts->file = optarg;
         break;

      case 'o':
         if (opts->out_file)
            continue;

         opts->out_file = optarg;
         break;

      case 's':
         if (opts->stage != MESA_SHADER_NONE)
            continue;

         if (!strcmp(optarg, "frag") || !strcmp(optarg, "f"))
            opts->stage = MESA_SHADER_FRAGMENT;
         else if (!strcmp(optarg, "vert") || !strcmp(optarg, "v"))
            opts->stage = MESA_SHADER_VERTEX;
         else {
            fprintf(stderr, "Unsupported stage \"%s\".\n", optarg);
            usage(argv[0]);
            return false;
         }

         break;

      case 'h':
      default:
         usage(argv[0]);
         return false;
      }
   }

   if (opts->stage == MESA_SHADER_NONE || !opts->file) {
      fprintf(stderr,
              "%s: --stage and --file are required arguments.\n",
              argv[0]);
      usage(argv[0]);
      return false;
   }

   if (!opts->out_file)
      opts->out_file = "out.bin";

   if (!opts->entry)
      opts->entry = "main";

   return true;
}

int main(int argc, char *argv[])
{
   /* Command-line options. */
   /* N.B. MESA_SHADER_NONE != 0 */
   compiler_opts opts = { .stage = MESA_SHADER_NONE, 0 };

   /* Input file data. */
   char *input_data;
   size_t input_size;

   /* Compiler context. */
   struct rogue_compiler *compiler;

   /* Multi-stage build context. */
   struct rogue_build_ctx *ctx;

   /* Output file. */
   FILE *fp;
   size_t bytes_written;

   /* Parse command-line options. */
   if (!parse_cmdline(argc, argv, &opts))
      return 1;

   /* Load SPIR-V input file. */
   input_data = os_read_file(opts.file, &input_size);
   if (!input_data) {
      fprintf(stderr, "Failed to read file \"%s\".\n", opts.file);
      return 1;
   }

   /* Create compiler context. */
   compiler = rogue_compiler_create(NULL);
   if (!compiler) {
      fprintf(stderr, "Failed to set up compiler context.\n");
      goto err_free_input;
   }

   /* Create build context. */
   ctx = rogue_build_context_create(compiler, NULL);
   if (!ctx) {
      fprintf(stderr, "Failed to set up build context.\n");
      goto err_destroy_compiler;
   }

   /* SPIR-V -> NIR. */
   ctx->nir[opts.stage] = rogue_spirv_to_nir(ctx,
                                             opts.stage,
                                             opts.entry,
                                             input_size / sizeof(uint32_t),
                                             (uint32_t *)input_data,
                                             0,
                                             NULL);
   if (!ctx->nir[opts.stage]) {
      fprintf(stderr, "Failed to translate SPIR-V input to NIR.\n");
      goto err_free_build_context;
   }

   /* NIR -> Rogue. */
   ctx->rogue[opts.stage] = rogue_nir_to_rogue(ctx, ctx->nir[opts.stage]);
   if (!ctx->rogue[opts.stage]) {
      fprintf(stderr, "Failed to translate NIR input to Rogue.\n");
      goto err_free_build_context;
   }

   rogue_encode_shader(ctx, ctx->rogue[opts.stage], &ctx->binary[opts.stage]);

   /* Write shader binary to disk. */
   fp = fopen(opts.out_file, "wb");
   if (!fp) {
      fprintf(stderr, "Failed to open output file \"%s\".\n", opts.out_file);
      goto err_free_build_context;
   }

   bytes_written = fwrite(util_dynarray_begin(&ctx->binary[opts.stage]),
                          1,
                          ctx->binary[opts.stage].size,
                          fp);
   if (bytes_written != ctx->binary[opts.stage].size) {
      fprintf(
         stderr,
         "Failed to write to output file \"%s\" (%zu bytes of %u written).\n",
         opts.out_file,
         bytes_written,
         ctx->binary[opts.stage].size);
      goto err_close_outfile;
   }

   /* Clean up. */
   fclose(fp);
   ralloc_free(ctx);
   ralloc_free(compiler);
   free(input_data);

   return 0;

err_close_outfile:
   fclose(fp);
err_free_build_context:
   ralloc_free(ctx);
err_destroy_compiler:
   ralloc_free(compiler);
err_free_input:
   free(input_data);

   return 1;
}
