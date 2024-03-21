#objdump: -drw -Mi8086
#name: i386 .nops 2

.*: +file format .*


Disassembly of section .text:

0+ <single>:
 +[a-f0-9]+:	90                   	nop

0+1 <pseudo_1>:
 +[a-f0-9]+:	90                   	nop

0+2 <pseudo_8>:
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+a <pseudo_8_4>:
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+12 <pseudo_20>:
 +[a-f0-9]+:	eb 12                	jmp    26 <pseudo_30>
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+26 <pseudo_30>:
 +[a-f0-9]+:	eb 1c                	jmp    44 <pseudo_129>
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+44 <pseudo_129>:
 +[a-f0-9]+:	eb 7f                	jmp    c5 <end>
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
 +[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+c5 <end>:
 +[a-f0-9]+:	66 31 c0             	xor    %eax,%eax
#pass
