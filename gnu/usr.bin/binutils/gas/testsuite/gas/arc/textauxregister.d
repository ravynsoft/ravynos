#as: -mcpu=arcem
#objdump: -dr

.*: +file format .*arc.*


Disassembly of section .text:

[0-9a-f]+ <.text>:
   0:	2067 0c00           	aex	r0,\[mlx\]
   4:	216a 0c40           	lr	r1,\[mly\]
   8:	266b 7c00 0000 0012 	sr	0x12,\[mlx\]
