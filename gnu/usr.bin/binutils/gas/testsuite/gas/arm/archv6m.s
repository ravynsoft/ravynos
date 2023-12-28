	.arch	armv6-m
	.syntax unified
	.thumb
	.text
	.align	2
	.global	foo
foo:
	msr apsr_nzcvq,r6
	msr epsr,r9
	mrs r2, iapsr
	yield
	wfe
	wfi
	sev
	add r0, r0, r1
	nop
	dmb
	dsb
	isb
	
