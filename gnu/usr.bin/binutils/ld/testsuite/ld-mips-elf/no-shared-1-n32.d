#source: no-shared-1-o32.s
#ld: -T no-shared-1.ld
#objdump: -dr -j.text -j.data -j.got

.*


Disassembly of section \.text:

00050000 <__start>:
   50000:	3c020007 	lui	v0,0x7
   50004:	24428000 	addiu	v0,v0,-32768
   50008:	8f828018 	lw	v0,-32744\(gp\)
   5000c:	8f828018 	lw	v0,-32744\(gp\)
#...
Disassembly of section \.data:

00060000 .*:
   60000:	00068000 	.*
#...
Disassembly of section \.got:

00060010 <_GLOBAL_OFFSET_TABLE_>:
   60010:	00000000 80000000 00068000  .*
