#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x00000000 2100 0080           	add	r0,r1,r2
0x00000004 256f 003f           	brk
0x00000008 2402 0143           	sub	r3,r4,r5
