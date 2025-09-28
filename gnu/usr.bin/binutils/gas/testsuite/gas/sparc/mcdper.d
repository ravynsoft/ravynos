#as: -Av9m
#objdump: -dr
#name: sparc OSA2015 %mcdper asr

.*: +file format .*sparc.*

Disassembly of section .text:

0+ <.text>:
   0:	83 43 80 00 	rd  %mcdper, %g1
   4:	9d 80 60 03 	wr  %g1, 3, %mcdper
   8:	9d 80 40 02 	wr  %g1, %g2, %mcdper



