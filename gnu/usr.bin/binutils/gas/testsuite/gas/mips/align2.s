	.text
	.align	5
	.type	f1,@function
f1:
	.set	mips16
	addiu	$2,1
	.align	3
	addiu	$3,1
	.size	f1,.-f1

	.align	2
	.set	nomips16
	.type	f2,@function
f2:
	addiu	$2,$2,1
	addiu	$3,$3,1
	.align	4
	addiu	$4,$4,1
	.align	3
	.size	f2,.-f2

	.set	mips16
	.type	f3,@function
f3:
	addiu	$16,$16,1
	.align	3
	.size	f3,.-f3

	.section .text.a,"ax",@progbits
	.align	4
	.set	nomips16
	.type	f4,@function
f4:
	addiu	$5,$5,1
	.size	f4,.-f4
