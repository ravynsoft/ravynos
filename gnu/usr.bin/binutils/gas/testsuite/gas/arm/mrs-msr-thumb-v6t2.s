	.arch armv6t2
	.text
	.thumb
	
	mrs r4, apsr
	mrs r5, cpsr
	mrs r6, spsr
	msr apsr_nzcvqg, r4
	msr cpsr_f, r5
	msr spsr, r6
