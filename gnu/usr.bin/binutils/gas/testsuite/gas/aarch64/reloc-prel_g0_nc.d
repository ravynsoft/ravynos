#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	8a000000 	and	x0, x0, x0
   4:	92400000 	and	x0, x0, #0x1
   8:	f2800004 	movk	x4, #0x0
			8: R_AARCH64_(P32_|)MOVW_PREL_G0_NC	tempy
   c:	f2800007 	movk	x7, #0x0
			c: R_AARCH64_(P32_|)MOVW_PREL_G0_NC	tempy2
  10:	f2800011 	movk	x17, #0x0
			10: R_AARCH64_(P32_|)MOVW_PREL_G0_NC	tempy3
