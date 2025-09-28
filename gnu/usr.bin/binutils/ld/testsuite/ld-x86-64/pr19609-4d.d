#source: pr19609-4.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 -Ttext=0x70000000 -Tdata=0xa0000000
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

70000000 <_start>:
[ 	]*[a-f0-9]+:	40 c7 c0 00 00 00 a0 	rex mov \$0xa0000000,%eax
[ 	]*[a-f0-9]+:	41 c7 c3 00 00 00 a0 	mov    \$0xa0000000,%r11d
