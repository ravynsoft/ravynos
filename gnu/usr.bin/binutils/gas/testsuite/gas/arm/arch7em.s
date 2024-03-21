# Instructions included in v7E-M architecture over v7-M.

	.text
	.thumb
	.syntax unified

pkh:
	pkhbt	r0, r0, r0
	pkhbt	r9, r0, r0
	pkhbt	r0, r9, r0
	pkhbt	r0, r0, r9
	pkhbt	r0, r0, r0, lsl #0x14
	pkhbt	r0, r0, r0, lsl #3
	pkhtb	r1, r2, r3
	pkhtb	r1, r2, r3, asr #0x11

qadd:
	qadd		r1, r2, r3
	qadd16		r1, r2, r3
	qadd8		r1, r2, r3
	qasx		r1, r2, r3
	qaddsubx	r1, r2, r3
	qdadd		r1, r2, r3
	qdsub		r1, r2, r3
	qsub		r1, r2, r3
	qsub16		r1, r2, r3
	qsub8		r1, r2, r3
	qsax		r1, r2, r3
	qsubaddx	r1, r2, r3
	sadd16		r1, r2, r3
	sadd8		r1, r2, r3
	sasx		r1, r2, r3
	saddsubx	r1, r2, r3
	ssub16		r1, r2, r3
	ssub8		r1, r2, r3
	ssax		r1, r2, r3
	ssubaddx	r1, r2, r3
	shadd16		r1, r2, r3
	shadd8		r1, r2, r3
	shasx		r1, r2, r3
	shaddsubx	r1, r2, r3
	shsub16		r1, r2, r3
	shsub8		r1, r2, r3
	shsax		r1, r2, r3
	shsubaddx	r1, r2, r3
	uadd16		r1, r2, r3
	uadd8		r1, r2, r3
	uasx		r1, r2, r3
	uaddsubx	r1, r2, r3
	usub16		r1, r2, r3
	usub8		r1, r2, r3
	usax		r1, r2, r3
	usubaddx	r1, r2, r3
	uhadd16		r1, r2, r3
	uhadd8		r1, r2, r3
	uhasx		r1, r2, r3
	uhaddsubx	r1, r2, r3
	uhsub16		r1, r2, r3
	uhsub8		r1, r2, r3
	uhsax		r1, r2, r3
	uhsubaddx	r1, r2, r3
	uqadd16		r1, r2, r3
	uqadd8		r1, r2, r3
	uqasx		r1, r2, r3
	uqaddsubx	r1, r2, r3
	uqsub16		r1, r2, r3
	uqsub8		r1, r2, r3
	uqsax		r1, r2, r3
	uqsubaddx	r1, r2, r3
	sel		r1, r2, r3

smla:
	smlabb	r0, r0, r0, r0
	smlabb	r9, r0, r0, r0
	smlabb	r0, r9, r0, r0
	smlabb	r0, r0, r9, r0
	smlabb	r0, r0, r0, r9

	smlatb	r0, r0, r0, r0
	smlabt	r0, r0, r0, r0
	smlatt	r0, r0, r0, r0
	smlawb	r0, r0, r0, r0
	smlawt	r0, r0, r0, r0
	smlad	r0, r0, r0, r0
	smladx	r0, r0, r0, r0
	smlsd	r0, r0, r0, r0
	smlsdx	r0, r0, r0, r0
	smmla	r0, r0, r0, r0
	smmlar	r0, r0, r0, r0
	smmls	r0, r0, r0, r0
	smmlsr	r0, r0, r0, r0
	usada8	r0, r0, r0, r0

smlal:
	smlalbb	r0, r0, r0, r0
	smlalbb	r9, r0, r0, r0
	smlalbb	r0, r9, r0, r0
	smlalbb	r0, r0, r9, r0
	smlalbb	r0, r0, r0, r9

	smlaltb	r0, r0, r0, r0
	smlalbt	r0, r0, r0, r0
	smlaltt	r0, r0, r0, r0
	smlald	r0, r0, r0, r0
	smlaldx	r0, r0, r0, r0
	smlsld	r0, r0, r0, r0
	smlsldx	r0, r0, r0, r0
	umaal	r0, r0, r0, r0

smul:
	smulbb	r0, r0, r0
	smulbb	r9, r0, r0
	smulbb	r0, r9, r0
	smulbb	r0, r0, r9

	smultb	r0, r0, r0
	smulbt	r0, r0, r0
	smultt	r0, r0, r0
	smulwb	r0, r0, r0
	smulwt	r0, r0, r0
	smmul	r0, r0, r0
	smmulr	r0, r0, r0
	smuad	r0, r0, r0
	smuadx	r0, r0, r0
	smusd	r0, r0, r0
	smusdx	r0, r0, r0
	usad8	r0, r0, r0

sat:
	ssat16	r0, #1, r0
	ssat16	r9, #1, r0
	ssat16	r0, #10, r0
	ssat16	r0, #1, r9

	usat16	r0, #0, r0
	usat16	r9, #0, r0
	usat16	r0, #9, r0
	usat16	r0, #0, r9

xt:
	sxtb16	r1, r2
	sxtb16	r8, r9
	uxtb16	r1, r2
	uxtb16	r8, r9

xta:
	sxtab	r0, r0, r0
	sxtab	r0, r0, r0, ror #0
	sxtab	r9, r0, r0, ror #8
	sxtab	r0, r9, r0, ror #16
	sxtab	r0, r0, r9, ror #24

	sxtab16	r1, r2, r3
	sxtah	r1, r2, r3
	uxtab	r1, r2, r3
	uxtab16	r1, r2, r3
	uxtah	r1, r2, r3
