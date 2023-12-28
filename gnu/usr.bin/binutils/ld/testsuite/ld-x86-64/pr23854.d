#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

[a-f0-9]+ <_start>:
 +[a-f0-9]+:	66 13 05 ([0-9a-f]{2} ){4} *	adc    0x[a-f0-9]+\(%rip\),%ax        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 03 1d ([0-9a-f]{2} ){4} *	add    0x[a-f0-9]+\(%rip\),%bx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 23 0d ([0-9a-f]{2} ){4} *	and    0x[a-f0-9]+\(%rip\),%cx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 3b 15 ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%dx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 0b 3d ([0-9a-f]{2} ){4} *	or     0x[a-f0-9]+\(%rip\),%di        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 1b 35 ([0-9a-f]{2} ){4} *	sbb    0x[a-f0-9]+\(%rip\),%si        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 2b 2d ([0-9a-f]{2} ){4} *	sub    0x[a-f0-9]+\(%rip\),%bp        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 44 33 05 ([0-9a-f]{2} ){4} *	xor    0x[a-f0-9]+\(%rip\),%r8w        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 85 0d ([0-9a-f]{2} ){4} *	test   %cx,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 13 05 ([0-9a-f]{2} ){4} *	adc    0x[a-f0-9]+\(%rip\),%ax        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 03 1d ([0-9a-f]{2} ){4} *	add    0x[a-f0-9]+\(%rip\),%bx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 23 0d ([0-9a-f]{2} ){4} *	and    0x[a-f0-9]+\(%rip\),%cx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 3b 15 ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%dx        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 0b 3d ([0-9a-f]{2} ){4} *	or     0x[a-f0-9]+\(%rip\),%di        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 1b 35 ([0-9a-f]{2} ){4} *	sbb    0x[a-f0-9]+\(%rip\),%si        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 2b 2d ([0-9a-f]{2} ){4} *	sub    0x[a-f0-9]+\(%rip\),%bp        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 44 33 05 ([0-9a-f]{2} ){4} *	xor    0x[a-f0-9]+\(%rip\),%r8w        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
 +[a-f0-9]+:	66 85 0d ([0-9a-f]{2} ){4} *	test   %cx,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <_start\+0x[a-f0-9]+>
#pass
