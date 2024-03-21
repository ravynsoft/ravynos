#objdump: -drwMintel
#name: x86-64 RdSeed(Intel mode)
#source: x86-64-rdseed.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[       ]*[a-f0-9]+:	66 0f c7 f8          	rdseed ax
[       ]*[a-f0-9]+:	0f c7 f8             	rdseed eax
[       ]*[a-f0-9]+:	48 0f c7 f8          	rdseed rax
[       ]*[a-f0-9]+:	66 41 0f c7 fb       	rdseed r11w
[       ]*[a-f0-9]+:	41 0f c7 fb          	rdseed r11d
[       ]*[a-f0-9]+:	49 0f c7 fb          	rdseed r11
[       ]*[a-f0-9]+:	66 0f c7 fb          	rdseed bx
[       ]*[a-f0-9]+:	0f c7 fb             	rdseed ebx
[       ]*[a-f0-9]+:	48 0f c7 fb          	rdseed rbx
[       ]*[a-f0-9]+:	66 41 0f c7 fb       	rdseed r11w
[       ]*[a-f0-9]+:	41 0f c7 fb          	rdseed r11d
[       ]*[a-f0-9]+:	49 0f c7 fb          	rdseed r11
#pass
