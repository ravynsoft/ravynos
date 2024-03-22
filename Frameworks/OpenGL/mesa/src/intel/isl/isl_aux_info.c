/*
 * Copyright 2019 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "isl/isl.h"

#ifdef IN_UNIT_TEST
/* STATIC_ASSERT is a do { ... } while(0) statement */
UNUSED static void static_assert_func(void) {
   STATIC_ASSERT(ISL_AUX_OP_ASSERT == ((enum isl_aux_op) 0));
   STATIC_ASSERT(ISL_AUX_STATE_ASSERT == ((enum isl_aux_state) 0));
}

#undef unreachable
#define unreachable(str) return 0

#undef assert
#define assert(cond) do { \
   if (!(cond)) { \
      return 0; \
   } \
} while (0)
#endif

/* How writes with an isl_aux_usage behave. */
enum write_behavior {
   /* Writes only touch the main surface. */
   WRITES_ONLY_TOUCH_MAIN = 0,

   /* Writes using the 3D engine are compressed. */
   WRITES_COMPRESS,

   /* Writes using the 3D engine are either compressed or substituted with
    * fast-cleared blocks.
    */
   WRITES_COMPRESS_CLEAR,

   /* Writes implicitly fully resolve the compression block and write the data
    * uncompressed into the main surface. The resolved aux blocks are
    * ambiguated and left in the pass-through state.
    */
   WRITES_RESOLVE_AMBIGUATE,
};

/* A set of features supported by an isl_aux_usage. */
struct aux_usage_info {

   /* How writes affect the surface(s) in use. */
   enum write_behavior write_behavior;

   /* Aux supports "real" compression beyond just fast-clears. */
   bool compressed;

   /* SW can perform ISL_AUX_OP_FAST_CLEAR. */
   bool fast_clear;

   /* SW can perform ISL_AUX_OP_PARTIAL_RESOLVE. */
   bool partial_resolve;

   /* Performing ISL_AUX_OP_FULL_RESOLVE includes ISL_AUX_OP_AMBIGUATE. */
   bool full_resolves_ambiguate;
};

