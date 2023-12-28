#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	f2800010 	movk	x16, #0x0
			0: R_AARCH64_(P32_|)TLSLD_MOVW_DTPREL_G0_NC	x
