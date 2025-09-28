#name: s390x opcode
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	b9 f5 b0 69 [	 ]*ncrk	%r6,%r9,%r11
.*:	b9 e5 b0 69 [	 ]*ncgrk	%r6,%r9,%r11
.*:	e5 0a 6f a0 9f a0 [	 ]*mvcrl	4000\(%r6\),4000\(%r9\)
.*:	b9 74 b0 69 [	 ]*nnrk	%r6,%r9,%r11
.*:	b9 64 b0 69 [	 ]*nngrk	%r6,%r9,%r11
.*:	b9 76 b0 69 [	 ]*nork	%r6,%r9,%r11
.*:	b9 66 b0 69 [	 ]*nogrk	%r6,%r9,%r11
.*:	b9 77 b0 69 [	 ]*nxrk	%r6,%r9,%r11
.*:	b9 67 b0 69 [	 ]*nxgrk	%r6,%r9,%r11
.*:	b9 75 b0 69 [	 ]*ocrk	%r6,%r9,%r11
.*:	b9 65 b0 69 [	 ]*ocgrk	%r6,%r9,%r11
.*:	b9 e1 00 69 [	 ]*popcnt	%r6,%r9
.*:	b9 e1 d0 69 [	 ]*popcnt	%r6,%r9,13
.*:	b9 f0 bd 69 [	 ]*selrnh	%r6,%r9,%r11
.*:	b9 f0 b1 69 [	 ]*selro	%r6,%r9,%r11
.*:	b9 f0 b2 69 [	 ]*selrh	%r6,%r9,%r11
.*:	b9 f0 b2 69 [	 ]*selrh	%r6,%r9,%r11
.*:	b9 f0 b3 69 [	 ]*selrnle	%r6,%r9,%r11
.*:	b9 f0 b4 69 [	 ]*selrl	%r6,%r9,%r11
.*:	b9 f0 b4 69 [	 ]*selrl	%r6,%r9,%r11
.*:	b9 f0 b5 69 [	 ]*selrnhe	%r6,%r9,%r11
.*:	b9 f0 b6 69 [	 ]*selrlh	%r6,%r9,%r11
.*:	b9 f0 b7 69 [	 ]*selrne	%r6,%r9,%r11
.*:	b9 f0 b7 69 [	 ]*selrne	%r6,%r9,%r11
.*:	b9 f0 b8 69 [	 ]*selre	%r6,%r9,%r11
.*:	b9 f0 b8 69 [	 ]*selre	%r6,%r9,%r11
.*:	b9 f0 b9 69 [	 ]*selrnlh	%r6,%r9,%r11
.*:	b9 f0 ba 69 [	 ]*selrhe	%r6,%r9,%r11
.*:	b9 f0 bb 69 [	 ]*selrnl	%r6,%r9,%r11
.*:	b9 f0 bb 69 [	 ]*selrnl	%r6,%r9,%r11
.*:	b9 f0 bc 69 [	 ]*selrle	%r6,%r9,%r11
.*:	b9 f0 bd 69 [	 ]*selrnh	%r6,%r9,%r11
.*:	b9 f0 bd 69 [	 ]*selrnh	%r6,%r9,%r11
.*:	b9 f0 be 69 [	 ]*selrno	%r6,%r9,%r11
.*:	b9 e3 bd 69 [	 ]*selgrnh	%r6,%r9,%r11
.*:	b9 e3 b1 69 [	 ]*selgro	%r6,%r9,%r11
.*:	b9 e3 b2 69 [	 ]*selgrh	%r6,%r9,%r11
.*:	b9 e3 b2 69 [	 ]*selgrh	%r6,%r9,%r11
.*:	b9 e3 b3 69 [	 ]*selgrnle	%r6,%r9,%r11
.*:	b9 e3 b4 69 [	 ]*selgrl	%r6,%r9,%r11
.*:	b9 e3 b4 69 [	 ]*selgrl	%r6,%r9,%r11
.*:	b9 e3 b5 69 [	 ]*selgrnhe	%r6,%r9,%r11
.*:	b9 e3 b6 69 [	 ]*selgrlh	%r6,%r9,%r11
.*:	b9 e3 b7 69 [	 ]*selgrne	%r6,%r9,%r11
.*:	b9 e3 b7 69 [	 ]*selgrne	%r6,%r9,%r11
.*:	b9 e3 b8 69 [	 ]*selgre	%r6,%r9,%r11
.*:	b9 e3 b8 69 [	 ]*selgre	%r6,%r9,%r11
.*:	b9 e3 b9 69 [	 ]*selgrnlh	%r6,%r9,%r11
.*:	b9 e3 ba 69 [	 ]*selgrhe	%r6,%r9,%r11
.*:	b9 e3 bb 69 [	 ]*selgrnl	%r6,%r9,%r11
.*:	b9 e3 bb 69 [	 ]*selgrnl	%r6,%r9,%r11
.*:	b9 e3 bc 69 [	 ]*selgrle	%r6,%r9,%r11
.*:	b9 e3 bd 69 [	 ]*selgrnh	%r6,%r9,%r11
.*:	b9 e3 bd 69 [	 ]*selgrnh	%r6,%r9,%r11
.*:	b9 e3 be 69 [	 ]*selgrno	%r6,%r9,%r11
.*:	b9 c0 bd 69 [	 ]*selfhrnh	%r6,%r9,%r11
.*:	b9 c0 b1 69 [	 ]*selfhro	%r6,%r9,%r11
.*:	b9 c0 b2 69 [	 ]*selfhrh	%r6,%r9,%r11
.*:	b9 c0 b2 69 [	 ]*selfhrh	%r6,%r9,%r11
.*:	b9 c0 b3 69 [	 ]*selfhrnle	%r6,%r9,%r11
.*:	b9 c0 b4 69 [	 ]*selfhrl	%r6,%r9,%r11
.*:	b9 c0 b4 69 [	 ]*selfhrl	%r6,%r9,%r11
.*:	b9 c0 b5 69 [	 ]*selfhrnhe	%r6,%r9,%r11
.*:	b9 c0 b6 69 [	 ]*selfhrlh	%r6,%r9,%r11
.*:	b9 c0 b7 69 [	 ]*selfhrne	%r6,%r9,%r11
.*:	b9 c0 b7 69 [	 ]*selfhrne	%r6,%r9,%r11
.*:	b9 c0 b8 69 [	 ]*selfhre	%r6,%r9,%r11
.*:	b9 c0 b8 69 [	 ]*selfhre	%r6,%r9,%r11
.*:	b9 c0 b9 69 [	 ]*selfhrnlh	%r6,%r9,%r11
.*:	b9 c0 ba 69 [	 ]*selfhrhe	%r6,%r9,%r11
.*:	b9 c0 bb 69 [	 ]*selfhrnl	%r6,%r9,%r11
.*:	b9 c0 bb 69 [	 ]*selfhrnl	%r6,%r9,%r11
.*:	b9 c0 bc 69 [	 ]*selfhrle	%r6,%r9,%r11
.*:	b9 c0 bd 69 [	 ]*selfhrnh	%r6,%r9,%r11
.*:	b9 c0 bd 69 [	 ]*selfhrnh	%r6,%r9,%r11
.*:	b9 c0 be 69 [	 ]*selfhrno	%r6,%r9,%r11
.*:	e6 f6 9f a0 d0 06 [	 ]*vlbr	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 06 [	 ]*vlbrh	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 06 [	 ]*vlbrf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 06 [	 ]*vlbrg	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 40 06 [	 ]*vlbrq	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 07 [	 ]*vler	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 07 [	 ]*vlerh	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 07 [	 ]*vlerf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 07 [	 ]*vlerg	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 04 [	 ]*vllebrz	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 04 [	 ]*vllebrzh	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 04 [	 ]*vllebrzf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 04 [	 ]*ldrv	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 04 [	 ]*ldrv	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 60 04 [	 ]*lerv	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 60 04 [	 ]*lerv	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 01 [	 ]*vlebrh	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 d0 03 [	 ]*vlebrf	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 d0 02 [	 ]*vlebrg	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 d0 05 [	 ]*vlbrrep	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 05 [	 ]*vlbrreph	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 05 [	 ]*vlbrrepf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 05 [	 ]*vlbrrepg	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 0e [	 ]*vstbr	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 0e [	 ]*vstbrh	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 0e [	 ]*vstbrf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 0e [	 ]*vstbrg	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 40 0e [	 ]*vstbrq	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 0f [	 ]*vster	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 10 0f [	 ]*vsterh	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 20 0f [	 ]*vsterf	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 30 0f [	 ]*vsterg	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 09 [	 ]*vstebrh	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 d0 0b [	 ]*vstebrf	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 00 0b [	 ]*sterv	%v15,4000\(%r6,%r9\)
.*:	e6 f6 9f a0 d0 0a [	 ]*vstebrg	%v15,4000\(%r6,%r9\),13
.*:	e6 f6 9f a0 00 0a [	 ]*stdrv	%v15,4000\(%r6,%r9\)
.*:	e7 f1 40 fd 06 86 [	 ]*vsld	%v15,%v17,%v20,253
.*:	e7 f1 40 fd 06 87 [	 ]*vsrd	%v15,%v17,%v20,253
.*:	e7 f1 4d 00 87 8b [	 ]*vstrs	%v15,%v17,%v20,%v24,13
.*:	e7 f1 4d c0 87 8b [	 ]*vstrs	%v15,%v17,%v20,%v24,13,12
.*:	e7 f1 40 00 87 8b [	 ]*vstrsb	%v15,%v17,%v20,%v24
.*:	e7 f1 40 d0 87 8b [	 ]*vstrsb	%v15,%v17,%v20,%v24,13
.*:	e7 f1 41 00 87 8b [	 ]*vstrsh	%v15,%v17,%v20,%v24
.*:	e7 f1 41 d0 87 8b [	 ]*vstrsh	%v15,%v17,%v20,%v24,13
.*:	e7 f1 42 00 87 8b [	 ]*vstrsf	%v15,%v17,%v20,%v24
.*:	e7 f1 42 d0 87 8b [	 ]*vstrsf	%v15,%v17,%v20,%v24,13
.*:	e7 f1 40 20 87 8b [	 ]*vstrszb	%v15,%v17,%v20,%v24
.*:	e7 f1 41 20 87 8b [	 ]*vstrszh	%v15,%v17,%v20,%v24
.*:	e7 f1 42 20 87 8b [	 ]*vstrszf	%v15,%v17,%v20,%v24
.*:	e7 f1 00 bc d4 c3 [	 ]*vcfps	%v15,%v17,13,12,11
.*:	e7 f1 00 cd 24 c3 [	 ]*wcefb	%v15,%v17,5,12
.*:	e7 f1 00 cd 24 c3 [	 ]*wcefb	%v15,%v17,5,12
.*:	e7 f1 00 bc d4 c1 [	 ]*vcfpl	%v15,%v17,13,12,11
.*:	e7 f1 00 cd 24 c1 [	 ]*wcelfb	%v15,%v17,5,12
.*:	e7 f1 00 cd 24 c1 [	 ]*wcelfb	%v15,%v17,5,12
.*:	e7 f1 00 bc d4 c2 [	 ]*vcsfp	%v15,%v17,13,12,11
.*:	e7 f1 00 cd 24 c2 [	 ]*wcfeb	%v15,%v17,5,12
.*:	e7 f1 00 cd 24 c2 [	 ]*wcfeb	%v15,%v17,5,12
.*:	e7 f1 00 bc d4 c0 [	 ]*vclfp	%v15,%v17,13,12,11
.*:	e7 f1 00 cd 24 c0 [	 ]*wclfeb	%v15,%v17,5,12
.*:	e7 f1 00 cd 24 c0 [	 ]*wclfeb	%v15,%v17,5,12
.*:	b9 39 b0 69 [	 ]*dfltcc	%r6,%r9,%r11
.*:	b9 38 00 69 [	 ]*sortl	%r6,%r9
.*:	e6 6f 00 d0 00 50 [	 ]*vcvb	%r6,%v15,13
.*:	e6 6f 00 dc 00 50 [	 ]*vcvb	%r6,%v15,13,12
.*:	e6 6f 00 d0 00 52 [	 ]*vcvbg	%r6,%v15,13
.*:	e6 6f 00 dc 00 52 [	 ]*vcvbg	%r6,%v15,13,12
.*:	b9 3a 00 69 [	 ]*kdsa	%r6,%r9
.*:	07 07 [	 ]*nopr	%r7
