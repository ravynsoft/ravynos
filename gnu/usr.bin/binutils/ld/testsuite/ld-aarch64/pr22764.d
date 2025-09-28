#source: pr22764.s
#target: [check_shared_lib_support]
#ld: -shared -T relocs.ld -defsym sym_abs1=0x1 -defsym sym_abs2=0x2 -defsym sym_abs3=0x3 -e0 --emit-relocs -z notext
#notarget: aarch64_be-*-*
#objdump: -dr
#...

Disassembly of section \.text:

0000000000010000 \<\.text\>:
   10000:	d503201f 	nop
	...
			10004: R_AARCH64_ABS64	sym_abs1
   1000c:	00000002 	\.word	0x00000002
			1000c: R_AARCH64_ABS32	sym_abs2
   10010:	0003      	\.short	0x0003
			10010: R_AARCH64_ABS16	sym_abs3
   10012:	0000      	\.short	0x0000
   10014:	d503201f 	nop
