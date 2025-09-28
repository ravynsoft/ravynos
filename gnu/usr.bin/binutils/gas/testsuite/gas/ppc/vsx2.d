#as: -mpower8
#objdump: -dr -Mpower8
#name: VSX ISA 2.07 instructions

.*

Disassembly of section \.text:

0+00 <vsx2>:
.*:	(7f ce d0 19|19 d0 ce 7f) 	lxsiwzx vs62,r14,r26
.*:	(7d 00 c8 19|19 c8 00 7d) 	lxsiwzx vs40,0,r25
.*:	(7f 20 d0 98|98 d0 20 7f) 	lxsiwax vs25,0,r26
.*:	(7c 60 18 98|98 18 60 7c) 	lxsiwax vs3,0,r3
.*:	(7f cc 00 66|66 00 cc 7f) 	(mffprd  r12,f30|mfvsrd  r12,vs30)
.*:	(7f cc 00 66|66 00 cc 7f) 	(mffprd  r12,f30|mfvsrd  r12,vs30)
.*:	(7f cc 00 67|67 00 cc 7f) 	(mfvrd   r12,v30|mfvsrd  r12,vs62)
.*:	(7f cc 00 67|67 00 cc 7f) 	(mfvrd   r12,v30|mfvsrd  r12,vs62)
.*:	(7d 94 00 e6|e6 00 94 7d) 	(mffprwz r20,f12|mfvsrwz r20,vs12)
.*:	(7d 94 00 e6|e6 00 94 7d) 	(mffprwz r20,f12|mfvsrwz r20,vs12)
.*:	(7d 95 00 e7|e7 00 95 7d) 	(mfvrwz  r21,v12|mfvsrwz r21,vs44)
.*:	(7d 95 00 e7|e7 00 95 7d) 	(mfvrwz  r21,v12|mfvsrwz r21,vs44)
.*:	(7d c9 71 18|18 71 c9 7d) 	stxsiwx vs14,r9,r14
.*:	(7e a0 41 18|18 41 a0 7e) 	stxsiwx vs21,0,r8
.*:	(7d 7c 01 66|66 01 7c 7d) 	(mtfprd  f11,r28|mtvsrd  vs11,r28)
.*:	(7d 7c 01 66|66 01 7c 7d) 	(mtfprd  f11,r28|mtvsrd  vs11,r28)
.*:	(7d 7d 01 67|67 01 7d 7d) 	(mtvrd   v11,r29|mtvsrd  vs43,r29)
.*:	(7d 7d 01 67|67 01 7d 7d) 	(mtvrd   v11,r29|mtvsrd  vs43,r29)
.*:	(7f 16 01 a6|a6 01 16 7f) 	(mtfprwa f24,r22|mtvsrwa vs24,r22)
.*:	(7f 16 01 a6|a6 01 16 7f) 	(mtfprwa f24,r22|mtvsrwa vs24,r22)
.*:	(7f 37 01 a7|a7 01 37 7f) 	(mtvrwa  v25,r23|mtvsrwa vs57,r23)
.*:	(7f 37 01 a7|a7 01 37 7f) 	(mtvrwa  v25,r23|mtvsrwa vs57,r23)
.*:	(7f 5b 01 e6|e6 01 5b 7f) 	(mtfprwz f26,r27|mtvsrwz vs26,r27)
.*:	(7f 5b 01 e6|e6 01 5b 7f) 	(mtfprwz f26,r27|mtvsrwz vs26,r27)
.*:	(7f 7c 01 e7|e7 01 7c 7f) 	(mtvrwz  v27,r28|mtvsrwz vs59,r28)
.*:	(7f 7c 01 e7|e7 01 7c 7f) 	(mtvrwz  v27,r28|mtvsrwz vs59,r28)
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
#pass
