#objdump: -dwMintel
#name: i386 RdSeed (Intel disassembly)
#source: rdseed.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[       ]*[a-f0-9]+:	66 0f c7 f8          	rdseed ax
[       ]*[a-f0-9]+:	0f c7 f8             	rdseed eax
[       ]*[a-f0-9]+:	66 0f c7 fb          	rdseed bx
[       ]*[a-f0-9]+:	0f c7 fb             	rdseed ebx
#pass
