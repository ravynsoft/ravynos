.text
.global	_start
_start:
	beq $r0, $r1, main
	bne $r0, $r1, main
	beqz $r0, main
	bnez $r0, main
	bgez $r0, main
	bgezal $r0, main
	bgtz $r0, main
	blez $r0, main
	bltz $r0, main
	bltzal $r0, main
.section .text.2, "ax"
.globl main
main:
	nop

