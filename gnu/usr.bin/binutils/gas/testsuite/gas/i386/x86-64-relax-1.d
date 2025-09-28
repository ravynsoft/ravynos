#name: x86-64 relax 1
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
 358:	74 06                	je     (0x)?360( .*)?
 35a:	66 0f 1f 44 00 00    	nopw   0x0\(%rax,%rax,1\)
 360:	90                   	nop
#pass
