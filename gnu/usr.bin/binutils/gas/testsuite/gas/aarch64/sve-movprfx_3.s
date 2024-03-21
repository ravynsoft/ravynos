/* Prefixed register used as input on valid sve instruction at PC+4.
   Diagnostic required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   neg z1.s, p0/m, z1.s
   ret
