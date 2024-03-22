/*
 * Copyright © 2021 Intel Corporation
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

#include "brw_compiler.h"
#include "brw_kernel.h"
#include "common/intel_disasm.h"
#include "compiler/clc/clc.h"
#include "compiler/glsl_types.h"
#include "dev/intel_debug.h"
#include "util/build_id.h"
#include "util/disk_cache.h"
#include "util/macros.h"
#include "util/mesa-sha1.h"
#include "util/u_dynarray.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

/* Shader functions */
#define SPIR_V_MAGIC_NUMBER 0x07230203

static struct disk_cache *
get_disk_cache(struct brw_compiler *compiler)
{
#ifdef ENABLE_SHADER_CACHE
   char renderer[14];
   ASSERTED int len = snprintf(renderer, sizeof(renderer), "brw_clc_%04x",
                               compiler->devinfo->pci_device_id);
   assert(len == sizeof(renderer) - 2);

   const struct build_id_note *note =
      build_id_find_nhdr_for_addr(get_disk_cache);
   if (note == NULL) {
      fprintf(stderr, "Failed to find build-id\n");
      abort();
   }

   unsigned build_id_len = build_id_length(note);
   if (build_id_len < 20) {
      fprintf(stderr, "build-id too short.  It needs to be a SHA\n");
      abort();
   }

   struct mesa_sha1 sha1_ctx;
   uint8_t sha1[20];
   _mesa_sha1_init(&sha1_ctx);
   _mesa_sha1_update(&sha1_ctx, build_id_data(note), build_id_len);
   _mesa_sha1_final(&sha1_ctx, sha1);

   char timestamp[41];
   _mesa_sha1_format(timestamp, sha1);

   const uint64_t driver_flags = brw_get_compiler_config_value(compiler);

   return disk_cache_create(renderer, timestamp, driver_flags);
#endif
   return NULL;
}

static void
compiler_log(void *data, unsigned *id, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   if (INTEL_DEBUG(DEBUG_CS))
      vfprintf(stderr, fmt, args);
   va_end(args);
}

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
   fprintf(fp, "static const uint32_t %s_%s[] = {", prefix, arr_name);
   for (unsigned i = 0; i < (len / 4); i++) {
      if (i % 4 == 0)
         fprintf(fp,"\n   ");

      fprintf(fp, " 0x%08" PRIx32 ",", data[i]);
   }
   fprintf(fp, "\n};\n");
}

static const char *
reloc_type_str(enum brw_shader_reloc_type type)
{
   switch (type) {
#define CASE(e) case e: return #e;
   CASE(BRW_SHADER_RELOC_TYPE_U32)
   CASE(BRW_SHADER_RELOC_TYPE_MOV_IMM)
#undef CASE
   default:
      unreachable("Unknown relocation type");
   }
}

