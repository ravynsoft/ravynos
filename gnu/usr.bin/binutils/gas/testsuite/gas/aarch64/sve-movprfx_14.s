/* incorrect usage of movprfx, predicate register not the same as movprfx.
   Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/m, z3.s
   neg z1.s, p0/m, z2.s
   ret
