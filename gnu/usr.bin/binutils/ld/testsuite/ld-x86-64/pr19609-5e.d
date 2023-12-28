#source: pr19609-5.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64 -Ttext=0x80000000 --no-relax
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ ]+[a-f0-9]+:	ff 15 ([0-9a-f]{2} ){4} *	call   \*-?0x[a-f0-9]+\(%rip\)        # [a-f0-9]+ <.*>
