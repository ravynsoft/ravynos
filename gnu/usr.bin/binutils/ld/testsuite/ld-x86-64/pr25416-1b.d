#name: X32 GDesc -> LE 1
#source: pr25416-1.s
#as: --x32
#ld: -melf32_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	40 c7 c0 ([0-9a-f]{2} ){4}[ \t]+rex mov \$0x[a-f0-9]+,%eax
 +[a-f0-9]+:	0f 1f 00             	nopl   \(%rax\)
#pass
