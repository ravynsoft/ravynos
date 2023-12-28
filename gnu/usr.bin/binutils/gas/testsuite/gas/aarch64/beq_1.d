#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	54000000 	b\.eq	0 <bar>  // b\.none
			0: R_AARCH64_(P32_|)CONDBR19	bar\+0x100000
