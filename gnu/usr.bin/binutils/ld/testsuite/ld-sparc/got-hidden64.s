	.text
.LLGETPC0:
	retl
	 add	%o7, %l7, %l7
	.global foo
	.type foo, #function
	.proc   04
foo:
	save    %sp, -160, %sp
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %l7
	call	.LLGETPC0
	 add	%l7, %lo(_GLOBAL_OFFSET_TABLE_+4), %l7
	sethi 	%hi(var), %g1
	or 	%g1, %lo(var), %g1
	ldx 	[%l7+%g1], %g1
	ld 	[%g1], %i0
	ret
	 restore
