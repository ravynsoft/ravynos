	mov.b d0, ($18,s)
	neg.b $123456
	ld x, #$123456
	jmp   (-$23, s)  	; Make sure this isn't misinterpreted as the start of a predecrement operand
