#name: x32 size 2
#source: ../size-2.s
#objdump: -dwr

.*: +file format .*


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
#pass
