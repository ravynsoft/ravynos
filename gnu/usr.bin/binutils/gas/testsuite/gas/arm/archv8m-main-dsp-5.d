#name: ARMv8-M Mainline with DSP instructions (extension)
#source: arch7em.s
#as: -march=armv8-m.main+dsp
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> eac0 0000 	pkhbt	r0, r0, r0
0[0-9a-f]+ <[^>]+> eac0 0900 	pkhbt	r9, r0, r0
0[0-9a-f]+ <[^>]+> eac9 0000 	pkhbt	r0, r9, r0
0[0-9a-f]+ <[^>]+> eac0 0009 	pkhbt	r0, r0, r9
0[0-9a-f]+ <[^>]+> eac0 5000 	pkhbt	r0, r0, r0, lsl #20
0[0-9a-f]+ <[^>]+> eac0 00c0 	pkhbt	r0, r0, r0, lsl #3
0[0-9a-f]+ <[^>]+> eac3 0102 	pkhbt	r1, r3, r2
0[0-9a-f]+ <[^>]+> eac2 4163 	pkhtb	r1, r2, r3, asr #17
0[0-9a-f]+ <[^>]+> fa83 f182 	qadd	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f113 	qadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f113 	qadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f113 	qasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f113 	qasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa83 f192 	qdadd	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa83 f1b2 	qdsub	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa83 f1a2 	qsub	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f113 	qsub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f113 	qsub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f113 	qsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f113 	qsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f103 	sadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f103 	sadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f103 	sasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f103 	sasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f103 	ssub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f103 	ssub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f103 	ssax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f103 	ssax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f123 	shadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f123 	shadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f123 	shasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f123 	shasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f123 	shsub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f123 	shsub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f123 	shsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f123 	shsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f143 	uadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f143 	uadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f143 	uasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f143 	uasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f143 	usub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f143 	usub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f143 	usax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f143 	usax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f163 	uhadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f163 	uhadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f163 	uhasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f163 	uhasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f163 	uhsub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f163 	uhsub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f163 	uhsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f163 	uhsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa92 f153 	uqadd16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa82 f153 	uqadd8	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f153 	uqasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f153 	uqasx	r1, r2, r3
0[0-9a-f]+ <[^>]+> fad2 f153 	uqsub16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fac2 f153 	uqsub8	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f153 	uqsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> fae2 f153 	uqsax	r1, r2, r3
0[0-9a-f]+ <[^>]+> faa2 f183 	sel	r1, r2, r3
0[0-9a-f]+ <[^>]+> fb10 0000 	smlabb	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 0900 	smlabb	r9, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb19 0000 	smlabb	r0, r9, r0, r0
0[0-9a-f]+ <[^>]+> fb10 0009 	smlabb	r0, r0, r9, r0
0[0-9a-f]+ <[^>]+> fb10 9000 	smlabb	r0, r0, r0, r9
0[0-9a-f]+ <[^>]+> fb10 0020 	smlatb	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 0010 	smlabt	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 0030 	smlatt	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb30 0000 	smlawb	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb30 0010 	smlawt	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb20 0000 	smlad	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb20 0010 	smladx	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb40 0000 	smlsd	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb40 0010 	smlsdx	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb50 0000 	smmla	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb50 0010 	smmlar	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb60 0000 	smmls	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb60 0010 	smmlsr	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb70 0000 	usada8	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 0080 	smlalbb	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 9080 	smlalbb	r9, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 0980 	smlalbb	r0, r9, r0, r0
0[0-9a-f]+ <[^>]+> fbc9 0080 	smlalbb	r0, r0, r9, r0
0[0-9a-f]+ <[^>]+> fbc0 0089 	smlalbb	r0, r0, r0, r9
0[0-9a-f]+ <[^>]+> fbc0 00a0 	smlaltb	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 0090 	smlalbt	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 00b0 	smlaltt	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 00c0 	smlald	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbc0 00d0 	smlaldx	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbd0 00c0 	smlsld	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbd0 00d0 	smlsldx	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fbe0 0060 	umaal	r0, r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 f000 	smulbb	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 f900 	smulbb	r9, r0, r0
0[0-9a-f]+ <[^>]+> fb19 f000 	smulbb	r0, r9, r0
0[0-9a-f]+ <[^>]+> fb10 f009 	smulbb	r0, r0, r9
0[0-9a-f]+ <[^>]+> fb10 f020 	smultb	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 f010 	smulbt	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb10 f030 	smultt	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb30 f000 	smulwb	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb30 f010 	smulwt	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb50 f000 	smmul	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb50 f010 	smmulr	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb20 f000 	smuad	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb20 f010 	smuadx	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb40 f000 	smusd	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb40 f010 	smusdx	r0, r0, r0
0[0-9a-f]+ <[^>]+> fb70 f000 	usad8	r0, r0, r0
0[0-9a-f]+ <[^>]+> f320 0000 	ssat16	r0, #1, r0
0[0-9a-f]+ <[^>]+> f320 0900 	ssat16	r9, #1, r0
0[0-9a-f]+ <[^>]+> f320 0009 	ssat16	r0, #10, r0
0[0-9a-f]+ <[^>]+> f329 0000 	ssat16	r0, #1, r9
0[0-9a-f]+ <[^>]+> f3a0 0000 	usat16	r0, #0, r0
0[0-9a-f]+ <[^>]+> f3a0 0900 	usat16	r9, #0, r0
0[0-9a-f]+ <[^>]+> f3a0 0009 	usat16	r0, #9, r0
0[0-9a-f]+ <[^>]+> f3a9 0000 	usat16	r0, #0, r9
0[0-9a-f]+ <[^>]+> fa2f f182 	sxtb16	r1, r2
0[0-9a-f]+ <[^>]+> fa2f f889 	sxtb16	r8, r9
0[0-9a-f]+ <[^>]+> fa3f f182 	uxtb16	r1, r2
0[0-9a-f]+ <[^>]+> fa3f f889 	uxtb16	r8, r9
0[0-9a-f]+ <[^>]+> fa40 f080 	sxtab	r0, r0, r0
0[0-9a-f]+ <[^>]+> fa40 f080 	sxtab	r0, r0, r0
0[0-9a-f]+ <[^>]+> fa40 f990 	sxtab	r9, r0, r0, ror #8
0[0-9a-f]+ <[^>]+> fa49 f0a0 	sxtab	r0, r9, r0, ror #16
0[0-9a-f]+ <[^>]+> fa40 f0b9 	sxtab	r0, r0, r9, ror #24
0[0-9a-f]+ <[^>]+> fa22 f183 	sxtab16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa02 f183 	sxtah	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa52 f183 	uxtab	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa32 f183 	uxtab16	r1, r2, r3
0[0-9a-f]+ <[^>]+> fa12 f183 	uxtah	r1, r2, r3
