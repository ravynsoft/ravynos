#source: pr19175.s
#as: --64
#ld: -Bsymbolic -shared -melf_x86_64 -T pr19175.t
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

#...
[ 	]*[a-f0-9]+:	48 8d 05 ([0-9a-f]{2} ){4} *	lea    -0x[a-f0-9]+\(%rip\),%rax        # [a-f0-9]+ <_start>
#pass
