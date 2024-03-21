#objdump: -dw
#name: i386 arch 6

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt %ecx,%ebx
[ 	]*[a-f0-9]+:	f2 0f 38 f1 d9       	crc32  %ecx,%ebx
#pass
