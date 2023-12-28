#source: pr19609-6.s
#as: --x32 -mrelax-relocations=yes
#ld: -melf32_x86_64 --defsym foobar=0x80000000
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ 	]*[a-f0-9]+:	40 c7 c0 00 00 00 80 	rex mov \$0x80000000,%eax
#pass
