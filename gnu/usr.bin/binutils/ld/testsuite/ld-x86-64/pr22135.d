#as: --64
#ld: -pie -melf_x86_64 --no-keep-memory
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	8d 05 ([0-9a-f]{2} ){4} *	lea    -0x[a-f0-9]+\(%rip\),%eax        # [a-f0-9]+ <foo>
#pass
