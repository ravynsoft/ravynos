#as: -J
#objdump: -drw --syms
#name: x86-64 rip addressing 2

.*: +file format .*

SYMBOL TABLE:
0000000000000000 l       .text	0000000000000000 _start
0000000080000006 l       .text	0000000000000000 test1
ffffffff8000000e l       .text	0000000000000000 test2
00000000f000000e l       .text	0000000000000000 test3
ffffffff1000000e l       .text	0000000000000000 test4



Disassembly of section .text:

0000000000000000 <_start>:
 +0:	48 8b 05 ff ff ff 7f 	mov    0x7fffffff\(%rip\),%rax        # 80000006 <test1>
 +7:	48 8b 05 00 00 00 80 	mov    -0x80000000\(%rip\),%rax        # ffffffff8000000e <test2>
#pass