#define AUX(wb, c, fc, pr, fra, type)                   \
   [ISL_AUX_USAGE_ ## type] = { WRITES_ ## wb, c, fc, pr, fra},
#define Y true
#define x false
static const struct aux_usage_info info[] = {
/*         write_behavior c fc pr fra */
   AUX(         COMPRESS, Y, Y, x, x, HIZ)
   AUX(         COMPRESS, Y, Y, x, x, HIZ_CCS)
   AUX(         COMPRESS, Y, Y, x, x, HIZ_CCS_WT)
   AUX(         COMPRESS, Y, Y, Y, x, MCS)
   AUX(         COMPRESS, Y, Y, Y, x, MCS_CCS)
   AUX(         COMPRESS, Y, Y, Y, Y, CCS_E)
   AUX(   COMPRESS_CLEAR, Y, Y, Y, Y, FCV_CCS_E)
   AUX(RESOLVE_AMBIGUATE, x, Y, x, Y, CCS_D)
   AUX(RESOLVE_AMBIGUATE, Y, x, x, Y, MC)
   AUX(         COMPRESS, Y, x, x, Y, STC_CCS)
};
#undef x
#undef Y
#undef AUX

ASSERTED static bool
aux_state_possible(enum isl_aux_state state,
                   enum isl_aux_usage usage)
{
   switch (state) {
   case ISL_AUX_STATE_CLEAR:
   case ISL_AUX_STATE_PARTIAL_CLEAR:
      return info[usage].fast_clear;
   case ISL_AUX_STATE_COMPRESSED_CLEAR:
      return info[usage].fast_clear && info[usage].compressed;
   case ISL_AUX_STATE_COMPRESSED_NO_CLEAR:
      return info[usage].compressed;
   case ISL_AUX_STATE_RESOLVED:
   case ISL_AUX_STATE_PASS_THROUGH:
   case ISL_AUX_STATE_AUX_INVALID:
      return true;
#ifdef IN_UNIT_TEST
   case ISL_AUX_STATE_ASSERT:
      break;
#endif
   }

   unreachable("Invalid aux state.");
}

enum isl_aux_op
isl_aux_prepare_access(enum isl_aux_state initial_state,
                       enum isl_aux_usage usage,
                       bool fast_clear_supported)
{
   if (usage != ISL_AUX_USAGE_NONE) {
      UNUSED const enum isl_aux_usage state_superset_usage =
         usage == ISL_AUX_USAGE_CCS_D ? ISL_AUX_USAGE_CCS_E : usage;
      assert(aux_state_possible(initial_state, state_superset_usage));
   }
   assert(!fast_clear_supported || info[usage].fast_clear);

   switch (initial_state) {
   case ISL_AUX_STATE_COMPRESSED_CLEAR:
      if (!info[usage].compressed)
         return ISL_AUX_OP_FULL_RESOLVE;
      FALLTHROUGH;
   case ISL_AUX_STATE_CLEAR:
   case ISL_AUX_STATE_PARTIAL_CLEAR:
      return fast_clear_supported ?
                ISL_AUX_OP_NONE :
             info[usage].partial_resolve ?
                ISL_AUX_OP_PARTIAL_RESOLVE : ISL_AUX_OP_FULL_RESOLVE;
   case ISL_AUX_STATE_COMPRESSED_NO_CLEAR:
      return info[usage].compressed ?
             ISL_AUX_OP_NONE : ISL_AUX_OP_FULL_RESOLVE;
   case ISL_AUX_STATE_RESOLVED:
   case ISL_AUX_STATE_PASS_THROUGH:
      return ISL_AUX_OP_NONE;
   case ISL_AUX_STATE_AUX_INVALID:
      return info[usage].write_behavior == WRITES_ONLY_TOUCH_MAIN ?
             ISL_AUX_OP_NONE : ISL_AUX_OP_AMBIGUATE;
#ifdef IN_UNIT_TEST
   case ISL_AUX_STATE_ASSERT:
      break;
#endif
   }

   unreachable("Invalid aux state.");
}

enum isl_aux_state
isl_aux_state_transition_aux_op(enum isl_aux_state initial_state,
                                enum isl_aux_usage usage,
                                enum isl_aux_op op)
{
   assert(aux_state_possible(initial_state, usage));
   assert(usage != ISL_AUX_USAGE_NONE || op == ISL_AUX_OP_NONE);

   switch (op) {
   case ISL_AUX_OP_NONE:
      return initial_state;
   case ISL_AUX_OP_FAST_CLEAR:
      assert(info[usage].fast_clear);
      return ISL_AUX_STATE_CLEAR;
   case ISL_AUX_OP_PARTIAL_RESOLVE:
      assert(isl_aux_state_has_valid_aux(initial_state));
      assert(info[usage].partial_resolve);
      return initial_state == ISL_AUX_STATE_CLEAR ||
             initial_state == ISL_AUX_STATE_PARTIAL_CLEAR ||
             initial_state == ISL_AUX_STATE_COMPRESSED_CLEAR ?
             ISL_AUX_STATE_COMPRESSED_NO_CLEAR : initial_state;
   case ISL_AUX_OP_FULL_RESOLVE:
      assert(isl_aux_state_has_valid_aux(initial_state));
      return info[usage].full_resolves_ambiguate ||
             initial_state == ISL_AUX_STATE_PASS_THROUGH ?
             ISL_AUX_STATE_PASS_THROUGH : ISL_AUX_STATE_RESOLVED;
   case ISL_AUX_OP_AMBIGUATE:
      return ISL_AUX_STATE_PASS_THROUGH;
#if IN_UNIT_TEST
   case ISL_AUX_OP_ASSERT:
      break;
#endif
   }

   unreachable("Invalid aux op.");
}

enum isl_aux_state
isl_aux_state_transition_write(enum isl_aux_state initial_state,
                               enum isl_aux_usage usage,
                               bool full_surface)
{
   if (info[usage].write_behavior == WRITES_ONLY_TOUCH_MAIN) {
      assert(full_surface || isl_aux_state_has_valid_primary(initial_state));

      return initial_state == ISL_AUX_STATE_PASS_THROUGH ?
             ISL_AUX_STATE_PASS_THROUGH : ISL_AUX_STATE_AUX_INVALID;
   }

   assert(isl_aux_state_has_valid_aux(initial_state));
   assert(aux_state_possible(initial_state, usage));
   assert(info[usage].write_behavior == WRITES_COMPRESS ||
          info[usage].write_behavior == WRITES_COMPRESS_CLEAR ||
          info[usage].write_behavior == WRITES_RESOLVE_AMBIGUATE);

   if (full_surface) {
      return info[usage].write_behavior == WRITES_COMPRESS ?
                ISL_AUX_STATE_COMPRESSED_NO_CLEAR :
             info[usage].write_behavior == WRITES_COMPRESS_CLEAR ?
                ISL_AUX_STATE_COMPRESSED_CLEAR : ISL_AUX_STATE_PASS_THROUGH;
   }

   switch (initial_state) {
   case ISL_AUX_STATE_CLEAR:
   case ISL_AUX_STATE_PARTIAL_CLEAR:
      return info[usage].write_behavior == WRITES_RESOLVE_AMBIGUATE ?
             ISL_AUX_STATE_PARTIAL_CLEAR : ISL_AUX_STATE_COMPRESSED_CLEAR;
   case ISL_AUX_STATE_RESOLVED:
   case ISL_AUX_STATE_PASS_THROUGH:
   case ISL_AUX_STATE_COMPRESSED_NO_CLEAR:
      return info[usage].write_behavior == WRITES_COMPRESS ?
                ISL_AUX_STATE_COMPRESSED_NO_CLEAR :
             info[usage].write_behavior == WRITES_COMPRESS_CLEAR ?
                ISL_AUX_STATE_COMPRESSED_CLEAR : initial_state;
   case ISL_AUX_STATE_COMPRESSED_CLEAR:
   case ISL_AUX_STATE_AUX_INVALID:
      return initial_state;
#ifdef IN_UNIT_TEST
   case ISL_AUX_STATE_ASSERT:
      break;
#endif
   }

   unreachable("Invalid aux state.");
}

bool
isl_aux_usage_has_fast_clears(enum isl_aux_usage usage)
{
   return info[usage].fast_clear;
}

bool
isl_aux_usage_has_compression(enum isl_aux_usage usage)
{
   return info[usage].compressed;
}
