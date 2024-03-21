/* Checks that CPY is allowed after a movprfx, special case in that SIMD&Scalar
   version is also valid and Pg is 4 bits rather than 3.
   Has invalid usages.  Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
  /* OK, immediate predicated, alias mov.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, #12

  /* OK, immediate predicated, alias mov, fmov.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, #0

  /* OK, immediate predicated, alias mov.  */
   movprfx z1.d, p1/m, z3.d
   fmov z1.d, p1/m, #0

  /* Not OK, immediate predicated, but different predicate registers.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p9/m, #12

  /* Not OK, zeroing predicate.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/z, #12

  /* OK, scalar predicated, alias mov.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, x2

   /* OK, scalar predicated, alias mov.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, x1

   /* OK, SIMD&FP predicated, alias mov  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, d2

   /* Not OK, SIMD&FP predicated, but register d1 is architecturally the
      same.  */
   movprfx z1.d, p1/m, z3.d
   cpy z1.d, p1/m, d1
   ret

