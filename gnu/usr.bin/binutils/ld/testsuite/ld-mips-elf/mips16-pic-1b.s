	.abicalls
	.set	noreorder
	.include "mips16-pic-1.inc"

	callpic	unused6,mips16
	jals	unused7,mips16
	jals	unused10,mips16
	callpic	used8,nomips16
	jals	used9,nomips16
	callpic	used15,nomips16
	jals	used16,nomips16
	callpic	used21,mips16

	.globl	__start
	.ent	__start
	.set	nomips16
__start:
	nop
	.end	__start
