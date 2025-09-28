#name: TLS with PIE (2 x86)
#as: --32
#ld: -melf_i386 -pie
#objdump: -dw

.*: +file format .*


Disassembly of section .text:

[0-9a-f]+ <_start>:
[ 	]*[a-f0-9]+:	8d 05 fc ff ff ff    	lea    0xfffffffc,%eax
#pass
