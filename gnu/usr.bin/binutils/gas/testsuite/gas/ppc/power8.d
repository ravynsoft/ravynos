#as: -mpower8
#objdump: -dr -Mpower8
#name: POWER8 tests (includes Altivec, VSX and HTM)

.*

Disassembly of section \.text:

0+00 <power8>:
.*:	(7c 05 07 1d|1d 07 05 7c) 	tabort\. r5
.*:	(7c e8 86 1d|1d 86 e8 7c) 	tabortwc\. 7,r8,r16
.*:	(7e 8b 56 5d|5d 56 8b 7e) 	tabortdc\. 20,r11,r10
.*:	(7e 2a 9e 9d|9d 9e 2a 7e) 	tabortwci\. 17,r10,-13
.*:	(7f a3 de dd|dd de a3 7f) 	tabortdci\. 29,r3,-5
.*:	(7c 00 05 1d|1d 05 00 7c) 	tbegin\.
.*:	(7f 80 05 9c|9c 05 80 7f) 	tcheck  cr7
.*:	(7c 00 05 5d|5d 05 00 7c) 	tend\.
.*:	(7c 00 05 5d|5d 05 00 7c) 	tend\.
.*:	(7e 00 05 5d|5d 05 00 7e) 	tendall\.
.*:	(7e 00 05 5d|5d 05 00 7e) 	tendall\.
.*:	(7c 18 07 5d|5d 07 18 7c) 	treclaim\. r24
.*:	(7c 00 07 dd|dd 07 00 7c) 	trechkpt\.
.*:	(7c 00 05 dd|dd 05 00 7c) 	tsuspend\.
.*:	(7c 00 05 dd|dd 05 00 7c) 	tsuspend\.
.*:	(7c 20 05 dd|dd 05 20 7c) 	tresume\.
.*:	(7c 20 05 dd|dd 05 20 7c) 	tresume\.
.*:	(60 42 00 00|00 00 42 60) 	ori     r2,r2,0
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 42 00 00|00 00 42 60) 	ori     r2,r2,0
.*:	(4c 00 01 24|24 01 00 4c) 	rfebb   0
.*:	(4c 00 09 24|24 09 00 4c) 	rfebb
.*:	(4c 00 09 24|24 09 00 4c) 	rfebb
.*:	(4d d5 04 60|60 04 d5 4d) 	bgttar- cr5
.*:	(4c c7 04 61|61 04 c7 4c) 	bnstarl- cr1
.*:	(4d ec 04 60|60 04 ec 4d) 	blttar\+ cr3
.*:	(4c e2 04 61|61 04 e2 4c) 	bnetarl\+
.*:	(4c 88 0c 60|60 0c 88 4c) 	bgetar  cr2,1
.*:	(4c 87 14 61|61 14 87 4c) 	bnstarl cr1,2
.*:	(7c 00 00 3c|3c 00 00 7c) 	waitasec
.*:	(7c 00 41 1c|1c 41 00 7c) 	msgsndp r8
.*:	(7c 20 01 26|26 01 20 7c) 	mtsle   1
.*:	(7c 00 d9 5c|5c d9 00 7c) 	msgclrp r27
.*:	(7d 4a 61 6d|6d 61 4a 7d) 	stqcx\.  r10,r10,r12
.*:	(7f 80 39 6d|6d 39 80 7f) 	stqcx\.  r28,0,r7
.*:	(7f 13 5a 28|28 5a 13 7f) 	lqarx   r24,r19,r11
.*:	(7e c0 5a 28|28 5a c0 7e) 	lqarx   r22,0,r11
.*:	(7e 80 32 5c|5c 32 80 7e) 	mfbhrbe r20,6
.*:	(7f b1 83 29|29 83 b1 7f) 	pbt\.    r29,r17,r16
.*:	(7d c0 3b 29|29 3b c0 7d) 	pbt\.    r14,0,r7
.*:	(7c 00 03 5c|5c 03 00 7c) 	clrbhrb
.*:	(11 6a 05 ed|ed 05 6a 11) 	vpermxor v11,v10,v0,v23
.*:	(13 02 39 3c|3c 39 02 13) 	vaddeuqm v24,v2,v7,v4
.*:	(11 4a 40 bd|bd 40 4a 11) 	vaddecuq v10,v10,v8,v2
.*:	(10 af 44 fe|fe 44 af 10) 	vsubeuqm v5,v15,v8,v19
.*:	(11 9f 87 7f|7f 87 9f 11) 	vsubecuq v12,v31,v16,v29
.*:	(12 9d 68 88|88 68 9d 12) 	vmulouw v20,v29,v13
.*:	(13 a0 d0 89|89 d0 a0 13) 	vmuluwm v29,v0,v26
.*:	(11 15 e0 c0|c0 e0 15 11) 	vaddudm v8,v21,v28
.*:	(10 3a 08 c2|c2 08 3a 10) 	vmaxud  v1,v26,v1
.*:	(12 83 08 c4|c4 08 83 12) 	vrld    v20,v3,v1
.*:	(10 93 58 c7|c7 58 93 10) 	vcmpequd v4,v19,v11
.*:	(12 ee f1 00|00 f1 ee 12) 	vadduqm v23,v14,v30
.*:	(11 08 69 40|40 69 08 11) 	vaddcuq v8,v8,v13
.*:	(13 9b 21 88|88 21 9b 13) 	vmulosw v28,v27,v4
.*:	(10 64 21 c2|c2 21 64 10) 	vmaxsd  v3,v4,v4
.*:	(10 13 aa 88|88 aa 13 10) 	vmuleuw v0,v19,v21
.*:	(13 14 9a c2|c2 9a 14 13) 	vminud  v24,v20,v19
.*:	(10 1c 7a c7|c7 7a 1c 10) 	vcmpgtud v0,v28,v15
.*:	(12 a0 13 88|88 13 a0 12) 	vmulesw v21,v0,v2
.*:	(11 3a 4b c2|c2 4b 3a 11) 	vminsd  v9,v26,v9
.*:	(13 3d 5b c4|c4 5b 3d 13) 	vsrad   v25,v29,v11
.*:	(11 7c 5b c7|c7 5b 7c 11) 	vcmpgtsd v11,v28,v11
.*:	(10 a8 d6 01|01 d6 a8 10) 	bcdadd\. v5,v8,v26,1
.*:	(10 83 64 08|08 64 83 10) 	vpmsumb v4,v3,v12
.*:	(13 5f ae 41|41 ae 5f 13) 	bcdsub\. v26,v31,v21,1
.*:	(10 b1 84 48|48 84 b1 10) 	vpmsumh v5,v17,v16
.*:	(12 f1 a4 4e|4e a4 f1 12) 	vpkudum v23,v17,v20
.*:	(13 15 ec 88|88 ec 15 13) 	vpmsumw v24,v21,v29
.*:	(11 36 6c c8|c8 6c 36 11) 	vpmsumd v9,v22,v13
.*:	(12 53 94 ce|ce 94 53 12) 	vpkudus v18,v19,v18
.*:	(13 d0 b5 00|00 b5 d0 13) 	vsubuqm v30,v16,v22
.*:	(11 cb 3d 08|08 3d cb 11) 	vcipher v14,v11,v7
.*:	(11 42 b5 09|09 b5 42 11) 	vcipherlast v10,v2,v22
.*:	(12 e0 6d 0c|0c 6d e0 12) 	vgbbd   v23,v13
.*:	(12 19 85 40|40 85 19 12) 	vsubcuq v16,v25,v16
.*:	(13 e1 2d 44|44 2d e1 13) 	vorc    v31,v1,v5
.*:	(10 91 fd 48|48 fd 91 10) 	vncipher v4,v17,v31
.*:	(13 02 dd 49|49 dd 02 13) 	vncipherlast v24,v2,v27
.*:	(12 f5 bd 4c|4c bd f5 12) 	vbpermq v23,v21,v23
.*:	(13 72 4d 4e|4e 4d 72 13) 	vpksdus v27,v18,v9
.*:	(13 7d dd 84|84 dd 7d 13) 	vnand   v27,v29,v27
.*:	(12 73 c5 c4|c4 c5 73 12) 	vsld    v19,v19,v24
.*:	(10 ad 05 c8|c8 05 ad 10) 	vsbox   v5,v13
.*:	(13 23 3d ce|ce 3d 23 13) 	vpksdss v25,v3,v7
.*:	(13 88 04 c7|c7 04 88 13) 	vcmpequd\. v28,v8,v0
.*:	(13 40 d6 4e|4e d6 40 13) 	vupkhsw v26,v26
.*:	(10 a7 36 82|82 36 a7 10) 	vshasigmaw v5,v7,0,6
.*:	(13 95 76 84|84 76 95 13) 	veqv    v28,v21,v14
.*:	(10 28 9e 8c|8c 9e 28 10) 	vmrgow  v1,v8,v19
.*:	(10 0a 56 c2|c2 56 0a 10) 	vshasigmad v0,v10,0,10
.*:	(10 bb 76 c4|c4 76 bb 10) 	vsrd    v5,v27,v14
.*:	(11 60 6e ce|ce 6e 60 11) 	vupklsw v11,v13
.*:	(11 c0 87 02|02 87 c0 11) 	vclzb   v14,v16
.*:	(12 80 df 03|03 df 80 12) 	vpopcntb v20,v27
.*:	(13 80 5f 42|42 5f 80 13) 	vclzh   v28,v11
.*:	(13 00 4f 43|43 4f 00 13) 	vpopcnth v24,v9
.*:	(13 60 ff 82|82 ff 60 13) 	vclzw   v27,v31
.*:	(12 20 9f 83|83 9f 20 12) 	vpopcntw v17,v19
.*:	(11 80 ef c2|c2 ef 80 11) 	vclzd   v12,v29
.*:	(12 e0 b7 c3|c3 b7 e0 12) 	vpopcntd v23,v22
.*:	(13 14 ee c7|c7 ee 14 13) 	vcmpgtud\. v24,v20,v29
.*:	(11 26 df c7|c7 df 26 11) 	vcmpgtsd\. v9,v6,v27
.*:	(7f ce d0 19|19 d0 ce 7f) 	lxsiwzx vs62,r14,r26
.*:	(7d 00 c8 19|19 c8 00 7d) 	lxsiwzx vs40,0,r25
.*:	(7f 20 d0 98|98 d0 20 7f) 	lxsiwax vs25,0,r26
.*:	(7c 60 18 98|98 18 60 7c) 	lxsiwax vs3,0,r3
.*:	(7f cc 00 67|67 00 cc 7f) 	(mfvrd   r12,v30|mfvsrd  r12,vs62)
.*:	(7d 94 00 e6|e6 00 94 7d) 	(mffprwz r20,f12|mfvsrwz r20,vs12)
.*:	(7d c9 71 18|18 71 c9 7d) 	stxsiwx vs14,r9,r14
.*:	(7e a0 41 18|18 41 a0 7e) 	stxsiwx vs21,0,r8
.*:	(7e 0b 01 67|67 01 0b 7e) 	(mtvrd   v16,r11|mtvsrd  vs48,r11)
.*:	(7f f7 01 a7|a7 01 f7 7f) 	(mtvrwa  v31,r23|mtvsrwa vs63,r23)
.*:	(7e 1a 01 e6|e6 01 1a 7e) 	(mtfprwz f16,r26|mtvsrwz vs16,r26)
.*:	(7d b3 6c 18|18 6c b3 7d) 	lxsspx  vs13,r19,r13
.*:	(7e 40 6c 18|18 6c 40 7e) 	lxsspx  vs18,0,r13
.*:	(7d 62 25 19|19 25 62 7d) 	stxsspx vs43,r2,r4
.*:	(7e e0 5d 19|19 5d e0 7e) 	stxsspx vs55,0,r11
.*:	(f2 d0 c8 05|05 c8 d0 f2) 	xsaddsp vs54,vs48,vs25
.*:	(f1 d2 08 0c|0c 08 d2 f1) 	xsmaddasp vs14,vs50,vs1
.*:	(f3 56 50 42|42 50 56 f3) 	xssubsp vs26,vs22,vs42
.*:	(f3 75 a0 4e|4e a0 75 f3) 	xsmaddmsp vs27,vs53,vs52
.*:	(f1 00 d8 2a|2a d8 00 f1) 	xsrsqrtesp vs8,vs59
.*:	(f1 80 48 2e|2e 48 80 f1) 	xssqrtsp vs12,vs41
.*:	(f3 2b 00 83|83 00 2b f3) 	xsmulsp vs57,vs11,vs32
.*:	(f0 d4 d0 89|89 d0 d4 f0) 	xsmsubasp vs38,vs20,vs26
.*:	(f3 53 30 c0|c0 30 53 f3) 	xsdivsp vs26,vs19,vs6
.*:	(f0 65 b8 cf|cf b8 65 f0) 	xsmsubmsp vs35,vs37,vs55
.*:	(f3 60 40 69|69 40 60 f3) 	xsresp  vs59,vs8
.*:	(f1 81 0c 0f|0f 0c 81 f1) 	xsnmaddasp vs44,vs33,vs33
.*:	(f2 3e f4 4c|4c f4 3e f2) 	xsnmaddmsp vs17,vs62,vs30
.*:	(f2 d4 fc 8d|8d fc d4 f2) 	xsnmsubasp vs54,vs52,vs31
.*:	(f0 a5 d4 cb|cb d4 a5 f0) 	xsnmsubmsp vs37,vs5,vs58
.*:	(f3 d6 65 56|56 65 d6 f3) 	xxlorc  vs30,vs54,vs44
.*:	(f2 2e ed 91|91 ed 2e f2) 	xxlnand vs49,vs14,vs29
.*:	(f3 d6 f5 d1|d1 f5 d6 f3) 	xxleqv  vs62,vs22,vs30
.*:	(f3 80 b4 2f|2f b4 80 f3) 	xscvdpspn vs60,vs54
.*:	(f2 c0 6c 66|66 6c c0 f2) 	xsrsp   vs22,vs45
.*:	(f3 40 dc a2|a2 dc 40 f3) 	xscvuxdsp vs26,vs59
.*:	(f0 c0 8c e3|e3 8c c0 f0) 	xscvsxdsp vs38,vs49
.*:	(f3 60 d5 2d|2d d5 60 f3) 	xscvspdpn vs59,vs26
.*:	(ff 0e 16 8c|8c 16 0e ff) 	fmrgow  f24,f14,f2
.*:	(fe c7 2f 8c|8c 2f c7 fe) 	fmrgew  f22,f7,f5
.*:	(7c 00 71 9c|9c 71 00 7c) 	msgsnd  r14
.*:	(7c 00 b9 dc|dc b9 00 7c) 	msgclr  r23
.*:	(7d 00 2e 99|99 2e 00 7d) 	lxvd2x  vs40,0,r5
.*:	(7d 00 2e 99|99 2e 00 7d) 	lxvd2x  vs40,0,r5
.*:	(7d 54 36 98|98 36 54 7d) 	lxvd2x  vs10,r20,r6
.*:	(7d 54 36 98|98 36 54 7d) 	lxvd2x  vs10,r20,r6
.*:	(7d 20 3f 99|99 3f 20 7d) 	stxvd2x vs41,0,r7
.*:	(7d 20 3f 99|99 3f 20 7d) 	stxvd2x vs41,0,r7
.*:	(7d 75 47 98|98 47 75 7d) 	stxvd2x vs11,r21,r8
.*:	(7d 75 47 98|98 47 75 7d) 	stxvd2x vs11,r21,r8
.*:	(7e 80 38 68|68 38 80 7e) 	lbarx   r20,0,r7
.*:	(7e 80 38 68|68 38 80 7e) 	lbarx   r20,0,r7
.*:	(7e 80 38 69|69 38 80 7e) 	lbarx   r20,0,r7,1
.*:	(7e 81 38 68|68 38 81 7e) 	lbarx   r20,r1,r7
.*:	(7e 81 38 68|68 38 81 7e) 	lbarx   r20,r1,r7
.*:	(7e 81 38 69|69 38 81 7e) 	lbarx   r20,r1,r7,1
.*:	(7e a0 40 a8|a8 40 a0 7e) 	ldarx   r21,0,r8
.*:	(7e a0 40 a8|a8 40 a0 7e) 	ldarx   r21,0,r8
.*:	(7e a0 40 a9|a9 40 a0 7e) 	ldarx   r21,0,r8,1
.*:	(7e a1 40 a8|a8 40 a1 7e) 	ldarx   r21,r1,r8
.*:	(7e a1 40 a8|a8 40 a1 7e) 	ldarx   r21,r1,r8
.*:	(7e a1 40 a9|a9 40 a1 7e) 	ldarx   r21,r1,r8,1
.*:	(7e c0 48 e8|e8 48 c0 7e) 	lharx   r22,0,r9
.*:	(7e c0 48 e8|e8 48 c0 7e) 	lharx   r22,0,r9
.*:	(7e c0 48 e9|e9 48 c0 7e) 	lharx   r22,0,r9,1
.*:	(7e c1 48 e8|e8 48 c1 7e) 	lharx   r22,r1,r9
.*:	(7e c1 48 e8|e8 48 c1 7e) 	lharx   r22,r1,r9
.*:	(7e c1 48 e9|e9 48 c1 7e) 	lharx   r22,r1,r9,1
.*:	(7e e0 50 28|28 50 e0 7e) 	lwarx   r23,0,r10
.*:	(7e e0 50 28|28 50 e0 7e) 	lwarx   r23,0,r10
.*:	(7e e0 50 29|29 50 e0 7e) 	lwarx   r23,0,r10,1
.*:	(7e e1 50 28|28 50 e1 7e) 	lwarx   r23,r1,r10
.*:	(7e e1 50 28|28 50 e1 7e) 	lwarx   r23,r1,r10
.*:	(7e e1 50 29|29 50 e1 7e) 	lwarx   r23,r1,r10,1
.*:	(7d 40 3d 6d|6d 3d 40 7d) 	stbcx\.  r10,0,r7
.*:	(7d 41 3d 6d|6d 3d 41 7d) 	stbcx\.  r10,r1,r7
.*:	(7d 60 45 ad|ad 45 60 7d) 	sthcx\.  r11,0,r8
.*:	(7d 61 45 ad|ad 45 61 7d) 	sthcx\.  r11,r1,r8
.*:	(7d 80 49 2d|2d 49 80 7d) 	stwcx\.  r12,0,r9
.*:	(7d 81 49 2d|2d 49 81 7d) 	stwcx\.  r12,r1,r9
.*:	(7d a0 51 ad|ad 51 a0 7d) 	stdcx\.  r13,0,r10
.*:	(7d a1 51 ad|ad 51 a1 7d) 	stdcx\.  r13,r1,r10
.*:	(4e 80 04 60|60 04 80 4e) 	btar
.*:	(4e 80 04 60|60 04 80 4e) 	btar
.*:	(4e 00 04 60|60 04 00 4e) 	bdnztar
.*:	(4e 00 04 61|61 04 00 4e) 	bdnztarl
.*:	(4e 40 04 60|60 04 40 4e) 	bdztar
.*:	(4e 40 04 61|61 04 40 4e) 	bdztarl
.*:	(4e 80 04 60|60 04 80 4e) 	btar
.*:	(4e 80 04 61|61 04 80 4e) 	btarl
.*:	(4f 00 04 60|60 04 00 4f) 	bdnztar-
.*:	(4f 00 04 61|61 04 00 4f) 	bdnztarl-
.*:	(4f 20 04 60|60 04 20 4f) 	bdnztar\+
.*:	(4f 20 04 61|61 04 20 4f) 	bdnztarl\+
.*:	(4f 40 04 60|60 04 40 4f) 	bdztar-
.*:	(4f 40 04 61|61 04 40 4f) 	bdztarl-
.*:	(4f 60 04 60|60 04 60 4f) 	bdztar\+
.*:	(4f 60 04 61|61 04 60 4f) 	bdztarl\+
.*:	(4c 90 04 60|60 04 90 4c) 	bgetar  cr4
.*:	(4c 90 04 60|60 04 90 4c) 	bgetar  cr4
.*:	(4c 90 04 61|61 04 90 4c) 	bgetarl cr4
.*:	(4c 90 04 61|61 04 90 4c) 	bgetarl cr4
.*:	(4c 91 04 60|60 04 91 4c) 	bletar  cr4
.*:	(4c 91 04 60|60 04 91 4c) 	bletar  cr4
.*:	(4c 91 04 61|61 04 91 4c) 	bletarl cr4
.*:	(4c 91 04 61|61 04 91 4c) 	bletarl cr4
.*:	(4c 92 04 60|60 04 92 4c) 	bnetar  cr4
.*:	(4c 92 04 61|61 04 92 4c) 	bnetarl cr4
.*:	(4c 93 04 60|60 04 93 4c) 	bnstar  cr4
.*:	(4c 93 04 60|60 04 93 4c) 	bnstar  cr4
.*:	(4c 93 04 61|61 04 93 4c) 	bnstarl cr4
.*:	(4c 93 04 61|61 04 93 4c) 	bnstarl cr4
.*:	(4c d0 04 60|60 04 d0 4c) 	bgetar- cr4
.*:	(4c d0 04 60|60 04 d0 4c) 	bgetar- cr4
.*:	(4c d0 04 61|61 04 d0 4c) 	bgetarl- cr4
.*:	(4c d0 04 61|61 04 d0 4c) 	bgetarl- cr4
.*:	(4c d1 04 60|60 04 d1 4c) 	bletar- cr4
.*:	(4c d1 04 60|60 04 d1 4c) 	bletar- cr4
.*:	(4c d1 04 61|61 04 d1 4c) 	bletarl- cr4
.*:	(4c d1 04 61|61 04 d1 4c) 	bletarl- cr4
.*:	(4c d2 04 60|60 04 d2 4c) 	bnetar- cr4
.*:	(4c d2 04 61|61 04 d2 4c) 	bnetarl- cr4
.*:	(4c d3 04 60|60 04 d3 4c) 	bnstar- cr4
.*:	(4c d3 04 60|60 04 d3 4c) 	bnstar- cr4
.*:	(4c d3 04 61|61 04 d3 4c) 	bnstarl- cr4
.*:	(4c d3 04 61|61 04 d3 4c) 	bnstarl- cr4
.*:	(4c f0 04 60|60 04 f0 4c) 	bgetar\+ cr4
.*:	(4c f0 04 60|60 04 f0 4c) 	bgetar\+ cr4
.*:	(4c f0 04 61|61 04 f0 4c) 	bgetarl\+ cr4
.*:	(4c f0 04 61|61 04 f0 4c) 	bgetarl\+ cr4
.*:	(4c f1 04 60|60 04 f1 4c) 	bletar\+ cr4
.*:	(4c f1 04 60|60 04 f1 4c) 	bletar\+ cr4
.*:	(4c f1 04 61|61 04 f1 4c) 	bletarl\+ cr4
.*:	(4c f1 04 61|61 04 f1 4c) 	bletarl\+ cr4
.*:	(4c f2 04 60|60 04 f2 4c) 	bnetar\+ cr4
.*:	(4c f2 04 61|61 04 f2 4c) 	bnetarl\+ cr4
.*:	(4c f3 04 60|60 04 f3 4c) 	bnstar\+ cr4
.*:	(4c f3 04 60|60 04 f3 4c) 	bnstar\+ cr4
.*:	(4c f3 04 61|61 04 f3 4c) 	bnstarl\+ cr4
.*:	(4c f3 04 61|61 04 f3 4c) 	bnstarl\+ cr4
.*:	(4d 90 04 60|60 04 90 4d) 	blttar  cr4
.*:	(4d 90 04 61|61 04 90 4d) 	blttarl cr4
.*:	(4d 91 04 60|60 04 91 4d) 	bgttar  cr4
.*:	(4d 91 04 61|61 04 91 4d) 	bgttarl cr4
.*:	(4d 92 04 60|60 04 92 4d) 	beqtar  cr4
.*:	(4d 92 04 61|61 04 92 4d) 	beqtarl cr4
.*:	(4d 93 04 60|60 04 93 4d) 	bsotar  cr4
.*:	(4d 93 04 60|60 04 93 4d) 	bsotar  cr4
.*:	(4d 93 04 61|61 04 93 4d) 	bsotarl cr4
.*:	(4d 93 04 61|61 04 93 4d) 	bsotarl cr4
.*:	(4d d0 04 60|60 04 d0 4d) 	blttar- cr4
.*:	(4d d0 04 61|61 04 d0 4d) 	blttarl- cr4
.*:	(4d d1 04 60|60 04 d1 4d) 	bgttar- cr4
.*:	(4d d1 04 61|61 04 d1 4d) 	bgttarl- cr4
.*:	(4d d2 04 60|60 04 d2 4d) 	beqtar- cr4
.*:	(4d d2 04 61|61 04 d2 4d) 	beqtarl- cr4
.*:	(4d d3 04 60|60 04 d3 4d) 	bsotar- cr4
.*:	(4d d3 04 60|60 04 d3 4d) 	bsotar- cr4
.*:	(4d d3 04 61|61 04 d3 4d) 	bsotarl- cr4
.*:	(4d d3 04 61|61 04 d3 4d) 	bsotarl- cr4
.*:	(4d f0 04 60|60 04 f0 4d) 	blttar\+ cr4
.*:	(4d f0 04 61|61 04 f0 4d) 	blttarl\+ cr4
.*:	(4d f1 04 60|60 04 f1 4d) 	bgttar\+ cr4
.*:	(4d f1 04 61|61 04 f1 4d) 	bgttarl\+ cr4
.*:	(4d f2 04 60|60 04 f2 4d) 	beqtar\+ cr4
.*:	(4d f2 04 61|61 04 f2 4d) 	beqtarl\+ cr4
.*:	(4d f3 04 60|60 04 f3 4d) 	bsotar\+ cr4
.*:	(4d f3 04 60|60 04 f3 4d) 	bsotar\+ cr4
.*:	(4d f3 04 61|61 04 f3 4d) 	bsotarl\+ cr4
.*:	(4d f3 04 61|61 04 f3 4d) 	bsotarl\+ cr4
.*:	(4c 08 04 60|60 04 08 4c) 	bdnzftar 4\*cr2\+lt
.*:	(4c 08 04 61|61 04 08 4c) 	bdnzftarl 4\*cr2\+lt
.*:	(4c 48 04 60|60 04 48 4c) 	bdzftar 4\*cr2\+lt
.*:	(4c 48 04 61|61 04 48 4c) 	bdzftarl 4\*cr2\+lt
.*:	(4c 88 04 60|60 04 88 4c) 	bgetar  cr2
.*:	(4c 88 04 61|61 04 88 4c) 	bgetarl cr2
.*:	(4c c8 04 60|60 04 c8 4c) 	bgetar- cr2
.*:	(4c c8 04 61|61 04 c8 4c) 	bgetarl- cr2
.*:	(4c e8 04 60|60 04 e8 4c) 	bgetar\+ cr2
.*:	(4c e8 04 61|61 04 e8 4c) 	bgetarl\+ cr2
.*:	(4d 08 04 60|60 04 08 4d) 	bdnzttar 4\*cr2\+lt
.*:	(4d 08 04 61|61 04 08 4d) 	bdnzttarl 4\*cr2\+lt
.*:	(4d 48 04 60|60 04 48 4d) 	bdzttar 4\*cr2\+lt
.*:	(4d 48 04 61|61 04 48 4d) 	bdzttarl 4\*cr2\+lt
.*:	(4d 88 04 60|60 04 88 4d) 	blttar  cr2
.*:	(4d 88 04 61|61 04 88 4d) 	blttarl cr2
.*:	(4d c8 04 60|60 04 c8 4d) 	blttar- cr2
.*:	(4d c8 04 61|61 04 c8 4d) 	blttarl- cr2
.*:	(4d e8 04 60|60 04 e8 4d) 	blttar\+ cr2
.*:	(4d e8 04 61|61 04 e8 4d) 	blttarl\+ cr2
.*:	(4d ca 04 60|60 04 ca 4d) 	beqtar- cr2
.*:	(4d ca 04 61|61 04 ca 4d) 	beqtarl- cr2
.*:	(4d ea 04 60|60 04 ea 4d) 	beqtar\+ cr2
.*:	(4d ea 04 61|61 04 ea 4d) 	beqtarl\+ cr2
.*:	(4d 89 04 60|60 04 89 4d) 	bgttar  cr2
.*:	(4d 89 04 60|60 04 89 4d) 	bgttar  cr2
.*:	(4d 89 1c 60|60 1c 89 4d) 	bgttar  cr2,3
.*:	(4d 89 04 61|61 04 89 4d) 	bgttarl cr2
.*:	(4d 89 04 61|61 04 89 4d) 	bgttarl cr2
.*:	(4d 89 1c 61|61 1c 89 4d) 	bgttarl cr2,3
.*:	(7f 5a d3 78|78 d3 5a 7f) 	miso
.*:	(7f 5a d3 78|78 d3 5a 7f) 	miso
#pass
