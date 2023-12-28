#as: -mpower10
#objdump: -dr -Mpower10
#name: bit manipulation

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(7f df e9 b8|b8 e9 df 7f) 	cfuged  r31,r30,r29
.*:	(7f 7c d0 76|76 d0 7c 7f) 	cntlzdm r28,r27,r26
.*:	(7f 19 bc 76|76 bc 19 7f) 	cnttzdm r25,r24,r23
.*:	(7e b6 a1 38|38 a1 b6 7e) 	pdepd   r22,r21,r20
.*:	(7e 53 89 78|78 89 53 7e) 	pextd   r19,r18,r17
.*:	(12 0f 77 84|84 77 0f 12) 	vclzdm  v16,v15,v14
.*:	(11 ac 5f c4|c4 5f ac 11) 	vctzdm  v13,v12,v11
.*:	(11 49 45 cd|cd 45 49 11) 	vpdepd  v10,v9,v8
.*:	(10 e6 2d 8d|8d 2d e6 10) 	vpextd  v7,v6,v5
.*:	(10 83 15 4d|4d 15 83 10) 	vcfuged v4,v3,v2
.*:	(10 27 04 cc|cc 04 27 10) 	vgnb    r1,v0,7
.*:	(05 00 00 3f|3f 00 00 05) 	xxeval  vs63,vs31,vs62,vs30,63
.*:	(8b ff f7 93|93 f7 ff 8b) 
