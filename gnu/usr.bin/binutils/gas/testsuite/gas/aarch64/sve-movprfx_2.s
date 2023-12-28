/* Prefixed register not used in valid sve instruction at PC+4.
   Diagnostic required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   neg z2.s, p0/m, z2.s
   ret
