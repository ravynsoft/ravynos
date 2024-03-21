#as: -Av9v
#objdump: -dr
#name: sparc IMA

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	91 ba 84 0c 	fpmaddx  %f10, %f12, %f2, %f8
   4:	a5 bb 8e 88 	fpmaddxhi  %f14, %f8, %f38, %f18
