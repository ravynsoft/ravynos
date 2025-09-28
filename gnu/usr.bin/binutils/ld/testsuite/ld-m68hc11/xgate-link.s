;;; Test 16bit relocate with XGATE
;;; 
	.sect .text
	.globl _start
_start:

	ldw	r1,#var1 	; expands to two IMM8 %hi,%lo relocate
	add	r5,#var2 	; expands to two IMM8 %hi,%lo relocate
	ldl	r2,#%lovar4 ; test explicit %lo
	ldh	r2,#%hivar4 ; test explicit %hi
	ldl	r3,#0x21 	; regular IMM8
	ldh	r6,#var5 	; IMM8 with relocate
	cmp r1,#0xabcd	; expands to two IMM8 with constant
	cmp r2,#var3 	; expands to two IMM8 %hi,%lo relocate
  ldw r1,#var6
  ldw r2,#var6+0x104 ; check for correct carry
