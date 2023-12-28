#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	8a000000 	and	x0, x0, x0
   4:	92400000 	and	x0, x0, #0x1
   8:	d2800004 	mov	x4, #0x0                   	// #0
			8: R_AARCH64_(P32_|)MOVW_PREL_G0	tempy
   c:	d2800011 	mov	x17, #0x0                   	// #0
			c: R_AARCH64_(P32_|)MOVW_PREL_G0	tempy2
