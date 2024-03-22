/*
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef MESA_CLC_H
#define MESA_CLC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nir_shader nir_shader;
struct nir_shader_compiler_options;

struct clc_named_value {
   const char *name;
   const char *value;
};

enum clc_spirv_version {
   CLC_SPIRV_VERSION_MAX = 0,
   CLC_SPIRV_VERSION_1_0,
   CLC_SPIRV_VERSION_1_1,
   CLC_SPIRV_VERSION_1_2,
   CLC_SPIRV_VERSION_1_3,
   CLC_SPIRV_VERSION_1_4,
};

struct clc_optional_features {
   bool fp16;
   bool fp64;
   bool int64;
   bool images;
   bool images_read_write;
   bool images_write_3d;
   bool integer_dot_product;
   bool intel_subgroups;
   /* OpenCL core subgroups */
   bool subgroups;
   /* OpenCL extension cl_khr_subgroups, which requires independent forward
    * progress
    */
   bool subgroups_ifp;
   bool subgroups_shuffle;
   bool subgroups_shuffle_relative;
};

struct clc_compile_args {
   const struct clc_named_value *headers;
   unsigned num_headers;
   struct clc_named_value source;
   const char * const *args;
   unsigned num_args;

   /* SPIRV version to target. */
   enum clc_spirv_version spirv_version;
   struct clc_optional_features features;

   /* Allowed extensions SPIRV extensions the OpenCL->SPIRV translation can
    * enable. A pointer to a NULL terminated array of strings, allow any
    * extension if NULL.
    */
   const char * const *allowed_spirv_extensions;

   unsigned address_bits;
};

struct clc_validator_options {
   uint32_t limit_max_function_arg;
};

struct clc_binary {
   void *data;
   size_t size;
};

struct clc_linker_args {
   const struct clc_binary * const *in_objs;
   unsigned num_in_objs;
   unsigned create_library;
};

typedef void (*clc_msg_callback)(void *priv, const char *msg);

struct clc_logger {
   void *priv;
   clc_msg_callback error;
   clc_msg_callback warning;
};

enum clc_kernel_arg_type_qualifier {
   CLC_KERNEL_ARG_TYPE_CONST = 1 << 0,
   CLC_KERNEL_ARG_TYPE_RESTRICT = 1 << 1,
   CLC_KERNEL_ARG_TYPE_VOLATILE = 1 << 2,
};

enum clc_kernel_arg_access_qualifier {
   CLC_KERNEL_ARG_ACCESS_READ = 1 << 0,
   CLC_KERNEL_ARG_ACCESS_WRITE = 1 << 1,
};

enum clc_kernel_arg_address_qualifier {
   CLC_KERNEL_ARG_ADDRESS_PRIVATE,
   CLC_KERNEL_ARG_ADDRESS_CONSTANT,
   CLC_KERNEL_ARG_ADDRESS_LOCAL,
   CLC_KERNEL_ARG_ADDRESS_GLOBAL,
};

struct clc_kernel_arg {
   const char *name;
   const char *type_name;
   unsigned type_qualifier;
   unsigned access_qualifier;
   enum clc_kernel_arg_address_qualifier address_qualifier;
};

enum clc_vec_hint_type {
   CLC_VEC_HINT_TYPE_CHAR = 0,
   CLC_VEC_HINT_TYPE_SHORT = 1,
   CLC_VEC_HINT_TYPE_INT = 2,
   CLC_VEC_HINT_TYPE_LONG = 3,
   CLC_VEC_HINT_TYPE_HALF = 4,
   CLC_VEC_HINT_TYPE_FLOAT = 5,
   CLC_VEC_HINT_TYPE_DOUBLE = 6
};

struct clc_kernel_info {
   const char *name;
   size_t num_args;
   const struct clc_kernel_arg *args;

   unsigned vec_hint_size;
   enum clc_vec_hint_type vec_hint_type;

