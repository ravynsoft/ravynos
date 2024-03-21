#as: --32 --divide
#objdump: -dw
#name: i386 fpu bad opcodes

.*: +file format .*

Disassembly of section .text:

0+ <start>:
 +[a-f0-9]+:	dd f0                	\(bad\)
#pass
