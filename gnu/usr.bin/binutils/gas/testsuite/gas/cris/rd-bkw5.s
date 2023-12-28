	.ifndef extra
	.set extra,0
	.endif

	.macro	case
	.if	\@ < 10
	.word	.L1000\@-.
	.elseif	\@ < 100
	.word	.L100\@-.
	.elseif	\@ < 1000
	.word	.L10\@-.
	.else
	.word	.L1\@-.
	.endif
	.set	counter,1+\@
	.endm

	.macro case_label
.L\@:	nop
	.endm

	.macro padder
	.endm

	.text
	.align 1
	.global x
	.type	x,@function
x:
	.rept 4095+extra
	case
	.endr

	.rept 10000-counter
	padder
	.endr

after:
	; Force a secondary jump-table entry for every label.
	.fill 32768

	.rept counter
	case_label
	.endr

	move.d	esymbol,$r10
	.size	x,.-x
	.align 1
