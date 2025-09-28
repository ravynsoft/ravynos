#PROG: objcopy
#objdump: -d
#name: Check that the disassembler properly dump instruction and data.

.*: +file format .*aarch64.*

Disassembly of section \.text:

0+000 <l1>:
   0:	d503201f 	nop

0+004 <l2>:
   4:	d503201f 	nop
   8:	00c0ffee 	\.word	0x00c0ffee

Disassembly of section .fini:

0+000 <\.fini>:
   0:	0000dead 	\.word	0x0000dead
