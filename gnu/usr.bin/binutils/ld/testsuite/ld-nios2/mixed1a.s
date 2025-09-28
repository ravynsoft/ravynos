# Test linking incompatible object file types. 

.text
.global	_start
_start:
	movhi	r2, %hiadj(foo)
	addi	r2, r2, %lo(foo)
	ldw	r2, 0(r2)
	cmpeq	r2, r2, zero
