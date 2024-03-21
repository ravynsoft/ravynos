#name: x86-64 size 2
#source: size-2.s
#objdump: -dhwr

.*: +file format .*

Sections:
Idx Name +Size .*
  0 \.text +0*3c .*
  1 \.data +0*5c .*
  2 \.bss +0*199999(ae|b0) .*
#...
Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	b8 50 00 00 00       	mov    \$0x50,%eax
[ 	]*[a-f0-9]+:	b8 48 00 00 00       	mov    \$0x48,%eax
[ 	]*[a-f0-9]+:	b8 58 00 00 00       	mov    \$0x58,%eax
[ 	]*[a-f0-9]+:	b8 1e 00 00 00       	mov    \$0x1e,%eax
[ 	]*[a-f0-9]+:	b8 0e 00 00 00       	mov    \$0xe,%eax
[ 	]*[a-f0-9]+:	b8 2e 00 00 00       	mov    \$0x2e,%eax
[ 	]*[a-f0-9]+:	b8 90 99 99 19       	mov    \$0x19999990,%eax
[ 	]*[a-f0-9]+:	b8 70 99 99 19       	mov    \$0x19999970,%eax
[ 	]*[a-f0-9]+:	b8 b0 99 99 19       	mov    \$0x199999b0,%eax
[ 	]*[a-f0-9]+:	b8 3c 00 00 00       	mov    \$0x3c,%eax
[ 	]*[a-f0-9]+:	b8 60 00 00 00       	mov    \$0x60,%eax
[ 	]*[a-f0-9]+:	b8 (ae|b0) 99 99 09       	mov    \$0x99999(ae|b0),%eax
#pass
