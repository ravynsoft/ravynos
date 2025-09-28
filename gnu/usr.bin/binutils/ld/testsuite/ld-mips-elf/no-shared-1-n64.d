#ld: -T no-shared-1.ld
#objdump: -dr -j.text -j.data -j.got

.*


Disassembly of section \.text:

0000000000050000 <__start>:
   50000:	3c020007 	lui	v0,0x7
   50004:	64428000 	daddiu	v0,v0,-32768
   50008:	df828020 	ld	v0,-32736\(gp\)
   5000c:	df828020 	ld	v0,-32736\(gp\)
#...
Disassembly of section \.data:

0000000000060000 .*:
   60000:	00000000 	.*
   60004:	00068000 	.*
#...
Disassembly of section \.got:

0000000000060010 <_GLOBAL_OFFSET_TABLE_>:
	\.\.\.
   60018:	80000000 00000000 00000000 00068000  .*
