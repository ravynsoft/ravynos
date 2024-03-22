/*
 * Copyright © 2015 Intel Corporation
 * Copyright © Microsoft Corporation
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
 * A simple executable that opens a SPIR-V shader, converts it to DXIL via
 * NIR, and dumps out the result.  This should be useful for testing the
 * nir_to_dxil code.  Based on spirv2nir.c.
 */

#include "nir_to_dxil.h"
#include "dxil_validator.h"
#include "spirv/nir_spirv.h"
#include "spirv_to_dxil.h"
#include "dxil_spirv_nir.h"

#include "util/os_file.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#define WORD_SIZE 4

static gl_shader_stage
stage_to_enum(char *stage)
{
   if (!strcmp(stage, "vertex"))
      return MESA_SHADER_VERTEX;
   else if (!strcmp(stage, "tess-ctrl"))
      return MESA_SHADER_TESS_CTRL;
   else if (!strcmp(stage, "tess-eval"))
      return MESA_SHADER_TESS_EVAL;
   else if (!strcmp(stage, "geometry"))
      return MESA_SHADER_GEOMETRY;
   else if (!strcmp(stage, "fragment"))
      return MESA_SHADER_FRAGMENT;
   else if (!strcmp(stage, "compute"))
      return MESA_SHADER_COMPUTE;
   else
      return MESA_SHADER_NONE;
}

static void
log_spirv_to_dxil_error(void *priv, const char *msg)
{
   fprintf(stderr, "spirv_to_dxil error: %s", msg);
}

struct shader {
   const char *entry_point;
   const char *output_file;
   nir_shader *nir;
};

bool validate = false, debug = false;
enum dxil_validator_version val_ver = DXIL_VALIDATOR_1_4;

struct nir_shader_compiler_options nir_options;

static bool
compile_shader(const char *filename, gl_shader_stage shader_stage, struct shader *shader,
               struct dxil_spirv_runtime_conf *conf)
{
   size_t file_size;
   char *file_contents = os_read_file(filename, &file_size);
   if (!file_contents) {
      fprintf(stderr, "Failed to open %s\n", filename);
      return false;
   }

   if (file_size % WORD_SIZE != 0) {
      fprintf(stderr, "%s size == %zu is not a multiple of %d\n", filename,
              file_size, WORD_SIZE);
      free(file_contents);
      return false;
   }

   size_t word_count = file_size / WORD_SIZE;

   const struct spirv_to_nir_options *spirv_opts = dxil_spirv_nir_get_spirv_options();

   shader->nir = spirv_to_nir(
      (const uint32_t *)file_contents, word_count, NULL,
      0, (gl_shader_stage)shader_stage, shader->entry_point,
      spirv_opts, &nir_options);
   free(file_contents);
   if (!shader->nir) {
      fprintf(stderr, "SPIR-V to NIR failed\n");
      return false;
   }

   nir_validate_shader(shader->nir,
                       "Validate before feeding NIR to the DXIL compiler");

   dxil_spirv_nir_prep(shader->nir);

   bool requires_runtime_data;
   dxil_spirv_nir_passes(shader->nir, conf, &requires_runtime_data);

   if (debug)
      nir_print_shader(shader->nir, stderr);

   return true;
}

#if DETECT_OS_WINDOWS

static bool
validate_dxil(struct blob *blob)
{
   struct dxil_validator *val = dxil_create_validator(NULL);

   char *err;
   bool res = dxil_validate_module(val, blob->data,
                                   blob->size, &err);
   if (!res && err)
      fprintf(stderr, "DXIL: %s\n\n", err);

   dxil_destroy_validator(val);
   return res;
}

#else

static bool
validate_dxil(struct blob *blob)
{
   fprintf(stderr, "DXIL validation only available in Windows.\n");
   return false;
}

#endif

