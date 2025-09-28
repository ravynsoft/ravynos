/* Prefixed register used in valid sve instruction at PC+4. Label does not
   change flow.  No Diagnostic.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
bar:
   neg z1.s, p0/m, z2.s
   ret
