#as: -Av9m8
#objdump: -dr
#name: OSA2017 ONADD/ONSUB/ONMUL/ONDIV instructions.

.*: +file format .*

Disassembly of section .text:

0+ <.text>:
   0:	a1 b0 02 a8 	onadd  %f0, %f8, %f16
   4:	b5 b2 02 b0 	onsub  %f8, %f16, %f24
   8:	a9 b0 42 b8 	onmul  %f32, %f24, %f16
   c:	9d b2 02 a0 	ondiv  %f8, %f0, %f8
