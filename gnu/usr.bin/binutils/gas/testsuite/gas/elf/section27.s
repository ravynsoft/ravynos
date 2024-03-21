	.section	.text,"ax"
	.global	discard2
	.type	discard2, %function
discard2:
	.word	0

	.section	.data,"aw"
	.global	discard1
	.type	discard1, %object
discard1:
	.word	1

	.section	.bss,"aw"
	.global	discard0
	.type	discard0, %object
discard0:
	.zero	2

	.section	.bss,"awR",%nobits
	.global	retain0
	.type	retain0, %object
retain0:
	.zero	2

	.section	.data,"awR",%progbits
	.type	retain1, %object
retain1:
	.word	1

	.section	.text,"axR",%progbits
	.global	retain2
	.type	retain2, %function
retain2:
	.word	0
