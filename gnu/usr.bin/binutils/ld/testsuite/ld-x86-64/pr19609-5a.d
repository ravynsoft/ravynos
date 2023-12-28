#source: pr19609-5.s
#as: --64 -mrelax-relocations=yes
#ld: -melf_x86_64
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[a-f0-9]+ <_start>:
[ ]+[a-f0-9]+:	67 e8 ([0-9a-f]{2} ){4}[ 	]+addr32 call 0 <_start-0x[0-9a-f]+>
