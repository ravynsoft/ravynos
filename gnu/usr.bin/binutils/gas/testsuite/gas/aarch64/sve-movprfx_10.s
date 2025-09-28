/* incorrect usage of movprfx, register size is different.
   Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/M, z0.s
   neg z1.d, p1/m, z2.d
   ret
