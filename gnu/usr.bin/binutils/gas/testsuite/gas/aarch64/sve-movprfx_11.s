/* Correct usage of movprfx, unpredicated movprfx, any register size allowed for
   instruction at PC+4.  No diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   neg z1.d, p0/m, z2.d
   ret
