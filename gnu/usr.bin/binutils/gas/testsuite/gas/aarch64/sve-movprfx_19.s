/* unpredicated movprfx can be used with a predicated SVE instruction at PC+4
   with a merging predicate.  No diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z3
   add z1.s, p1/m, z1.s, z2.s
   ret
