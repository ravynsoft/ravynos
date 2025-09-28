#objdump: -dw
#name: i386 arch 9

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f a7 c0             	xstore-rng
#pass
