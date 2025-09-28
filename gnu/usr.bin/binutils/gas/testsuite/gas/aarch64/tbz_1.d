#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	36080000 	tbz	w0, #1, 0 <bar>
			0: R_AARCH64_(P32_|)TSTBR14	bar\+0x8000
