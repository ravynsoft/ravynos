#source: pr19609-7.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x80000000 --no-relax -z max-page-size=0x1000 -z separate-code
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ ]*[a-f0-9]+:	ff 15 ([0-9a-f]{2} ){4} *	call   \*-?0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <_start\+0x1000>
#pass
