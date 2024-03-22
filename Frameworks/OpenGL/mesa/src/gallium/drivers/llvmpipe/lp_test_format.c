/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "util/u_memory.h"
#include "util/u_pointer.h"
#include "util/u_string.h"
#include "util/format/u_format.h"
#include "util/format/u_format_tests.h"
#include "util/format/u_format_s3tc.h"

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_format.h"
#include "gallivm/lp_bld_init.h"

#include "lp_test.h"

static struct lp_build_format_cache *cache_ptr;

void
write_tsv_header(FILE *fp)
{
   fprintf(fp,
           "result\t"
           "format\n");

   fflush(fp);
}


static void
write_tsv_row(FILE *fp,
              const struct util_format_description *desc,
              bool success)
{
   fprintf(fp, "%s\t", success ? "pass" : "fail");

   fprintf(fp, "%s\n", desc->name);

   fflush(fp);
}


typedef void
(*fetch_ptr_t)(void *unpacked, const void *packed,
               unsigned i, unsigned j, struct lp_build_format_cache *cache);


static LLVMValueRef
add_fetch_rgba_test(struct gallivm_state *gallivm, unsigned verbose,
                    const struct util_format_description *desc,
                    struct lp_type type,
                    unsigned use_cache)
{
   char name[256];
   LLVMContextRef context = gallivm->context;
   LLVMModuleRef module = gallivm->module;
   LLVMBuilderRef builder = gallivm->builder;
   LLVMTypeRef args[5];
   LLVMValueRef func;
   LLVMValueRef packed_ptr;
   LLVMValueRef offset = LLVMConstNull(LLVMInt32TypeInContext(context));
   LLVMValueRef rgba_ptr;
   LLVMValueRef i;
   LLVMValueRef j;
   LLVMBasicBlockRef block;
   LLVMValueRef rgba;
   LLVMValueRef cache = NULL;

   snprintf(name, sizeof name, "fetch_%s_%s", desc->short_name,
            type.floating ? "float" : "unorm8");

   args[0] = LLVMPointerType(lp_build_vec_type(gallivm, type), 0);
   args[1] = LLVMPointerType(LLVMInt8TypeInContext(context), 0);
   args[3] = args[2] = LLVMInt32TypeInContext(context);
   args[4] = LLVMPointerType(lp_build_format_cache_type(gallivm), 0);

   func = LLVMAddFunction(module, name,
                          LLVMFunctionType(LLVMVoidTypeInContext(context),
                                           args, ARRAY_SIZE(args), 0));
   LLVMSetFunctionCallConv(func, LLVMCCallConv);
   rgba_ptr = LLVMGetParam(func, 0);
   packed_ptr = LLVMGetParam(func, 1);
   i = LLVMGetParam(func, 2);
   j = LLVMGetParam(func, 3);

   if (use_cache) {
      cache = LLVMGetParam(func, 4);
   }

   block = LLVMAppendBasicBlockInContext(context, func, "entry");
   LLVMPositionBuilderAtEnd(builder, block);

   rgba = lp_build_fetch_rgba_aos(gallivm, desc, type, true,
                                  packed_ptr, offset, i, j, cache);

   LLVMBuildStore(builder, rgba, rgba_ptr);

   LLVMBuildRetVoid(builder);

   gallivm_verify_function(gallivm, func);

   return func;
}


