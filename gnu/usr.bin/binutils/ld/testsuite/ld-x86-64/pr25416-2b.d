#name: X32 GDesc -> LE 2
#source: pr25416-2.s
#as: --x32
#ld: -melf32_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	48 c7 c0 ([0-9a-f]{2} ){4}[ \t]+mov    \$0x[a-f0-9]+,%rax
 +[a-f0-9]+:	66 90                	xchg   %ax,%ax
#pass
