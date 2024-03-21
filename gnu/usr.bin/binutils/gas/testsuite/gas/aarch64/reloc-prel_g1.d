#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	8a000000 	and	x0, x0, x0
   4:	92400000 	and	x0, x0, #0x1
   8:	d2a00004 	movz	x4, #0x0, lsl #16
			8: R_AARCH64_(P32_|)MOVW_PREL_G1	tempy
   c:	d2a00011 	movz	x17, #0x0, lsl #16
			c: R_AARCH64_(P32_|)MOVW_PREL_G1	tempy2
