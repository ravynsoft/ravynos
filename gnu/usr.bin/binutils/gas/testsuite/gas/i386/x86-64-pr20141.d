#objdump: -dw

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	62 e1 7d 48 e7 21    	vmovntdq %zmm20,\(%rcx\)
#pass
