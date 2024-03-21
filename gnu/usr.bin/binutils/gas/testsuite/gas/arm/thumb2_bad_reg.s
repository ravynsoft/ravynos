	.syntax unified
	.text
	.align	2
	.thumb
	.thumb_func
test:
	@ ADC (immediate)
	adc r13, r0,  #1
	adc r15, r0,  #1
	adc r0,  r13, #1
	adc r0,  r15, #1
	@ ADC (register)
	adc.w r13,  r0,  r1
	adc.w r15,  r0,  r1
	adc.w  r0, r13,  r1
	adc.w  r0, r15,  r1
	adc.w  r0,  r1, r13
	adc.w  r0,  r1, r15
	@ ADD (immediate)
	add.w r13, r0, #1
	add.w r15, r0, #1
	add.w r0, r13, #1		@ ADD (SP plus immediate)
	add.w r0, r15, #1		@ Converted implicitly to ADDW
	addw  r13, r0, #1
	addw  r15, r0, #1
	addw  r0, r13, #1		@ ADD (SP plus immediate)
	addw  r0, r15, #1		@ ADR
	@ ADD (register)
	add.w r13, r0, r1
	add.w r15, r0, r1	
	adds.w r15, r0, r1
	add.w r0, r13, r1		@ ADD (SP plus register)
	add.w r0, r15, r1	
	add.w r0, r1, r13
	add.w r0, r1, r15
	@ ADD (SP plus immediate)
	add.w   r0, r13,  #1		@ OK
	add.w  r15, r13,  #1
	adds.w r15, r13,  #1
	addw   r15, r13,  #1 
	@ ADD (SP plus register)
	add.w  r15, r13,  r0
	add.w   r0, r13, r13
	add.w   r0, r13, r15
	@ ADR
	adr.w  r13, test
	adr.w  r15, test
	@ AND (immediate)
	and    r13,  r0,  #1
	and    r15,  r0,  #1
	and    r0,  r13,  #1
	and    r0,  r15,  #1
	@ AND (register)
	and.w r13,  r0,  r1
	and.w r15,  r0,  r1
	and.w  r0, r13,  r1
	and.w  r0, r15,  r1
	and.w  r0,  r1, r13
	and.w  r0,  r1, r15
	@ ASR (immediate)
	asr    r13,  r0,  #1
	asr    r15,  r0,  #1
	asr    r0,  r13,  #1
	asr    r0,  r15,  #1
	@ ASR (register)
	asr.w r13,  r0,  r1
	asr.w r15,  r0,  r1
	asr.w  r0, r13,  r1
	asr.w  r0, r15,  r1
	asr.w  r0,  r1, r13
	asr.w  r0,  r1, r15
	@ BFC
	bfc   r13,  #1,  #1
	bfc   r15,  #1,  #1
	@ BFI
	bfi r13, r0, #1, #1
	bfi r15, r0, #1, #1
	bfi r0, r13, #1, #1
	bfi r0, r15, #1, #1
	@ BIC (immediate)
	bic r13, r0, #1
	bic r15, r0, #1
	bic r0, r13, #1
	bic r0, r15, #1
	@ BIC (register)
	bic.w r13,  r0,  r1
	bic.w r15,  r0,  r1
	bic.w  r0, r13,  r1
	bic.w  r0, r15,  r1
	bic.w  r0,  r1, r13
	bic.w  r0,  r1, r15
	@ BLX (register)
	blx r13				@ OK
	blx r15
	@ BXJ
	bxj r13
	bxj r15
	@ CLZ
	clz r13, r0
	clz r15, r0
	clz r0, r13
	clz r0, r15
	@ CMN (immediate)
	cmn r13, #1			@ OK
	cmn r15, #1
	@ CMN (register)
	cmn.w r13, r0			@ OK
	cmn.w r15, r0
	cmn.w r0, r13
	cmn.w r0, r15
	@ CMP (immediate)
	cmp.w r13, #1			@ OK
	cmp.w r15, #1
	@ CMP (register)
	cmp r13, r0			@ OK
	cmp r15, r0
	cmp r0, r13                     @ Deprecated
	cmp r0, r15                     
	cmp.n r0, r13                   @ Deprecated
	cmp.n r0, r15                   
	cmp.w r13, r0			@ OK
	cmp.w r15, r0
	cmp.w r0, r13
	cmp.w r0, r15
	@ EOR (immediate)
	eor r13, r0, #1
	eor r15, r0, #1
	eor r0, r13, #1
	eor r0, r15, #1
	@ EOR (register)
	eor.w r13,  r0,  r1
	eor.w r15,  r0,  r1
	eor.w  r0, r13,  r1
	eor.w  r0, r15,  r1
	eor.w  r0,  r1, r13
	eor.w  r0,  r1, r15
	@ LSL (immediate)
	lsl r13, r0, #1
	lsl r15, r0, #1
	lsl r0, r13, #1
	lsl r0, r15, #1
	@ LSL (register)
	lsl.w r13,  r0,  r1
	lsl.w r15,  r0,  r1
	lsl.w  r0, r13,  r1
	lsl.w  r0, r15,  r1
	lsl.w  r0,  r1, r13
	lsl.w  r0,  r1, r15
	@ LSR (immediate)
	lsr r13, r0, #1
	lsr r15, r0, #1
	lsr r0, r13, #1
	lsr r0, r15, #1
	@ LSR (register)
	lsr.w r13,  r0,  r1
	lsr.w r15,  r0,  r1
	lsr.w  r0, r13,  r1
	lsr.w  r0, r15,  r1
	lsr.w  r0,  r1, r13
	lsr.w  r0,  r1, r15
	@ MCR
	mcr p0, #1, r13, cr0, cr0
	mcr p0, #1, r15, cr0, cr0	@ OK
	@ MCRR
	mcrr p0, #1, r13, r0, cr0
	mcrr p0, #1, r15, r0, cr0
	mcrr p0, #1, r0, r13, cr0
	mcrr p0, #1, r0, r15, cr0
	@ MLA
	mla r13, r0, r0, r0
	mla r15, r0, r0, r0
	mla r0, r13, r0, r0
	mla r0, r15, r0, r0
	mla r0, r0, r13, r0
	mla r0, r0, r15, r0
	mla r0, r0, r0, r13
	mla r0, r0, r0, r15
	@ MLS
	mls r13, r0, r0, r0
	mls r15, r0, r0, r0
	mls r0, r13, r0, r0
	mls r0, r15, r0, r0
	mls r0, r0, r13, r0
	mls r0, r0, r15, r0
	mls r0, r0, r0, r13
	mls r0, r0, r0, r15
	@ MOV (immediate)
	mov.w r13, #1
	mov.w r15, #1
	@ MOV (register)
	mov r13, r0			@ OK
	mov r15, r0			@ OK
	mov.w r0, r13			@ OK
	mov.w r0, r15
	mov.w r15, r0
	mov.w r13, r0			@ OK
	movs.w r0, r13
	movs.w r0, r15
	movs.w r13, r0
	movs.w r15, r0
	mov.w r13, r13
	mov.w r15, r13
	mov.w r13, r15
	mov.w r15, r15
	mov r13, r13			@ Deprecated
	mov r15, r13			@ Deprecated
	mov r13, r15			@ Deprecated
	mov r15, r15			@ Deprecated
	movs r13, r13
	movs r15, r13
	movs r13, r15
	movs r15, r15
	@ MOVT
	movt r13, #1
	movt r15, #1
	@ MRC
	mrc p0, #1, r13, cr0, cr0
	mrc p0, #1, r15, cr0, cr0	@ OK
	@ MRCC
	mrrc p0, #1, r13, r0, cr0
	mrrc p0, #1, r15, r0, cr0
	mrrc p0, #1, r0, r13, cr0
	mrrc p0, #1, r0, r15, cr0
	@ MRS
	mrs r13, cpsr
	mrs r15, cpsr
	@ MSR (register)
	msr cpsr, r13
	msr cpsr, r15
	@ MUL
	mul r13, r0, r0
	mul r15, r0, r0
	mul r0, r13, r0
	mul r0, r15, r0
	mul r0, r0, r13
	mul r0, r0, r15
	@ MVN (immediate)
	mvn r13, #1
	mvn r15, #1
	@ MVN (register)
	mvn.w r13, r0
	mvn.w r15, r0
	mvn.w r0, r13
	mvn.w r0, r15
	@ ORN (immediate)
	orn r13, r0, #1
	orn r15, r0, #1
	orn r0, r13, #1
	orn r0, r15, #1
	@ ORN (register)
	orn r13, r0, r0
	orn r15, r0, r0
	orn r0, r13, r0
	orn r0, r15, r0
	orn r0, r0, r13
	orn r0, r0, r15
	@ ORR (immediate)
	orr r13, r0, #1
	orr r15, r0, #1
	orr r0, r13, #1
	orr r0, r15, #1
	@ ORR (register)
	orr r13, r0, r0
	orr r15, r0, r0
	orr r0, r13, r0
	orr r0, r15, r0
	orr r0, r0, r13
	orr r0, r0, r15
	@ PKH
	pkhbt r13, r0, r0
	pkhbt r15, r0, r0
	pkhbt r0, r13, r0
	pkhbt r0, r15, r0
	pkhbt r0, r0, r13
	pkhbt r0, r0, r15
	@ PLD (register)
	pld [r0, r13]
	pld [r0, r15]
	pld [r13, r0]			@ OK
	pld [r15, r0]
	@ PLI (register)
	pli [r0, r13]
	pli [r0, r15]
	pli [r13, r0]			@ OK
	pli [r15, r0]
	@ QADD
	qadd r13, r0, r0
	qadd r15, r0, r0
	qadd r0, r13, r0
	qadd r0, r15, r0
	qadd r0, r0, r13
	qadd r0, r0, r15
	@ QADD16
	qadd16 r13, r0, r0
	qadd16 r15, r0, r0
	qadd16 r0, r13, r0
	qadd16 r0, r15, r0
	qadd16 r0, r0, r13
	qadd16 r0, r0, r15
	@ QADD8
	qadd8 r13, r0, r0
	qadd8 r15, r0, r0
	qadd8 r0, r13, r0
	qadd8 r0, r15, r0
	qadd8 r0, r0, r13
	qadd8 r0, r0, r15
	@ QASX
	qasx r13, r0, r0
	qasx r15, r0, r0
	qasx r0, r13, r0
	qasx r0, r15, r0
	qasx r0, r0, r13
	qasx r0, r0, r15
	@ QDADD
	qdadd r13, r0, r0
	qdadd r15, r0, r0
	qdadd r0, r13, r0
	qdadd r0, r15, r0
	qdadd r0, r0, r13
	qdadd r0, r0, r15
	@ QDSUB
	qdsub r13, r0, r0
	qdsub r15, r0, r0
	qdsub r0, r13, r0
	qdsub r0, r15, r0
	qdsub r0, r0, r13
	qdsub r0, r0, r15
	@ QSAX
	qsax r13, r0, r0
	qsax r15, r0, r0
	qsax r0, r13, r0
	qsax r0, r15, r0
	qsax r0, r0, r13
	qsax r0, r0, r15
	@ QSUB
	qsub r13, r0, r0
	qsub r15, r0, r0
	qsub r0, r13, r0
	qsub r0, r15, r0
	qsub r0, r0, r13
	qsub r0, r0, r15
	@ QSUB16
	qsub16 r13, r0, r0
	qsub16 r15, r0, r0
	qsub16 r0, r13, r0
	qsub16 r0, r15, r0
	qsub16 r0, r0, r13
	qsub16 r0, r0, r15
	@ QSUB8
	qsub8 r13, r0, r0
	qsub8 r15, r0, r0
	qsub8 r0, r13, r0
	qsub8 r0, r15, r0
	qsub8 r0, r0, r13
	qsub8 r0, r0, r15
	@ RBIT
	rbit r13, r0
	rbit r15, r0
	rbit r0, r13
	rbit r0, r15
	@ REV
	rev.w r13, r0
	rev.w r15, r0
	rev.w r0, r13
	rev.w r0, r15
	@ REV16
	rev16.w r13, r0
	rev16.w r15, r0
	rev16.w r0, r13
	rev16.w r0, r15
	@ REVSH
	revsh.w r13, r0
	revsh.w r15, r0
	revsh.w r0, r13
	revsh.w r0, r15
	@ RFE
	rfedb r15
	rfeia r15
	@ ROR (immediate)
	ror r13, r0, #1
	ror r15, r0, #1
	ror r0, r13, #1
	ror r0, r15, #1
	@ ROR (register)
	ror.w r13,  r0,  r1
	ror.w r15,  r0,  r1
	ror.w  r0, r13,  r1
	ror.w  r0, r15,  r1
	ror.w  r0,  r1, r13
	ror.w  r0,  r1, r15
	@ RRX
	rrx r13, r0
	rrx r15, r0
	rrx r0, r13
	rrx r0, r15
	@ RSB (immediate)
	rsb.w r13, r0, #1
	rsb.w r15, r0, #1
	rsb.w r0, r13, #1
	rsb.w r0, r15, #1
	@ RSB (register)
	rsb r13,  r0,  r1
	rsb r15,  r0,  r1
	rsb  r0, r13,  r1
	rsb  r0, r15,  r1
	rsb  r0,  r1, r13
	rsb  r0,  r1, r15
	@ SADD16
	sadd16 r13, r0, r0
	sadd16 r15, r0, r0
	sadd16 r0, r13, r0
	sadd16 r0, r15, r0
	sadd16 r0, r0, r13
	sadd16 r0, r0, r15
	@ SADD8
	sadd8 r13, r0, r0
	sadd8 r15, r0, r0
	sadd8 r0, r13, r0
	sadd8 r0, r15, r0
	sadd8 r0, r0, r13
	sadd8 r0, r0, r15
	@ SASX
	sasx r13, r0, r0
	sasx r15, r0, r0
	sasx r0, r13, r0
	sasx r0, r15, r0
	sasx r0, r0, r13
	sasx r0, r0, r15
	@ SBC (immediate)
	sbc r13, r0, #1
	sbc r15, r0, #1
	sbc r0, r13, #1
	sbc r0, r15, #1
	@ SBC (register)
	sbc r13,  r0,  r1
	sbc r15,  r0,  r1
	sbc  r0, r13,  r1
	sbc  r0, r15,  r1
	sbc  r0,  r1, r13
	sbc  r0,  r1, r15
	@ SBFX (immediate)
	sbfx r13, r0, #1, #1
	sbfx r15, r0, #1, #1
	sbfx r0, r13, #1, #1
	sbfx r0, r15, #1, #1
	@ SDIV (register)
	sdiv r13,  r0,  r1
	sdiv r15,  r0,  r1
	sdiv  r0, r13,  r1
	sdiv  r0, r15,  r1
	sdiv  r0,  r1, r13
	sdiv  r0,  r1, r15
	@ SEL (register)
	sel r13,  r0,  r1
	sel r15,  r0,  r1
	sel  r0, r13,  r1
	sel  r0, r15,  r1
	sel  r0,  r1, r13
	sel  r0,  r1, r15
	@ SHADD16
	shadd16 r13, r0, r0
	shadd16 r15, r0, r0
	shadd16 r0, r13, r0
	shadd16 r0, r15, r0
	shadd16 r0, r0, r13
	shadd16 r0, r0, r15
	@ SHADD8
	shadd8 r13, r0, r0
	shadd8 r15, r0, r0
	shadd8 r0, r13, r0
	shadd8 r0, r15, r0
	shadd8 r0, r0, r13
	shadd8 r0, r0, r15
	@ SHASX
	shasx r13, r0, r0
	shasx r15, r0, r0
	shasx r0, r13, r0
	shasx r0, r15, r0
	shasx r0, r0, r13
	shasx r0, r0, r15
	@ SHSAX
	shsax r13, r0, r0
	shsax r15, r0, r0
	shsax r0, r13, r0
	shsax r0, r15, r0
	shsax r0, r0, r13
	shsax r0, r0, r15
	@ SHSUB16
	shsub16 r13, r0, r0
	shsub16 r15, r0, r0
	shsub16 r0, r13, r0
	shsub16 r0, r15, r0
	shsub16 r0, r0, r13
	shsub16 r0, r0, r15
	@ SHSUB8
	shsub8 r13, r0, r0
	shsub8 r15, r0, r0
	shsub8 r0, r13, r0
	shsub8 r0, r15, r0
	shsub8 r0, r0, r13
	shsub8 r0, r0, r15
	@ SMLABB
	smlabb r13, r0, r0, r0
	smlabb r15, r0, r0, r0
	smlabb r0, r13, r0, r0
	smlabb r0, r15, r0, r0
	smlabb r0, r0, r13, r0
	smlabb r0, r0, r15, r0
	smlabb r0, r0, r0, r13
	smlabb r0, r0, r0, r15
	@ SMLAD
	smlad r13, r0, r0, r0
	smlad r15, r0, r0, r0
	smlad r0, r13, r0, r0
	smlad r0, r15, r0, r0
	smlad r0, r0, r13, r0
	smlad r0, r0, r15, r0
	smlad r0, r0, r0, r13
	smlad r0, r0, r0, r15
	@ SMLAL
	smlal r13, r0, r0, r0
	smlal r15, r0, r0, r0
	smlal r0, r13, r0, r0
	smlal r0, r15, r0, r0
	smlal r0, r0, r13, r0
	smlal r0, r0, r15, r0
	smlal r0, r0, r0, r13
	smlal r0, r0, r0, r15
	@ SMLALBB
	smlalbb r13, r0, r0, r0
	smlalbb r15, r0, r0, r0
	smlalbb r0, r13, r0, r0
	smlalbb r0, r15, r0, r0
	smlalbb r0, r0, r13, r0
	smlalbb r0, r0, r15, r0
	smlalbb r0, r0, r0, r13
	smlalbb r0, r0, r0, r15
	@ SMLALD
	smlald r13, r0, r0, r0
	smlald r15, r0, r0, r0
	smlald r0, r13, r0, r0
	smlald r0, r15, r0, r0
	smlald r0, r0, r13, r0
	smlald r0, r0, r15, r0
	smlald r0, r0, r0, r13
	smlald r0, r0, r0, r15
	@ SMLAWB
	smlawb r13, r0, r0, r0
	smlawb r15, r0, r0, r0
	smlawb r0, r13, r0, r0
	smlawb r0, r15, r0, r0
	smlawb r0, r0, r13, r0
	smlawb r0, r0, r15, r0
	smlawb r0, r0, r0, r13
	smlawb r0, r0, r0, r15
	@ SMLSD
	smlsd r13, r0, r0, r0
	smlsd r15, r0, r0, r0
	smlsd r0, r13, r0, r0
	smlsd r0, r15, r0, r0
	smlsd r0, r0, r13, r0
	smlsd r0, r0, r15, r0
	smlsd r0, r0, r0, r13
	smlsd r0, r0, r0, r15
	@ SMLSLD
	smlsld r13, r0, r0, r0
	smlsld r15, r0, r0, r0
	smlsld r0, r13, r0, r0
	smlsld r0, r15, r0, r0
	smlsld r0, r0, r13, r0
	smlsld r0, r0, r15, r0
	smlsld r0, r0, r0, r13
	smlsld r0, r0, r0, r15
	@ SMMLA
	smmla r13, r0, r0, r0
	smmla r15, r0, r0, r0
	smmla r0, r13, r0, r0
	smmla r0, r15, r0, r0
	smmla r0, r0, r13, r0
	smmla r0, r0, r15, r0
	smmla r0, r0, r0, r13
	smmla r0, r0, r0, r15
	@ SMMLS
	smmls r13, r0, r0, r0
	smmls r15, r0, r0, r0
	smmls r0, r13, r0, r0
	smmls r0, r15, r0, r0
	smmls r0, r0, r13, r0
	smmls r0, r0, r15, r0
	smmls r0, r0, r0, r13
	smmls r0, r0, r0, r15		
	@ SMMUL
	smmul r13, r0, r0
	smmul r15, r0, r0
	smmul r0, r13, r0
	smmul r0, r15, r0
	smmul r0, r0, r13
	smmul r0, r0, r15
	@ SMUAD
	smuad r13, r0, r0
	smuad r15, r0, r0
	smuad r0, r13, r0
	smuad r0, r15, r0
	smuad r0, r0, r13
	smuad r0, r0, r15
	@ SMULBB
	smulbb r13, r0, r0
	smulbb r15, r0, r0
	smulbb r0, r13, r0
	smulbb r0, r15, r0
	smulbb r0, r0, r13
	smulbb r0, r0, r15
	@ SMULL
	smull r13, r0, r0, r0
	smull r15, r0, r0, r0
	smull r0, r13, r0, r0
	smull r0, r15, r0, r0
	smull r0, r0, r13, r0
	smull r0, r0, r15, r0
	smull r0, r0, r0, r13
	smull r0, r0, r0, r15		
	@ SMULWB
	smulwb r13, r0, r0
	smulwb r15, r0, r0
	smulwb r0, r13, r0
	smulwb r0, r15, r0
	smulwb r0, r0, r13
	smulwb r0, r0, r15
	@ SMUSD
	smusd r13, r0, r0
	smusd r15, r0, r0
	smusd r0, r13, r0
	smusd r0, r15, r0
	smusd r0, r0, r13
	smusd r0, r0, r15
	@ SSAT
	ssat r13, #1, r0
	ssat r15, #1, r0
	ssat r0, #1, r13
	ssat r0, #1, r15
	ssat r1, #1, r3,asr #32
	@ SSAT16
	ssat16 r13, #1, r0
	ssat16 r15, #1, r0
	ssat16 r0, #1, r13
	ssat16 r0, #1, r15
	@ SSAX
	ssax r13,  r0,  r1
	ssax r15,  r0,  r1
	ssax  r0, r13,  r1
	ssax  r0, r15,  r1
	ssax  r0,  r1, r13
	ssax  r0,  r1, r15
	@ SSUB16
	ssub16 r13,  r0,  r1
	ssub16 r15,  r0,  r1
	ssub16  r0, r13,  r1
	ssub16  r0, r15,  r1
	ssub16  r0,  r1, r13
	ssub16  r0,  r1, r15
	@ SSUB8
	ssub8 r13,  r0,  r1
	ssub8 r15,  r0,  r1
	ssub8  r0, r13,  r1
	ssub8  r0, r15,  r1
	ssub8  r0,  r1, r13
	ssub8  r0,  r1, r15
	@ SUB (immediate)
	sub.w r13, r0, #1
	sub.w r15, r0, #1
	sub.w r0, r13, #1		@ SUB (SP minus immediate)
	sub.w r0, r15, #1		@ ADR
	subw  r13, r0, #1
	subw  r15, r0, #1
	subw  r0, r13, #1		@ SUB (SP minus immediate)
	subw  r0, r15, #1		@ ADR
	@ SUB (register)
	sub.w r13,  r0, r1
	sub.w r15,  r0, r1
	sub.w  r0, r13, r1		@ SUB (SP minus register)
	sub.w  r0, r15, r1	
	sub.w  r0,  r1, r13
	sub.w  r0,  r1, r15
	@ SUB (SP minus immediate)
	sub.w   r0, r13,  #1		@ OK
	sub.w  r15, r13,  #1
	subs.w r15, r13,  #1
	subw   r15, r13,  #1 
	@ SUB (SP minus register)
	sub.w  r13, r13,  r0		@ OK
	sub.w  r15, r13,  r0
	sub.w   r0, r13, r13
	sub.w   r0, r13, r15
	@ SXTAB
	sxtab r13,  r0,  r1
	sxtab r15,  r0,  r1
	sxtab  r0, r13,  r1
	sxtab  r0, r15,  r1
	sxtab  r0,  r1, r13
	sxtab  r0,  r1, r15
	@ SXTAB16
	sxtab16 r13,  r0,  r1
	sxtab16 r15,  r0,  r1
	sxtab16  r0, r13,  r1
	sxtab16  r0, r15,  r1
	sxtab16  r0,  r1, r13
	sxtab16  r0,  r1, r15
	@ SXTAH
	sxtah r13,  r0,  r1
	sxtah r15,  r0,  r1
	sxtah  r0, r13,  r1
	sxtah  r0, r15,  r1
	sxtah  r0,  r1, r13
	sxtah  r0,  r1, r15
	@ SXTB
	sxtb r13, r0
	sxtb r15, r0
	sxtb r0, r13
	sxtb r0, r15
	@ SXTB16
	sxtb16 r13, r0
	sxtb16 r15, r0
	sxtb16 r0, r13
	sxtb16 r0, r15
	@ SXTH
	sxth r13, r0
	sxth r15, r0
	sxth r0, r13
	sxth r0, r15
	@ TBB
	tbb [r13, r0]
	tbb [r15, r0]			@ OK
	tbb [r0, r13]
	tbb [r0, r15]
	@ TBH
	tbh [r13, r0]
	tbh [r15, r0]			@ OK
	tbh [r0, r13]
	tbh [r0, r15]
	@ TEQ (immediate)
	teq r13, #1
	teq r15, #1
	@ TEQ (register)
	teq r13, r0
	teq r15, r0
	teq r0, r13
	teq r0, r15
	@ TST (immediate)
	tst r13, #1
	tst r15, #1
	@ TST (register)
	tst.w r13, r0
	tst.w r15, r0
	tst.w r0, r13
	tst.w r0, r15
	@ UADD16
	uadd16 r13, r0, r0
	uadd16 r15, r0, r0
	uadd16 r0, r13, r0
	uadd16 r0, r15, r0
	uadd16 r0, r0, r13
	uadd16 r0, r0, r15
	@ UADD8
	uadd8 r13, r0, r0
	uadd8 r15, r0, r0
	uadd8 r0, r13, r0
	uadd8 r0, r15, r0
	uadd8 r0, r0, r13
	uadd8 r0, r0, r15
	@ UASX
	uasx r13, r0, r0
	uasx r15, r0, r0
	uasx r0, r13, r0
	uasx r0, r15, r0
	uasx r0, r0, r13
	uasx r0, r0, r15
	@ UBFX (immediate)
	ubfx r13, r0, #1, #1
	ubfx r15, r0, #1, #1
	ubfx r0, r13, #1, #1
	ubfx r0, r15, #1, #1
	@ UDIV (register)
	udiv r13,  r0,  r1
	udiv r15,  r0,  r1
	udiv  r0, r13,  r1
	udiv  r0, r15,  r1
	udiv  r0,  r1, r13
	udiv  r0,  r1, r15
	@ UHADD16
	uhadd16 r13, r0, r0
	uhadd16 r15, r0, r0
	uhadd16 r0, r13, r0
	uhadd16 r0, r15, r0
	uhadd16 r0, r0, r13
	uhadd16 r0, r0, r15
	@ UHADD8
	uhadd8 r13, r0, r0
	uhadd8 r15, r0, r0
	uhadd8 r0, r13, r0
	uhadd8 r0, r15, r0
	uhadd8 r0, r0, r13
	uhadd8 r0, r0, r15
	@ UHASX
	uhasx r13, r0, r0
	uhasx r15, r0, r0
	uhasx r0, r13, r0
	uhasx r0, r15, r0
	uhasx r0, r0, r13
	uhasx r0, r0, r15
	@ UHSAX
	uhsax r13, r0, r0
	uhsax r15, r0, r0
	uhsax r0, r13, r0
	uhsax r0, r15, r0
	uhsax r0, r0, r13
	uhsax r0, r0, r15
	@ UHSUB16
	uhsub16 r13, r0, r0
	uhsub16 r15, r0, r0
	uhsub16 r0, r13, r0
	uhsub16 r0, r15, r0
	uhsub16 r0, r0, r13
	uhsub16 r0, r0, r15
	@ UHSUB8
	uhsub8 r13, r0, r0
	uhsub8 r15, r0, r0
	uhsub8 r0, r13, r0
	uhsub8 r0, r15, r0
	uhsub8 r0, r0, r13
	uhsub8 r0, r0, r15
	@ UMAAL
	umaal r13, r0, r0, r0
	umaal r15, r0, r0, r0
	umaal r0, r13, r0, r0
	umaal r0, r15, r0, r0
	umaal r0, r0, r13, r0
	umaal r0, r0, r15, r0
	umaal r0, r0, r0, r13
	umaal r0, r0, r0, r15		
	@ UMLAL
	umlal r13, r0, r0, r0
	umlal r15, r0, r0, r0
	umlal r0, r13, r0, r0
	umlal r0, r15, r0, r0
	umlal r0, r0, r13, r0
	umlal r0, r0, r15, r0
	umlal r0, r0, r0, r13
	umlal r0, r0, r0, r15		
	@ UMULL
	umull r13, r0, r0, r0
	umull r15, r0, r0, r0
	umull r0, r13, r0, r0
	umull r0, r15, r0, r0
	umull r0, r0, r13, r0
	umull r0, r0, r15, r0
	umull r0, r0, r0, r13
	umull r0, r0, r0, r15		
	@ UQADD16
	uqadd16 r13, r0, r0
	uqadd16 r15, r0, r0
	uqadd16 r0, r13, r0
	uqadd16 r0, r15, r0
	uqadd16 r0, r0, r13
	uqadd16 r0, r0, r15
	@ UQADD8
	uqadd8 r13, r0, r0
	uqadd8 r15, r0, r0
	uqadd8 r0, r13, r0
	uqadd8 r0, r15, r0
	uqadd8 r0, r0, r13
	uqadd8 r0, r0, r15
	@ UQASX
	uqasx r13, r0, r0
	uqasx r15, r0, r0
	uqasx r0, r13, r0
	uqasx r0, r15, r0
	uqasx r0, r0, r13
	uqasx r0, r0, r15
	@ UQSAX
	uqsax r13, r0, r0
	uqsax r15, r0, r0
	uqsax r0, r13, r0
	uqsax r0, r15, r0
	uqsax r0, r0, r13
	uqsax r0, r0, r15
	@ UQSUB16
	uqsub16 r13, r0, r0
	uqsub16 r15, r0, r0
	uqsub16 r0, r13, r0
	uqsub16 r0, r15, r0
	uqsub16 r0, r0, r13
	uqsub16 r0, r0, r15
	@ UQSUB8
	uqsub8 r13, r0, r0
	uqsub8 r15, r0, r0
	uqsub8 r0, r13, r0
	uqsub8 r0, r15, r0
	uqsub8 r0, r0, r13
	uqsub8 r0, r0, r15
	@ USAD8
	usad8 r13, r0, r0
	usad8 r15, r0, r0
	usad8 r0, r13, r0
	usad8 r0, r15, r0
	usad8 r0, r0, r13
	usad8 r0, r0, r15
	@ USADA8
	usada8 r13, r0, r0, r0
	usada8 r15, r0, r0, r0
	usada8 r0, r13, r0, r0
	usada8 r0, r15, r0, r0
	usada8 r0, r0, r13, r0
	usada8 r0, r0, r15, r0
	usada8 r0, r0, r0, r13
	usada8 r0, r0, r0, r15		
	@ USAT
	usat r13, #1, r0
	usat r15, #1, r0
	usat r0, #1, r13
	usat r0, #1, r15
	usat r1, #1, r3,asr #32
	@ USAT16
	usat16 r13, #1, r0
	usat16 r15, #1, r0
	usat16 r0, #1, r13
	usat16 r0, #1, r15
	@ USAX
	usax r13,  r0,  r1
	usax r15,  r0,  r1
	usax  r0, r13,  r1
	usax  r0, r15,  r1
	usax  r0,  r1, r13
	usax  r0,  r1, r15
	@ USUB16
	usub16 r13,  r0,  r1
	usub16 r15,  r0,  r1
	usub16  r0, r13,  r1
	usub16  r0, r15,  r1
	usub16  r0,  r1, r13
	usub16  r0,  r1, r15
	@ USUB8
	usub8 r13,  r0,  r1
	usub8 r15,  r0,  r1
	usub8  r0, r13,  r1
	usub8  r0, r15,  r1
	usub8  r0,  r1, r13
	usub8  r0,  r1, r15
	@ UXTAB
	uxtab r13,  r0,  r1
	uxtab r15,  r0,  r1
	uxtab  r0, r13,  r1
	uxtab  r0, r15,  r1
	uxtab  r0,  r1, r13
	uxtab  r0,  r1, r15
	@ UXTAB16
	uxtab16 r13,  r0,  r1
	uxtab16 r15,  r0,  r1
	uxtab16  r0, r13,  r1
	uxtab16  r0, r15,  r1
	uxtab16  r0,  r1, r13
	uxtab16  r0,  r1, r15
	@ UXTAH
	uxtah r13,  r0,  r1
	uxtah r15,  r0,  r1
	uxtah  r0, r13,  r1
	uxtah  r0, r15,  r1
	uxtah  r0,  r1, r13
	uxtah  r0,  r1, r15
	@ UXTB
	uxtb r13, r0
	uxtb r15, r0
	uxtb r0, r13
	uxtb r0, r15
	@ UXTB16
	uxtb16 r13, r0
	uxtb16 r15, r0
	uxtb16 r0, r13
	uxtb16 r0, r15
	@ UXTH
	uxth r13, r0
	uxth r15, r0
	uxth r0, r13
	uxth r0, r15
