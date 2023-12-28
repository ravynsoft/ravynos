	.abicalls
	nop

	.macro	dummyfn,name
	.global	\name
	.ent	\name
\name:
	jr	$31
	.end	\name
	.endm

	dummyfn	extern1
	dummyfn	extern2
	dummyfn	extern3
	dummyfn	extern4
