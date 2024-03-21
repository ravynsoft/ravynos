#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	8b030041 	add	x1, x2, x3
   4:	10000000 	adr	x0, 0 <dummy>
			4: R_AARCH64_(P32_|)TLSLD_ADR_PREL21	dummy
