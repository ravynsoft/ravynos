/*
 * Copyright 2023 Alyssa Rosenzweig
 * Copyright 2020 Intel Corporation
 * SPDX-License-Identifier: MIT
 */

#include "asahi/compiler/agx_compile.h"
#include "compiler/clc/clc.h"
#include "compiler/glsl_types.h"
#include "compiler/spirv/nir_spirv.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/macros.h"
#include "util/mesa-sha1.h"
#include "util/u_dynarray.h"
#include "nir.h"
#include "nir_builder.h"
#include "nir_serialize.h"

#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

struct spirv_to_nir_options spirv_options = {
   .environment = NIR_SPIRV_OPENCL,
   .caps =
      {
         .address = true,
         .float16 = true,
         .float64 = true,
         .groups = true,
         .image_write_without_format = true,
         .int8 = true,
         .int16 = true,
         .int64 = true,
         .int64_atomics = false,
         .kernel = true,
         .linkage = true,
         .float_controls = true,
         .generic_pointers = true,
         .storage_8bit = true,
         .storage_16bit = true,
         .subgroup_arithmetic = true,
         .subgroup_basic = true,
         .subgroup_ballot = true,
         .subgroup_dispatch = true,
         .subgroup_quad = true,
         .subgroup_shuffle = true,
         .subgroup_vote = true,

         .intel_subgroup_shuffle = true,
         .intel_subgroup_buffer_block_io = true,
      },
   .shared_addr_format = nir_address_format_62bit_generic,
   .global_addr_format = nir_address_format_62bit_generic,
   .temp_addr_format = nir_address_format_62bit_generic,
   .constant_addr_format = nir_address_format_64bit_global,
   .create_library = true,
};

static bool
lower_builtins(nir_builder *b, nir_instr *instr, void *data)
{
   if (instr->type != nir_instr_type_call)
      return false;

   nir_call_instr *call = nir_instr_as_call(instr);
   nir_function *func = call->callee;

   if (strcmp(func->name, "nir_interleave_agx") == 0) {
      b->cursor = nir_instr_remove(&call->instr);
      nir_store_deref(
         b, nir_src_as_deref(call->params[0]),
         nir_interleave_agx(b, call->params[1].ssa, call->params[2].ssa), 1);

      return true;
   } else if (strcmp(func->name, "nir_doorbell_agx") == 0) {
      b->cursor = nir_instr_remove(&call->instr);
      nir_doorbell_agx(b, call->params[0].ssa);
      return true;
   } else if (strcmp(func->name, "nir_stack_map_agx") == 0) {
      b->cursor = nir_instr_remove(&call->instr);
      nir_stack_map_agx(b, call->params[0].ssa, call->params[1].ssa);
      return true;
   } else if (strcmp(func->name, "nir_stack_unmap_agx") == 0) {
      b->cursor = nir_instr_remove(&call->instr);
      nir_store_deref(b, nir_src_as_deref(call->params[0]),
                      nir_stack_unmap_agx(b, call->params[1].ssa), 1);
      return true;
   }

   return false;
}

/* Standard optimization loop */
static void
optimize(nir_shader *nir)
{
   bool progress;
   do {
      progress = false;

      NIR_PASS(progress, nir, nir_lower_var_copies);
      NIR_PASS(progress, nir, nir_lower_vars_to_ssa);

      NIR_PASS(progress, nir, nir_copy_prop);
      NIR_PASS(progress, nir, nir_opt_remove_phis);
      NIR_PASS(progress, nir, nir_lower_phis_to_scalar, true);
      NIR_PASS(progress, nir, nir_opt_dce);
      NIR_PASS(progress, nir, nir_opt_dead_cf);
      NIR_PASS(progress, nir, nir_opt_cse);
      NIR_PASS(progress, nir, nir_opt_peephole_select, 64, false, true);
      NIR_PASS(progress, nir, nir_opt_phi_precision);
      NIR_PASS(progress, nir, nir_opt_algebraic);
      NIR_PASS(progress, nir, nir_opt_constant_folding);

      NIR_PASS(progress, nir, nir_opt_deref);
      NIR_PASS(progress, nir, nir_opt_copy_prop_vars);
      NIR_PASS(progress, nir, nir_opt_undef);
      NIR_PASS(progress, nir, nir_lower_undef_to_zero);

      NIR_PASS(progress, nir, nir_opt_shrink_vectors);
      NIR_PASS(progress, nir, nir_opt_loop_unroll);

      NIR_PASS(progress, nir, nir_split_var_copies);
      NIR_PASS(progress, nir, nir_split_struct_vars, nir_var_function_temp);
   } while (progress);
}

