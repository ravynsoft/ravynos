#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	8a000000 	and	x0, x0, x0
   4:	92400000 	and	x0, x0, #0x1
   8:	d2e00004 	movz	x4, #0x0, lsl #48
			8: R_AARCH64_MOVW_PREL_G3	tempy
   c:	d2e00007 	movz	x7, #0x0, lsl #48
			c: R_AARCH64_MOVW_PREL_G3	tempy2
  10:	d2e00011 	movz	x17, #0x0, lsl #48
			10: R_AARCH64_MOVW_PREL_G3	tempy3
