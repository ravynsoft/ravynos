#as: --64
#objdump: -dw
#name: 64bit bad opcodes

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
 +[a-f0-9]+:	c5 ac 46 f5          	kxnorw %k5,\(bad\),%k6
 +[a-f0-9]+:	c5 2c 46 f5          	kxnorw %k5,\(bad\),\(bad\)
#pass
