#source: fence-as-lock-add.s
#as: -mfence-as-lock-add=no
#objdump: -dw
#name: i386  fence as lock add = no

.*: +file format .*i386.*

Disassembly of section .text:

0+ <main>:
[   ]*[a-f0-9]+:	0f ae e8[ ]*	lfence
[   ]*[a-f0-9]+:	0f ae f0[ ]*	mfence
[   ]*[a-f0-9]+:	0f ae f8[ ]*	sfence
#pass
