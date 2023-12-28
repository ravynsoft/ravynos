#as: -mpower10
#objdump: -dr -Mpower10
#name: vector integer mul/div/mod

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(10 22 1b 89|89 1b 22 10) 	vmulhsw v1,v2,v3
.*:	(10 85 32 89|89 32 85 10) 	vmulhuw v4,v5,v6
.*:	(10 e8 4b c9|c9 4b e8 10) 	vmulhsd v7,v8,v9
.*:	(11 4b 62 c9|c9 62 4b 11) 	vmulhud v10,v11,v12
.*:	(11 ae 79 c9|c9 79 ae 11) 	vmulld  v13,v14,v15
.*:	(12 11 91 8b|8b 91 11 12) 	vdivsw  v16,v17,v18
.*:	(12 74 a8 8b|8b a8 74 12) 	vdivuw  v19,v20,v21
.*:	(12 d7 c3 8b|8b c3 d7 12) 	vdivesw v22,v23,v24
.*:	(13 3a da 8b|8b da 3a 13) 	vdiveuw v25,v26,v27
.*:	(13 9d f1 cb|cb f1 9d 13) 	vdivsd  v28,v29,v30
.*:	(13 e0 08 cb|cb 08 e0 13) 	vdivud  v31,v0,v1
.*:	(10 43 23 cb|cb 23 43 10) 	vdivesd v2,v3,v4
.*:	(10 a6 3a cb|cb 3a a6 10) 	vdiveud v5,v6,v7
.*:	(11 09 57 8b|8b 57 09 11) 	vmodsw  v8,v9,v10
.*:	(11 6c 6e 8b|8b 6e 6c 11) 	vmoduw  v11,v12,v13
.*:	(11 cf 87 cb|cb 87 cf 11) 	vmodsd  v14,v15,v16
.*:	(12 32 9e cb|cb 9e 32 12) 	vmodud  v17,v18,v19
