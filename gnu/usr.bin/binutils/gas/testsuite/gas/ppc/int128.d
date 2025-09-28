#as: -mpower10
#objdump: -dr -Mpower10
#name: 128-bit binary integer ops

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(10 22 1a c8|c8 1a 22 10) 	vmuleud v1,v2,v3
.*:	(10 85 30 c8|c8 30 85 10) 	vmuloud v4,v5,v6
.*:	(10 e8 4b c8|c8 4b e8 10) 	vmulesd v7,v8,v9
.*:	(11 4b 61 c8|c8 61 4b 11) 	vmulosd v10,v11,v12
.*:	(11 ae 7c 17|17 7c ae 11) 	vmsumcud v13,v14,v15,v16
.*:	(12 32 99 0b|0b 99 32 12) 	vdivsq  v17,v18,v19
.*:	(12 95 a8 0b|0b a8 95 12) 	vdivuq  v20,v21,v21
.*:	(12 d7 c3 0b|0b c3 d7 12) 	vdivesq v22,v23,v24
.*:	(13 3a da 0b|0b da 3a 13) 	vdiveuq v25,v26,v27
.*:	(13 9d f7 0b|0b f7 9d 13) 	vmodsq  v28,v29,v30
.*:	(13 e0 0e 0b|0b 0e e0 13) 	vmoduq  v31,v0,v1
.*:	(10 5b 1e 02|02 1e 5b 10) 	vextsd2q v2,v3
.*:	(10 04 29 01|01 29 04 10) 	vcmpuq  cr0,v4,v5
.*:	(10 86 39 41|41 39 86 10) 	vcmpsq  cr1,v6,v7
.*:	(11 09 51 c7|c7 51 09 11) 	vcmpequq v8,v9,v10
.*:	(11 6c 6d c7|c7 6d 6c 11) 	vcmpequq. v11,v12,v13
.*:	(11 cf 83 87|87 83 cf 11) 	vcmpgtsq v14,v15,v16
.*:	(12 32 9f 87|87 9f 32 12) 	vcmpgtsq. v17,v18,v19
.*:	(12 95 b2 87|87 b2 95 12) 	vcmpgtuq v20,v21,v22
.*:	(12 f8 ce 87|87 ce f8 12) 	vcmpgtuq. v23,v24,v25
.*:	(13 5b e0 05|05 e0 5b 13) 	vrlq    v26,v27,v28
.*:	(13 be f9 45|45 f9 be 13) 	vrlqnm  v29,v30,v31
.*:	(10 01 10 45|45 10 01 10) 	vrlqmi  v0,v1,v2
.*:	(10 64 29 05|05 29 64 10) 	vslq    v3,v4,v5
.*:	(10 c7 42 05|05 42 c7 10) 	vsrq    v6,v7,v8
.*:	(11 2a 5b 05|05 5b 2a 11) 	vsraq   v9,v10,v11
.*:	(fd 80 6e 88|88 6e 80 fd) 	xscvqpuqz v12,v13
.*:	(fd c8 7e 88|88 7e c8 fd) 	xscvqpsqz v14,v15
.*:	(fe 03 8e 88|88 8e 03 fe) 	xscvuqqp v16,v17
.*:	(fe 4b 9e 88|88 9e 4b fe) 	xscvsqqp v18,v19
.*:	(fe 80 af c4|c4 af 80 fe) 	dcffixqq f20,v21
.*:	(fe e1 b7 c4|c4 b7 e1 fe) 	dctfixqq v23,f22
