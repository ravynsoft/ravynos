#objdump: -dr
#as: -march=armv8.3-a

.*:     file .*

Disassembly of section \.text:

0+ <.*>:
   0:	d5182100 	msr	apiakeylo_el1, x0
   4:	d5382100 	mrs	x0, apiakeylo_el1
   8:	d5182121 	msr	apiakeyhi_el1, x1
   c:	d5382121 	mrs	x1, apiakeyhi_el1
  10:	d5182142 	msr	apibkeylo_el1, x2
  14:	d5382142 	mrs	x2, apibkeylo_el1
  18:	d5182163 	msr	apibkeyhi_el1, x3
  1c:	d5382163 	mrs	x3, apibkeyhi_el1
  20:	d5182204 	msr	apdakeylo_el1, x4
  24:	d5382204 	mrs	x4, apdakeylo_el1
  28:	d5182225 	msr	apdakeyhi_el1, x5
  2c:	d5382225 	mrs	x5, apdakeyhi_el1
  30:	d5182246 	msr	apdbkeylo_el1, x6
  34:	d5382246 	mrs	x6, apdbkeylo_el1
  38:	d5182267 	msr	apdbkeyhi_el1, x7
  3c:	d5382267 	mrs	x7, apdbkeyhi_el1
  40:	d5182308 	msr	apgakeylo_el1, x8
  44:	d5382308 	mrs	x8, apgakeylo_el1
  48:	d5182329 	msr	apgakeyhi_el1, x9
  4c:	d5382329 	mrs	x9, apgakeyhi_el1