UTIL_ALIGN_STACK
static bool
test_format_float(unsigned verbose, FILE *fp,
                  const struct util_format_description *desc,
                  unsigned use_cache)
{
   LLVMContextRef context;
   struct gallivm_state *gallivm;
   LLVMValueRef fetch = NULL;
   fetch_ptr_t fetch_ptr;
   alignas(16) uint8_t packed[UTIL_FORMAT_MAX_PACKED_BYTES];
   alignas(16) float unpacked[4];
   bool first = true;
   bool success = true;
   unsigned i, j, k, l;

   context = LLVMContextCreate();
#if LLVM_VERSION_MAJOR == 15
   LLVMContextSetOpaquePointers(context, false);
#endif
   gallivm = gallivm_create("test_module_float", context, NULL);

   fetch = add_fetch_rgba_test(gallivm, verbose, desc,
                               lp_float32_vec4_type(), use_cache);

   gallivm_compile_module(gallivm);

   fetch_ptr = (fetch_ptr_t) gallivm_jit_function(gallivm, fetch);

   gallivm_free_ir(gallivm);

   for (l = 0; l < util_format_nr_test_cases; ++l) {
      const struct util_format_test_case *test = &util_format_test_cases[l];

      if (test->format == desc->format) {

         if (first) {
            printf("Testing %s (float) ...\n",
                   desc->name);
            fflush(stdout);
            first = false;
         }

         /* To ensure it's 16-byte aligned */
         memcpy(packed, test->packed, sizeof packed);

         for (i = 0; i < desc->block.height; ++i) {
            for (j = 0; j < desc->block.width; ++j) {
               bool match = true;

               memset(unpacked, 0, sizeof unpacked);

               fetch_ptr(unpacked, packed, j, i, use_cache ? cache_ptr : NULL);

               for (k = 0; k < 4; ++k) {
                  if (util_double_inf_sign(test->unpacked[i][j][k]) != util_inf_sign(unpacked[k])) {
                     match = false;
                  }

                  if (util_is_double_nan(test->unpacked[i][j][k]) != util_is_nan(unpacked[k])) {
                     match = false;
                  }

                  if (!util_is_double_inf_or_nan(test->unpacked[i][j][k]) &&
                      fabs((float)test->unpacked[i][j][k] - unpacked[k]) > FLT_EPSILON) {
                     match = false;
                  }
               }

               /* Ignore errors in S3TC for now */
               if (desc->layout == UTIL_FORMAT_LAYOUT_S3TC) {
                  match = true;
               }

               if (!match) {
                  printf("FAILED\n");
                  printf("  Packed: %02x %02x %02x %02x\n",
                         test->packed[0], test->packed[1], test->packed[2], test->packed[3]);
                  printf("  Unpacked (%u,%u): %.9g %.9g %.9g %.9g obtained\n",
                         j, i,
                         unpacked[0], unpacked[1], unpacked[2], unpacked[3]);
                  printf("                  %.9g %.9g %.9g %.9g expected\n",
                         test->unpacked[i][j][0],
                         test->unpacked[i][j][1],
                         test->unpacked[i][j][2],
                         test->unpacked[i][j][3]);
                  fflush(stdout);
                  success = false;
               }
            }
         }
      }
   }

   gallivm_destroy(gallivm);
   LLVMContextDispose(context);

   if (fp)
      write_tsv_row(fp, desc, success);

   return success;
}


