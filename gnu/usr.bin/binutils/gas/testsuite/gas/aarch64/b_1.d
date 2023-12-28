#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	14000000 	b	0 <bar>
			0: R_AARCH64_(P32_|)JUMP26	bar\+0x8000000
