#as: -mtune=i686
#source: nops-3.s
#objdump: -drw
#name: i386 -mtune=i686 nops 3

.*: +file format .*

Disassembly of section .text:

0+ <nop>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 1d                	jmp    20 <nop\+0x20>
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	89 c3                	mov    %eax,%ebx
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
[ 	]*[a-f0-9]+:	8d b4 26 00 00 00 00 	lea    0x0\(%esi,%eiz,1\),%esi
#pass
