	.arch armv7-a
	.text
	.arm
	
	mrs r4, apsr_nzcvq
	mrs r5, iapsr
	msr iapsr, r4
	msr apsr, r5
