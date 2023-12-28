#name: mcf-wdebug
#objdump: --architecture=m68k:5200 -d
#as: -m5208

.*:     file format .*

Disassembly of section .text:

0+ <foo>:
   0:	fbd0 0003      	wdebugl %a0@
   4:	fbd0 0003      	wdebugl %a0@
