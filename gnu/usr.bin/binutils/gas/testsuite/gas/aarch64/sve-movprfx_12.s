/* correct usage of movprfx. No diagnosis.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/z, z3.s
   neg z1.s, p1/m, z2.s
   ret