   unsigned local_size[3];
   unsigned local_size_hint[3];
};

enum clc_spec_constant_type {
   CLC_SPEC_CONSTANT_UNKNOWN,
   CLC_SPEC_CONSTANT_BOOL,
   CLC_SPEC_CONSTANT_FLOAT,
   CLC_SPEC_CONSTANT_DOUBLE,
   CLC_SPEC_CONSTANT_INT8,
   CLC_SPEC_CONSTANT_UINT8,
   CLC_SPEC_CONSTANT_INT16,
   CLC_SPEC_CONSTANT_UINT16,
   CLC_SPEC_CONSTANT_INT32,
   CLC_SPEC_CONSTANT_UINT32,
   CLC_SPEC_CONSTANT_INT64,
   CLC_SPEC_CONSTANT_UINT64,
};

struct clc_parsed_spec_constant {
   uint32_t id;
   enum clc_spec_constant_type type;
};

struct clc_parsed_spirv {
   const struct clc_kernel_info *kernels;
   unsigned num_kernels;

   const struct clc_parsed_spec_constant *spec_constants;
   unsigned num_spec_constants;
};

struct clc_libclc;

struct clc_libclc_options {
   unsigned optimize;
   const struct nir_shader_compiler_options *nir_options;
};

struct clc_libclc *clc_libclc_new(const struct clc_logger *logger, const struct clc_libclc_options *options);

void clc_free_libclc(struct clc_libclc *lib);

const nir_shader *clc_libclc_get_clc_shader(struct clc_libclc *lib);

void clc_libclc_serialize(struct clc_libclc *lib, void **serialized, size_t *size);
void clc_libclc_free_serialized(void *serialized);
struct clc_libclc *clc_libclc_deserialize(const void *serialized, size_t size);

bool
clc_compile_c_to_spir(const struct clc_compile_args *args,
                      const struct clc_logger *logger,
                      struct clc_binary *out_spir);

void
clc_free_spir(struct clc_binary *spir);

bool
clc_compile_spir_to_spirv(const struct clc_binary *in_spir,
                          const struct clc_logger *logger,
                          struct clc_binary *out_spirv);

void
clc_free_spirv(struct clc_binary *spirv);

bool
clc_compile_c_to_spirv(const struct clc_compile_args *args,
                       const struct clc_logger *logger,
                       struct clc_binary *out_spirv);

bool
clc_link_spirv(const struct clc_linker_args *args,
               const struct clc_logger *logger,
               struct clc_binary *out_spirv);

bool
clc_parse_spirv(const struct clc_binary *in_spirv,
                const struct clc_logger *logger,
                struct clc_parsed_spirv *out_data);

void
clc_free_parsed_spirv(struct clc_parsed_spirv *data);

typedef union {
   bool b;
   float f32;
   double f64;
   int8_t i8;
   uint8_t u8;
   int16_t i16;
   uint16_t u16;
   int32_t i32;
   uint32_t u32;
   int64_t i64;
   uint64_t u64;
} clc_spirv_const_value;

struct clc_spirv_specialization {
   uint32_t id;
   clc_spirv_const_value value;
   bool defined_on_module;
};

struct clc_spirv_specialization_consts {
   const struct clc_spirv_specialization *specializations;
   unsigned num_specializations;
};

bool
clc_specialize_spirv(const struct clc_binary *in_spirv,
                     const struct clc_parsed_spirv *parsed_data,
                     const struct clc_spirv_specialization_consts *consts,
                     struct clc_binary *out_spirv);

enum clc_debug_flags {
   CLC_DEBUG_DUMP_SPIRV = 1 << 0,
   CLC_DEBUG_DUMP_LLVM = 1 << 1,
   CLC_DEBUG_VERBOSE = 1 << 2,
};
uint64_t clc_debug_flags(void);

#ifdef __cplusplus
}
#endif

#endif /* MESA_CLC_H */
