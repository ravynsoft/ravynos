/* incorrect usage of movprfx, predicated instruction not followed by predicated
   instruction at PC+4.  Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/m, z3.s
   orr z1.d, z3.d, z2.d
   ret
