/* Only predicated vector BIC is allowed following a movprfx, and some pseudo
   instructions should be allowed due to the instructions they alias.
   Has invalid usages.  Diagnosis required.  */
  .text
  .arch armv8-a+sve

f:
  /* OK, vectored predicated.  */
   movprfx z1.D, p1/m, z3.D
   bic z1.D, p1/M, z1.D, z2.D

  /* Not OK, vectored unpredicated.  */
   movprfx z1.D, p1/m, z3.D
   bic z1.D, z1.D, z2.D

  /* Not OK, vectored unpredicated.  */
   movprfx z1, z3
   bic z1.D, z1.D, z2.D

  /* Not OK, immediate form, unpredicated.  */
   movprfx z1.D, p1/m, z3.D
   bic z1.D, z1.D, #12

  /* OK, immediate form alias of AND which is allowed.  */
   movprfx z1, z3
   bic z1.D, z1.D, #12

  /* OK, immediate form alias of EOR which is allowed.  */
   movprfx z1, z3
   eon z1.D, z1.D, #12

  /* OK, immediate form alias of ORR which is allowed.  */
   movprfx z1, z3
   orr z1.D, z1.D, #12
   ret

