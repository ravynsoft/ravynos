	.section	.text.discard0,"ax",%progbits
	.global	discard0
	.type	discard0, %function
discard0:
	.word	0

	.section	.data.discard1,"aw"
	.global	discard1
	.type	discard1, %object
discard1:
	.word	1

	.section	.bss.discard2,"aw"
	.global	discard2
	.type	discard2, %object
discard2:
	.zero	2

	.section	.bss.retain0,"awR",%nobits
	.global	retain0
	.type	retain0, %object
retain0:
	.zero	2

	.section	.data.retain1,"awR",%progbits
	.type	retain1, %object
retain1:
	.word	1

	.section	.text.retain2,"axR",%progbits
	.global	retain2
	.type	retain2, %function
retain2:
	.word	0