int
main(int argc, char **argv)
{
   glsl_type_singleton_init_or_ref();
   int ch;

   static struct option long_options[] = {
      {"stage", required_argument, 0, 's'},
      {"entry", required_argument, 0, 'e'},
      {"output", required_argument, 0, 'o'},
      {"validate", no_argument, 0, 'v'},
      {"debug", no_argument, 0, 'd'},
      {"shadermodel", required_argument, 0, 'm'},
      {"validatorver", required_argument, 0, 'x'},
      {0, 0, 0, 0}};

   struct shader shaders[MESA_SHADER_COMPUTE + 1];
   memset(shaders, 0, sizeof(shaders));
   struct shader cur_shader = {
      .entry_point = "main",
      .output_file = "",
   };
   gl_shader_stage shader_stage = MESA_SHADER_FRAGMENT;

   struct dxil_spirv_runtime_conf conf;
   memset(&conf, 0, sizeof(conf));
   conf.runtime_data_cbv.base_shader_register = 0;
   conf.runtime_data_cbv.register_space = 31;
   conf.zero_based_vertex_instance_id = true;
   conf.declared_read_only_images_as_srvs = true;
   conf.shader_model_max = SHADER_MODEL_6_2;

   bool any_shaders = false;
   while ((ch = getopt_long(argc, argv, "-s:e:o:m:x:vd", long_options, NULL)) !=
            -1) {
      switch (ch)
      {
      case 's':
         shader_stage = stage_to_enum(optarg);
         if (shader_stage == MESA_SHADER_NONE) {
            fprintf(stderr, "Unknown stage %s\n", optarg);
            return 1;
         }
         break;
      case 'e':
         cur_shader.entry_point = optarg;
         break;
      case 'o':
         cur_shader.output_file = optarg;
         break;
      case 'v':
         validate = true;
         break;
      case 'd':
         debug = true;
         break;
      case 'm':
         conf.shader_model_max = SHADER_MODEL_6_0 + atoi(optarg);
         break;
      case 'x':
         val_ver = DXIL_VALIDATOR_1_0 + atoi(optarg);
         break;
      case 1:
         if (!compile_shader(optarg, shader_stage, &cur_shader, &conf))
            return 1;
         shaders[shader_stage] = cur_shader;
         any_shaders = true;
         break;
      default:
         fprintf(stderr, "Unrecognized option.\n");
         return 1;
      }
   }

   const unsigned supported_bit_sizes = 16 | 32 | 64;
   dxil_get_nir_compiler_options(&nir_options, conf.shader_model_max, supported_bit_sizes, supported_bit_sizes);
   // We will manually handle base_vertex when vertex_id and instance_id have
   // have been already converted to zero-base.
   nir_options.lower_base_vertex = false;

   if (!any_shaders) {
      fprintf(stderr, "Specify a shader filename\n");
      return 1;
   }

   for (int32_t cur = MESA_SHADER_FRAGMENT; cur >= MESA_SHADER_VERTEX; --cur) {
      if (!shaders[cur].nir)
         continue;
      for (int32_t prev = cur - 1; prev >= MESA_SHADER_VERTEX; --prev) {
         if (!shaders[prev].nir)
            continue;
         bool requires_runtime_data;
         dxil_spirv_nir_link(shaders[cur].nir, shaders[prev].nir, &conf, &requires_runtime_data);
         break;
      }
   }

   struct nir_to_dxil_options opts = {
      .environment = DXIL_ENVIRONMENT_VULKAN,
      .shader_model_max = conf.shader_model_max,
      .validator_version_max = val_ver,
   };

   struct dxil_logger logger_inner = {.priv = NULL,
                                      .log = log_spirv_to_dxil_error};

   for (uint32_t i = 0; i <= MESA_SHADER_COMPUTE; ++i) {
      if (!shaders[i].nir)
         continue;
      struct blob dxil_blob;
      bool success = nir_to_dxil(shaders[i].nir, &opts, &logger_inner, &dxil_blob);
      ralloc_free(shaders[i].nir);

      if (!success) {
         fprintf(stderr, "Failed to convert to DXIL\n");
         if (dxil_blob.allocated)
            blob_finish(&dxil_blob);
         return false;
      }

      if (validate && !validate_dxil(&dxil_blob)) {
         fprintf(stderr, "Failed to validate DXIL\n");
         blob_finish(&dxil_blob);
         return 1;
      }

      if (shaders[i].output_file) {
         FILE *file = fopen(shaders[i].output_file, "wb");
         if (!file) {
            fprintf(stderr, "Failed to open %s, %s\n", shaders[i].output_file,
                    strerror(errno));
            blob_finish(&dxil_blob);
            return 1;
         }

         fwrite(dxil_blob.data, sizeof(char), dxil_blob.size, file);
         fclose(file);
         blob_finish(&dxil_blob);
      }
   }

   glsl_type_singleton_decref();

   return 0;
}