static nir_shader *
compile(void *memctx, const uint32_t *spirv, size_t spirv_size)
{
   const nir_shader_compiler_options *nir_options = &agx_nir_options;

   assert(spirv_size % 4 == 0);
   nir_shader *nir =
      spirv_to_nir(spirv, spirv_size / 4, NULL, 0, MESA_SHADER_KERNEL,
                   "library", &spirv_options, nir_options);
   nir_validate_shader(nir, "after spirv_to_nir");
   nir_validate_ssa_dominance(nir, "after spirv_to_nir");
   ralloc_steal(memctx, nir);

   NIR_PASS_V(nir, nir_lower_system_values);
   nir_shader_instructions_pass(nir, lower_builtins, nir_metadata_none, NULL);

   /* We have to lower away local constant initializers right before we
    * inline functions.  That way they get properly initialized at the top
    * of the function and not at the top of its caller.
    */
   NIR_PASS_V(nir, nir_lower_variable_initializers, nir_var_function_temp);
   NIR_PASS_V(nir, nir_lower_returns);
   NIR_PASS_V(nir, nir_inline_functions);
   NIR_PASS_V(nir, nir_remove_non_exported);
   NIR_PASS_V(nir, nir_copy_prop);
   NIR_PASS_V(nir, nir_opt_deref);

   /* We can go ahead and lower the rest of the constant initializers.  We do
    * this here so that nir_remove_dead_variables and split_per_member_structs
    * below see the corresponding stores.
    */
   NIR_PASS_V(nir, nir_lower_variable_initializers, ~0);

   /* LLVM loves take advantage of the fact that vec3s in OpenCL are 16B
    * aligned and so it can just read/write them as vec4s.  This results in a
    * LOT of vec4->vec3 casts on loads and stores.  One solution to this
    * problem is to get rid of all vec3 variables.
    */
   NIR_PASS_V(nir, nir_lower_vec3_to_vec4,
              nir_var_shader_temp | nir_var_function_temp | nir_var_mem_shared |
                 nir_var_mem_global | nir_var_mem_constant);

   /* We assign explicit types early so that the optimizer can take advantage
    * of that information and hopefully get rid of some of our memcpys.
    */
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_uniform | nir_var_shader_temp | nir_var_function_temp |
                 nir_var_mem_shared | nir_var_mem_global,
              glsl_get_cl_type_size_align);

   optimize(nir);

   NIR_PASS_V(nir, nir_remove_dead_variables, nir_var_all, NULL);

   /* Lower again, this time after dead-variables to get more compact variable
    * layouts.
    */
   NIR_PASS_V(nir, nir_lower_vars_to_explicit_types,
              nir_var_shader_temp | nir_var_function_temp | nir_var_mem_shared |
                 nir_var_mem_global | nir_var_mem_constant,
              glsl_get_cl_type_size_align);
   if (nir->constant_data_size > 0) {
      assert(nir->constant_data == NULL);
      nir->constant_data = rzalloc_size(nir, nir->constant_data_size);
      nir_gather_explicit_io_initializers(nir, nir->constant_data,
                                          nir->constant_data_size,
                                          nir_var_mem_constant);
   }

   NIR_PASS_V(nir, nir_lower_memcpy);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_mem_constant,
              nir_address_format_64bit_global);

   NIR_PASS_V(nir, nir_lower_explicit_io, nir_var_uniform,
              nir_address_format_32bit_offset_as_64bit);

   /* Note: we cannot lower explicit I/O here, because we need derefs in tact
    * for function calls into the library to work.
    */

   NIR_PASS_V(nir, nir_lower_convert_alu_types, NULL);
   NIR_PASS_V(nir, nir_opt_if, 0);
   NIR_PASS_V(nir, nir_opt_idiv_const, 16);

   optimize(nir);

   return nir;
}

/* Shader functions */
#define SPIR_V_MAGIC_NUMBER 0x07230203

static void
msg_callback(void *priv, const char *msg)
{
   (void)priv;
   fprintf(stderr, "%s", msg);
}

