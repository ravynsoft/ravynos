#source: fence-as-lock-add.s
#as: -mfence-as-lock-add=yes
#objdump: -dw
#name: x86-64 fence as lock add = yes

.*: +file format .*

Disassembly of section .text:

0+ <main>:
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%rsp\)
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%rsp\)
[   ]*[a-f0-9]+:	f0 83 04 24 00[ ]*	lock addl \$0x0,\(%rsp\)
#pass
