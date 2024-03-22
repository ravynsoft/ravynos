/*
 * Copyright 2023 Valve Corporation
 * Copyright 2014 Connor Abbott
 * SPDX-License-Identifier: MIT
 */

#ifndef NIR_LEGACY_H
#define NIR_LEGACY_H

#include "nir.h"

typedef struct {
   nir_def *handle;
   /** NULL for no indirect offset */
   nir_def *indirect;
   unsigned base_offset;
} nir_reg_src;

typedef struct {
   nir_def *handle;
   /** NULL for no indirect offset */
   nir_def *indirect;
   unsigned base_offset;
} nir_reg_dest;

typedef struct {
   bool is_ssa;

   union {
      nir_reg_src reg;
      nir_def *ssa;
   };
} nir_legacy_src;

typedef struct {
   bool is_ssa;

   union {
      nir_reg_dest reg;
      nir_def *ssa;
   };
} nir_legacy_dest;

typedef struct {
   /* Base source */
   nir_legacy_src src;

   /* For inputs interpreted as floating point, flips the sign bit. */
   bool fneg;

   /* Clears the sign bit for floating point values, Note that the negate
    * modifier acts after the absolute value modifier, therefore if both are set
    * then all inputs will become negative.
    */
   bool fabs;

   uint8_t swizzle[NIR_MAX_VEC_COMPONENTS];
} nir_legacy_alu_src;

typedef struct {
   /* Base destination */
   nir_legacy_dest dest;

   nir_component_mask_t write_mask;

   /**
    * Saturate output modifier
    *
    * Only valid for opcodes that output floating-point numbers. Clamps the
    * output to between 0.0 and 1.0 inclusive.
    */
   bool fsat;
} nir_legacy_alu_dest;

/* Prepare shader for use with legacy helpers. Must call on final NIR. */
void nir_legacy_trivialize(nir_shader *s, bool fuse_fabs);

/* Reconstruct a legacy source/destination (including registers) */
nir_legacy_src nir_legacy_chase_src(const nir_src *src);
nir_legacy_dest nir_legacy_chase_dest(nir_def *def);

/* Reconstruct a legacy ALU source/destination (including float modifiers) */
nir_legacy_alu_src nir_legacy_chase_alu_src(const nir_alu_src *src,
                                            bool fuse_fabs);
nir_legacy_alu_dest nir_legacy_chase_alu_dest(nir_def *def);

/* Check if a source modifier folds. If so, it may be skipped during instruction
 * selection, avoiding the need for backend dead code elimination.
 */
bool nir_legacy_float_mod_folds(nir_alu_instr *mod);

/* Check if an fsat destination modifier folds. If so, it must be skipped to
 * avoid multiple conflicting writes to the same definition.
 */
bool nir_legacy_fsat_folds(nir_alu_instr *fsat);

#endif
