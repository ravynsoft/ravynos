	.arch armv6
	.text
	.arm
	
	mrs r4, apsr
	mrs r5, cpsr
	mrs r6, spsr
	msr apsr_nzcvq, #0x40000000
	msr cpsr_f, #0x20000000
	msr spsr, #0x10000000
	msr apsr_nzcvq, r4
	msr cpsr_f, r5
	msr spsr, r6