static void
print_u32_data(FILE *fp, const char *prefix, const char *arr_name,
               const uint32_t *data, size_t len)
{
   assert(len % 4 == 0);
   fprintf(fp, "#include <stdint.h>\n");
   fprintf(fp, "static const uint32_t %s_%s[] = {", prefix, arr_name);
   for (unsigned i = 0; i < (len / 4); i++) {
      if (i % 4 == 0)
         fprintf(fp, "\n   ");

      fprintf(fp, " 0x%08" PRIx32 ",", data[i]);
   }
   fprintf(fp, "\n};\n");
}

static void
print_usage(char *exec_name, FILE *f)
{
   fprintf(
      f,
      "Usage: %s [options] -- [clang args]\n"
      "Options:\n"
      "  -h  --help              Print this help.\n"
      "      --prefix <prefix>   Prefix for variable names in generated C code.\n"
      "  -o, --out <filename>    Specify the output filename.\n"
      "  -i, --in <filename>     Specify one input filename. Accepted multiple times.\n"
      "  -s, --spv <filename>    Specify the output filename for spirv.\n"
      "  -v, --verbose           Print more information during compilation.\n",
      exec_name);
}

#define OPT_PREFIX 1000

static uint32_t
get_module_spirv_version(const uint32_t *spirv, size_t size)
{
   assert(size >= 8);
   assert(spirv[0] == SPIR_V_MAGIC_NUMBER);
   return spirv[1];
}

static void
set_module_spirv_version(uint32_t *spirv, size_t size, uint32_t version)
{
   assert(size >= 8);
   assert(spirv[0] == SPIR_V_MAGIC_NUMBER);
   spirv[1] = version;
}

