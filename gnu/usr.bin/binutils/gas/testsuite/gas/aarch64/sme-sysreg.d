#name: SME extension (system registers)
#as: -march=armv8-a+sme
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d53b4240 	mrs	x0, svcr
   4:	d53804a0 	mrs	x0, id_aa64smfr0_el1
   8:	d53812c0 	mrs	x0, smcr_el1
   c:	d53d12c0 	mrs	x0, smcr_el12
  10:	d53c12c0 	mrs	x0, smcr_el2
  14:	d53e12c0 	mrs	x0, smcr_el3
  18:	d5381280 	mrs	x0, smpri_el1
  1c:	d53c12a0 	mrs	x0, smprimap_el2
  20:	d53900c0 	mrs	x0, smidr_el1
  24:	d53bd0a0 	mrs	x0, tpidr2_el0
  28:	d538a560 	mrs	x0, mpamsm_el1
  2c:	d51b4240 	msr	svcr, x0
  30:	d51812c0 	msr	smcr_el1, x0
  34:	d51d12c0 	msr	smcr_el12, x0
  38:	d51c12c0 	msr	smcr_el2, x0
  3c:	d51e12c0 	msr	smcr_el3, x0
  40:	d5181280 	msr	smpri_el1, x0
  44:	d51c12a0 	msr	smprimap_el2, x0
  48:	d51bd0a0 	msr	tpidr2_el0, x0
  4c:	d518a560 	msr	mpamsm_el1, x0
