# Test for correct generation of XGATE insns.
	
	.globl _start
	.sect .text

_start:
	ldw R2, #block+1024
	ldw R3, #block
	ldw R1, #1
Loop:	
	bra test
	nop
	bne Loop
Stop:
	
	.byte 0xcd
	.byte 3	
	bra _start

test:
	ldw R5, #2
	bra test2
	rts

value = 23
		
	.globl test2
test2:
	ldw R3, #value
	stw R4, (R3, #0)
	ldw R4, #24+_start-44
	bra Stop
L1:	
	ldw R1, test2
	ldw R2, test2
	rts

	.sect .data

	.sect .bss
block:
	.space	1024
block_end:
