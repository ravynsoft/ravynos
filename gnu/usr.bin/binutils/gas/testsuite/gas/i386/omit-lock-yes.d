#source: omit-lock.s
#as: -momit-lock-prefix=yes
#objdump: -dw
#name: i386  omit lock = yes

.*: +file format .*i386.*

Disassembly of section .text:

0+ <main>:
   0:	83 00 01             	addl   \$0x1,\(%eax\)
#pass
