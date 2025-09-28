#as: -ma2
#objdump: -dr -Ma2
#name: A2 tests


.*


Disassembly of section \.text:

0+00 <start>:
.*:	(7c 85 32 15|15 32 85 7c) 	add\.    r4,r5,r6
.*:	(7c 85 32 14|14 32 85 7c) 	add     r4,r5,r6
.*:	(7c 85 30 15|15 30 85 7c) 	addc\.   r4,r5,r6
.*:	(7c 85 30 14|14 30 85 7c) 	addc    r4,r5,r6
.*:	(7c 85 34 15|15 34 85 7c) 	addco\.  r4,r5,r6
.*:	(7c 85 34 14|14 34 85 7c) 	addco   r4,r5,r6
.*:	(7c 85 31 15|15 31 85 7c) 	adde\.   r4,r5,r6
.*:	(7c 85 31 14|14 31 85 7c) 	adde    r4,r5,r6
.*:	(7c 85 35 15|15 35 85 7c) 	addeo\.  r4,r5,r6
.*:	(7c 85 35 14|14 35 85 7c) 	addeo   r4,r5,r6
.*:	(38 85 00 0d|0d 00 85 38) 	addi    r4,r5,13
.*:	(38 85 ff f3|f3 ff 85 38) 	addi    r4,r5,-13
.*:	(34 85 00 0d|0d 00 85 34) 	addic\.  r4,r5,13
.*:	(34 85 ff f3|f3 ff 85 34) 	addic\.  r4,r5,-13
.*:	(30 85 00 0d|0d 00 85 30) 	addic   r4,r5,13
.*:	(30 85 ff f3|f3 ff 85 30) 	addic   r4,r5,-13
.*:	(3c 85 00 17|17 00 85 3c) 	addis   r4,r5,23
.*:	(3c 85 ff e9|e9 ff 85 3c) 	addis   r4,r5,-23
.*:	(7c 85 01 d5|d5 01 85 7c) 	addme\.  r4,r5
.*:	(7c 85 01 d4|d4 01 85 7c) 	addme   r4,r5
.*:	(7c 85 05 d5|d5 05 85 7c) 	addmeo\. r4,r5
.*:	(7c 85 05 d4|d4 05 85 7c) 	addmeo  r4,r5
.*:	(7c 85 36 15|15 36 85 7c) 	addo\.   r4,r5,r6
.*:	(7c 85 36 14|14 36 85 7c) 	addo    r4,r5,r6
.*:	(7c 85 01 95|95 01 85 7c) 	addze\.  r4,r5
.*:	(7c 85 01 94|94 01 85 7c) 	addze   r4,r5
.*:	(7c 85 05 95|95 05 85 7c) 	addzeo\. r4,r5
.*:	(7c 85 05 94|94 05 85 7c) 	addzeo  r4,r5
.*:	(7c a4 30 39|39 30 a4 7c) 	and\.    r4,r5,r6
.*:	(7c a4 30 38|38 30 a4 7c) 	and     r4,r5,r6
.*:	(7c a4 30 79|79 30 a4 7c) 	andc\.   r4,r5,r6
.*:	(7c a4 30 78|78 30 a4 7c) 	andc    r4,r5,r6
.*:	(70 a4 00 06|06 00 a4 70) 	andi\.   r4,r5,6
.*:	(74 a4 00 06|06 00 a4 74) 	andis\.  r4,r5,6
.*:	(00 00 02 00|00 02 00 00) 	attn
.*:	(48 00 00 02|02 00 00 48) 	ba      0 <start>
.*: R_PPC(|64)_ADDR24	label_abs
.*:	(40 8a 00 00|00 00 8a 40) 	bne     cr2,90 <start\+0x90>
.*: R_PPC(|64)_REL14	foo
.*:	(40 ca 00 00|00 00 ca 40) 	bne-    cr2,94 <start\+0x94>
.*: R_PPC(|64)_REL14	foo
.*:	(40 ea 00 00|00 00 ea 40) 	bne\+    cr2,98 <start\+0x98>
.*: R_PPC(|64)_REL14	foo
.*:	(40 85 00 02|02 00 85 40) 	blea    cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(40 c5 00 02|02 00 c5 40) 	blea-   cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(40 e5 00 02|02 00 e5 40) 	blea\+   cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(4c 86 0c 20|20 0c 86 4c) 	bnectr  cr1,1
.*:	(4c c6 04 20|20 04 c6 4c) 	bnectr- cr1
.*:	(4c e6 04 20|20 04 e6 4c) 	bnectr\+ cr1
.*:	(4c 86 0c 21|21 0c 86 4c) 	bnectrl cr1,1
.*:	(4c c6 04 21|21 04 c6 4c) 	bnectrl- cr1
.*:	(4c e6 04 21|21 04 e6 4c) 	bnectrl\+ cr1
.*:	(40 8a 00 01|01 00 8a 40) 	bnel    cr2,c0 <start\+0xc0>
.*: R_PPC(|64)_REL14	foo
.*:	(40 ca 00 01|01 00 ca 40) 	bnel-   cr2,c4 <start\+0xc4>
.*: R_PPC(|64)_REL14	foo
.*:	(40 ea 00 01|01 00 ea 40) 	bnel\+   cr2,c8 <start\+0xc8>
.*: R_PPC(|64)_REL14	foo
.*:	(40 85 00 03|03 00 85 40) 	blela   cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(40 c5 00 03|03 00 c5 40) 	blela-  cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(40 e5 00 03|03 00 e5 40) 	blela\+  cr1,0 <start>
.*: R_PPC(|64)_ADDR14	foo_abs
.*:	(4c 86 08 20|20 08 86 4c) 	bnelr   cr1,1
.*:	(4c c6 00 20|20 00 c6 4c) 	bnelr-  cr1
.*:	(4c e6 00 20|20 00 e6 4c) 	bnelr\+  cr1
.*:	(4c 86 08 21|21 08 86 4c) 	bnelrl  cr1,1
.*:	(4c c6 00 21|21 00 c6 4c) 	bnelrl- cr1
.*:	(4c e6 00 21|21 00 e6 4c) 	bnelrl\+ cr1
.*:	(48 00 00 00|00 00 00 48) 	b       f0 <start\+0xf0>
.*: R_PPC(|64)_REL24	label
.*:	(48 00 00 03|03 00 00 48) 	bla     0 <start>
.*: R_PPC(|64)_ADDR24	label_abs
.*:	(48 00 00 01|01 00 00 48) 	bl      f8 <start\+0xf8>
.*: R_PPC(|64)_REL24	label
.*:	(7d 6a 61 f8|f8 61 6a 7d) 	bpermd  r10,r11,r12
.*:	(7c a7 40 00|00 40 a7 7c) 	cmpd    cr1,r7,r8
.*:	(7d 6a 63 f8|f8 63 6a 7d) 	cmpb    r10,r11,r12
.*:	(2c aa 00 0d|0d 00 aa 2c) 	cmpdi   cr1,r10,13
.*:	(2c aa ff f3|f3 ff aa 2c) 	cmpdi   cr1,r10,-13
.*:	(7c a7 40 40|40 40 a7 7c) 	cmpld   cr1,r7,r8
.*:	(28 aa 00 64|64 00 aa 28) 	cmpldi  cr1,r10,100
.*:	(7e b4 00 75|75 00 b4 7e) 	cntlzd\. r20,r21
.*:	(7e b4 00 74|74 00 b4 7e) 	cntlzd  r20,r21
.*:	(7e b4 00 35|35 00 b4 7e) 	cntlzw\. r20,r21
.*:	(7e b4 00 34|34 00 b4 7e) 	cntlzw  r20,r21
.*:	(4c 22 1a 02|02 1a 22 4c) 	crand   gt,eq,so
.*:	(4c 22 19 02|02 19 22 4c) 	crandc  gt,eq,so
.*:	(4c 22 1a 42|42 1a 22 4c) 	creqv   gt,eq,so
.*:	(4c 22 19 c2|c2 19 22 4c) 	crnand  gt,eq,so
.*:	(4c 22 18 42|42 18 22 4c) 	crnor   gt,eq,so
.*:	(4c 22 1b 82|82 1b 22 4c) 	cror    gt,eq,so
.*:	(4c 22 1b 42|42 1b 22 4c) 	crorc   gt,eq,so
.*:	(4c 22 19 82|82 19 22 4c) 	crxor   gt,eq,so
.*:	(7c 0a 5d ec|ec 5d 0a 7c) 	dcba    r10,r11
.*:	(7c 0a 58 ac|ac 58 0a 7c) 	dcbf    r10,r11
.*:	(7c 2a 58 ac|ac 58 2a 7c) 	dcbfl   r10,r11
.*:	(7c 0a 58 fe|fe 58 0a 7c) 	dcbfep  r10,r11
.*:	(7c 0a 5b ac|ac 5b 0a 7c) 	dcbi    r10,r11
.*:	(7c 0a 5b 0c|0c 5b 0a 7c) 	dcblc   r10,r11
.*:	(7c 2a 5b 0c|0c 5b 2a 7c) 	dcblc   1,r10,r11
.*:	(7c 0a 58 6c|6c 58 0a 7c) 	dcbst   r10,r11
.*:	(7c 0a 58 7e|7e 58 0a 7c) 	dcbstep r10,r11
.*:	(7c 0a 5a 2c|2c 5a 0a 7c) 	dcbt    r10,r11
.*:	(7c 2a 5a 2c|2c 5a 2a 7c) 	dcbt    1,r10,r11
.*:	(7d 4b 62 7e|7e 62 4b 7d) 	dcbtep  r10,r11,r12
.*:	(7c 0a 59 4c|4c 59 0a 7c) 	dcbtls  r10,r11
.*:	(7c 2a 59 4c|4c 59 2a 7c) 	dcbtls  1,r10,r11
.*:	(7c 0a 59 ec|ec 59 0a 7c) 	dcbtst  r10,r11
.*:	(7c 2a 59 ec|ec 59 2a 7c) 	dcbtst  1,r10,r11
.*:	(7d 4b 61 fe|fe 61 4b 7d) 	dcbtstep r10,r11,r12
.*:	(7c 0a 59 0c|0c 59 0a 7c) 	dcbtstls r10,r11
.*:	(7c 2a 59 0c|0c 59 2a 7c) 	dcbtstls 1,r10,r11
.*:	(7c 0a 5f ec|ec 5f 0a 7c) 	dcbz    r10,r11
.*:	(7c 0a 5f fe|fe 5f 0a 7c) 	dcbzep  r10,r11
.*:	(7c 00 03 8c|8c 03 00 7c) 	dccci
.*:	(7c 00 03 8c|8c 03 00 7c) 	dccci
.*:	(7c 00 03 8c|8c 03 00 7c) 	dccci
.*:	(7d 40 03 8c|8c 03 40 7d) 	dci     10
.*:	(7e 95 b3 d3|d3 b3 95 7e) 	divd\.   r20,r21,r22
.*:	(7e 95 b3 d2|d2 b3 95 7e) 	divd    r20,r21,r22
.*:	(7e 95 b7 d3|d3 b7 95 7e) 	divdo\.  r20,r21,r22
.*:	(7e 95 b7 d2|d2 b7 95 7e) 	divdo   r20,r21,r22
.*:	(7e 95 b3 93|93 b3 95 7e) 	divdu\.  r20,r21,r22
.*:	(7e 95 b3 92|92 b3 95 7e) 	divdu   r20,r21,r22
.*:	(7e 95 b7 93|93 b7 95 7e) 	divduo\. r20,r21,r22
.*:	(7e 95 b7 92|92 b7 95 7e) 	divduo  r20,r21,r22
.*:	(7e 95 b3 d7|d7 b3 95 7e) 	divw\.   r20,r21,r22
.*:	(7e 95 b3 d6|d6 b3 95 7e) 	divw    r20,r21,r22
.*:	(7e 95 b7 d7|d7 b7 95 7e) 	divwo\.  r20,r21,r22
.*:	(7e 95 b7 d6|d6 b7 95 7e) 	divwo   r20,r21,r22
.*:	(7e 95 b3 97|97 b3 95 7e) 	divwu\.  r20,r21,r22
.*:	(7e 95 b3 96|96 b3 95 7e) 	divwu   r20,r21,r22
.*:	(7e 95 b7 97|97 b7 95 7e) 	divwuo\. r20,r21,r22
.*:	(7e 95 b7 96|96 b7 95 7e) 	divwuo  r20,r21,r22
.*:	(7e b4 b2 39|39 b2 b4 7e) 	eqv\.    r20,r21,r22
.*:	(7e b4 b2 38|38 b2 b4 7e) 	eqv     r20,r21,r22
.*:	(7c 0a 58 66|66 58 0a 7c) 	eratilx 0,r10,r11
.*:	(7c 2a 58 66|66 58 2a 7c) 	eratilx 1,r10,r11
.*:	(7c ea 58 66|66 58 ea 7c) 	eratilx 7,r10,r11
.*:	(7d 4b 66 66|66 66 4b 7d) 	erativax r10,r11,r12
.*:	(7d 4b 01 66|66 01 4b 7d) 	eratre  r10,r11,0
.*:	(7d 4b 19 66|66 19 4b 7d) 	eratre  r10,r11,3
.*:	(7d 4b 61 27|27 61 4b 7d) 	eratsx\. r10,r11,r12
.*:	(7d 4b 61 26|26 61 4b 7d) 	eratsx  r10,r11,r12
.*:	(7d 4b 01 a6|a6 01 4b 7d) 	eratwe  r10,r11,0
.*:	(7d 4b 19 a6|a6 19 4b 7d) 	eratwe  r10,r11,3
.*:	(7d 6a 07 75|75 07 6a 7d) 	extsb\.  r10,r11
.*:	(7d 6a 07 74|74 07 6a 7d) 	extsb   r10,r11
.*:	(7d 6a 07 35|35 07 6a 7d) 	extsh\.  r10,r11
.*:	(7d 6a 07 34|34 07 6a 7d) 	extsh   r10,r11
.*:	(7d 6a 07 b5|b5 07 6a 7d) 	extsw\.  r10,r11
.*:	(7d 6a 07 b4|b4 07 6a 7d) 	extsw   r10,r11
.*:	(fe 80 aa 11|11 aa 80 fe) 	fabs\.   f20,f21
.*:	(fe 80 aa 10|10 aa 80 fe) 	fabs    f20,f21
.*:	(fe 95 b0 2b|2b b0 95 fe) 	fadd\.   f20,f21,f22
.*:	(fe 95 b0 2a|2a b0 95 fe) 	fadd    f20,f21,f22
.*:	(ee 95 b0 2b|2b b0 95 ee) 	fadds\.  f20,f21,f22
.*:	(ee 95 b0 2a|2a b0 95 ee) 	fadds   f20,f21,f22
.*:	(fe 80 ae 9d|9d ae 80 fe) 	fcfid\.  f20,f21
.*:	(fe 80 ae 9c|9c ae 80 fe) 	fcfid   f20,f21
.*:	(fc 14 a8 40|40 a8 14 fc) 	fcmpo   cr0,f20,f21
.*:	(fc 94 a8 40|40 a8 94 fc) 	fcmpo   cr1,f20,f21
.*:	(fc 14 a8 00|00 a8 14 fc) 	fcmpu   cr0,f20,f21
.*:	(fc 94 a8 00|00 a8 94 fc) 	fcmpu   cr1,f20,f21
.*:	(fe 95 b0 11|11 b0 95 fe) 	fcpsgn\. f20,f21,f22
.*:	(fe 95 b0 10|10 b0 95 fe) 	fcpsgn  f20,f21,f22
.*:	(fe 80 ae 5d|5d ae 80 fe) 	fctid\.  f20,f21
.*:	(fe 80 ae 5c|5c ae 80 fe) 	fctid   f20,f21
.*:	(fe 80 ae 5f|5f ae 80 fe) 	fctidz\. f20,f21
.*:	(fe 80 ae 5e|5e ae 80 fe) 	fctidz  f20,f21
.*:	(fe 80 a8 1d|1d a8 80 fe) 	fctiw\.  f20,f21
.*:	(fe 80 a8 1c|1c a8 80 fe) 	fctiw   f20,f21
.*:	(fe 80 a8 1f|1f a8 80 fe) 	fctiwz\. f20,f21
.*:	(fe 80 a8 1e|1e a8 80 fe) 	fctiwz  f20,f21
.*:	(fe 95 b0 25|25 b0 95 fe) 	fdiv\.   f20,f21,f22
.*:	(fe 95 b0 24|24 b0 95 fe) 	fdiv    f20,f21,f22
.*:	(ee 95 b0 25|25 b0 95 ee) 	fdivs\.  f20,f21,f22
.*:	(ee 95 b0 24|24 b0 95 ee) 	fdivs   f20,f21,f22
.*:	(fe 95 bd bb|bb bd 95 fe) 	fmadd\.  f20,f21,f22,f23
.*:	(fe 95 bd ba|ba bd 95 fe) 	fmadd   f20,f21,f22,f23
.*:	(ee 95 bd bb|bb bd 95 ee) 	fmadds\. f20,f21,f22,f23
.*:	(ee 95 bd ba|ba bd 95 ee) 	fmadds  f20,f21,f22,f23
.*:	(fe 80 a8 91|91 a8 80 fe) 	fmr\.    f20,f21
.*:	(fe 80 a8 90|90 a8 80 fe) 	fmr     f20,f21
.*:	(fe 95 bd b9|b9 bd 95 fe) 	fmsub\.  f20,f21,f22,f23
.*:	(fe 95 bd b8|b8 bd 95 fe) 	fmsub   f20,f21,f22,f23
.*:	(ee 95 bd b9|b9 bd 95 ee) 	fmsubs\. f20,f21,f22,f23
.*:	(ee 95 bd b8|b8 bd 95 ee) 	fmsubs  f20,f21,f22,f23
.*:	(fe 95 05 b3|b3 05 95 fe) 	fmul\.   f20,f21,f22
.*:	(fe 95 05 b2|b2 05 95 fe) 	fmul    f20,f21,f22
.*:	(ee 95 05 b3|b3 05 95 ee) 	fmuls\.  f20,f21,f22
.*:	(ee 95 05 b2|b2 05 95 ee) 	fmuls   f20,f21,f22
.*:	(fe 80 a9 11|11 a9 80 fe) 	fnabs\.  f20,f21
.*:	(fe 80 a9 10|10 a9 80 fe) 	fnabs   f20,f21
.*:	(fe 80 a8 51|51 a8 80 fe) 	fneg\.   f20,f21
.*:	(fe 80 a8 50|50 a8 80 fe) 	fneg    f20,f21
.*:	(fe 95 bd bf|bf bd 95 fe) 	fnmadd\. f20,f21,f22,f23
.*:	(fe 95 bd be|be bd 95 fe) 	fnmadd  f20,f21,f22,f23
.*:	(ee 95 bd bf|bf bd 95 ee) 	fnmadds\. f20,f21,f22,f23
.*:	(ee 95 bd be|be bd 95 ee) 	fnmadds f20,f21,f22,f23
.*:	(fe 95 bd bd|bd bd 95 fe) 	fnmsub\. f20,f21,f22,f23
.*:	(fe 95 bd bc|bc bd 95 fe) 	fnmsub  f20,f21,f22,f23
.*:	(ee 95 bd bd|bd bd 95 ee) 	fnmsubs\. f20,f21,f22,f23
.*:	(ee 95 bd bc|bc bd 95 ee) 	fnmsubs f20,f21,f22,f23
.*:	(fe 80 a8 31|31 a8 80 fe) 	fre\.    f20,f21
.*:	(fe 80 a8 30|30 a8 80 fe) 	fre     f20,f21
.*:	(fe 80 a8 31|31 a8 80 fe) 	fre\.    f20,f21
.*:	(fe 80 a8 30|30 a8 80 fe) 	fre     f20,f21
.*:	(fe 81 a8 31|31 a8 81 fe) 	fre\.    f20,f21,1
.*:	(fe 81 a8 30|30 a8 81 fe) 	fre     f20,f21,1
.*:	(ee 80 a8 31|31 a8 80 ee) 	fres\.   f20,f21
.*:	(ee 80 a8 30|30 a8 80 ee) 	fres    f20,f21
.*:	(ee 80 a8 31|31 a8 80 ee) 	fres\.   f20,f21
.*:	(ee 80 a8 30|30 a8 80 ee) 	fres    f20,f21
.*:	(ee 81 a8 31|31 a8 81 ee) 	fres\.   f20,f21,1
.*:	(ee 81 a8 30|30 a8 81 ee) 	fres    f20,f21,1
.*:	(fe 80 ab d1|d1 ab 80 fe) 	frim\.   f20,f21
.*:	(fe 80 ab d0|d0 ab 80 fe) 	frim    f20,f21
.*:	(fe 80 ab 11|11 ab 80 fe) 	frin\.   f20,f21
.*:	(fe 80 ab 10|10 ab 80 fe) 	frin    f20,f21
.*:	(fe 80 ab 91|91 ab 80 fe) 	frip\.   f20,f21
.*:	(fe 80 ab 90|90 ab 80 fe) 	frip    f20,f21
.*:	(fe 80 ab 51|51 ab 80 fe) 	friz\.   f20,f21
.*:	(fe 80 ab 50|50 ab 80 fe) 	friz    f20,f21
.*:	(fe 80 a8 19|19 a8 80 fe) 	frsp\.   f20,f21
.*:	(fe 80 a8 18|18 a8 80 fe) 	frsp    f20,f21
.*:	(fe 80 a8 35|35 a8 80 fe) 	frsqrte\. f20,f21
.*:	(fe 80 a8 34|34 a8 80 fe) 	frsqrte f20,f21
.*:	(fe 80 a8 35|35 a8 80 fe) 	frsqrte\. f20,f21
.*:	(fe 80 a8 34|34 a8 80 fe) 	frsqrte f20,f21
.*:	(fe 81 a8 35|35 a8 81 fe) 	frsqrte\. f20,f21,1
.*:	(fe 81 a8 34|34 a8 81 fe) 	frsqrte f20,f21,1
.*:	(ee 80 a8 34|34 a8 80 ee) 	frsqrtes f20,f21
.*:	(ee 80 a8 35|35 a8 80 ee) 	frsqrtes\. f20,f21
.*:	(ee 80 a8 34|34 a8 80 ee) 	frsqrtes f20,f21
.*:	(ee 80 a8 35|35 a8 80 ee) 	frsqrtes\. f20,f21
.*:	(ee 81 a8 34|34 a8 81 ee) 	frsqrtes f20,f21,1
.*:	(ee 81 a8 35|35 a8 81 ee) 	frsqrtes\. f20,f21,1
.*:	(fe 95 bd af|af bd 95 fe) 	fsel\.   f20,f21,f22,f23
.*:	(fe 95 bd ae|ae bd 95 fe) 	fsel    f20,f21,f22,f23
.*:	(fe 80 a8 2d|2d a8 80 fe) 	fsqrt\.  f20,f21
.*:	(fe 80 a8 2c|2c a8 80 fe) 	fsqrt   f20,f21
.*:	(ee 80 a8 2d|2d a8 80 ee) 	fsqrts\. f20,f21
.*:	(ee 80 a8 2c|2c a8 80 ee) 	fsqrts  f20,f21
.*:	(fe 95 b0 29|29 b0 95 fe) 	fsub\.   f20,f21,f22
.*:	(fe 95 b0 28|28 b0 95 fe) 	fsub    f20,f21,f22
.*:	(ee 95 b0 29|29 b0 95 ee) 	fsubs\.  f20,f21,f22
.*:	(ee 95 b0 28|28 b0 95 ee) 	fsubs   f20,f21,f22
.*:	(7c 0a 5f ac|ac 5f 0a 7c) 	icbi    r10,r11
.*:	(7c 0a 5f be|be 5f 0a 7c) 	icbiep  r10,r11
.*:	(7c 0a 58 2c|2c 58 0a 7c) 	icbt    r10,r11
.*:	(7c ea 58 2c|2c 58 ea 7c) 	icbt    7,r10,r11
.*:	(7c 0a 5b cc|cc 5b 0a 7c) 	icbtls  r10,r11
.*:	(7c ea 5b cc|cc 5b ea 7c) 	icbtls  7,r10,r11
.*:	(7c 00 07 8c|8c 07 00 7c) 	iccci
.*:	(7c 00 07 8c|8c 07 00 7c) 	iccci
.*:	(7c 00 07 8c|8c 07 00 7c) 	iccci
.*:	(7d 40 07 8c|8c 07 40 7d) 	ici     10
.*:	(7d 4b 63 2d|2d 63 4b 7d) 	icswx\.  r10,r11,r12
.*:	(7d 4b 63 2c|2c 63 4b 7d) 	icswx   r10,r11,r12
.*:	(7d 4b 65 de|de 65 4b 7d) 	isel    r10,r11,r12,4\*cr5\+so
.*:	(4c 00 01 2c|2c 01 00 4c) 	isync
.*:	(7d 4b 60 be|be 60 4b 7d) 	lbepx   r10,r11,r12
.*:	(89 4b ff ef|ef ff 4b 89) 	lbz     r10,-17\(r11\)
.*:	(89 4b 00 11|11 00 4b 89) 	lbz     r10,17\(r11\)
.*:	(8d 4b ff ff|ff ff 4b 8d) 	lbzu    r10,-1\(r11\)
.*:	(8d 4b 00 01|01 00 4b 8d) 	lbzu    r10,1\(r11\)
.*:	(7d 4b 68 ee|ee 68 4b 7d) 	lbzux   r10,r11,r13
.*:	(7d 4b 68 ae|ae 68 4b 7d) 	lbzx    r10,r11,r13
.*:	(e9 4b ff f8|f8 ff 4b e9) 	ld      r10,-8\(r11\)
.*:	(e9 4b 00 08|08 00 4b e9) 	ld      r10,8\(r11\)
.*:	(7d 4b 60 a8|a8 60 4b 7d) 	ldarx   r10,r11,r12
.*:	(7d 4b 60 a9|a9 60 4b 7d) 	ldarx   r10,r11,r12,1
.*:	(7d 4b 64 28|28 64 4b 7d) 	ldbrx   r10,r11,r12
.*:	(7d 4b 60 3a|3a 60 4b 7d) 	ldepx   r10,r11,r12
.*:	(e9 4b ff f9|f9 ff 4b e9) 	ldu     r10,-8\(r11\)
.*:	(e9 4b 00 09|09 00 4b e9) 	ldu     r10,8\(r11\)
.*:	(7d 4b 60 6a|6a 60 4b 7d) 	ldux    r10,r11,r12
.*:	(7d 4b 60 2a|2a 60 4b 7d) 	ldx     r10,r11,r12
.*:	(ca 8a ff f8|f8 ff 8a ca) 	lfd     f20,-8\(r10\)
.*:	(ca 8a 00 08|08 00 8a ca) 	lfd     f20,8\(r10\)
.*:	(7e 8a 5c be|be 5c 8a 7e) 	lfdepx  f20,r10,r11
.*:	(ce 8a ff f8|f8 ff 8a ce) 	lfdu    f20,-8\(r10\)
.*:	(ce 8a 00 08|08 00 8a ce) 	lfdu    f20,8\(r10\)
.*:	(7e 8a 5c ee|ee 5c 8a 7e) 	lfdux   f20,r10,r11
.*:	(7e 8a 5c ae|ae 5c 8a 7e) 	lfdx    f20,r10,r11
.*:	(7e 8a 5e ae|ae 5e 8a 7e) 	lfiwax  f20,r10,r11
.*:	(7e 8a 5e ee|ee 5e 8a 7e) 	lfiwzx  f20,r10,r11
.*:	(c2 8a ff fc|fc ff 8a c2) 	lfs     f20,-4\(r10\)
.*:	(c2 8a 00 04|04 00 8a c2) 	lfs     f20,4\(r10\)
.*:	(c6 8a ff fc|fc ff 8a c6) 	lfsu    f20,-4\(r10\)
.*:	(c6 8a 00 04|04 00 8a c6) 	lfsu    f20,4\(r10\)
.*:	(7e 8a 5c 6e|6e 5c 8a 7e) 	lfsux   f20,r10,r11
.*:	(7e 8a 5c 2e|2e 5c 8a 7e) 	lfsx    f20,r10,r11
.*:	(a9 4b 00 02|02 00 4b a9) 	lha     r10,2\(r11\)
.*:	(ad 4b ff fe|fe ff 4b ad) 	lhau    r10,-2\(r11\)
.*:	(7d 4b 62 ee|ee 62 4b 7d) 	lhaux   r10,r11,r12
.*:	(7d 4b 62 ae|ae 62 4b 7d) 	lhax    r10,r11,r12
.*:	(7d 4b 66 2c|2c 66 4b 7d) 	lhbrx   r10,r11,r12
.*:	(7d 4b 62 3e|3e 62 4b 7d) 	lhepx   r10,r11,r12
.*:	(a1 4b ff fe|fe ff 4b a1) 	lhz     r10,-2\(r11\)
.*:	(a1 4b 00 02|02 00 4b a1) 	lhz     r10,2\(r11\)
.*:	(a5 4b ff fe|fe ff 4b a5) 	lhzu    r10,-2\(r11\)
.*:	(a5 4b 00 02|02 00 4b a5) 	lhzu    r10,2\(r11\)
.*:	(7d 4b 62 6e|6e 62 4b 7d) 	lhzux   r10,r11,r12
.*:	(7d 4b 62 2e|2e 62 4b 7d) 	lhzx    r10,r11,r12
.*:	(e9 4b ff fe|fe ff 4b e9) 	lwa     r10,-4\(r11\)
.*:	(e9 4b 00 06|06 00 4b e9) 	lwa     r10,4\(r11\)
.*:	(7d 4b 60 28|28 60 4b 7d) 	lwarx   r10,r11,r12
.*:	(7d 4b 60 29|29 60 4b 7d) 	lwarx   r10,r11,r12,1
.*:	(7d 4b 62 ea|ea 62 4b 7d) 	lwaux   r10,r11,r12
.*:	(7d 4b 62 aa|aa 62 4b 7d) 	lwax    r10,r11,r12
.*:	(7d 4b 64 2c|2c 64 4b 7d) 	lwbrx   r10,r11,r12
.*:	(7d 4b 60 3e|3e 60 4b 7d) 	lwepx   r10,r11,r12
.*:	(81 4b ff fc|fc ff 4b 81) 	lwz     r10,-4\(r11\)
.*:	(81 4b 00 04|04 00 4b 81) 	lwz     r10,4\(r11\)
.*:	(85 4b ff fc|fc ff 4b 85) 	lwzu    r10,-4\(r11\)
.*:	(85 4b 00 04|04 00 4b 85) 	lwzu    r10,4\(r11\)
.*:	(7d 4b 60 6e|6e 60 4b 7d) 	lwzux   r10,r11,r12
.*:	(7d 4b 60 2e|2e 60 4b 7d) 	lwzx    r10,r11,r12
.*:	(7c 00 06 ac|ac 06 00 7c) 	mbar
.*:	(7c 00 06 ac|ac 06 00 7c) 	mbar
.*:	(7c 00 06 ac|ac 06 00 7c) 	mbar
.*:	(7c 20 06 ac|ac 06 20 7c) 	mbar    1
.*:	(4c 04 00 00|00 00 04 4c) 	mcrf    cr0,cr1
.*:	(fd 90 00 80|80 00 90 fd) 	mcrfs   cr3,cr4
.*:	(7c 00 04 00|00 04 00 7c) 	mcrxr   cr0
.*:	(7d 80 04 00|00 04 80 7d) 	mcrxr   cr3
.*:	(7c 60 00 26|26 00 60 7c) 	mfcr    r3
.*:	(7c 70 20 26|26 20 70 7c) 	mfocrf  r3,2
.*:	(7c 70 10 26|26 10 70 7c) 	mfocrf  r3,1
.*:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
.*:	(7d 4a 3a 87|87 3a 4a 7d) 	mfdcr\.  r10,234
.*:	(7d 4a 3a 86|86 3a 4a 7d) 	mfdcr   r10,234
.*:	(7d 4b 02 07|07 02 4b 7d) 	mfdcrx\. r10,r11
.*:	(7d 4b 02 06|06 02 4b 7d) 	mfdcrx  r10,r11
.*:	(fe 80 04 8f|8f 04 80 fe) 	mffs\.   f20
.*:	(fe 80 04 8e|8e 04 80 fe) 	mffs    f20
.*:	(7d 40 00 a6|a6 00 40 7d) 	mfmsr   r10
.*:	(7c 70 10 26|26 10 70 7c) 	mfocrf  r3,1
.*:	(7c 78 00 26|26 00 78 7c) 	mfocrf  r3,128
.*:	(7d 4a 3a a6|a6 3a 4a 7d) 	mfspr   r10,234
.*:	(7d 4c 42 a6|a6 42 4c 7d) 	mftb    r10
.*:	(7d 4d 42 a6|a6 42 4d 7d) 	mftbu   r10
.*:	(7c 00 51 dc|dc 51 00 7c) 	msgclr  r10
.*:	(7c 00 51 9c|9c 51 00 7c) 	msgsnd  r10
.*:	(7c 60 01 20|20 01 60 7c) 	mtcrf   0,r3
.*:	(7c 70 11 20|20 11 70 7c) 	mtocrf  1,r3
.*:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
.*:	(7c 6f f1 20|20 f1 6f 7c) 	mtcr    r3
.*:	(7d 4a 3b 87|87 3b 4a 7d) 	mtdcr\.  234,r10
.*:	(7d 4a 3b 86|86 3b 4a 7d) 	mtdcr   234,r10
.*:	(7d 6a 03 07|07 03 6a 7d) 	mtdcrx\. r10,r11
.*:	(7d 6a 03 06|06 03 6a 7d) 	mtdcrx  r10,r11
.*:	(fc 60 00 8d|8d 00 60 fc) 	mtfsb0\. 3
.*:	(fc 60 00 8c|8c 00 60 fc) 	mtfsb0  3
.*:	(fc 60 00 4d|4d 00 60 fc) 	mtfsb1\. 3
.*:	(fc 60 00 4c|4c 00 60 fc) 	mtfsb1  3
.*:	(fc 0c a5 8f|8f a5 0c fc) 	mtfsf\.  6,f20
.*:	(fc 0c a5 8e|8e a5 0c fc) 	mtfsf   6,f20
.*:	(fc 0c a5 8f|8f a5 0c fc) 	mtfsf\.  6,f20
.*:	(fc 0c a5 8e|8e a5 0c fc) 	mtfsf   6,f20
.*:	(fe 0d a5 8f|8f a5 0d fe) 	mtfsf\.  6,f20,1,1
.*:	(fe 0d a5 8e|8e a5 0d fe) 	mtfsf   6,f20,1,1
.*:	(ff 00 01 0d|0d 01 00 ff) 	mtfsfi\. 6,0
.*:	(ff 00 01 0c|0c 01 00 ff) 	mtfsfi  6,0
.*:	(ff 00 d1 0d|0d d1 00 ff) 	mtfsfi\. 6,13
.*:	(ff 00 d1 0c|0c d1 00 ff) 	mtfsfi  6,13
.*:	(ff 01 d1 0d|0d d1 01 ff) 	mtfsfi\. 6,13,1
.*:	(ff 01 d1 0c|0c d1 01 ff) 	mtfsfi  6,13,1
.*:	(7d 40 01 24|24 01 40 7d) 	mtmsr   r10
.*:	(7d 40 01 24|24 01 40 7d) 	mtmsr   r10
.*:	(7d 41 01 24|24 01 41 7d) 	mtmsr   r10,1
.*:	(7c 70 11 20|20 11 70 7c) 	mtocrf  1,r3
.*:	(7c 78 01 20|20 01 78 7c) 	mtocrf  128,r3
.*:	(7d 4a 3b a6|a6 3b 4a 7d) 	mtspr   234,r10
.*:	(7e 95 b0 93|93 b0 95 7e) 	mulhd\.  r20,r21,r22
.*:	(7e 95 b0 92|92 b0 95 7e) 	mulhd   r20,r21,r22
.*:	(7e 95 b0 13|13 b0 95 7e) 	mulhdu\. r20,r21,r22
.*:	(7e 95 b0 12|12 b0 95 7e) 	mulhdu  r20,r21,r22
.*:	(7e 95 b0 97|97 b0 95 7e) 	mulhw\.  r20,r21,r22
.*:	(7e 95 b0 96|96 b0 95 7e) 	mulhw   r20,r21,r22
.*:	(7e 95 b0 17|17 b0 95 7e) 	mulhwu\. r20,r21,r22
.*:	(7e 95 b0 16|16 b0 95 7e) 	mulhwu  r20,r21,r22
.*:	(7e 95 b1 d3|d3 b1 95 7e) 	mulld\.  r20,r21,r22
.*:	(7e 95 b1 d2|d2 b1 95 7e) 	mulld   r20,r21,r22
.*:	(7e 95 b5 d3|d3 b5 95 7e) 	mulldo\. r20,r21,r22
.*:	(7e 95 b5 d2|d2 b5 95 7e) 	mulldo  r20,r21,r22
.*:	(1e 95 00 64|64 00 95 1e) 	mulli   r20,r21,100
.*:	(1e 95 ff 9c|9c ff 95 1e) 	mulli   r20,r21,-100
.*:	(7e 95 b1 d7|d7 b1 95 7e) 	mullw\.  r20,r21,r22
.*:	(7e 95 b1 d6|d6 b1 95 7e) 	mullw   r20,r21,r22
.*:	(7e 95 b5 d7|d7 b5 95 7e) 	mullwo\. r20,r21,r22
.*:	(7e 95 b5 d6|d6 b5 95 7e) 	mullwo  r20,r21,r22
.*:	(7e b4 b3 b9|b9 b3 b4 7e) 	nand\.   r20,r21,r22
.*:	(7e b4 b3 b8|b8 b3 b4 7e) 	nand    r20,r21,r22
.*:	(7e 95 00 d1|d1 00 95 7e) 	neg\.    r20,r21
.*:	(7e 95 00 d0|d0 00 95 7e) 	neg     r20,r21
.*:	(7e 95 04 d1|d1 04 95 7e) 	nego\.   r20,r21
.*:	(7e 95 04 d0|d0 04 95 7e) 	nego    r20,r21
.*:	(7e b4 b0 f9|f9 b0 b4 7e) 	nor\.    r20,r21,r22
.*:	(7e b4 b0 f8|f8 b0 b4 7e) 	nor     r20,r21,r22
.*:	(7e b4 b3 79|79 b3 b4 7e) 	or\.     r20,r21,r22
.*:	(7e b4 b3 78|78 b3 b4 7e) 	or      r20,r21,r22
.*:	(7e b4 b3 39|39 b3 b4 7e) 	orc\.    r20,r21,r22
.*:	(7e b4 b3 38|38 b3 b4 7e) 	orc     r20,r21,r22
.*:	(62 b4 10 00|00 10 b4 62) 	ori     r20,r21,4096
.*:	(66 b4 10 00|00 10 b4 66) 	oris    r20,r21,4096
.*:	(7d 6a 00 f4|f4 00 6a 7d) 	popcntb r10,r11
.*:	(7d 6a 03 f4|f4 03 6a 7d) 	popcntd r10,r11
.*:	(7d 6a 02 f4|f4 02 6a 7d) 	popcntw r10,r11
.*:	(7d 6a 01 74|74 01 6a 7d) 	prtyd   r10,r11
.*:	(7d 6a 01 34|34 01 6a 7d) 	prtyw   r10,r11
.*:	(4c 00 00 66|66 00 00 4c) 	rfci
.*:	(4c 00 00 cc|cc 00 00 4c) 	rfgi
.*:	(4c 00 00 64|64 00 00 4c) 	rfi
.*:	(4c 00 00 4c|4c 00 00 4c) 	rfmci
.*:	(79 6a 67 f1|f1 67 6a 79) 	rldcl\.  r10,r11,r12,63
.*:	(79 6a 67 f0|f0 67 6a 79) 	rldcl   r10,r11,r12,63
.*:	(79 6a 67 f3|f3 67 6a 79) 	rldcr\.  r10,r11,r12,63
.*:	(79 6a 67 f2|f2 67 6a 79) 	rldcr   r10,r11,r12,63
.*:	(79 6a bf e9|e9 bf 6a 79) 	rldic\.  r10,r11,23,63
.*:	(79 6a bf e8|e8 bf 6a 79) 	rldic   r10,r11,23,63
.*:	(79 6a bf e1|e1 bf 6a 79) 	rldicl\. r10,r11,23,63
.*:	(79 6a bf e0|e0 bf 6a 79) 	rldicl  r10,r11,23,63
.*:	(79 6a bf e5|e5 bf 6a 79) 	rldicr\. r10,r11,23,63
.*:	(79 6a bf e4|e4 bf 6a 79) 	rldicr  r10,r11,23,63
.*:	(79 6a bf ed|ed bf 6a 79) 	rldimi\. r10,r11,23,63
.*:	(79 6a bf ec|ec bf 6a 79) 	rldimi  r10,r11,23,63
.*:	(51 6a b8 3f|3f b8 6a 51) 	rlwimi\. r10,r11,23,0,31
.*:	(51 6a b8 3e|3e b8 6a 51) 	rlwimi  r10,r11,23,0,31
.*:	(55 6a b8 3f|3f b8 6a 55) 	rotlwi\. r10,r11,23
.*:	(55 6a b8 3e|3e b8 6a 55) 	rotlwi  r10,r11,23
.*:	(5d 6a b8 3f|3f b8 6a 5d) 	rotlw\.  r10,r11,r23
.*:	(5d 6a b8 3e|3e b8 6a 5d) 	rotlw   r10,r11,r23
.*:	(44 00 00 02|02 00 00 44) 	sc
.*:	(44 00 0c 82|82 0c 00 44) 	sc      100
.*:	(7d 6a 60 37|37 60 6a 7d) 	sld\.    r10,r11,r12
.*:	(7d 6a 60 36|36 60 6a 7d) 	sld     r10,r11,r12
.*:	(7d 6a 60 31|31 60 6a 7d) 	slw\.    r10,r11,r12
.*:	(7d 6a 60 30|30 60 6a 7d) 	slw     r10,r11,r12
.*:	(7d 6a 66 35|35 66 6a 7d) 	srad\.   r10,r11,r12
.*:	(7d 6a 66 34|34 66 6a 7d) 	srad    r10,r11,r12
.*:	(7d 6a fe 77|77 fe 6a 7d) 	sradi\.  r10,r11,63
.*:	(7d 6a fe 76|76 fe 6a 7d) 	sradi   r10,r11,63
.*:	(7d 6a 66 31|31 66 6a 7d) 	sraw\.   r10,r11,r12
.*:	(7d 6a 66 30|30 66 6a 7d) 	sraw    r10,r11,r12
.*:	(7d 6a fe 71|71 fe 6a 7d) 	srawi\.  r10,r11,31
.*:	(7d 6a fe 70|70 fe 6a 7d) 	srawi   r10,r11,31
.*:	(7d 6a 64 37|37 64 6a 7d) 	srd\.    r10,r11,r12
.*:	(7d 6a 64 36|36 64 6a 7d) 	srd     r10,r11,r12
.*:	(7d 6a 64 31|31 64 6a 7d) 	srw\.    r10,r11,r12
.*:	(7d 6a 64 30|30 64 6a 7d) 	srw     r10,r11,r12
.*:	(99 4b ff ff|ff ff 4b 99) 	stb     r10,-1\(r11\)
.*:	(99 4b 00 01|01 00 4b 99) 	stb     r10,1\(r11\)
.*:	(7d 4b 61 be|be 61 4b 7d) 	stbepx  r10,r11,r12
.*:	(9d 4b ff ff|ff ff 4b 9d) 	stbu    r10,-1\(r11\)
.*:	(9d 4b 00 01|01 00 4b 9d) 	stbu    r10,1\(r11\)
.*:	(7d 4b 61 ee|ee 61 4b 7d) 	stbux   r10,r11,r12
.*:	(7d 4b 61 ae|ae 61 4b 7d) 	stbx    r10,r11,r12
.*:	(f9 4b ff f8|f8 ff 4b f9) 	std     r10,-8\(r11\)
.*:	(f9 4b 00 08|08 00 4b f9) 	std     r10,8\(r11\)
.*:	(7d 4b 65 28|28 65 4b 7d) 	stdbrx  r10,r11,r12
.*:	(7d 4b 61 ad|ad 61 4b 7d) 	stdcx\.  r10,r11,r12
.*:	(7d 4b 61 3a|3a 61 4b 7d) 	stdepx  r10,r11,r12
.*:	(f9 4b ff f9|f9 ff 4b f9) 	stdu    r10,-8\(r11\)
.*:	(f9 4b 00 09|09 00 4b f9) 	stdu    r10,8\(r11\)
.*:	(7d 4b 61 6a|6a 61 4b 7d) 	stdux   r10,r11,r12
.*:	(7d 4b 61 2a|2a 61 4b 7d) 	stdx    r10,r11,r12
.*:	(da 8a ff f8|f8 ff 8a da) 	stfd    f20,-8\(r10\)
.*:	(da 8a 00 08|08 00 8a da) 	stfd    f20,8\(r10\)
.*:	(7e 8a 5d be|be 5d 8a 7e) 	stfdepx f20,r10,r11
.*:	(de 8a ff f8|f8 ff 8a de) 	stfdu   f20,-8\(r10\)
.*:	(de 8a 00 08|08 00 8a de) 	stfdu   f20,8\(r10\)
.*:	(7e 8a 5d ee|ee 5d 8a 7e) 	stfdux  f20,r10,r11
.*:	(7e 8a 5d ae|ae 5d 8a 7e) 	stfdx   f20,r10,r11
.*:	(7e 8a 5f ae|ae 5f 8a 7e) 	stfiwx  f20,r10,r11
.*:	(d2 8a ff fc|fc ff 8a d2) 	stfs    f20,-4\(r10\)
.*:	(d2 8a 00 04|04 00 8a d2) 	stfs    f20,4\(r10\)
.*:	(d6 8a ff fc|fc ff 8a d6) 	stfsu   f20,-4\(r10\)
.*:	(d6 8a 00 04|04 00 8a d6) 	stfsu   f20,4\(r10\)
.*:	(7e 8a 5d 6e|6e 5d 8a 7e) 	stfsux  f20,r10,r11
.*:	(7e 8a 5d 2e|2e 5d 8a 7e) 	stfsx   f20,r10,r11
.*:	(b1 4b ff fe|fe ff 4b b1) 	sth     r10,-2\(r11\)
.*:	(b1 4b 00 02|02 00 4b b1) 	sth     r10,2\(r11\)
.*:	(b1 4b ff fc|fc ff 4b b1) 	sth     r10,-4\(r11\)
.*:	(b1 4b 00 04|04 00 4b b1) 	sth     r10,4\(r11\)
.*:	(7d 4b 67 2c|2c 67 4b 7d) 	sthbrx  r10,r11,r12
.*:	(7d 4b 63 3e|3e 63 4b 7d) 	sthepx  r10,r11,r12
.*:	(b5 4b ff fe|fe ff 4b b5) 	sthu    r10,-2\(r11\)
.*:	(b5 4b 00 02|02 00 4b b5) 	sthu    r10,2\(r11\)
.*:	(7d 4b 63 6e|6e 63 4b 7d) 	sthux   r10,r11,r12
.*:	(7d 4b 63 2e|2e 63 4b 7d) 	sthx    r10,r11,r12
.*:	(7d 4b 65 2c|2c 65 4b 7d) 	stwbrx  r10,r11,r12
.*:	(7d 4b 61 2d|2d 61 4b 7d) 	stwcx\.  r10,r11,r12
.*:	(7d 4b 61 3e|3e 61 4b 7d) 	stwepx  r10,r11,r12
.*:	(95 4b ff fc|fc ff 4b 95) 	stwu    r10,-4\(r11\)
.*:	(95 4b 00 04|04 00 4b 95) 	stwu    r10,4\(r11\)
.*:	(7d 4b 61 6e|6e 61 4b 7d) 	stwux   r10,r11,r12
.*:	(7d 4b 61 2e|2e 61 4b 7d) 	stwx    r10,r11,r12
.*:	(7e 95 b0 51|51 b0 95 7e) 	subf\.   r20,r21,r22
.*:	(7e 95 b0 50|50 b0 95 7e) 	subf    r20,r21,r22
.*:	(7e 95 b0 11|11 b0 95 7e) 	subfc\.  r20,r21,r22
.*:	(7e 95 b0 10|10 b0 95 7e) 	subfc   r20,r21,r22
.*:	(7e 95 b4 11|11 b4 95 7e) 	subfco\. r20,r21,r22
.*:	(7e 95 b4 10|10 b4 95 7e) 	subfco  r20,r21,r22
.*:	(7e 95 b1 11|11 b1 95 7e) 	subfe\.  r20,r21,r22
.*:	(7e 95 b1 10|10 b1 95 7e) 	subfe   r20,r21,r22
.*:	(7e 95 b5 11|11 b5 95 7e) 	subfeo\. r20,r21,r22
.*:	(7e 95 b5 10|10 b5 95 7e) 	subfeo  r20,r21,r22
.*:	(22 95 00 64|64 00 95 22) 	subfic  r20,r21,100
.*:	(22 95 ff 9c|9c ff 95 22) 	subfic  r20,r21,-100
.*:	(7e 95 01 d1|d1 01 95 7e) 	subfme\. r20,r21
.*:	(7e 95 01 d0|d0 01 95 7e) 	subfme  r20,r21
.*:	(7e 95 05 d1|d1 05 95 7e) 	subfmeo\. r20,r21
.*:	(7e 95 05 d0|d0 05 95 7e) 	subfmeo r20,r21
.*:	(7e 95 b4 51|51 b4 95 7e) 	subfo\.  r20,r21,r22
.*:	(7e 95 b4 50|50 b4 95 7e) 	subfo   r20,r21,r22
.*:	(7e 95 01 91|91 01 95 7e) 	subfze\. r20,r21
.*:	(7e 95 01 90|90 01 95 7e) 	subfze  r20,r21
.*:	(7e 95 05 91|91 05 95 7e) 	subfzeo\. r20,r21
.*:	(7e 95 05 90|90 05 95 7e) 	subfzeo r20,r21
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 00 04 ac|ac 04 00 7c) 	hwsync
.*:	(7c 20 04 ac|ac 04 20 7c) 	lwsync
.*:	(7c aa 58 88|88 58 aa 7c) 	tdlge   r10,r11
.*:	(08 aa 00 64|64 00 aa 08) 	tdlgei  r10,100
.*:	(08 aa ff 9c|9c ff aa 08) 	tdlgei  r10,-100
.*:	(7c 6a 58 24|24 58 6a 7c) 	tlbilxva r10,r11
.*:	(7c 0a 5e 24|24 5e 0a 7c) 	tlbivax r10,r11
.*:	(7c 00 07 64|64 07 00 7c) 	tlbre
.*:	(7d 4b 3f 64|64 3f 4b 7d) 	tlbre   r10,r11,7
.*:	(7c 0a 5e a5|a5 5e 0a 7c) 	tlbsrx\. r10,r11
.*:	(7d 4b 67 25|25 67 4b 7d) 	tlbsx\.  r10,r11,r12
.*:	(7d 4b 67 24|24 67 4b 7d) 	tlbsx   r10,r11,r12
.*:	(7c 00 04 6c|6c 04 00 7c) 	tlbsync
.*:	(7c 00 07 a4|a4 07 00 7c) 	tlbwe
.*:	(7d 4b 3f a4|a4 3f 4b 7d) 	tlbwe   r10,r11,7
.*:	(7c aa 58 08|08 58 aa 7c) 	twlge   r10,r11
.*:	(0c aa 00 64|64 00 aa 0c) 	twlgei  r10,100
.*:	(0c aa ff 9c|9c ff aa 0c) 	twlgei  r10,-100
.*:	(7c 00 00 7c|7c 00 00 7c) 	wait
.*:	(7c 00 00 7c|7c 00 00 7c) 	wait
.*:	(7c 20 00 7c|7c 00 20 7c) 	waitrsv
.*:	(7c 40 00 7c|7c 00 40 7c) 	waitimpl
.*:	(7c 40 00 7c|7c 00 40 7c) 	waitimpl
.*:	(7c 20 00 7c|7c 00 20 7c) 	waitrsv
.*:	(7c 00 01 6c|6c 01 00 7c) 	wchkall
.*:	(7c 00 01 6c|6c 01 00 7c) 	wchkall
.*:	(7d 80 01 6c|6c 01 80 7d) 	wchkall cr3
.*:	(7c 2a 5f 4c|4c 5f 2a 7c) 	wclr    1,r10,r11
.*:	(7c 20 07 4c|4c 07 20 7c) 	wclrall 1
.*:	(7c 4a 5f 4c|4c 5f 4a 7c) 	wclrone r10,r11
.*:	(7d 40 01 06|06 01 40 7d) 	wrtee   r10
.*:	(7c 00 81 46|46 81 00 7c) 	wrteei  1
.*:	(7d 6a 62 79|79 62 6a 7d) 	xor\.    r10,r11,r12
.*:	(7d 6a 62 78|78 62 6a 7d) 	xor     r10,r11,r12
.*:	(69 6a 10 00|00 10 6a 69) 	xori    r10,r11,4096
.*:	(6d 6a 10 00|00 10 6a 6d) 	xoris   r10,r11,4096
#pass
