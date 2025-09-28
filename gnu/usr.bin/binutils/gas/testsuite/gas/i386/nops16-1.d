#source: nops16-1.s
#objdump: -drw -Mi8086
#name: i386 nops 16bit 1

.*: +file format .*

Disassembly of section .text:

0+ <nop31>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 1d                	jmp    20 <nop30>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+20 <nop30>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 1c                	jmp    40 <nop29>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+40 <nop29>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 1b                	jmp    60 <nop28>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+60 <nop28>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 1a                	jmp    80 <nop27>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+80 <nop27>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 19                	jmp    a0 <nop26>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+a0 <nop26>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 18                	jmp    c0 <nop25>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+c0 <nop25>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 17                	jmp    e0 <nop24>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+e0 <nop24>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 16                	jmp    100 <nop23>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+100 <nop23>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 15                	jmp    120 <nop22>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+120 <nop22>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 14                	jmp    140 <nop21>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+140 <nop21>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 13                	jmp    160 <nop20>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+160 <nop20>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 12                	jmp    180 <nop19>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+180 <nop19>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 11                	jmp    1a0 <nop18>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+1a0 <nop18>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 10                	jmp    1c0 <nop17>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+1c0 <nop17>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0f                	jmp    1e0 <nop16>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+1e0 <nop16>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0e                	jmp    200 <nop15>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+200 <nop15>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0d                	jmp    210 <nop14>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+210 <nop14>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0c                	jmp    220 <nop13>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+220 <nop13>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0b                	jmp    230 <nop12>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+230 <nop12>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	eb 0a                	jmp    240 <nop11>
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+240 <nop11>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+250 <nop10>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+260 <nop9>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+270 <nop8>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+280 <nop7>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+290 <nop6>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax

0+2a0 <nop5>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si
[ 	]*[a-f0-9]+:	90                   	nop

0+2b0 <nop4>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d b4 00 00          	lea    0x0\(%si\),%si

0+2c0 <nop3>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	8d 74 00             	lea    0x0\(%si\),%si

0+2d0 <nop2>:
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	90                   	nop
[ 	]*[a-f0-9]+:	66 90                	xchg   %eax,%eax
#pass
