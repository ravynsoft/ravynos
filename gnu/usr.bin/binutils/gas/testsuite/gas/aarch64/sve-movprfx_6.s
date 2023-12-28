/* Prefixed register used in valid sve instruction at PC+4, but used as input
   as well.  Prefix block opened twice without closing first one.
   Two diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   movprfx z2, z3
   neg z2.s, p0/m, z2.s
   ret
