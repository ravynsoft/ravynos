#as: -Aleon
#objdump: -dr -m sparc
#name: LEON instructions

.*: +file format .*

Disassembly of section .text:

0+ <foo>:
   0:	83 88 00 10 	pwr  %l0, %psr
   4:	83 88 00 00 	pwr  %g0, %psr
   8:	83 88 20 00 	pwr  %g0, %psr
   c:	83 88 3f ff 	pwr  -1, %psr
