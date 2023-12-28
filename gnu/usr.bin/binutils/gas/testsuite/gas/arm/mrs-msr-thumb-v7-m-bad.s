	.arch armv7-m
	.text
	.thumb
	
	mrs r4, cpsr
	mrs r5, spsr
	msr apsr_nzcvqg, r4
	msr iapsr_nzcvqg, r5
	msr xpsr_nncvq, r6
	msr xpsr_nzcv, r7
	msr cpsr_f, r7
	msr spsr, r8
	msr primask_nzcvq, r9
