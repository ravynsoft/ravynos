#source: abs32-reloc.s
#as: -a32
#ld: -melf32ppc -static --defsym a=1 --defsym 'HIDDEN(b=2)' --defsym c=0x12345678
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

01800074 <_start>:
 1800074:	(80 fe ff f0|f0 ff fe 80) 	lwz     r7,-16\(r30\)
 1800078:	(81 1e ff f8|f8 ff 1e 81) 	lwz     r8,-8\(r30\)
 180007c:	(81 3e ff ec|ec ff 3e 81) 	lwz     r9,-20\(r30\)
 1800080:	(81 5e ff f4|f4 ff 5e 81) 	lwz     r10,-12\(r30\)

Disassembly of section \.got:

01810094 <\.got>:
 1810094:	(00 00 00 02|02 00 00 00) 	.*
 1810098:	(01 81 00 84|84 00 81 01) 	.*
 181009c:	(12 34 56 78|78 56 34 12) 	.*
 18100a0:	(00 00 00 01|01 00 00 00) 	.*
 18100a4:	(4e 80 00 21|21 00 80 4e) 	blrl

018100a8 <_GLOBAL_OFFSET_TABLE_>:
	\.\.\.
