/* Instruction at PC+4 after prefix sequence opening is not an SVE instruction.
   Diagnostic required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   mov x0, sp
   neg z2.s, p0/m, z2.s
   ret
