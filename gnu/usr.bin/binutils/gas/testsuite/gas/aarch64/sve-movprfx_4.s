/* Prefixed register not used, movprfx last instruction.
   Diagnostic required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
