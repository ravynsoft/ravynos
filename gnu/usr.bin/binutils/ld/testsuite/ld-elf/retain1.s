	.global	discard0
	.section	.bss.discard0,"aw"
	.type	discard0, %object
discard0:
	.zero	2

	.global	discard1
	.section	.bss.discard1,"aw"
	.type	discard1, %object
discard1:
	.zero	2

	.global	discard2
	.section	.data.discard2,"aw"
	.type	discard2, %object
discard2:
	.word	1

	.section	.bss.sdiscard0,"aw"
	.type	sdiscard0, %object
sdiscard0:
	.zero	2

	.section	.bss.sdiscard1,"aw"
	.type	sdiscard1, %object
sdiscard1:
	.zero	2

	.section	.data.sdiscard2,"aw"
	.type	sdiscard2, %object
sdiscard2:
	.word	1

	.section	.text.fndiscard0,"ax"
	.global	fndiscard0
	.type	fndiscard0, %function
fndiscard0:
	.word 0

	.global	retain0
	.section	.bss.retain0,"awR"
	.type	retain0, %object
retain0:
	.zero	2

	.global	retain1
	.section	.bss.retain1,"awR"
	.type	retain1, %object
retain1:
	.zero	2

	.global	retain2
	.section	.data.retain2,"awR"
	.type	retain2, %object
retain2:
	.word	1

	.section	.bss.sretain0,"awR"
	.type	sretain0, %object
sretain0:
	.zero	2

	.section	.bss.sretain1,"awR"
	.type	sretain1, %object
sretain1:
	.zero	2

	.section	.data.sretain2,"aRw"
	.type	sretain2, %object
sretain2:
	.word	1

	.section	.text.fnretain1,"Rax"
	.global	fnretain1
	.type	fnretain1, %function
fnretain1:
	.word	0

	.section	.text.fndiscard2,"ax"
	.global	fndiscard2
	.type	fndiscard2, %function
fndiscard2:
	.word	0

	.section	.bss.lsretain0,"awR"
	.type	lsretain0.2, %object
lsretain0.2:
	.zero	2

	.section	.bss.lsretain1,"aRw"
	.type	lsretain1.1, %object
lsretain1.1:
	.zero	2

	.section	.data.lsretain2,"aRw"
	.type	lsretain2.0, %object
lsretain2.0:
	.word	1

	.section	.text._start,"ax"
	.global	_start
	.type	_start, %function
_start:
	.word 0
