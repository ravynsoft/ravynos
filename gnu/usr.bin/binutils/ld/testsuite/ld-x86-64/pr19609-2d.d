#source: pr19609-2.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x70000000 -Tdata=0xa0000000 --no-relax
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

70000000 <_start>:
#pass
[ 	]*[a-f0-9]+:	48 3b 05 ([0-9a-f]{2} ){4}	cmp    -?0x[a-f0-9]+\(%rip\),%rax        # .*
