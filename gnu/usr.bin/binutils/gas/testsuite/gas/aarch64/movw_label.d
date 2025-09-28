#objdump: -dr
#name: movw relocation symbol name
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	f2800002 	movk	x2, #0x0
			0: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	x3.22
   4:	f2800002 	movk	x2, #0x0
			4: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	x8
   8:	f2800002 	movk	x2, #0x0
			8: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	w3
   c:	f2800002 	movk	x2, #0x0
			c: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	w8.22
  10:	f2800002 	movk	x2, #0x0
			10: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	sp
  14:	f2800002 	movk	x2, #0x0
			14: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	wzr
  18:	f2800002 	movk	x2, #0x0
			18: R_AARCH64_(P32_|)MOVW_UABS_G0_NC	xzr
