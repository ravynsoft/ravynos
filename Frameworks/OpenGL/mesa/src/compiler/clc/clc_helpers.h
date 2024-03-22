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

#ifndef MESA_CLC_HELPERS_H
#define MESA_CLC_HELPERS_H

#include "glsl_types.h"

#include "clc.h"
#include "util/u_string.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void
clc_initialize_llvm(void);

bool
clc_spirv_get_kernels_info(const struct clc_binary *spvbin,
                           const struct clc_kernel_info **kernels,
                           unsigned *num_kernels,
                           const struct clc_parsed_spec_constant **spec_constants,
                           unsigned *num_spec_constants,
                           const struct clc_logger *logger);

void
clc_free_kernels_info(const struct clc_kernel_info *kernels,
                      unsigned num_kernels);

int
clc_c_to_spir(const struct clc_compile_args *args,
              const struct clc_logger *logger,
              struct clc_binary *out_spir);

int
clc_spir_to_spirv(const struct clc_binary *in_spir,
                  const struct clc_logger *logger,
                  struct clc_binary *out_spirv);

int
clc_c_to_spirv(const struct clc_compile_args *args,
               const struct clc_logger *logger,
               struct clc_binary *out_spirv);

int
clc_link_spirv_binaries(const struct clc_linker_args *args,
                        const struct clc_logger *logger,
                        struct clc_binary *out_spirv);

bool
clc_validate_spirv(const struct clc_binary *spirv,
                   const struct clc_logger *logger,
                   const struct clc_validator_options *options);

int
clc_spirv_specialize(const struct clc_binary *in_spirv,
                     const struct clc_parsed_spirv *parsed_data,
                     const struct clc_spirv_specialization_consts *consts,
                     struct clc_binary *out_spirv);

void
clc_dump_spirv(const struct clc_binary *spvbin, FILE *f);

void
clc_free_spir_binary(struct clc_binary *spir);

void
clc_free_spirv_binary(struct clc_binary *spvbin);

#define clc_log(logger, level, fmt, ...) do {        \
      if (!logger || !logger->level) break;          \
      char *_msg = NULL;                             \
      asprintf(&_msg, fmt, ##__VA_ARGS__);           \
      assert(_msg);                                  \
      logger->level(logger->priv, _msg);             \
      free(_msg);                                    \
   } while (0)

#define clc_error(logger, fmt, ...) clc_log(logger, error, fmt, ##__VA_ARGS__)
#define clc_warning(logger, fmt, ...) clc_log(logger, warning, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* MESA_CLC_HELPERS_H */
