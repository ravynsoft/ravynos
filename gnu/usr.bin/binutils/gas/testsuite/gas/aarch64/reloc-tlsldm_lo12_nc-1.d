#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	91000000 	add	x0, x0, #0x0
			0: R_AARCH64_(P32_|)TLSLD_ADD_LO12_NC	x

