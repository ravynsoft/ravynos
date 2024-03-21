#source: omit-lock.s
#as: -momit-lock-prefix=yes  -momit-lock-prefix=no
#objdump: -dw
#name: i386  omit lock = no

.*: +file format .*i386.*

Disassembly of section .text:

0+ <main>:
   0:	f0 f0 83 00 01       	lock lock addl \$0x1,\(%eax\)
#pass
