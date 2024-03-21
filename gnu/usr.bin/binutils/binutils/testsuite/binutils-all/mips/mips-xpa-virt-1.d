#PROG: objcopy
#objdump: -d --prefix-addresses --show-raw-insn -m mips:3000 -M cp0-names=mips32
#name: MIPS XPA and Virtualization ASE instruction disassembly 1
#source: mips-xpa-virt.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 40020800 	mfc0	v0,c0_random
[0-9a-f]+ <[^>]*> 40420800 	cfc0	v0,\$1
[0-9a-f]+ <[^>]*> 40620800 	.word	0x40620800
[0-9a-f]+ <[^>]*> 40620c00 	.word	0x40620c00
	\.\.\.
