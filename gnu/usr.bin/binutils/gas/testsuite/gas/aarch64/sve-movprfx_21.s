/* movprfx dest register can be used in destructive operations where required.
   Valid usage.  No diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z3
   add z1.s, p1/m, z1.s, z2.s
   ret
