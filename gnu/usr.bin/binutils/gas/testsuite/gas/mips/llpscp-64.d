#objdump: -dr
#as: -32
#name: Paired LL/SC for mips64r6

.*: +file format .*

Disassembly of section \.text:

00000000 <test>:
   0:	7c821877 	lldp	v0,v1,a0
   4:	7c821877 	lldp	v0,v1,a0
   8:	24821234 	addiu	v0,a0,4660
   c:	7c420077 	lldp	v0,zero,v0
  10:	24430000 	addiu	v1,v0,0
			10: R_MIPS_LO16	.data
  14:	7c621877 	lldp	v0,v1,v1
  18:	3c020123 	lui	v0,0x123
  1c:	00431021 	addu	v0,v0,v1
  20:	24424567 	addiu	v0,v0,17767
  24:	7c421877 	lldp	v0,v1,v0
  28:	3c010000 	lui	at,0x0
			28: R_MIPS_HI16	.data
  2c:	24210000 	addiu	at,at,0
			2c: R_MIPS_LO16	.data
  30:	00240821 	addu	at,at,a0
  34:	7c200077 	lldp	zero,zero,at
  38:	7c821867 	scdp	v0,v1,a0
  3c:	7c821867 	scdp	v0,v1,a0
  40:	24811234 	addiu	at,a0,4660
  44:	7c220067 	scdp	v0,zero,at
  48:	24410000 	addiu	at,v0,0
			48: R_MIPS_LO16	.data
  4c:	7c221867 	scdp	v0,v1,at
  50:	3c010123 	lui	at,0x123
  54:	00230821 	addu	at,at,v1
  58:	24214567 	addiu	at,at,17767
  5c:	7c221867 	scdp	v0,v1,at
  60:	3c010000 	lui	at,0x0
			60: R_MIPS_HI16	.data
  64:	24210000 	addiu	at,at,0
			64: R_MIPS_LO16	.data
  68:	00240821 	addu	at,at,a0
  6c:	7c200067 	scdp	zero,zero,at
	...
