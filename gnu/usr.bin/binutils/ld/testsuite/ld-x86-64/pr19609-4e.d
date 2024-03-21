#source: pr19609-4.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -Ttext=0x70000000 -Tdata=0xa0000000 --no-relax -z max-page-size=0x1000 -z separate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+70000000 <_start>:
[ 	]*[a-f0-9]+:	48 8b 05 ([0-9a-f]{2} ){4} *	mov    [-]?0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <_start\+0x1000>
[ 	]*[a-f0-9]+:	4c 8b 1d ([0-9a-f]{2} ){4} *	mov    [-]?0x[a-f0-9]+\(%rip\),%r11        # [a-f0-9]+ <_start\+0x1000>
