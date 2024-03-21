;;; Test branches and branch relocate with XGATE
;;; 
	.sect .text
	.globl _start
_start:

	ldw	r1,#var1 	; expands to two IMM8 %hi,%lo relocate
    tst r1
    beq     linked_ad1
    tst r2
    beq     the_end
    bra     linked_ad2

the_end:
    rts

    .sect   .data
var1:   fdb 0x1234
