#source: fence-as-lock-add.s
#as: -mfence-as-lock-add=yes
#objdump: -dw
#name: i386  fence as lock add = yes

.*: +file format .*i386.*

Disassembly of section .text:

0+ <main>:
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%esp\)
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%esp\)
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%esp\)
#pass
