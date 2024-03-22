/**************************************************************************
 *
 * Copyright 2011 VMware, Inc.
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


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "util/u_pointer.h"
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_cpu_detect.h"

#include "gallivm/lp_bld.h"
#include "gallivm/lp_bld_debug.h"
#include "gallivm/lp_bld_init.h"
#include "gallivm/lp_bld_arit.h"

#include "lp_test.h"


void
write_tsv_header(FILE *fp)
{
   fprintf(fp,
           "result\t"
           "format\n");

   fflush(fp);
}


typedef void (*unary_func_t)(float *out, const float *in);


/**
 * Describe a test case of one unary function.
 */
struct unary_test_t
{
   /*
    * Test name -- name of the mathematical function under test.
    */

   const char *name;

   LLVMValueRef
   (*builder)(struct lp_build_context *bld, LLVMValueRef a);

   /*
    * Reference (pure-C) function.
    */
   float
   (*ref)(float a);

   /*
    * Test values.
    */
   const float *values;
   unsigned num_values;

   /*
    * Required precision in bits.
    */
   double precision;
};


static float negf(float x)
{
   return -x;
}


static float sgnf(float x)
{
   if (x > 0.0f) {
      return 1.0f;
   }
   if (x < 0.0f) {
      return -1.0f;
   }
   return 0.0f;
}


const float sgn_values[] = {
   -INFINITY,
   -60,
   -4,
   -2,
   -1,
   -1e-007,
   0,
   1e-007,
   0.01,
   0.1,
   0.9,
   0.99,
   1,
   2,
   4,
   60,
   INFINITY,
   NAN
};


const float exp2_values[] = {
   -INFINITY,
   -60,
   -4,
   -2,
   -1,
   -1e-007,
   0,
   1e-007,
   0.01,
   0.1,
   0.9,
   0.99,
   1, 
   2, 
   4, 
   60,
   INFINITY,
   NAN
};


const float log2_values[] = {
#if 0
   /* 
    * Smallest denormalized number; meant just for experimentation, but not
    * validation.
    */
   1.4012984643248171e-45,
#endif
   -INFINITY,
   0,
   1e-007,
   0.1,
   0.5,
   0.99,
   1,
   1.01,
   1.1,
   1.9,
   1.99,
   2,
   4,
   100000,
   1e+018,
   INFINITY,
   NAN
};


static float rcpf(float x)
{
   return 1.0/x;
}


const float rcp_values[] = {
   -0.0, 0.0,
   -1.0, 1.0,
   -1e-007, 1e-007,
   -4.0, 4.0,
   -1e+035, -100000,
   100000, 1e+035,
   5.88e-39f, // denormal
   INFINITY, -INFINITY,
};


static float rsqrtf(float x)
{
   return 1.0/(float)sqrt(x);
}


const float rsqrt_values[] = {
   // http://msdn.microsoft.com/en-us/library/windows/desktop/bb147346.aspx
   0.0, // must yield infinity
   1.0, // must yield 1.0
   1e-007, 4.0,
   100000, 1e+035,
   5.88e-39f, // denormal
   INFINITY,
};


const float sincos_values[] = {
   -INFINITY,
   -5*M_PI/4,
   -4*M_PI/4,
   -4*M_PI/4,
   -3*M_PI/4,
   -2*M_PI/4,
   -1*M_PI/4,
   1*M_PI/4,
   2*M_PI/4,
   3*M_PI/4,
   4*M_PI/4,
   5*M_PI/4,
   INFINITY,
   NAN
};

const float round_values[] = {
      -10.0, -1, 0.0, 12.0,
      -1.49, -0.25, 1.25, 2.51,
      -0.99, -0.01, 0.01, 0.99,
      -1.5, -0.5, 0.5, 1.5,
      1.401298464324817e-45f, // smallest denormal
      -1.401298464324817e-45f,
      1.62981451e-08f,
      -1.62981451e-08f,
      1.62981451e15f, // large number not representable as 32bit int
      -1.62981451e15f,
      FLT_EPSILON,
      -FLT_EPSILON,
      1.0f - 0.5f*FLT_EPSILON,
      -1.0f + FLT_EPSILON,
      FLT_MAX,
      -FLT_MAX
};

static float fractf(float x)
{
   x -= floorf(x);
   if (x >= 1.0f) {
      // clamp to the largest number smaller than one
      x = 1.0f - 0.5f*FLT_EPSILON;
   }
   return x;
}


const float fract_values[] = {
   // http://en.wikipedia.org/wiki/IEEE_754-1985#Examples
   0.0f,
   -0.0f,
   1.0f,
   -1.0f,
   0.5f,
   -0.5f,
   1.401298464324817e-45f, // smallest denormal
   -1.401298464324817e-45f,
   5.88e-39f, // middle denormal
   1.18e-38f, // largest denormal
   -1.18e-38f,
   -1.62981451e-08f,
   FLT_EPSILON,
   -FLT_EPSILON,
   1.0f - 0.5f*FLT_EPSILON,
   -1.0f + FLT_EPSILON,
   FLT_MAX,
   -FLT_MAX
};


