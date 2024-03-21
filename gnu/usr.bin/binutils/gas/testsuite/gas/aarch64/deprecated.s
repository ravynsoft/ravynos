// deprecated.s Test file for diagnostics on deprecated features.

.text
	mrs	x0, spsr_svc
	msr	spsr_hyp, x15
