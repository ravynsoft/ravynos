#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	8a000000 	and	x0, x0, x0
   4:	92400000 	and	x0, x0, #0x1
   8:	f2c00004 	movk	x4, #0x0, lsl #32
			8: R_AARCH64_MOVW_PREL_G2_NC	tempy
   c:	f2c00007 	movk	x7, #0x0, lsl #32
			c: R_AARCH64_MOVW_PREL_G2_NC	tempy2
  10:	f2c00011 	movk	x17, #0x0, lsl #32
			10: R_AARCH64_MOVW_PREL_G2_NC	tempy3
