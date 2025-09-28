#as: -mpower7
#objdump: -dr -Mpower7
#name: POWER7 tests (includes DFP, Altivec and VSX)

.*

Disassembly of section \.text:

0+00 <power7>:
.*:	(7c 64 2e 98|98 2e 64 7c) 	lxvd2x  vs3,r4,r5
.*:	(7d 64 2e 99|99 2e 64 7d) 	lxvd2x  vs43,r4,r5
.*:	(7c 64 2f 98|98 2f 64 7c) 	stxvd2x vs3,r4,r5
.*:	(7d 64 2f 99|99 2f 64 7d) 	stxvd2x vs43,r4,r5
.*:	(f0 64 28 50|50 28 64 f0) 	xxmrghd vs3,vs4,vs5
.*:	(f1 6c 68 57|57 68 6c f1) 	xxmrghd vs43,vs44,vs45
.*:	(f0 64 2b 50|50 2b 64 f0) 	xxmrgld vs3,vs4,vs5
.*:	(f1 6c 6b 57|57 6b 6c f1) 	xxmrgld vs43,vs44,vs45
.*:	(f0 64 28 50|50 28 64 f0) 	xxmrghd vs3,vs4,vs5
.*:	(f1 6c 68 57|57 68 6c f1) 	xxmrghd vs43,vs44,vs45
.*:	(f0 64 2b 50|50 2b 64 f0) 	xxmrgld vs3,vs4,vs5
.*:	(f1 6c 6b 57|57 6b 6c f1) 	xxmrgld vs43,vs44,vs45
.*:	(f0 64 29 50|50 29 64 f0) 	xxpermdi vs3,vs4,vs5,1
.*:	(f1 6c 69 57|57 69 6c f1) 	xxpermdi vs43,vs44,vs45,1
.*:	(f0 64 2a 50|50 2a 64 f0) 	xxpermdi vs3,vs4,vs5,2
.*:	(f1 6c 6a 57|57 6a 6c f1) 	xxpermdi vs43,vs44,vs45,2
.*:	(f0 64 27 80|80 27 64 f0) 	xvmovdp vs3,vs4
.*:	(f1 6c 67 87|87 67 6c f1) 	xvmovdp vs43,vs44
.*:	(f0 64 27 80|80 27 64 f0) 	xvmovdp vs3,vs4
.*:	(f1 6c 67 87|87 67 6c f1) 	xvmovdp vs43,vs44
.*:	(f0 64 2f 80|80 2f 64 f0) 	xvcpsgndp vs3,vs4,vs5
.*:	(f1 6c 6f 87|87 6f 6c f1) 	xvcpsgndp vs43,vs44,vs45
.*:	(4c 00 03 24|24 03 00 4c) 	doze
.*:	(4c 00 03 64|64 03 00 4c) 	nap
.*:	(4c 00 03 a4|a4 03 00 4c) 	sleep
.*:	(4c 00 03 e4|e4 03 00 4c) 	rvwinkle
.*:	(7c 83 01 34|34 01 83 7c) 	prtyw   r3,r4
.*:	(7d cd 01 74|74 01 cd 7d) 	prtyd   r13,r14
.*:	(7d 5c 02 a6|a6 02 5c 7d) 	mfcfar  r10
.*:	(7d 7c 03 a6|a6 03 7c 7d) 	mtcfar  r11
.*:	(7c 83 2b f8|f8 2b 83 7c) 	cmpb    r3,r4,r5
.*:	(7d 4b 66 2a|2a 66 4b 7d) 	lwzcix  r10,r11,r12
.*:	(ee 11 90 04|04 90 11 ee) 	dadd    f16,f17,f18
.*:	(fe 96 c0 04|04 c0 96 fe) 	daddq   f20,f22,f24
.*:	(7c 60 06 6c|6c 06 60 7c) 	dss     3
.*:	(7e 00 06 6c|6c 06 00 7e) 	dssall
.*:	(7c 25 22 ac|ac 22 25 7c) 	dst     r5,r4,1
.*:	(7e 08 3a ac|ac 3a 08 7e) 	dstt    r8,r7,0
.*:	(7c 65 32 ec|ec 32 65 7c) 	dstst   r5,r6,3
.*:	(7e 44 2a ec|ec 2a 44 7e) 	dststt  r4,r5,2
.*:	(7d 4b 63 56|56 63 4b 7d) 	divwe   r10,r11,r12
.*:	(7d 6c 6b 57|57 6b 6c 7d) 	divwe\.  r11,r12,r13
.*:	(7d 8d 77 56|56 77 8d 7d) 	divweo  r12,r13,r14
.*:	(7d ae 7f 57|57 7f ae 7d) 	divweo\. r13,r14,r15
.*:	(7d 4b 63 16|16 63 4b 7d) 	divweu  r10,r11,r12
.*:	(7d 6c 6b 17|17 6b 6c 7d) 	divweu\. r11,r12,r13
.*:	(7d 8d 77 16|16 77 8d 7d) 	divweuo r12,r13,r14
.*:	(7d ae 7f 17|17 7f ae 7d) 	divweuo\. r13,r14,r15
.*:	(7e 27 d9 f8|f8 d9 27 7e) 	bpermd  r7,r17,r27
.*:	(7e 8a 02 f4|f4 02 8a 7e) 	popcntw r10,r20
.*:	(7e 8a 03 f4|f4 03 8a 7e) 	popcntd r10,r20
.*:	(7e 95 b4 28|28 b4 95 7e) 	ldbrx   r20,r21,r22
.*:	(7e 95 b5 28|28 b5 95 7e) 	stdbrx  r20,r21,r22
.*:	(7d 40 56 ee|ee 56 40 7d) 	lfiwzx  f10,0,r10
.*:	(7d 49 56 ee|ee 56 49 7d) 	lfiwzx  f10,r9,r10
.*:	(ec 80 2e 9c|9c 2e 80 ec) 	fcfids  f4,f5
.*:	(ec 80 2e 9d|9d 2e 80 ec) 	fcfids\. f4,f5
.*:	(ec 80 2f 9c|9c 2f 80 ec) 	fcfidus f4,f5
.*:	(ec 80 2f 9d|9d 2f 80 ec) 	fcfidus\. f4,f5
.*:	(fc 80 29 1c|1c 29 80 fc) 	fctiwu  f4,f5
.*:	(fc 80 29 1d|1d 29 80 fc) 	fctiwu\. f4,f5
.*:	(fc 80 29 1e|1e 29 80 fc) 	fctiwuz f4,f5
.*:	(fc 80 29 1f|1f 29 80 fc) 	fctiwuz\. f4,f5
.*:	(fc 80 2f 5c|5c 2f 80 fc) 	fctidu  f4,f5
.*:	(fc 80 2f 5d|5d 2f 80 fc) 	fctidu\. f4,f5
.*:	(fc 80 2f 5e|5e 2f 80 fc) 	fctiduz f4,f5
.*:	(fc 80 2f 5f|5f 2f 80 fc) 	fctiduz\. f4,f5
.*:	(fc 80 2f 9c|9c 2f 80 fc) 	fcfidu  f4,f5
.*:	(fc 80 2f 9d|9d 2f 80 fc) 	fcfidu\. f4,f5
.*:	(fc 0a 59 00|00 59 0a fc) 	ftdiv   cr0,f10,f11
.*:	(ff 8a 59 00|00 59 8a ff) 	ftdiv   cr7,f10,f11
.*:	(fc 00 51 40|40 51 00 fc) 	ftsqrt  cr0,f10
.*:	(ff 80 51 40|40 51 80 ff) 	ftsqrt  cr7,f10
.*:	(7e 08 4a 2c|2c 4a 08 7e) 	dcbtt   r8,r9
.*:	(7e 08 49 ec|ec 49 08 7e) 	dcbtstt r8,r9
.*:	(ed 40 66 44|44 66 40 ed) 	dcffix  f10,f12
.*:	(ee 80 b6 45|45 b6 80 ee) 	dcffix\. f20,f22
.*:	(fd c0 78 30|30 78 c0 fd) 	fre     f14,f15
.*:	(fd c0 78 31|31 78 c0 fd) 	fre\.    f14,f15
.*:	(ed c0 78 30|30 78 c0 ed) 	fres    f14,f15
.*:	(ed c0 78 31|31 78 c0 ed) 	fres\.   f14,f15
.*:	(fd c0 78 34|34 78 c0 fd) 	frsqrte f14,f15
.*:	(fd c0 78 35|35 78 c0 fd) 	frsqrte\. f14,f15
.*:	(ed c0 78 34|34 78 c0 ed) 	frsqrtes f14,f15
.*:	(ed c0 78 35|35 78 c0 ed) 	frsqrtes\. f14,f15
.*:	(7c 43 27 1e|1e 27 43 7c) 	isel    r2,r3,r4,4\*cr7\+lt
.*:	(7f 7b db 78|78 db 7b 7f) 	yield
.*:	(7f 7b db 78|78 db 7b 7f) 	yield
.*:	(60 42 00 00|00 00 42 60) 	ori     r2,r2,0
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 00 00 00|00 00 00 60) 	nop
.*:	(60 42 00 00|00 00 42 60) 	ori     r2,r2,0
.*:	(7f bd eb 78|78 eb bd 7f) 	mdoio
.*:	(7f bd eb 78|78 eb bd 7f) 	mdoio
.*:	(7f de f3 78|78 f3 de 7f) 	mdoom
.*:	(7f de f3 78|78 f3 de 7f) 	mdoom
.*:	(7d 60 52 64|64 52 60 7d) 	tlbie   r10,r11
#pass
