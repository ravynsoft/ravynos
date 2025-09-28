#as: -EL
#objdump: -dzr
#source: branch1.s
#name: branch1.le

.*: *file format elf32-mep-little

Disassembly of section \.text:

.* <.*>:
 *0:	00 00  *	nop
 *2:	51 e4 00 00 *	beq \$4,\$5,.* <foo-0x8>
[ 	]*2: *R_MEP_PCREL17A2	foo
 *6:	00 00  *	nop
 *8:	00 00  *	nop

.* <foo>:
 *a:	00 00  *	nop
