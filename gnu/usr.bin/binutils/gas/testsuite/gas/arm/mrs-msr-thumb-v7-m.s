	.arch armv7-m
	.text
	.thumb
	
	mrs r4, apsr
	mrs r5, eapsr
	mrs r6, primask
	msr xpsr_nzcvq, r3
	msr apsr_nzcvq, r4
	msr iapsr_nzcvq, r5
	msr primask, r6
