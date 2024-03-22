/*
 * Copyright © 2023 Collabora, Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "nak_private.h"
#include "nir_builder.h"

static nir_def *
cluster_mask(nir_builder *b, unsigned cluster_size)
{
   nir_def *mask = nir_ballot(b, 1, 32, nir_imm_true(b));

   if (cluster_size < 32) {
      nir_def *idx = nir_load_subgroup_invocation(b);
      nir_def *cluster = nir_iand_imm(b, idx, ~(uint64_t)(cluster_size - 1));

      nir_def *cluster_mask = nir_imm_int(b, BITFIELD_MASK(cluster_size));
      cluster_mask = nir_ishl(b, cluster_mask, cluster);

      mask = nir_iand(b, mask, cluster_mask);
   }

   return mask;
}

static nir_def *
build_scan_bool(nir_builder *b, nir_intrinsic_op op, nir_op red_op,
                nir_def *data, unsigned cluster_size)
{
   /* Handle a couple of special cases first */
   if (op == nir_intrinsic_reduce && cluster_size == 32) {
      switch (red_op) {
      case nir_op_iand:
         return nir_vote_all(b, 1, data);
      case nir_op_ior:
         return nir_vote_any(b, 1, data);
      case nir_op_ixor:
         /* The generic path is fine */
         break;
      default:
         unreachable("Unsupported boolean reduction op");
      }
   }

   nir_def *mask = cluster_mask(b, cluster_size);
   switch (op) {
   case nir_intrinsic_exclusive_scan:
      mask = nir_iand(b, mask, nir_load_subgroup_lt_mask(b, 1, 32));
      break;
   case nir_intrinsic_inclusive_scan:
      mask = nir_iand(b, mask, nir_load_subgroup_le_mask(b, 1, 32));
      break;
   case nir_intrinsic_reduce:
      break;
   default:
      unreachable("Unsupported scan/reduce op");
   }

   data = nir_ballot(b, 1, 32, data);

   switch (red_op) {
   case nir_op_iand:
      return nir_ieq_imm(b, nir_iand(b, nir_inot(b, data), mask), 0);
   case nir_op_ior:
      return nir_ine_imm(b, nir_iand(b, data, mask), 0);
   case nir_op_ixor: {
      nir_def *count = nir_bit_count(b, nir_iand(b, data, mask));
      return nir_ine_imm(b, nir_iand_imm(b, count, 1), 0);
   }
   default:
      unreachable("Unsupported boolean reduction op");
   }
}

static nir_def *
build_identity(nir_builder *b, unsigned bit_size, nir_op op)
{
   nir_const_value ident_const = nir_alu_binop_identity(op, bit_size);
   return nir_build_imm(b, 1, bit_size, &ident_const);
}

/* Implementation of scan/reduce that assumes a full subgroup */
static nir_def *
build_scan_full(nir_builder *b, nir_intrinsic_op op, nir_op red_op,
                nir_def *data, unsigned cluster_size)
{
   switch (op) {
   case nir_intrinsic_exclusive_scan:
   case nir_intrinsic_inclusive_scan: {
      for (unsigned i = 1; i < cluster_size; i *= 2) {
         nir_def *idx = nir_load_subgroup_invocation(b);
         nir_def *has_buddy = nir_ige_imm(b, idx, i);

         nir_def *buddy_data = nir_shuffle_up(b, data, nir_imm_int(b, i));
         nir_def *accum = nir_build_alu2(b, red_op, data, buddy_data);
         data = nir_bcsel(b, has_buddy, accum, data);
      }

      if (op == nir_intrinsic_exclusive_scan) {
         /* For exclusive scans, we need to shift one more time and fill in the
          * bottom channel with identity.
          */
         assert(cluster_size == 32);
         nir_def *idx = nir_load_subgroup_invocation(b);
         nir_def *has_buddy = nir_ige_imm(b, idx, 1);

         nir_def *buddy_data = nir_shuffle_up(b, data, nir_imm_int(b, 1));
         nir_def *identity = build_identity(b, data->bit_size, red_op);
         data = nir_bcsel(b, has_buddy, buddy_data, identity);
      }

      return data;
   }

   case nir_intrinsic_reduce: {
      for (unsigned i = 1; i < cluster_size; i *= 2) {
         nir_def *buddy_data = nir_shuffle_xor(b, data, nir_imm_int(b, i));
         data = nir_build_alu2(b, red_op, data, buddy_data);
      }
      return data;
   }

   default:
      unreachable("Unsupported scan/reduce op");
   }
}

