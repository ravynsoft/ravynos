/* Instructions not allowed to be used with predicated movprfx. Invalid usage.
   Diagnosis required.  */
  .text
  .arch armv8-a+sve

   /* All of these should be invalid because the predicated movprfx is used
      with an unpredicated instruction.  */

   .macro test_sametwo inst
   .irp sz, h,s,d
   movprfx z1.\sz, p1/m, z3.\sz
   \inst z1.\sz, p1.\sz
   .endr
   .endm

   .macro test_samethree inst
   .irp sz, b,h,s,d
   movprfx z1.\sz, p1/m, z3.\sz
   \inst z1.\sz, p1, z1.\sz, z1.\sz
   .endr
   .endm


f:
   test_sametwo incp
   test_sametwo decp

   test_sametwo sqincp
   test_sametwo sqdecp

   test_samethree clasta
   test_samethree clastb
   ret

