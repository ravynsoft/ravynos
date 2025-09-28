#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -pie --hash-style=sysv --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

00000138 <_start>:
 138:	(80 fe ff f0|f0 ff fe 80) 	lwz     r7,-16\(r30\)
 13c:	(81 1e ff f8|f8 ff 1e 81) 	lwz     r8,-8\(r30\)
 140:	(81 3e ff ec|ec ff 3e 81) 	lwz     r9,-20\(r30\)
 144:	(81 5e ff f4|f4 ff 5e 81) 	lwz     r10,-12\(r30\)

Disassembly of section \.got:

000101d8 <\.got>:
   101d8:	(00 00 00 02|02 00 00 00) 	.*
   101dc:	(00 00 00 00|00 00 00 00) 	.*
   101e0:	(12 34 56 78|78 56 34 12) 	.*
   101e4:	(00 00 00 01|01 00 00 00) 	.*
   101e8:	(4e 80 00 21|21 00 80 4e) 	blrl

000101ec <_GLOBAL_OFFSET_TABLE_>:
   101ec:	(00 01 01 48|48 01 01 00) 00 00 00 00 00 00 00 00 .*