/* Fully generic implementation of scan/reduce that takes a mask */
static nir_def *
build_scan_reduce(nir_builder *b, nir_intrinsic_op op, nir_op red_op,
                  nir_def *data, nir_def *mask, unsigned max_mask_bits)
{
   nir_def *lt_mask = nir_load_subgroup_lt_mask(b, 1, 32);

   /* Mask of all channels whose values we need to accumulate.  Our own value
    * is already in accum, if inclusive, thanks to the initialization above.
    * We only need to consider lower indexed invocations.
    */
   nir_def *remaining = nir_iand(b, mask, lt_mask);

   for (unsigned i = 1; i < max_mask_bits; i *= 2) {
      /* At each step, our buddy channel is the first channel we have yet to
       * take into account in the accumulator.
       */
      nir_def *has_buddy = nir_ine_imm(b, remaining, 0);
      nir_def *buddy = nir_ufind_msb(b, remaining);

      /* Accumulate with our buddy channel, if any */
      nir_def *buddy_data = nir_shuffle(b, data, buddy);
      nir_def *accum = nir_build_alu2(b, red_op, data, buddy_data);
      data = nir_bcsel(b, has_buddy, accum, data);

      /* We just took into account everything in our buddy's accumulator from
       * the previous step.  The only things remaining are whatever channels
       * were remaining for our buddy.
       */
      nir_def *buddy_remaining = nir_shuffle(b, remaining, buddy);
      remaining = nir_bcsel(b, has_buddy, buddy_remaining, nir_imm_int(b, 0));
   }

   switch (op) {
   case nir_intrinsic_exclusive_scan: {
      /* For exclusive scans, we need to shift one more time and fill in the
       * bottom channel with identity.
       *
       * Some of this will get CSE'd with the first step but that's okay. The
       * code is cleaner this way.
       */
      nir_def *lower = nir_iand(b, mask, lt_mask);
      nir_def *has_buddy = nir_ine_imm(b, lower, 0);
      nir_def *buddy = nir_ufind_msb(b, lower);

      nir_def *buddy_data = nir_shuffle(b, data, buddy);
      nir_def *identity = build_identity(b, data->bit_size, red_op);
      return nir_bcsel(b, has_buddy, buddy_data, identity);
   }

   case nir_intrinsic_inclusive_scan:
      return data;

   case nir_intrinsic_reduce: {
      /* For reductions, we need to take the top value of the scan */
      nir_def *idx = nir_ufind_msb(b, mask);
      return nir_shuffle(b, data, idx);
   }

   default:
      unreachable("Unsupported scan/reduce op");
   }
}

static bool
nak_nir_lower_scan_reduce_intrin(nir_builder *b,
                                 nir_intrinsic_instr *intrin,
                                 UNUSED void *_data)
{
   switch (intrin->intrinsic) {
   case nir_intrinsic_exclusive_scan:
   case nir_intrinsic_inclusive_scan:
   case nir_intrinsic_reduce:
      break;
   default:
      return false;
   }

   const nir_op red_op = nir_intrinsic_reduction_op(intrin);

   /* Grab the cluster size, defaulting to 32 */
   unsigned cluster_size = 32;
   if (nir_intrinsic_has_cluster_size(intrin)) {
      cluster_size = nir_intrinsic_cluster_size(intrin);
      if (cluster_size == 0 || cluster_size > 32)
         cluster_size = 32;
   }

   b->cursor = nir_before_instr(&intrin->instr);

   nir_def *data;
   if (cluster_size == 1) {
      /* Simple case where we're not actually doing any reducing at all. */
      assert(intrin->intrinsic == nir_intrinsic_reduce);
      data = intrin->src[0].ssa;
   } else if (intrin->src[0].ssa->bit_size == 1) {
      data = build_scan_bool(b, intrin->intrinsic, red_op,
                             intrin->src[0].ssa, cluster_size);
   } else {
      /* First, we need a mask of all invocations to be included in the
       * reduction or scan.  For trivial cluster sizes, that's just the mask
       * of enabled channels.
       */
      nir_def *mask = cluster_mask(b, cluster_size);

      nir_def *full, *partial;
      nir_push_if(b, nir_ieq_imm(b, mask, -1));
      {
         full = build_scan_full(b, intrin->intrinsic, red_op,
                                intrin->src[0].ssa, cluster_size);
      }
      nir_push_else(b, NULL);
      {
         partial = build_scan_reduce(b, intrin->intrinsic, red_op,
                                     intrin->src[0].ssa, mask, cluster_size);
      }
      nir_pop_if(b, NULL);
      data = nir_if_phi(b, full, partial);
   }

   nir_def_rewrite_uses(&intrin->def, data);
   nir_instr_remove(&intrin->instr);

   return true;
}

bool
nak_nir_lower_scan_reduce(nir_shader *nir)
{
   return nir_shader_intrinsics_pass(nir, nak_nir_lower_scan_reduce_intrin,
                                     nir_metadata_none, NULL);
}
