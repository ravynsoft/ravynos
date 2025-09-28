#source: lea1.s
#as: --64
#ld: -pie -melf_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	8d 05 ([0-9a-f]{2} ){4} *	lea    -0x[a-f0-9]+\(%rip\),%eax        # [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	44 8d 1d ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%rip\),%r11d        # [a-f0-9]+ <bar>
[ 	]*[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4} *	lea    -0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <foo>
[ 	]*[a-f0-9]+:	4c 8d 1d ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%rip\),%r11        # [a-f0-9]+ <bar>
[ 	]*[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <__start_my_section>
[ 	]*[a-f0-9]+:	4c 8d 1d ([0-9a-f]{2} ){4} *	lea    0x[a-f0-9]+\(%rip\),%r11        # [a-f0-9]+ <.*>
#pass
