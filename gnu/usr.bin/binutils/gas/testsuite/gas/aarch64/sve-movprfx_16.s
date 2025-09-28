/* movprfx's dest register must be the same width.  Correct usage. No
   diagnosis.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/m, z3.s
   fcvt z1.s, p1/m, z2.h
   ret
