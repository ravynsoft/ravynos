#as: -mabi=lp64
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	10000001 	adr	x1, 0 <bar>
			0: R_AARCH64_ADR_PREL_LO21	bar\+0x80000000