/*
 * Unary test cases.
 */

#ifdef _MSC_VER
#define WRAP(func) \
static float \
wrap_ ## func(float x) \
{ \
   return func(x); \
}
WRAP(expf)
WRAP(logf)
WRAP(sinf)
WRAP(cosf)
WRAP(floorf)
WRAP(ceilf)
#define expf wrap_expf
#define logf wrap_logf
#define sinf wrap_sinf
#define cosf wrap_cosf
#define floorf wrap_floorf
#define ceilf wrap_ceilf
#endif

static const struct unary_test_t
unary_tests[] = {
   {"abs", &lp_build_abs, &fabsf, sgn_values, ARRAY_SIZE(sgn_values), 20.0 },
   {"neg", &lp_build_negate, &negf, sgn_values, ARRAY_SIZE(sgn_values), 20.0 },
   {"sgn", &lp_build_sgn, &sgnf, sgn_values, ARRAY_SIZE(sgn_values), 20.0 },
   {"exp2", &lp_build_exp2, &exp2f, exp2_values, ARRAY_SIZE(exp2_values), 18.0 },
   {"log2", &lp_build_log2_safe, &log2f, log2_values, ARRAY_SIZE(log2_values), 20.0 },
   {"exp", &lp_build_exp, &expf, exp2_values, ARRAY_SIZE(exp2_values), 18.0 },
   {"log", &lp_build_log_safe, &logf, log2_values, ARRAY_SIZE(log2_values), 20.0 },
   {"rcp", &lp_build_rcp, &rcpf, rcp_values, ARRAY_SIZE(rcp_values), 20.0 },
   {"rsqrt", &lp_build_rsqrt, &rsqrtf, rsqrt_values, ARRAY_SIZE(rsqrt_values), 20.0 },
   {"sin", &lp_build_sin, &sinf, sincos_values, ARRAY_SIZE(sincos_values), 20.0 },
   {"cos", &lp_build_cos, &cosf, sincos_values, ARRAY_SIZE(sincos_values), 20.0 },
   {"sgn", &lp_build_sgn, &sgnf, sgn_values, ARRAY_SIZE(sgn_values), 20.0 },
   {"round", &lp_build_round, &nearbyintf, round_values, ARRAY_SIZE(round_values), 24.0 },
   {"trunc", &lp_build_trunc, &truncf, round_values, ARRAY_SIZE(round_values), 24.0 },
   {"floor", &lp_build_floor, &floorf, round_values, ARRAY_SIZE(round_values), 24.0 },
   {"ceil", &lp_build_ceil, &ceilf, round_values, ARRAY_SIZE(round_values), 24.0 },
   {"fract", &lp_build_fract_safe, &fractf, fract_values, ARRAY_SIZE(fract_values), 24.0 },
};


/*
 * Build LLVM function that exercises the unary operator builder.
 */
static LLVMValueRef
build_unary_test_func(struct gallivm_state *gallivm,
                      const struct unary_test_t *test,
                      unsigned length,
                      const char *test_name)
{
   struct lp_type type = lp_type_float_vec(32, length * 32);
   LLVMContextRef context = gallivm->context;
   LLVMModuleRef module = gallivm->module;
   LLVMTypeRef vf32t = lp_build_vec_type(gallivm, type);
   LLVMTypeRef args[2] = { LLVMPointerType(vf32t, 0), LLVMPointerType(vf32t, 0) };
   LLVMValueRef func = LLVMAddFunction(module, test_name,
                                       LLVMFunctionType(LLVMVoidTypeInContext(context),
                                                        args, ARRAY_SIZE(args), 0));
   LLVMValueRef arg0 = LLVMGetParam(func, 0);
   LLVMValueRef arg1 = LLVMGetParam(func, 1);
   LLVMBuilderRef builder = gallivm->builder;
   LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext(context, func, "entry");
   LLVMValueRef ret;

   struct lp_build_context bld;

   lp_build_context_init(&bld, gallivm, type);

   LLVMSetFunctionCallConv(func, LLVMCCallConv);

   LLVMPositionBuilderAtEnd(builder, block);

   arg1 = LLVMBuildLoad2(builder, vf32t, arg1, "");

   ret = test->builder(&bld, arg1);

   LLVMBuildStore(builder, ret, arg0);

   LLVMBuildRetVoid(builder);

   gallivm_verify_function(gallivm, func);

   return func;
}


/*
 * Flush denorms to zero.
 */
