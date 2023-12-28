#name: i386 size 4
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
#pass
