#name: Valid Armv8.1-M Mainline BF instruction with RELA
#as: -march=armv8.1-m.main
#objdump: -dr --prefix-addresses --show-raw-insn
#source:armv8_1-m-bf-rel.s
#noskip: *-vxworks

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> f0c0 e001 	bf	2, 00000004 <.target\+0x4>
			0: R_ARM_THM_BF16	.target-0x4

