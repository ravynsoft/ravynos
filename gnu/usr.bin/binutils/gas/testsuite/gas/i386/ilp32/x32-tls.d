#objdump: -dw
#name: x86-64 (ILP32) TLS

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	48 8b 05 00 00 00 00 	mov    0x0\(%rip\),%rax        # 7 <_start\+0x7>
[ 	]*[a-f0-9]+:	4c 8b 25 00 00 00 00 	mov    0x0\(%rip\),%r12        # e <_start\+0xe>
[ 	]*[a-f0-9]+:	40 03 05 00 00 00 00 	rex add 0x0\(%rip\),%eax        # 15 <_start\+0x15>
[ 	]*[a-f0-9]+:	44 03 25 00 00 00 00 	add    0x0\(%rip\),%r12d        # 1c <_start\+0x1c>
[ 	]*[a-f0-9]+:	40 8d 05 00 00 00 00 	rex lea 0x0\(%rip\),%eax        # 23 <_start\+0x23>
[ 	]*[a-f0-9]+:	44 8d 25 00 00 00 00 	lea    0x0\(%rip\),%r12d        # 2a <_start\+0x2a>
#pass
