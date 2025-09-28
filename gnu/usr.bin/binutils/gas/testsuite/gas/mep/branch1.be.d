#as: -EB
#objdump: -dzr
#source: branch1.s
#name: branch1.be

.*: *file format elf32-mep

Disassembly of section \.text:

.* <.*>:
 *0:	00 00  *	nop
 *2:	e4 51 00 00 *	beq \$4,\$5,.* <foo-0x8>
[ 	]*2: *R_MEP_PCREL17A2	foo
 *6:	00 00  *	nop
 *8:	00 00  *	nop

.* <foo>:
 *a:	00 00  *	nop
