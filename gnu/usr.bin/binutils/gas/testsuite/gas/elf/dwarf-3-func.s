	.text

	.ifndef LOCAL
efunc1:
	.nop
	.nop
	.global efunc1
	.type efunc1, %function
	.size efunc1, .-efunc1
	.endif

	.ifndef GLOBAL
lfunc1:
	.nops 16
	.nop
	.type lfunc1, %function
	.size lfunc1, .-lfunc1
	.endif

	.ifndef LOCAL
efunc2:
	.nop
	.nops 32
	.nop
	.global efunc2
	.type efunc2, %function
	.size efunc2, .-efunc2
	.endif

	.global efunc3
	.type efunc3, %function

	.ifndef GLOBAL
lfunc2:
	.nop
	.nop
	.nop
	.type lfunc2, %function
	.size lfunc2, .-lfunc2
	.endif