static float
flush_denorm_to_zero(float val)
{
   /*
    * If we have a denorm manually set it to (+-)0.
    * This is because the reference may or may not do the right thing
    * otherwise because we want the result according to treating all
    * denormals as zero (FTZ/DAZ). Not using fpclassify because
    * a) some compilers are stuck at c89 (msvc)
    * b) not sure it reliably works with non-standard ftz/daz mode
    * And, right now we only disable denorms with jited code on x86/sse
    * (albeit this should be classified as a bug) so to get results which
    * match we must only flush them to zero here in that case too.
    */
   union fi fi_val;

   fi_val.f = val;

#if DETECT_ARCH_SSE
   if (util_get_cpu_caps()->has_sse) {
      if ((fi_val.ui & 0x7f800000) == 0) {
         fi_val.ui &= 0xff800000;
      }
   }
#endif

   return fi_val.f;
}

/*
 * Test one LLVM unary arithmetic builder function.
 */
static bool
test_unary(unsigned verbose, FILE *fp, const struct unary_test_t *test, unsigned length)
{
   char test_name[128];
   snprintf(test_name, sizeof test_name, "%s.v%u", test->name, length);
   LLVMContextRef context;
   struct gallivm_state *gallivm;
   LLVMValueRef test_func;
   unary_func_t test_func_jit;
   bool success = true;
   int i, j;
   float *in, *out;

   in = align_malloc(length * 4, length * 4);
   out = align_malloc(length * 4, length * 4);

   /* random NaNs or 0s could wreak havoc */
   for (i = 0; i < length; i++) {
      in[i] = 1.0;
   }

   context = LLVMContextCreate();
#if LLVM_VERSION_MAJOR == 15
   LLVMContextSetOpaquePointers(context, false);
#endif
   gallivm = gallivm_create("test_module", context, NULL);

   test_func = build_unary_test_func(gallivm, test, length, test_name);

   gallivm_compile_module(gallivm);

   test_func_jit = (unary_func_t) gallivm_jit_function(gallivm, test_func);

   gallivm_free_ir(gallivm);

   for (j = 0; j < (test->num_values + length - 1) / length; j++) {
      int num_vals = ((j + 1) * length <= test->num_values) ? length :
                                                              test->num_values % length;

      for (i = 0; i < num_vals; ++i) {
         in[i] = test->values[i+j*length];
      }

      test_func_jit(out, in);
      for (i = 0; i < num_vals; ++i) {
         float testval, ref;
         double error, precision;
         bool expected_pass = true;
         bool pass;

         testval = flush_denorm_to_zero(in[i]);
         ref = flush_denorm_to_zero(test->ref(testval));

         if (util_inf_sign(ref) && util_inf_sign(out[i]) == util_inf_sign(ref)) {
            error = 0;
         } else {
            error = fabs(out[i] - ref);
         }
         precision = error ? -log2(error/fabs(ref)) : FLT_MANT_DIG;

         pass = precision >= test->precision;

         if (isnan(ref)) {
            continue;
         }

         if (!util_get_cpu_caps()->has_neon &&
             util_get_cpu_caps()->family != CPU_S390X &&
             test->ref == &nearbyintf && length == 2 &&
             ref != roundf(testval)) {
            /* FIXME: The generic (non SSE) path in lp_build_iround, which is
             * always taken for length==2 regardless of native round support,
             * does not round to even. */
            expected_pass = false;
         }

         if (test->ref == &expf && util_inf_sign(testval) == -1) {
            /* Some older 64-bit MSVCRT versions return -inf instead of 0
	     * for expf(-inf). As detecting the VC runtime version is
	     * non-trivial, just ignore the test result. */
#if defined(_MSC_VER) && defined(_WIN64)
            expected_pass = pass;
#endif
         }

         if (pass != expected_pass || verbose) {
            printf("%s(%.9g): ref = %.9g, out = %.9g, precision = %f bits, %s%s\n",
                  test_name, in[i], ref, out[i], precision,
                  pass ? "PASS" : "FAIL",
                  !expected_pass ? (pass ? " (unexpected)" : " (expected)" ): "");
            fflush(stdout);
         }

         if (pass != expected_pass) {
            success = false;
         }
      }
   }

   gallivm_destroy(gallivm);
   LLVMContextDispose(context);

   align_free(in);
   align_free(out);

   return success;
}


bool
test_all(unsigned verbose, FILE *fp)
{
   bool success = true;
   int i;

   for (i = 0; i < ARRAY_SIZE(unary_tests); ++i) {
      unsigned max_length = lp_native_vector_width / 32;
      unsigned length;
      for (length = 1; length <= max_length; length *= 2) {
         if (!test_unary(verbose, fp, &unary_tests[i], length)) {
            success = false;
         }
      }
   }

   return success;
}


bool
test_some(unsigned verbose, FILE *fp,
          unsigned long n)
{
   /*
    * Not randomly generated test cases, so test all.
    */

   return test_all(verbose, fp);
}


bool
test_single(unsigned verbose, FILE *fp)
{
   return true;
}
