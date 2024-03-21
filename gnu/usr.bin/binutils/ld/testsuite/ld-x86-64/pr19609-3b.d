#source: pr19609-3.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x70000000 -Tdata=0xa0000000
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

70000000 <_start>:
[ 	]*[a-f0-9]+:	81 f8 00 00 00 a0    	cmp    \$0xa0000000,%eax
[ 	]*[a-f0-9]+:	41 81 fb 00 00 00 a0 	cmp    \$0xa0000000,%r11d
#pass
