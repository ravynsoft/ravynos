#as: -mpower10
#objdump: -dr -Mpower10
#name: POWER10 alignment of labels test

.*


Disassembly of section \.text:

0+00 <_start>:
   0:	(48 00 00 3c|3c 00 00 48) 	b       3c <_start\+0x3c>
   4:	(48 00 00 3c|3c 00 00 48) 	b       40 <_start\+0x40>
   8:	(48 00 00 40|40 00 00 48) 	b       48 <_start\+0x48>
   c:	(7f e0 00 08|08 00 e0 7f) 	trap
  10:	(7f e0 00 08|08 00 e0 7f) 	trap
  14:	(7f e0 00 08|08 00 e0 7f) 	trap
  18:	(7f e0 00 08|08 00 e0 7f) 	trap
  1c:	(7f e0 00 08|08 00 e0 7f) 	trap
  20:	(7f e0 00 08|08 00 e0 7f) 	trap
  24:	(7f e0 00 08|08 00 e0 7f) 	trap
  28:	(7f e0 00 08|08 00 e0 7f) 	trap
  2c:	(7f e0 00 08|08 00 e0 7f) 	trap
  30:	(7f e0 00 08|08 00 e0 7f) 	trap
  34:	(7f e0 00 08|08 00 e0 7f) 	trap
  38:	(7f e0 00 08|08 00 e0 7f) 	trap
  3c:	(60 00 00 00|00 00 00 60) 	nop
  40:	(07 00 00 00|00 00 00 07) 	pnop
  44:	(00 00 00 00|00 00 00 00) 
  48:	(4e 80 00 20|20 00 80 4e) 	blr
#pass