UTIL_ALIGN_STACK
static bool
test_format_unorm8(unsigned verbose, FILE *fp,
                   const struct util_format_description *desc,
                   unsigned use_cache)
{
   LLVMContextRef context;
   struct gallivm_state *gallivm;
   LLVMValueRef fetch = NULL;
   fetch_ptr_t fetch_ptr;
   alignas(16) uint8_t packed[UTIL_FORMAT_MAX_PACKED_BYTES];
   uint8_t unpacked[4];
   bool first = true;
   bool success = true;
   unsigned i, j, k, l;

   context = LLVMContextCreate();
#if LLVM_VERSION_MAJOR == 15
   LLVMContextSetOpaquePointers(context, false);
#endif
   gallivm = gallivm_create("test_module_unorm8", context, NULL);

   fetch = add_fetch_rgba_test(gallivm, verbose, desc,
                               lp_unorm8_vec4_type(), use_cache);

   gallivm_compile_module(gallivm);

   fetch_ptr = (fetch_ptr_t) gallivm_jit_function(gallivm, fetch);

   gallivm_free_ir(gallivm);

   for (l = 0; l < util_format_nr_test_cases; ++l) {
      const struct util_format_test_case *test = &util_format_test_cases[l];

      if (test->format == desc->format) {

         if (first) {
            printf("Testing %s (unorm8) ...\n",
                   desc->name);
            first = false;
         }

         /* To ensure it's 16-byte aligned */
         /* Could skip this and use unaligned lp_build_fetch_rgba_aos */
         memcpy(packed, test->packed, sizeof packed);

         for (i = 0; i < desc->block.height; ++i) {
            for (j = 0; j < desc->block.width; ++j) {
               bool match;

               memset(unpacked, 0, sizeof unpacked);

               fetch_ptr(unpacked, packed, j, i, use_cache ? cache_ptr : NULL);

               match = true;
               for (k = 0; k < 4; ++k) {
                  int error = float_to_ubyte(test->unpacked[i][j][k]) - unpacked[k];

                  if (util_is_double_nan(test->unpacked[i][j][k]))
                     continue;

                  if (error < 0)
                     error = -error;

                  if (error > 1)
                     match = false;
               }

               /* Ignore errors in S3TC as we only implement a poor man approach */
               if (desc->layout == UTIL_FORMAT_LAYOUT_S3TC) {
                  match = true;
               }

               if (!match) {
                  printf("FAILED\n");
                  printf("  Packed: %02x %02x %02x %02x\n",
                         test->packed[0], test->packed[1], test->packed[2], test->packed[3]);
                  printf("  Unpacked (%u,%u): %02x %02x %02x %02x obtained\n",
                         j, i,
                         unpacked[0], unpacked[1], unpacked[2], unpacked[3]);
                  printf("                  %02x %02x %02x %02x expected\n",
                         float_to_ubyte(test->unpacked[i][j][0]),
                         float_to_ubyte(test->unpacked[i][j][1]),
                         float_to_ubyte(test->unpacked[i][j][2]),
                         float_to_ubyte(test->unpacked[i][j][3]));

                  success = false;
               }
            }
         }
      }
   }

   gallivm_destroy(gallivm);
   LLVMContextDispose(context);

   if (fp)
      write_tsv_row(fp, desc, success);

   return success;
}




static bool
test_one(unsigned verbose, FILE *fp,
         const struct util_format_description *format_desc,
         unsigned use_cache)
{
   bool success = true;

   if (!test_format_float(verbose, fp, format_desc, use_cache)) {
     success = false;
   }

   if (!test_format_unorm8(verbose, fp, format_desc, use_cache)) {
     success = false;
   }

   return success;
}


bool
test_all(unsigned verbose, FILE *fp)
{
   enum pipe_format format;
   bool success = true;
   unsigned use_cache;

   cache_ptr = align_malloc(sizeof(struct lp_build_format_cache), 16);

   for (use_cache = 0; use_cache < 2; use_cache++) {
      for (format = 1; format < PIPE_FORMAT_COUNT; ++format) {
         const struct util_format_description *format_desc;

         format_desc = util_format_description(format);

         /*
          * TODO: test more
          */

         if (format_desc->colorspace == UTIL_FORMAT_COLORSPACE_ZS) {
            continue;
         }

         if (util_format_is_pure_integer(format))
            continue;

         /* The codegen sometimes falls back to calling the precompiled fetch
          * func, so if we don't have one of those (some compressed formats,
          * some ), we can't reliably test it.  We'll surely have a
          * precompiled fetch func for any format before we write LLVM code to
          * fetch from it.
          */
         if (!util_format_fetch_rgba_func(format))
            continue;

         /* only test twice with formats which can use cache */
         if (format_desc->layout != UTIL_FORMAT_LAYOUT_S3TC && use_cache) {
            continue;
         }

         if (!test_one(verbose, fp, format_desc, use_cache)) {
            success = false;
         }
      }
   }
   align_free(cache_ptr);

   return success;
}


bool
test_some(unsigned verbose, FILE *fp,
          unsigned long n)
{
   return test_all(verbose, fp);
}


bool
test_single(unsigned verbose, FILE *fp)
{
   printf("no test_single()");
   return true;
}
