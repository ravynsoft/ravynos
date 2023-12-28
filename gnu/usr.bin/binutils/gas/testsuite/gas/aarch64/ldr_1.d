#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	58000001 	ldr	x1, 0 <bar>
			0: R_AARCH64_(P32_|)LD_PREL_LO19	bar\+0x100000
