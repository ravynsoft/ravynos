#objdump: -dw
#name: Disassemble bad broadcast

.*: +file format .*


Disassembly of section .text:

0+ <.text>:
 +[a-f0-9]+:	62 c3 8c 1d 66\s+\(bad\)
 +[a-f0-9]+:	90\s+nop
 +[a-f0-9]+:	66 90\s+xchg   %ax,%ax
 +[a-f0-9]+:	66 90\s+xchg   %ax,%ax
 +[a-f0-9]+:	62 c1 ff 38 2a 20\s+vcvtsi2sd \(%eax\){bad},%xmm0,%xmm4
