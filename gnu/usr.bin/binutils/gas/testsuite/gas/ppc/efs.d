#as: -a32 -mbig -mvle
#objdump: -d -Mefs -Mvle -Mefs2
#name: Validate EFS instructions

.*: +file format elf.*-powerpc.*

Disassembly of section .text:

00000000 <.text>:
   0:	10 00 12 d1 	efscfsi r0,r2
   4:	10 00 12 d5 	efsctsi r0,r2
   8:	10 00 12 f1 	efdcfsi r0,r2
   c:	10 00 12 f5 	efdctsi r0,r2
  10:	10 01 12 c2 	efsmadd r0,r1,r2
  14:	10 01 12 c3 	efsmsub r0,r1,r2
  18:	10 01 12 ca 	efsnmadd r0,r1,r2
  1c:	10 01 12 cb 	efsnmsub r0,r1,r2
  20:	10 01 12 e2 	efdmadd r0,r1,r2
  24:	10 01 12 e3 	efdmsub r0,r1,r2
  28:	10 01 12 ea 	efdnmadd r0,r1,r2
  2c:	10 01 12 eb 	efdnmsub r0,r1,r2
  30:	10 01 12 f0 	efdcfuid r0,r2
  34:	10 01 12 f1 	efdcfsid r0,r2
  38:	10 01 12 f8 	efdctuidz r0,r2
  3c:	10 01 12 fa 	efdctsidz r0,r2