int
main(int argc, char **argv)
{
   static struct option long_options[] = {
      {"help", no_argument, 0, 'h'},
      {"prefix", required_argument, 0, OPT_PREFIX},
      {"in", required_argument, 0, 'i'},
      {"out", required_argument, 0, 'o'},
      {"spv", required_argument, 0, 's'},
      {"verbose", no_argument, 0, 'v'},
      {0, 0, 0, 0},
   };

   char *outfile = NULL, *spv_outfile = NULL, *prefix = NULL;
   struct util_dynarray clang_args;
   struct util_dynarray input_files;
   struct util_dynarray spirv_objs;
   struct util_dynarray spirv_ptr_objs;

   void *mem_ctx = ralloc_context(NULL);

   util_dynarray_init(&clang_args, mem_ctx);
   util_dynarray_init(&input_files, mem_ctx);
   util_dynarray_init(&spirv_objs, mem_ctx);
   util_dynarray_init(&spirv_ptr_objs, mem_ctx);

   int ch;
   while ((ch = getopt_long(argc, argv, "he:p:s:i:o:v", long_options, NULL)) !=
          -1) {
      switch (ch) {
      case 'h':
         print_usage(argv[0], stdout);
         return 0;
      case 'o':
         outfile = optarg;
         break;
      case 'i':
         util_dynarray_append(&input_files, char *, optarg);
         break;
      case 's':
         spv_outfile = optarg;
         break;
      case OPT_PREFIX:
         prefix = optarg;
         break;
      default:
         fprintf(stderr, "Unrecognized option \"%s\".\n", optarg);
         print_usage(argv[0], stderr);
         return 1;
      }
   }

   for (int i = optind; i < argc; i++) {
      util_dynarray_append(&clang_args, char *, argv[i]);
   }

   if (util_dynarray_num_elements(&input_files, char *) == 0) {
      fprintf(stderr, "No input file(s).\n");
      print_usage(argv[0], stderr);
      return -1;
   }

   if (prefix == NULL) {
      fprintf(stderr, "No prefix specified.\n");
      print_usage(argv[0], stderr);
      return -1;
   }

   struct clc_logger logger = {
      .error = msg_callback,
      .warning = msg_callback,
   };

   util_dynarray_foreach(&input_files, char *, infile) {
      int fd = open(*infile, O_RDONLY);
      if (fd < 0) {
         fprintf(stderr, "Failed to open %s\n", *infile);
         ralloc_free(mem_ctx);
         return 1;
      }

      off_t len = lseek(fd, 0, SEEK_END);
      const void *map = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
      close(fd);
      if (map == MAP_FAILED) {
         fprintf(stderr, "Failed to mmap the file: errno=%d, %s\n", errno,
                 strerror(errno));
         ralloc_free(mem_ctx);
         return 1;
      }

      const char *allowed_spirv_extensions[] = {
         "SPV_EXT_shader_atomic_float_add",
         "SPV_EXT_shader_atomic_float_min_max",
         "SPV_KHR_float_controls",
         "SPV_INTEL_subgroups",
         NULL,
      };

      struct clc_compile_args clc_args = {
         .source =
            {
               .name = *infile,
               .value = map,
            },
         .features =
            {
               .fp16 = true,
               .intel_subgroups = true,
               .subgroups = true,
               .subgroups_ifp = true,
            },
         .args = util_dynarray_begin(&clang_args),
         .num_args = util_dynarray_num_elements(&clang_args, char *),
         .allowed_spirv_extensions = allowed_spirv_extensions,
      };

      struct clc_binary *spirv_out =
         util_dynarray_grow(&spirv_objs, struct clc_binary, 1);

      if (!clc_compile_c_to_spirv(&clc_args, &logger, spirv_out)) {
         ralloc_free(mem_ctx);
         return 1;
      }
   }

   util_dynarray_foreach(&spirv_objs, struct clc_binary, p) {
      util_dynarray_append(&spirv_ptr_objs, struct clc_binary *, p);
   }

   /* The SPIRV-Tools linker started checking that all modules have the same
    * version. But SPIRV-LLVM-Translator picks the lower required version for
    * each module it compiles. So we have to iterate over all of them and set
    * the max found to make SPIRV-Tools link our modules.
    *
    * TODO: This is not the correct thing to do. We need SPIRV-LLVM-Translator
    *       to pick a given SPIRV version given to it and have all the modules
    *       at that version. We should remove this hack when this issue is
    *       fixed :
    *       https://github.com/KhronosGroup/SPIRV-LLVM-Translator/issues/1445
    */
   uint32_t max_spirv_version = 0;
   util_dynarray_foreach(&spirv_ptr_objs, struct clc_binary *, module) {
      max_spirv_version =
         MAX2(max_spirv_version,
              get_module_spirv_version((*module)->data, (*module)->size));
   }

   assert(max_spirv_version > 0);
   util_dynarray_foreach(&spirv_ptr_objs, struct clc_binary *, module) {
      set_module_spirv_version((*module)->data, (*module)->size,
                               max_spirv_version);
   }

   struct clc_linker_args link_args = {
      .in_objs = util_dynarray_begin(&spirv_ptr_objs),
      .num_in_objs =
         util_dynarray_num_elements(&spirv_ptr_objs, struct clc_binary *),
      .create_library = true,
   };
   struct clc_binary final_spirv;
   if (!clc_link_spirv(&link_args, &logger, &final_spirv)) {
      ralloc_free(mem_ctx);
      return 1;
   }

   if (spv_outfile) {
      FILE *fp = fopen(spv_outfile, "w");
      fwrite(final_spirv.data, final_spirv.size, 1, fp);
      fclose(fp);
   }

   FILE *fp = stdout;
   if (outfile != NULL)
      fp = fopen(outfile, "w");

   glsl_type_singleton_init_or_ref();

   fprintf(fp, "/*\n");
   fprintf(fp, " * Copyright The Asahi Linux Contributors\n");
   fprintf(fp, " * SPDX-License-Identifier: MIT\n");
   fprintf(fp, " *\n");
   fprintf(fp, " * Autogenerated file, do not edit\n");
   fprintf(fp, " */\n");
   spirv_library_to_nir_builder(fp, final_spirv.data, final_spirv.size / 4,
                                &spirv_options);

   /* Compile SPIR-V to NIR */
   nir_shader *nir = compile(NULL, final_spirv.data, final_spirv.size);

   /* Serialize NIR for embedding */
   struct blob blob;
   blob_init(&blob);
   nir_serialize(&blob, nir, false /* strip */);
   print_u32_data(fp, prefix, "nir", (const uint32_t *)blob.data, blob.size);
   blob_finish(&blob);

   glsl_type_singleton_decref();

   if (fp != stdout)
      fclose(fp);

   ralloc_free(mem_ctx);

   return 0;
}
