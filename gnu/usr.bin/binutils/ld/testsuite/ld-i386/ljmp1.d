#name: Absolute non-overflowing relocs in ljmp segments
#as: --32
#source: ljmp1.s
#source: ljmp.s
#ld: -melf_i386 -z noseparate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+[a-f0-9]+ <_start>:
 +[a-f0-9]+:	0f 22 c0             	mov    %eax,%cr0
 +[a-f0-9]+:	ea ([0-9a-f]{2} ){4}08 00 	ljmp   \$0x8,\$0x[a-f0-9]+

0+[a-f0-9]+ <foo>:
 +[a-f0-9]+:	66 ea 00 00 18 00    	ljmpw  \$0x18,\$0x0
