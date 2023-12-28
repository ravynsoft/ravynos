#objdump: -d -Mcom
#as: -a32 -mcom
#name: PowerPC COMMON instructions

.*

Disassembly of section \.text:

0+00 <start>:

.*:	(7c 83 28 39|39 28 83 7c) 	and.    r3,r4,r5
.*:	(7c 83 28 38|38 28 83 7c) 	and     r3,r4,r5
.*:	(7d cd 78 78|78 78 cd 7d) 	andc    r13,r14,r15
.*:	(7e 30 90 79|79 90 30 7e) 	andc.   r16,r17,r18
.*:	(48 00 00 02|02 00 00 48) 	ba      0 <start>
.*:	(40 01 00 00|00 00 01 40) 	bdnzf-  1,14 <start\+0x14>
.*:	(40 85 00 02|02 00 85 40) 	blea-   1,0 <start>
.*:	(40 43 00 01|01 00 43 40) 	bdzfl-  3,1c <start\+0x1c>
.*:	(41 47 00 03|03 00 47 41) 	bdztla- 7,0 <start>
.*:	(4e 80 04 20|20 04 80 4e) 	bctr
.*:	(4e 80 04 21|21 04 80 4e) 	bctrl
.*:	(42 40 00 02|02 00 40 42) 	bdza-   0 <start>
.*:	(42 40 00 00|00 00 40 42) 	bdz-    30 <start\+0x30>
.*:	(42 40 00 03|03 00 40 42) 	bdzla-  0 <start>
.*:	(42 40 00 01|01 00 40 42) 	bdzl-   38 <start\+0x38>
.*:	(41 82 00 00|00 00 82 41) 	beq-    3c <start\+0x3c>
.*:	(41 8a 00 02|02 00 8a 41) 	beqa-   2,0 <start>
.*:	(41 86 00 01|01 00 86 41) 	beql-   1,44 <start\+0x44>
.*:	(41 8e 00 03|03 00 8e 41) 	beqla-  3,0 <start>
.*:	(40 80 00 00|00 00 80 40) 	bge-    4c <start\+0x4c>
.*:	(40 90 00 02|02 00 90 40) 	bgea-   4,0 <start>
.*:	(40 88 00 01|01 00 88 40) 	bgel-   2,54 <start\+0x54>
.*:	(40 98 00 03|03 00 98 40) 	bgela-  6,0 <start>
.*:	(41 91 00 00|00 00 91 41) 	bgt-    4,5c <start\+0x5c>
.*:	(41 99 00 02|02 00 99 41) 	bgta-   6,0 <start>
.*:	(41 95 00 01|01 00 95 41) 	bgtl-   5,64 <start\+0x64>
.*:	(41 9d 00 03|03 00 9d 41) 	bgtla-  7,0 <start>
.*:	(48 00 00 00|00 00 00 48) 	b       6c <start\+0x6c>
.*:	(48 00 00 03|03 00 00 48) 	bla     0 <start>
.*:	(40 81 00 00|00 00 81 40) 	ble-    74 <start\+0x74>
.*:	(40 91 00 02|02 00 91 40) 	blea-   4,0 <start>
.*:	(40 89 00 01|01 00 89 40) 	blel-   2,7c <start\+0x7c>
.*:	(40 99 00 03|03 00 99 40) 	blela-  6,0 <start>
.*:	(48 00 00 01|01 00 00 48) 	bl      84 <start\+0x84>
.*:	(41 80 00 00|00 00 80 41) 	blt-    88 <start\+0x88>
.*:	(41 88 00 02|02 00 88 41) 	blta-   2,0 <start>
.*:	(41 84 00 01|01 00 84 41) 	bltl-   1,90 <start\+0x90>
.*:	(41 8c 00 03|03 00 8c 41) 	bltla-  3,0 <start>
.*:	(40 82 00 00|00 00 82 40) 	bne-    98 <start\+0x98>
.*:	(40 8a 00 02|02 00 8a 40) 	bnea-   2,0 <start>
.*:	(40 86 00 01|01 00 86 40) 	bnel-   1,a0 <start\+0xa0>
.*:	(40 8e 00 03|03 00 8e 40) 	bnela-  3,0 <start>
.*:	(40 85 00 00|00 00 85 40) 	ble-    1,a8 <start\+0xa8>
.*:	(40 95 00 02|02 00 95 40) 	blea-   5,0 <start>
.*:	(40 8d 00 01|01 00 8d 40) 	blel-   3,b0 <start\+0xb0>
.*:	(40 9d 00 03|03 00 9d 40) 	blela-  7,0 <start>
.*:	(40 84 00 00|00 00 84 40) 	bge-    1,b8 <start\+0xb8>
.*:	(40 94 00 02|02 00 94 40) 	bgea-   5,0 <start>
.*:	(40 8c 00 01|01 00 8c 40) 	bgel-   3,c0 <start\+0xc0>
.*:	(40 9c 00 03|03 00 9c 40) 	bgela-  7,0 <start>
.*:	(40 93 00 00|00 00 93 40) 	bns-    4,c8 <start\+0xc8>
.*:	(40 9b 00 02|02 00 9b 40) 	bnsa-   6,0 <start>
.*:	(40 97 00 01|01 00 97 40) 	bnsl-   5,d0 <start\+0xd0>
.*:	(40 9f 00 03|03 00 9f 40) 	bnsla-  7,0 <start>
.*:	(41 93 00 00|00 00 93 41) 	bso-    4,d8 <start\+0xd8>
.*:	(41 9b 00 02|02 00 9b 41) 	bsoa-   6,0 <start>
.*:	(41 97 00 01|01 00 97 41) 	bsol-   5,e0 <start\+0xe0>
.*:	(41 9f 00 03|03 00 9f 41) 	bsola-  7,0 <start>
.*:	(4c 85 32 02|02 32 85 4c) 	crand   4,5,6
.*:	(4c 64 29 02|02 29 64 4c) 	crandc  3,4,5
.*:	(4c e0 0a 42|42 0a e0 4c) 	creqv   7,0,1
.*:	(4c 22 19 c2|c2 19 22 4c) 	crnand  1,2,3
.*:	(4c 01 10 42|42 10 01 4c) 	crnor   0,1,2
.*:	(4c a6 3b 82|82 3b a6 4c) 	cror    5,6,7
.*:	(4c a6 33 82|82 33 a6 4c) 	crmove  5,6
.*:	(4c a6 33 82|82 33 a6 4c) 	crmove  5,6
.*:	(4c 43 23 42|42 23 43 4c) 	crorc   2,3,4
.*:	(4c c7 01 82|82 01 c7 4c) 	crxor   6,7,0
.*:	(7d 6a 62 39|39 62 6a 7d) 	eqv.    r10,r11,r12
.*:	(7d 6a 62 38|38 62 6a 7d) 	eqv     r10,r11,r12
.*:	(fe a0 fa 11|11 fa a0 fe) 	fabs.   f21,f31
.*:	(fe a0 fa 10|10 fa a0 fe) 	fabs    f21,f31
.*:	(fd 8a 58 40|40 58 8a fd) 	fcmpo   3,f10,f11
.*:	(fd 84 28 00|00 28 84 fd) 	fcmpu   3,f4,f5
.*:	(fc 60 20 91|91 20 60 fc) 	fmr.    f3,f4
.*:	(fc 60 20 90|90 20 60 fc) 	fmr     f3,f4
.*:	(fe 80 f1 11|11 f1 80 fe) 	fnabs.  f20,f30
.*:	(fe 80 f1 10|10 f1 80 fe) 	fnabs   f20,f30
.*:	(fc 60 20 51|51 20 60 fc) 	fneg.   f3,f4
.*:	(fc 60 20 50|50 20 60 fc) 	fneg    f3,f4
.*:	(fc c0 38 18|18 38 c0 fc) 	frsp    f6,f7
.*:	(fd 00 48 19|19 48 00 fd) 	frsp.   f8,f9
.*:	(89 21 00 00|00 00 21 89) 	lbz     r9,0\(r1\)
.*:	(8d 41 00 01|01 00 41 8d) 	lbzu    r10,1\(r1\)
.*:	(7e 95 b0 ee|ee b0 95 7e) 	lbzux   r20,r21,r22
.*:	(7c 64 28 ae|ae 28 64 7c) 	lbzx    r3,r4,r5
.*:	(ca a1 00 08|08 00 a1 ca) 	lfd     f21,8\(r1\)
.*:	(ce c1 00 10|10 00 c1 ce) 	lfdu    f22,16\(r1\)
.*:	(7e 95 b4 ee|ee b4 95 7e) 	lfdux   f20,r21,r22
.*:	(7d ae 7c ae|ae 7c ae 7d) 	lfdx    f13,r14,r15
.*:	(c2 61 00 00|00 00 61 c2) 	lfs     f19,0\(r1\)
.*:	(c6 81 00 04|04 00 81 c6) 	lfsu    f20,4\(r1\)
.*:	(7d 4b 64 6e|6e 64 4b 7d) 	lfsux   f10,r11,r12
.*:	(7d 4b 64 2e|2e 64 4b 7d) 	lfsx    f10,r11,r12
.*:	(a9 e1 00 06|06 00 e1 a9) 	lha     r15,6\(r1\)
.*:	(ae 01 00 08|08 00 01 ae) 	lhau    r16,8\(r1\)
.*:	(7d 2a 5a ee|ee 5a 2a 7d) 	lhaux   r9,r10,r11
.*:	(7d 2a 5a ae|ae 5a 2a 7d) 	lhax    r9,r10,r11
.*:	(7c 64 2e 2c|2c 2e 64 7c) 	lhbrx   r3,r4,r5
.*:	(a1 a1 00 00|00 00 a1 a1) 	lhz     r13,0\(r1\)
.*:	(a5 c1 00 02|02 00 c1 a5) 	lhzu    r14,2\(r1\)
.*:	(7e 96 c2 6e|6e c2 96 7e) 	lhzux   r20,r22,r24
.*:	(7e f8 ca 2e|2e ca f8 7e) 	lhzx    r23,r24,r25
.*:	(4c 04 00 00|00 00 04 4c) 	mcrf    0,1
.*:	(fd 90 00 80|80 00 90 fd) 	mcrfs   3,4
.*:	(7d 80 04 00|00 04 80 7d) 	mcrxr   3
.*:	(7c 60 00 26|26 00 60 7c) 	mfcr    r3
.*:	(7c 69 02 a6|a6 02 69 7c) 	mfctr   r3
.*:	(7c b3 02 a6|a6 02 b3 7c) 	mfdar   r5
.*:	(7c 92 02 a6|a6 02 92 7c) 	mfdsisr r4
.*:	(ff c0 04 8e|8e 04 c0 ff) 	mffs    f30
.*:	(ff e0 04 8f|8f 04 e0 ff) 	mffs.   f31
.*:	(7c 48 02 a6|a6 02 48 7c) 	mflr    r2
.*:	(7e 60 00 a6|a6 00 60 7e) 	mfmsr   r19
.*:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
.*:	(7c 25 02 a6|a6 02 25 7c) 	mfrtcl  r1
.*:	(7c 04 02 a6|a6 02 04 7c) 	mfrtcu  r0
.*:	(7c d9 02 a6|a6 02 d9 7c) 	mfsdr1  r6
.*:	(7c 60 22 a6|a6 22 60 7c) 	mfspr   r3,128
.*:	(7c fa 02 a6|a6 02 fa 7c) 	mfsrr0  r7
.*:	(7d 1b 02 a6|a6 02 1b 7d) 	mfsrr1  r8
.*:	(7f c1 02 a6|a6 02 c1 7f) 	mfxer   r30
.*:	(7f fe fb 79|79 fb fe 7f) 	mr.     r30,r31
.*:	(7f fe fb 79|79 fb fe 7f) 	mr.     r30,r31
.*:	(7f fe fb 78|78 fb fe 7f) 	mr      r30,r31
.*:	(7f fe fb 78|78 fb fe 7f) 	mr      r30,r31
.*:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
.*:	(7c 68 01 20|20 01 68 7c) 	mtcrf   128,r3
.*:	(7e 69 03 a6|a6 03 69 7e) 	mtctr   r19
.*:	(7e b3 03 a6|a6 03 b3 7e) 	mtdar   r21
.*:	(7f 16 03 a6|a6 03 16 7f) 	mtdec   r24
.*:	(7e 92 03 a6|a6 03 92 7e) 	mtdsisr r20
.*:	(fc 60 00 8d|8d 00 60 fc) 	mtfsb0. 3
.*:	(fc 60 00 8c|8c 00 60 fc) 	mtfsb0  3
.*:	(fc 60 00 4d|4d 00 60 fc) 	mtfsb1. 3
.*:	(fc 60 00 4c|4c 00 60 fc) 	mtfsb1  3
.*:	(fc 0c 55 8e|8e 55 0c fc) 	mtfsf   6,f10
.*:	(fc 0c 5d 8f|8f 5d 0c fc) 	mtfsf.  6,f11
.*:	(ff 00 01 0c|0c 01 00 ff) 	mtfsfi  6,0
.*:	(ff 00 f1 0d|0d f1 00 ff) 	mtfsfi. 6,15
.*:	(7e 48 03 a6|a6 03 48 7e) 	mtlr    r18
.*:	(7d 40 01 24|24 01 40 7d) 	mtmsr   r10
.*:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
.*:	(7e f5 03 a6|a6 03 f5 7e) 	mtrtcl  r23
.*:	(7e d4 03 a6|a6 03 d4 7e) 	mtrtcu  r22
.*:	(7f 39 03 a6|a6 03 39 7f) 	mtsdr1  r25
.*:	(7c 60 23 a6|a6 23 60 7c) 	mtspr   128,r3
.*:	(7f 5a 03 a6|a6 03 5a 7f) 	mtsrr0  r26
.*:	(7f 7b 03 a6|a6 03 7b 7f) 	mtsrr1  r27
.*:	(7e 21 03 a6|a6 03 21 7e) 	mtxer   r17
.*:	(7f bc f3 b9|b9 f3 bc 7f) 	nand.   r28,r29,r30
.*:	(7f bc f3 b8|b8 f3 bc 7f) 	nand    r28,r29,r30
.*:	(7c 64 00 d1|d1 00 64 7c) 	neg.    r3,r4
.*:	(7c 64 00 d0|d0 00 64 7c) 	neg     r3,r4
.*:	(7e 11 04 d0|d0 04 11 7e) 	nego    r16,r17
.*:	(7e 53 04 d1|d1 04 53 7e) 	nego.   r18,r19
.*:	(7e b4 b0 f9|f9 b0 b4 7e) 	nor.    r20,r21,r22
.*:	(7e b4 b0 f8|f8 b0 b4 7e) 	nor     r20,r21,r22
.*:	(7e b4 a8 f9|f9 a8 b4 7e) 	not.    r20,r21
.*:	(7e b4 a8 f9|f9 a8 b4 7e) 	not.    r20,r21
.*:	(7e b4 a8 f8|f8 a8 b4 7e) 	not     r20,r21
.*:	(7e b4 a8 f8|f8 a8 b4 7e) 	not     r20,r21
.*:	(7c 40 23 78|78 23 40 7c) 	or      r0,r2,r4
.*:	(7d cc 83 79|79 83 cc 7d) 	or.     r12,r14,r16
.*:	(7e 0f 8b 38|38 8b 0f 7e) 	orc     r15,r16,r17
.*:	(7e 72 a3 39|39 a3 72 7e) 	orc.    r18,r19,r20
.*:	(4c 00 00 64|64 00 00 4c) 	rfi
.*:	(99 61 00 02|02 00 61 99) 	stb     r11,2\(r1\)
.*:	(9d 81 00 03|03 00 81 9d) 	stbu    r12,3\(r1\)
.*:	(7d ae 79 ee|ee 79 ae 7d) 	stbux   r13,r14,r15
.*:	(7c 64 29 ae|ae 29 64 7c) 	stbx    r3,r4,r5
.*:	(db 21 00 20|20 00 21 db) 	stfd    f25,32\(r1\)
.*:	(df 41 00 28|28 00 41 df) 	stfdu   f26,40\(r1\)
.*:	(7c 01 15 ee|ee 15 01 7c) 	stfdux  f0,r1,r2
.*:	(7f be fd ae|ae fd be 7f) 	stfdx   f29,r30,r31
.*:	(d2 e1 00 14|14 00 e1 d2) 	stfs    f23,20\(r1\)
.*:	(d7 01 00 18|18 00 01 d7) 	stfsu   f24,24\(r1\)
.*:	(7f 5b e5 6e|6e e5 5b 7f) 	stfsux  f26,r27,r28
.*:	(7e f8 cd 2e|2e cd f8 7e) 	stfsx   f23,r24,r25
.*:	(b2 21 00 0a|0a 00 21 b2) 	sth     r17,10\(r1\)
.*:	(7c c7 47 2c|2c 47 c7 7c) 	sthbrx  r6,r7,r8
.*:	(b6 41 00 0c|0c 00 41 b6) 	sthu    r18,12\(r1\)
.*:	(7e b6 bb 6e|6e bb b6 7e) 	sthux   r21,r22,r23
.*:	(7d 8d 73 2e|2e 73 8d 7d) 	sthx    r12,r13,r14
.*:	(7f dd fa 79|79 fa dd 7f) 	xor.    r29,r30,r31
.*:	(7f dd fa 78|78 fa dd 7f) 	xor     r29,r30,r31
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(68 00 00 00|00 00 00 68) 	xnop
.*:	(68 00 00 00|00 00 00 68) 	xnop
#pass
