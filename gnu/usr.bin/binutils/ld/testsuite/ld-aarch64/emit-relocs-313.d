#source: emit-relocs-313.s
#ld: -T relocs.ld --defsym globala=0x11000 --defsym globalb=0x45000 --defsym globalc=0x1234  -e0 --emit-relocs
#objdump: -dr

.*: +file format .*


Disassembly of section .text:

0000000000010000 <\.text>:
   10000:	90000082 	adrp	x2, 20000 <_GLOBAL_OFFSET_TABLE_>
			10000: R_AARCH64_ADR_PREL_PG_HI21	_GLOBAL_OFFSET_TABLE_
   10004:	f9400840 	ldr	x0, \[x2, #16\]
			10004: R_AARCH64_LD64_GOTPAGE_LO15	globala
   10008:	f9400c40 	ldr	x0, \[x2, #24\]
			10008: R_AARCH64_LD64_GOTPAGE_LO15	globalb
   1000c:	f9400440 	ldr	x0, \[x2, #8\]
			1000c: R_AARCH64_LD64_GOTPAGE_LO15	globalc
