#as: -mcpu=arc700
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*

Disassembly of section .text:
0x[0-9a-f]+ 6178                	add_s	r0,r1,r3
0x[0-9a-f]+ 70e7                	add_s	r0,r0,pcl
