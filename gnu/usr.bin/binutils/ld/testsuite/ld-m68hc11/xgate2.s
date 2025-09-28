;;; Part2 of branch test
;;; 
.globl  linked_ad1, linked_ad2
	.sect .text

linked_ad1:
    cmpl    r4,#1
    bne     linked_ad2

label1:
    nop
    par     r5

linked_ad2:
    csem    #2
    rts
