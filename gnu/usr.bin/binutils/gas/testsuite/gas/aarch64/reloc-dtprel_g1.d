#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	d2a00009 	movz	x9, #0x0, lsl #16
			0: R_AARCH64_(P32_|)TLSLD_MOVW_DTPREL_G1	x
