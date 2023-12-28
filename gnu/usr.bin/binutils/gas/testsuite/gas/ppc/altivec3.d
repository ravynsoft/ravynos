#as: -mpower9
#objdump: -dr -Mpower9
#name: Altivec ISA 3.0 instructions

.*


Disassembly of section \.text:

0+00 <start>:

.*:	(11 7e 00 01|01 00 7e 11) 	vmul10cuq v11,v30
.*:	(13 c1 b8 07|07 b8 c1 13) 	vcmpneb v30,v1,v23
.*:	(13 d3 f7 7b|7b f7 d3 13) 	vpermr  v30,v19,v30,v29
.*:	(12 94 88 41|41 88 94 12) 	vmul10ecuq v20,v20,v17
.*:	(13 73 f8 47|47 f8 73 13) 	vcmpneh v27,v19,v31
.*:	(10 c9 b8 85|85 b8 c9 10) 	vrlwmi  v6,v9,v23
.*:	(12 da 08 87|87 08 da 12) 	vcmpnew v22,v26,v1
.*:	(13 1e c8 c5|c5 c8 1e 13) 	vrldmi  v24,v30,v25
.*:	(12 7d b1 07|07 b1 7d 12) 	vcmpnezb v19,v29,v22
.*:	(11 17 99 47|47 99 17 11) 	vcmpnezh v8,v23,v19
.*:	(13 78 59 85|85 59 78 13) 	vrlwnm  v27,v24,v11
.*:	(12 ad 51 87|87 51 ad 12) 	vcmpnezw v21,v13,v10
.*:	(10 b4 e9 c5|c5 e9 b4 10) 	vrldnm  v5,v20,v29
.*:	(13 d3 02 01|01 02 d3 13) 	vmul10uq v30,v19
.*:	(13 0c aa 0d|0d aa 0c 13) 	vextractub v24,v21,12
.*:	(10 13 e2 41|41 e2 13 10) 	vmul10euq v0,v19,v28
.*:	(11 4c 1a 4d|4d 1a 4c 11) 	vextractuh v10,v3,12
.*:	(13 87 62 8d|8d 62 87 13) 	vextractuw v28,v12,7
.*:	(13 c1 da cd|cd da c1 13) 	vextractd v30,v27,1
.*:	(13 24 fb 0d|0d fb 24 13) 	vinsertb v25,v31,4
.*:	(12 ae f3 41|41 f3 ae 12) 	bcdcpsgn\. v21,v14,v30
.*:	(12 c5 93 4d|4d 93 c5 12) 	vinserth v22,v18,5
.*:	(13 a1 b3 8d|8d b3 a1 13) 	vinsertw v29,v22,1
.*:	(13 a7 6b cd|cd 6b a7 13) 	vinsertd v29,v13,7
.*:	(12 d9 44 07|07 44 d9 12) 	vcmpneb\. v22,v25,v8
.*:	(12 0f ac 47|47 ac 0f 12) 	vcmpneh\. v16,v15,v21
.*:	(12 d5 fc 81|81 fc d5 12) 	bcdus\.  v22,v21,v31
.*:	(10 2c 64 87|87 64 2c 10) 	vcmpnew\. v1,v12,v12
.*:	(10 a3 46 c1|c1 46 a3 10) 	bcds\.   v5,v3,v8,1
.*:	(13 76 0d 01|01 0d 76 13) 	bcdtrunc\. v27,v22,v1,0
.*:	(10 5a 05 07|07 05 5a 10) 	vcmpnezb\. v2,v26,v0
.*:	(13 4e 3d 41|41 3d 4e 13) 	bcdutrunc\. v26,v14,v7
.*:	(12 05 65 47|47 65 05 12) 	vcmpnezh\. v16,v5,v12
.*:	(13 00 2d 81|81 2d 00 13) 	bcdctsq\. v24,v5
.*:	(10 e2 05 81|81 05 e2 10) 	bcdcfsq\. v7,v0,0
.*:	(13 c4 67 81|81 67 c4 13) 	bcdctz\. v30,v12,1
.*:	(12 25 bd 81|81 bd 25 12) 	bcdctn\. v17,v23
.*:	(10 86 7f 81|81 7f 86 10) 	bcdcfz\. v4,v15,1
.*:	(13 a7 2f 81|81 2f a7 13) 	bcdcfn\. v29,v5,1
.*:	(13 7f 65 81|81 65 7f 13) 	bcdsetsgn\. v27,v12,0
.*:	(11 dc cd 87|87 cd dc 11) 	vcmpnezw\. v14,v28,v25
.*:	(10 42 37 c1|c1 37 42 10) 	bcdsr\.  v2,v2,v6,1
.*:	(13 20 2d cc|cc 2d 20 13) 	vbpermd v25,v0,v5
.*:	(13 80 ce 02|02 ce 80 13) 	vclzlsbb r28,v25
.*:	(10 41 c6 02|02 c6 41 10) 	vctzlsbb r2,v24
.*:	(12 a6 5e 02|02 5e a6 12) 	vnegw   v21,v11
.*:	(12 27 de 02|02 de 27 12) 	vnegd   v17,v27
.*:	(13 e8 be 02|02 be e8 13) 	vprtybw v31,v23
.*:	(12 a9 be 02|02 be a9 12) 	vprtybd v21,v23
.*:	(12 aa 96 02|02 96 aa 12) 	vprtybq v21,v18
.*:	(13 d0 26 02|02 26 d0 13) 	vextsb2w v30,v4
.*:	(10 71 d6 02|02 d6 71 10) 	vextsh2w v3,v26
.*:	(11 78 8e 02|02 8e 78 11) 	vextsb2d v11,v17
.*:	(10 b9 56 02|02 56 b9 10) 	vextsh2d v5,v10
.*:	(11 ba ce 02|02 ce ba 11) 	vextsw2d v13,v25
.*:	(13 3c 16 02|02 16 3c 13) 	vctzb   v25,v2
.*:	(10 1d 1e 02|02 1e 1d 10) 	vctzh   v0,v3
.*:	(12 de 36 02|02 36 de 12) 	vctzw   v22,v6
.*:	(13 5f c6 02|02 c6 5f 13) 	vctzd   v26,v24
.*:	(10 df 16 0d|0d 16 df 10) 	vextublx r6,r31,v2
.*:	(11 a0 96 4d|4d 96 a0 11) 	vextuhlx r13,r0,v18
.*:	(11 de fe 8d|8d fe de 11) 	vextuwlx r14,r30,v31
.*:	(11 ec 77 04|04 77 ec 11) 	vsrv    v15,v12,v14
.*:	(12 8a f7 0d|0d f7 8a 12) 	vextubrx r20,r10,v30
.*:	(12 b5 17 44|44 17 b5 12) 	vslv    v21,v21,v2
.*:	(11 e9 0f 4d|4d 0f e9 11) 	vextuhrx r15,r9,v1
.*:	(12 b1 87 8d|8d 87 b1 12) 	vextuwrx r21,r17,v16
.*:	(12 95 b5 e3|e3 b5 95 12) 	vmsumudm v20,v21,v22,v23
#pass
