	.arch armv7e-m
	.text
	.thumb
	
	mrs r4, apsr
	mrs r5, eapsr
	mrs r6, primask
	msr apsr_nzcvqg, r4
	msr iapsr_g, r5
	msr basepri_max, r6
