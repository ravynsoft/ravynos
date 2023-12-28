#objdump: -dr
#as: -march=armv8.2-a+profile

.*:     file .*


Disassembly of section .text:

0+ <.*>:
   [0-9a-f]+:	d5380725 	mrs	x5, id_aa64mmfr1_el1
   [0-9a-f]+:	d5380747 	mrs	x7, id_aa64mmfr2_el1
   [0-9a-f]+:	d5385305 	mrs	x5, erridr_el1
   [0-9a-f]+:	d5185327 	msr	errselr_el1, x7
  [0-9a-f]+:	d5385327 	mrs	x7, errselr_el1
  [0-9a-f]+:	d5385405 	mrs	x5, erxfr_el1
  [0-9a-f]+:	d5185425 	msr	erxctlr_el1, x5
  [0-9a-f]+:	d5385425 	mrs	x5, erxctlr_el1
  [0-9a-f]+:	d5185445 	msr	erxstatus_el1, x5
  [0-9a-f]+:	d5385445 	mrs	x5, erxstatus_el1
  [0-9a-f]+:	d5185465 	msr	erxaddr_el1, x5
  [0-9a-f]+:	d5385465 	mrs	x5, erxaddr_el1
  [0-9a-f]+:	d5185505 	msr	erxmisc0_el1, x5
  [0-9a-f]+:	d5385505 	mrs	x5, erxmisc0_el1
  [0-9a-f]+:	d5185525 	msr	erxmisc1_el1, x5
  [0-9a-f]+:	d5385525 	mrs	x5, erxmisc1_el1
  [0-9a-f]+:	d53c5265 	mrs	x5, vsesr_el2
  [0-9a-f]+:	d518c125 	msr	disr_el1, x5
  [0-9a-f]+:	d538c125 	mrs	x5, disr_el1
  [0-9a-f]+:	d53cc125 	mrs	x5, vdisr_el2
  [0-9a-f]+:	d50b7a20 	dc	cvac, x0
  [0-9a-f]+:	d50b7b21 	dc	cvau, x1
  [0-9a-f]+:	d50b7c22 	dc	cvap, x2
  [0-9a-f]+:	d5087900 	at	s1e1rp, x0
  [0-9a-f]+:	d5087921 	at	s1e1wp, x1
  [0-9a-f]+:	d5189a07 	msr	pmblimitr_el1, x7
  [0-9a-f]+:	d5389a07 	mrs	x7, pmblimitr_el1
  [0-9a-f]+:	d5189a27 	msr	pmbptr_el1, x7
  [0-9a-f]+:	d5389a27 	mrs	x7, pmbptr_el1
  [0-9a-f]+:	d5189a67 	msr	pmbsr_el1, x7
  [0-9a-f]+:	d5389a67 	mrs	x7, pmbsr_el1
  [0-9a-f]+:	d5189907 	msr	pmscr_el1, x7
  [0-9a-f]+:	d5389907 	mrs	x7, pmscr_el1
  [0-9a-f]+:	d5189947 	msr	pmsicr_el1, x7
  [0-9a-f]+:	d5389947 	mrs	x7, pmsicr_el1
  [0-9a-f]+:	d5189967 	msr	pmsirr_el1, x7
  [0-9a-f]+:	d5389967 	mrs	x7, pmsirr_el1
  [0-9a-f]+:	d5189987 	msr	pmsfcr_el1, x7
  [0-9a-f]+:	d5389987 	mrs	x7, pmsfcr_el1
  [0-9a-f]+:	d51899a7 	msr	pmsevfr_el1, x7
  [0-9a-f]+:	d53899a7 	mrs	x7, pmsevfr_el1
  [0-9a-f]+:	d51899c7 	msr	pmslatfr_el1, x7
  [0-9a-f]+:	d53899c7 	mrs	x7, pmslatfr_el1
  [0-9a-f]+:	d51c9907 	msr	pmscr_el2, x7
  [0-9a-f]+:	d53c9907 	mrs	x7, pmscr_el2
  [0-9a-f]+:	d51d9907 	msr	pmscr_el12, x7
  [0-9a-f]+:	d53d9907 	mrs	x7, pmscr_el12
  [0-9a-f]+:	d5389ae7 	mrs	x7, pmbidr_el1
  [0-9a-f]+:	d53899e7 	mrs	x7, pmsidr_el1
