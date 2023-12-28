#as: -mppc -me6500
#objdump: -dr -Me6500
#name: Power E6500 nop tests

.*

Disassembly of section \.text:

0+00 <start>:
   0:	(60 00 00 00|00 00 00 60) 	nop
   4:	(60 00 00 00|00 00 00 60) 	nop
   8:	(60 00 00 00|00 00 00 60) 	nop
   c:	(60 00 00 00|00 00 00 60) 	nop
