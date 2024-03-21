#objdump: -dw
#name: i386 arch 7

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	f3 0f b8 d9          	popcnt %ecx,%ebx
[ 	]*[a-f0-9]+:	f3 0f bd d9          	lzcnt  %ecx,%ebx
#pass
