/* predicated movprfx must be used with a predicated SVE instruction at PC+4.
   Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1.s, p1/m, z3.s
   add z1.s, z1.s, #10
   ret
