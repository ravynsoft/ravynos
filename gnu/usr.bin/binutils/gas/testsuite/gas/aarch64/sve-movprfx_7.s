/* New section started without sequence closed. Diagnostic required.  */
  .text
  .arch armv8-a+sve

f:
   ptrue p0.s
   movprfx z1, z0
   .section foo
g:
   ret
