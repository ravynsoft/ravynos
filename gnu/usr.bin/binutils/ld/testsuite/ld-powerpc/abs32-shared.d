#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -shared --hash-style=sysv --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

00000160 <_start>:
 160:	(80 fe ff f0|f0 ff fe 80) 	lwz     r7,-16\(r30\)
 164:	(81 1e ff f8|f8 ff 1e 81) 	lwz     r8,-8\(r30\)
 168:	(81 3e ff ec|ec ff 3e 81) 	lwz     r9,-20\(r30\)
 16c:	(81 5e ff f4|f4 ff 5e 81) 	lwz     r10,-12\(r30\)

Disassembly of section \.got:

000101f0 <\.got>:
   101f0:	(00 00 00 02|02 00 00 00) 	.*
	.\.\.
   10200:	(4e 80 00 21|21 00 80 4e) 	blrl

00010204 <_GLOBAL_OFFSET_TABLE_>:
   10204:	(00 01 01 70|70 01 01 00) 00 00 00 00 00 00 00 00 .*
