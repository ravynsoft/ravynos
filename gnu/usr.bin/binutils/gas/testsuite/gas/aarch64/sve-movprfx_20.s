/* unpredicated movprfx cannot be used with a unpredicated SVE instruction at
   PC+4 that is not valid for use following a movprfx.
   Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z3
   add z1.s, z2.s, z2.s
   ret
