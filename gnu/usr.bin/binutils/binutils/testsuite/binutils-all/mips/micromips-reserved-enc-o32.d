#PROG: objcopy
#objdump: -d --prefix-addresses --show-raw-insn
#name: microMIPS source file contains reserved encoding (o32)
#source: micromips-reserved-enc.s

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7f6e 5d4c 	\.short	0x7f6e, 0x5d4c
	\.\.\.
