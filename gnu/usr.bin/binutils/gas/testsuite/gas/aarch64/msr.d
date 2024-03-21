#as: -march=armv8.2-a+profile
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d50340df 	msr	daifset, #0x0
   4:	d50341df 	msr	daifset, #0x1
   8:	d5034fdf 	msr	daifset, #0xf
   c:	d50340ff 	msr	daifclr, #0x0
  10:	d50341ff 	msr	daifclr, #0x1
  14:	d5034fff 	msr	daifclr, #0xf
  18:	d51b4220 	msr	daif, x0
  1c:	d53b4220 	mrs	x0, daif
  20:	d50040bf 	msr	spsel, #0x0
  24:	d50041bf 	msr	spsel, #0x1
  28:	d51a0000 	msr	csselr_el1, x0
  2c:	d53a0000 	mrs	x0, csselr_el1
  30:	d51c5260 	msr	vsesr_el2, x0
  34:	d53c5260 	mrs	x0, vsesr_el2
  38:	d5100040 	msr	osdtrrx_el1, x0
  3c:	d5300040 	mrs	x0, osdtrrx_el1
  40:	d5100340 	msr	osdtrtx_el1, x0
  44:	d5300340 	mrs	x0, osdtrtx_el1
  48:	d53899e0 	mrs	x0, pmsidr_el1
