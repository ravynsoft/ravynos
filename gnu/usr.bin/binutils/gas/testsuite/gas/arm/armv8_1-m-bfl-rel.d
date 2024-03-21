#name: Valid Armv8.1-M Mainline BFL instruction with REL
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-wince-* *-vxworks

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f0ff c7ff 	bfl	2, 00000000 <.target>
			0: R_ARM_THM_BF18	.target
