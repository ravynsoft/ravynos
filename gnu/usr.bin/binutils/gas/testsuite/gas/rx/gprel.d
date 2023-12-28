#source: ./gprel.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	ce f1 04 00                   	mov\.b	4\[r15\], r1
			2: R_RX_SYM	_foo
			2: R_RX_SYM	__gp
			2: R_RX_OPsub	__gp
			2: R_RX_ABS16U	__gp
   4:	ce f1 08 00                   	mov\.b	8\[r15\], r1
			6: R_RX_SYM	_bar
			6: R_RX_SYM	__gp
			6: R_RX_OPsub	__gp
			6: R_RX_ABS16U	__gp
   8:	ce f1 00 00                   	mov\.b	0\[r15\], r1
			a: R_RX_SYM	_grill
			a: R_RX_SYM	__gp
			a: R_RX_OPsub	__gp
			a: R_RX_ABS16U	__gp
   c:	de f1 02 00                   	mov\.w	4\[r15\], r1
			e: R_RX_SYM	_foo
			e: R_RX_SYM	__gp
			e: R_RX_OPsub	__gp
			e: R_RX_ABS16UW	__gp
  10:	de f1 04 00                   	mov\.w	8\[r15\], r1
			12: R_RX_SYM	_bar
			12: R_RX_SYM	__gp
			12: R_RX_OPsub	__gp
			12: R_RX_ABS16UW	__gp
  14:	de f1 00 00                   	mov\.w	0\[r15\], r1
			16: R_RX_SYM	_grill
			16: R_RX_SYM	__gp
			16: R_RX_OPsub	__gp
			16: R_RX_ABS16UW	__gp
  18:	ee f1 01 00                   	mov\.l	4\[r15\], r1
			1a: R_RX_SYM	_foo
			1a: R_RX_SYM	__gp
			1a: R_RX_OPsub	__gp
			1a: R_RX_ABS16UL	__gp
  1c:	ee f1 02 00                   	mov\.l	8\[r15\], r1
			1e: R_RX_SYM	_bar
			1e: R_RX_SYM	__gp
			1e: R_RX_OPsub	__gp
			1e: R_RX_ABS16UL	__gp
  20:	ee f1 00 00                   	mov\.l	0\[r15\], r1
			22: R_RX_SYM	_grill
			22: R_RX_SYM	__gp
			22: R_RX_OPsub	__gp
			22: R_RX_ABS16UL	__gp
