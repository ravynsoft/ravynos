#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	91000341 	add	x1, x26, #0x0
			0: R_AARCH64_(P32_|)TLSLD_ADD_DTPREL_HI12	x