static void
print_cs_prog_data_fields(FILE *fp, const char *prefix, const char *pad,
                          const struct brw_cs_prog_data *cs_prog_data)
{
#define PROG_DATA_FIELD(fmt, field) \
   fprintf(fp, "%s." #field " = " fmt ",\n", pad, cs_prog_data->field)

#define PROG_DATA_BOOL_FIELD(field) \
   fprintf(fp, "%s." #field " = %s,\n", pad, \
           cs_prog_data->field ? "true" : "false")

   PROG_DATA_FIELD("%u", base.nr_params);
   assert(cs_prog_data->base.stage == MESA_SHADER_COMPUTE);
   fprintf(fp, "%s.base.stage = MESA_SHADER_COMPUTE,\n", pad);
   assert(cs_prog_data->base.zero_push_reg == 0);
   assert(cs_prog_data->base.push_reg_mask_param == 0);
   PROG_DATA_FIELD("%u", base.curb_read_length);
   PROG_DATA_FIELD("%u", base.total_scratch);
   PROG_DATA_FIELD("%u", base.total_shared);
   PROG_DATA_FIELD("%u", base.program_size);
   PROG_DATA_FIELD("%u", base.const_data_size);
   PROG_DATA_FIELD("%u", base.const_data_offset);
   PROG_DATA_FIELD("%u", base.num_relocs);
   fprintf(fp, "%s.base.relocs = %s_relocs,\n", pad, prefix);
   assert(!cs_prog_data->base.has_ubo_pull);
   assert(cs_prog_data->base.dispatch_grf_start_reg == 0);
   assert(!cs_prog_data->base.use_alt_mode);
   assert(cs_prog_data->base.param == 0);
   PROG_DATA_BOOL_FIELD(base.uses_atomic_load_store);
   fprintf(fp, "%s.local_size = { %u, %u, %u },\n", pad,
           cs_prog_data->local_size[0],
           cs_prog_data->local_size[1],
           cs_prog_data->local_size[2]);
   fprintf(fp, "%s.prog_offset = { %u, %u, %u },\n", pad,
           cs_prog_data->prog_offset[0],
           cs_prog_data->prog_offset[1],
           cs_prog_data->prog_offset[2]);
   PROG_DATA_FIELD("%u", prog_mask);
   PROG_DATA_FIELD("%u", prog_spilled);
   PROG_DATA_BOOL_FIELD(uses_barrier);
   PROG_DATA_BOOL_FIELD(uses_num_work_groups);
   assert(!cs_prog_data->uses_inline_data);
   assert(!cs_prog_data->uses_btd_stack_ids);
   PROG_DATA_FIELD("%u", push.per_thread.dwords);
   PROG_DATA_FIELD("%u", push.per_thread.regs);
   PROG_DATA_FIELD("%u", push.per_thread.size);
   PROG_DATA_FIELD("%u", push.cross_thread.dwords);
   PROG_DATA_FIELD("%u", push.cross_thread.regs);
   PROG_DATA_FIELD("%u", push.cross_thread.size);

#undef PROG_DATA_FIELD
#undef PROG_DATA_BOOL_FIELD
}

static void
print_kernel(FILE *fp, const char *prefix,
             const struct brw_kernel *kernel,
             const struct brw_isa_info *isa)
{
   struct mesa_sha1 sha1_ctx;
   _mesa_sha1_init(&sha1_ctx);

#define SHA1_UPDATE_VALUE(val) \
   _mesa_sha1_update(&sha1_ctx, &val, sizeof(val))

   fprintf(fp, "#include \"intel/compiler/brw_kernel.h\"\n");
   fprintf(fp, "\n");

   fprintf(fp, "static const struct brw_shader_reloc %s_relocs[] = {\n",
           prefix);
   for (unsigned i = 0; i < kernel->prog_data.base.num_relocs; i++) {
      const struct brw_shader_reloc *reloc = &kernel->prog_data.base.relocs[i];
      fprintf(fp, "   { %"PRIu32", %s, %"PRIu32", %"PRIu32" },\n",
              reloc->id, reloc_type_str(reloc->type),
              reloc->offset, reloc->delta);
   }
   fprintf(fp, "};\n");
   _mesa_sha1_update(&sha1_ctx, kernel->prog_data.base.relocs,
                     kernel->prog_data.base.num_relocs *
                     sizeof(kernel->prog_data.base.relocs[0]));

   /* Get rid of the pointers before we hash */
   struct brw_cs_prog_data cs_prog_data = kernel->prog_data;
   cs_prog_data.base.relocs = NULL;
   assert(cs_prog_data.base.param == NULL);
   _mesa_sha1_update(&sha1_ctx, &cs_prog_data, sizeof(cs_prog_data));

   SHA1_UPDATE_VALUE(kernel->args_size);
   SHA1_UPDATE_VALUE(kernel->arg_count);
   _mesa_sha1_update(&sha1_ctx, kernel->args,
                     kernel->arg_count * sizeof(kernel->args[0]));

   fprintf(fp, "static const struct brw_kernel_arg_desc %s_args[] = {\n",
           prefix);
   for (unsigned i = 0; i < kernel->arg_count; i++) {
      fprintf(fp, "   { %d, %d },\n",
              kernel->args[i].offset, kernel->args[i].size);
   }
   fprintf(fp, "};\n\n");

   _mesa_sha1_update(&sha1_ctx, kernel->code,
                     kernel->prog_data.base.program_size);

   fprintf(fp, "#if 0  /* BEGIN KERNEL ASSEMBLY */\n");
   fprintf(fp, "\n");
   intel_disassemble(isa, kernel->code, 0, fp);
   fprintf(fp, "\n");
   fprintf(fp, "#endif /* END KERNEL ASSEMBLY */\n");
   print_u32_data(fp, prefix, "code", kernel->code,
                  kernel->prog_data.base.program_size);

   fprintf(fp, "static const struct brw_kernel %s = {\n", prefix);
   fprintf(fp, "   .prog_data = {\n");
   print_cs_prog_data_fields(fp, prefix, "      ", &kernel->prog_data);
   fprintf(fp, "   },\n");
   fprintf(fp, "   .args_size = %d,\n", (int)kernel->args_size);
   fprintf(fp, "   .arg_count = %d,\n", (int)kernel->arg_count);
   fprintf(fp, "   .args = %s_args,\n", prefix);
   fprintf(fp, "   .code = %s_code,\n", prefix);
   fprintf(fp, "};\n");

   unsigned char sha1[20];
   _mesa_sha1_final(&sha1_ctx, sha1);
   char sha1_str[41];
   _mesa_sha1_format(sha1_str, sha1);
   fprintf(fp, "const char *%s_sha1 = \"%s\";\n", prefix, sha1_str);
}

static void
print_usage(char *exec_name, FILE *f)
{
   fprintf(f,
"Usage: %s [options] -- [clang args]\n"
"Options:\n"
"  -h  --help              Print this help.\n"
"  -e, --entrypoint <name> Specify the entry-point name.\n"
"  -p, --platform <name>   Specify the target platform name.\n"
"      --prefix <prefix>   Prefix for variable names in generated C code.\n"
"  -o, --out <filename>    Specify the output filename.\n"
"  -i, --in <filename>     Specify one input filename. Accepted multiple times.\n"
"  -s, --spv <filename>    Specify the output filename for spirv.\n"
"  -v, --verbose           Print more information during compilation.\n"
   , exec_name);
}

#define OPT_PREFIX 1000

int main(int argc, char **argv)
{
   int exit_code = 0;

   brw_process_intel_debug_variable();

   static struct option long_options[] ={
      {"help",       no_argument,         0, 'h'},
      {"entrypoint", required_argument,   0, 'e'},
      {"platform",   required_argument,   0, 'p'},
      {"prefix",     required_argument,   0, OPT_PREFIX},
      {"in",         required_argument,   0, 'i'},
      {"out",        required_argument,   0, 'o'},
      {"spv",        required_argument,   0, 's'},
      {"verbose",    no_argument,         0, 'v'},
      {0, 0, 0, 0}
   };

   char *entry_point = NULL, *platform = NULL, *outfile = NULL, *spv_outfile = NULL, *prefix = NULL;
   struct util_dynarray clang_args;
   struct util_dynarray input_files;
   bool print_info = false;

   struct clc_binary spirv_obj = {0};
   struct clc_parsed_spirv parsed_spirv_data = {0};
   struct disk_cache *disk_cache = NULL;

   void *mem_ctx = ralloc_context(NULL);

   util_dynarray_init(&clang_args, mem_ctx);
   util_dynarray_init(&input_files, mem_ctx);

   int ch;
   while ((ch = getopt_long(argc, argv, "he:p:s:i:o:v", long_options, NULL)) != -1)
   {
      switch (ch)
      {
      case 'h':
         print_usage(argv[0], stdout);
         goto end;
      case 'e':
         entry_point = optarg;
         break;
      case 'p':
         platform = optarg;
         break;
      case 'o':
         outfile = optarg;
         break;
      case 'i':
         util_dynarray_append(&input_files, char *, optarg);
	 break;
      case 's':
         spv_outfile = optarg;
         break;
      case 'v':
         print_info = true;
         break;
      case OPT_PREFIX:
         prefix = optarg;
         break;
      default:
         fprintf(stderr, "Unrecognized option \"%s\".\n", optarg);
         print_usage(argv[0], stderr);
         goto fail;
      }
   }

   for (int i = optind; i < argc; i++) {
      util_dynarray_append(&clang_args, char *, argv[i]);
   }

   if (util_dynarray_num_elements(&input_files, char *) == 0) {
      fprintf(stderr, "No input file(s).\n");
      print_usage(argv[0], stderr);
      goto fail;
   }

   if (platform == NULL) {
      fprintf(stderr, "No target platform name specified.\n");
      print_usage(argv[0], stderr);
      goto fail;
   }

   int pci_id = intel_device_name_to_pci_device_id(platform);
   if (pci_id < 0) {
      fprintf(stderr, "Invalid target platform name: %s\n", platform);
      goto fail;
   }

   struct intel_device_info _devinfo, *devinfo = &_devinfo;
   if (!intel_get_device_info_from_pci_id(pci_id, devinfo)) {
      fprintf(stderr, "Failed to get device information.\n");
      goto fail;
   }

   if (devinfo->verx10 < 125) {
      fprintf(stderr, "Platform currently not supported.\n");
      goto fail;
   }

   struct brw_isa_info _isa, *isa = &_isa;
   brw_init_isa_info(isa, devinfo);

   if (entry_point == NULL) {
      fprintf(stderr, "No entry-point name specified.\n");
      print_usage(argv[0], stderr);
      goto fail;
   }

   struct clc_logger logger = {
      .error = msg_callback,
      .warning = msg_callback,
   };

   size_t total_size = 0;
   char *all_inputs = NULL;
   util_dynarray_foreach(&input_files, char *, infile) {
      int fd = open(*infile, O_RDONLY);
      if (fd < 0) {
         fprintf(stderr, "Failed to open %s\n", *infile);
         goto fail;
      }

      off_t len = lseek(fd, 0, SEEK_END);
      size_t new_size = total_size + len;
      all_inputs = reralloc_size(mem_ctx, all_inputs, new_size + 1);
      if (!all_inputs) {
         fprintf(stderr, "Failed to allocate memory\n");
         goto fail;
      }
      lseek(fd, 0, SEEK_SET);
      read(fd, all_inputs + total_size, len);
      close(fd);
      total_size = new_size;
      all_inputs[total_size] = '\0';
   }

   const char *allowed_spirv_extensions[] = {
      "SPV_EXT_shader_atomic_float_add",
      "SPV_EXT_shader_atomic_float_min_max",
      "SPV_KHR_float_controls",
      "SPV_INTEL_subgroups",
      NULL,
   };

   struct clc_compile_args clc_args = {
      .source = {
         .name = "intel_clc_files",
         .value = all_inputs,
      },
      .features = {
         .fp16 = true,
         .intel_subgroups = true,
         .subgroups = true,
         .subgroups_ifp = true,
      },
      .args = util_dynarray_begin(&clang_args),
      .num_args = util_dynarray_num_elements(&clang_args, char *),
      .allowed_spirv_extensions = allowed_spirv_extensions,
   };

   if (!clc_compile_c_to_spirv(&clc_args, &logger, &spirv_obj)) {
      goto fail;
   }

   if (spv_outfile) {
      FILE *fp = fopen(spv_outfile, "w");
      fwrite(spirv_obj.data, spirv_obj.size, 1, fp);
      fclose(fp);
   }

   if (!clc_parse_spirv(&spirv_obj, &logger, &parsed_spirv_data)) {
      goto fail;
   }

   const struct clc_kernel_info *kernel_info = NULL;
   for (unsigned i = 0; i < parsed_spirv_data.num_kernels; i++) {
      if (strcmp(parsed_spirv_data.kernels[i].name, entry_point) == 0) {
         kernel_info = &parsed_spirv_data.kernels[i];
         break;
      }
   }
   if (kernel_info == NULL) {
      fprintf(stderr, "Kernel entrypoint %s not found\n", entry_point);
      goto fail;
   }

   struct brw_kernel kernel = {};
   char *error_str;

   struct brw_compiler *compiler = brw_compiler_create(mem_ctx, devinfo);
   compiler->shader_debug_log = compiler_log;
   compiler->shader_perf_log = compiler_log;
   disk_cache = get_disk_cache(compiler);

   glsl_type_singleton_init_or_ref();

   if (!brw_kernel_from_spirv(compiler, disk_cache, &kernel, NULL, mem_ctx,
                              spirv_obj.data, spirv_obj.size,
                              entry_point, &error_str)) {
      fprintf(stderr, "Compile failed: %s\n", error_str);
      goto fail;
   }

   if (print_info) {
      fprintf(stdout, "kernel info:\n");
      fprintf(stdout, "   uses_barrier           : %u\n", kernel.prog_data.uses_barrier);
      fprintf(stdout, "   uses_num_work_groups   : %u\n", kernel.prog_data.uses_num_work_groups);
      fprintf(stdout, "   uses_inline_data       : %u\n", kernel.prog_data.uses_inline_data);
      fprintf(stdout, "   local_size             : %ux%ux%u\n",
              kernel.prog_data.local_size[0],
              kernel.prog_data.local_size[1],
              kernel.prog_data.local_size[2]);
      fprintf(stdout, "   curb_read_length       : %u\n", kernel.prog_data.base.curb_read_length);
      fprintf(stdout, "   total_scratch          : %u\n", kernel.prog_data.base.total_scratch);
      fprintf(stdout, "   total_shared           : %u\n", kernel.prog_data.base.total_shared);
      fprintf(stdout, "   program_size           : %u\n", kernel.prog_data.base.program_size);
      fprintf(stdout, "   const_data_size        : %u\n", kernel.prog_data.base.const_data_size);
      fprintf(stdout, "   uses_atomic_load_store : %u\n", kernel.prog_data.base.uses_atomic_load_store);
      fprintf(stdout, "   dispatch_grf_start_reg : %u\n", kernel.prog_data.base.dispatch_grf_start_reg);
   }

   glsl_type_singleton_decref();

   char prefix_tmp[256];
   if (prefix == NULL) {
      bool is_pt_5 = (devinfo->verx10 % 10) == 5;
      snprintf(prefix_tmp, sizeof(prefix_tmp), "gfx%d%s_clc_%s",
               devinfo->ver, is_pt_5 ? "5" : "", entry_point);
      prefix = prefix_tmp;
   }

   if (outfile != NULL) {
      FILE *fp = fopen(outfile, "w");
      print_kernel(fp, prefix, &kernel, isa);
      fclose(fp);
   } else {
      print_kernel(stdout, prefix, &kernel, isa);
   }

   goto end;

fail:
   exit_code = 1;

end:
   disk_cache_destroy(disk_cache);
   clc_free_parsed_spirv(&parsed_spirv_data);
   clc_free_spirv(&spirv_obj);
   ralloc_free(mem_ctx);

   return exit_code;
}
