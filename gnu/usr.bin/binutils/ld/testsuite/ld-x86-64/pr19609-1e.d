#source: pr19609-1.s
#as: --64 -mrelax-relocations=yes
#ld: -shared -E -Bsymbolic -melf_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	48 3b 05 ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	3b 0d ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%ecx        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	4c 3b 1d ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%r11        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	44 3b 25 ([0-9a-f]{2} ){4} *	cmp    0x[a-f0-9]+\(%rip\),%r12d        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	48 8b 05 ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	8b 0d ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%ecx        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	4c 8b 1d ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%r11        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	44 8b 25 ([0-9a-f]{2} ){4} *	mov    0x[a-f0-9]+\(%rip\),%r12d        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	48 85 05 ([0-9a-f]{2} ){4} *	test   %rax,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	85 0d ([0-9a-f]{2} ){4} *	test   %ecx,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	4c 85 1d ([0-9a-f]{2} ){4} *	test   %r11,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <.*>
[ 	]*[a-f0-9]+:	44 85 25 ([0-9a-f]{2} ){4} *	test   %r12d,0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <.*>
