#as: -mpower10
#objdump: -dr -Mpower10
#name: SIMD permute

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(10 01 10 d8|d8 10 01 10) 	vextdubvlx v0,v1,v2,r3
.*:	(10 85 31 d9|d9 31 85 10) 	vextdubvrx v4,v5,v6,r7
.*:	(11 09 52 da|da 52 09 11) 	vextduhvlx v8,v9,v10,r11
.*:	(11 8d 73 db|db 73 8d 11) 	vextduhvrx v12,v13,v14,r15
.*:	(12 11 94 dc|dc 94 11 12) 	vextduwvlx v16,v17,v18,r19
.*:	(12 95 b5 dd|dd b5 95 12) 	vextduwvrx v20,v21,v22,r23
.*:	(13 19 d6 de|de d6 19 13) 	vextddvlx v24,v25,v26,r27
.*:	(13 9d f7 df|df f7 9d 13) 	vextddvrx v28,v29,v30,r31
.*:	(10 01 12 0f|0f 12 01 10) 	vinsblx v0,r1,r2
.*:	(10 64 2b 0f|0f 2b 64 10) 	vinsbrx v3,r4,r5
.*:	(10 c7 42 4f|4f 42 c7 10) 	vinshlx v6,r7,r8
.*:	(11 2a 5b 4f|4f 5b 2a 11) 	vinshrx v9,r10,r11
.*:	(11 8d 72 8f|8f 72 8d 11) 	vinswlx v12,r13,r14
.*:	(11 f0 8b 8f|8f 8b f0 11) 	vinswrx v15,r16,r17
.*:	(12 53 a2 cf|cf a2 53 12) 	vinsdlx v18,r19,r20
.*:	(12 b6 bb cf|cf bb b6 12) 	vinsdrx v21,r22,r23
.*:	(13 19 d0 0f|0f d0 19 13) 	vinsbvlx v24,r25,v26
.*:	(13 7c e9 0f|0f e9 7c 13) 	vinsbvrx v27,r28,v29
.*:	(13 df 00 4f|4f 00 df 13) 	vinshvlx v30,r31,v0
.*:	(10 22 19 4f|4f 19 22 10) 	vinshvrx v1,r2,v3
.*:	(10 85 30 8f|8f 30 85 10) 	vinswvlx v4,r5,v6
.*:	(10 e8 49 8f|8f 49 e8 10) 	vinswvrx v7,r8,v9
.*:	(11 4c 58 cf|cf 58 4c 11) 	vinsw   v10,r11,12
.*:	(11 a3 71 cf|cf 71 a3 11) 	vinsd   v13,r14,3
.*:	(11 f0 89 56|56 89 f0 11) 	vsldbi  v15,v16,v17,5
.*:	(12 53 a3 d6|d6 a3 53 12) 	vsrdbi  v18,v19,v20,7
.*:	(05 00 01 23|23 01 00 05) 	xxspltiw vs63,19088743
.*:	(83 e7 45 67|67 45 e7 83) 
.*:	(05 00 89 ab|ab 89 00 05) 	xxsplti32dx vs62,1,2309737967
.*:	(83 c3 cd ef|ef cd c3 83) 
.*:	(05 00 01 23|23 01 00 05) 	xxspltidp vs61,19088743
.*:	(83 a5 45 67|67 45 a5 83) 
.*:	(f3 9f c2 d1|d1 c2 9f f3) 	lxvkq   vs60,24
.*:	(05 00 00 00|00 00 00 05) 	xxblendvb vs59,vs58,vs57,vs56
.*:	(87 7a ce 0f|0f ce 7a 87) 
.*:	(05 00 00 00|00 00 00 05) 	xxblendvh vs55,vs54,vs53,vs52
.*:	(86 f6 ad 1f|1f ad f6 86) 
.*:	(05 00 00 00|00 00 00 05) 	xxblendvw vs51,vs50,vs49,vs48
.*:	(86 72 8c 2f|2f 8c 72 86) 
.*:	(05 00 00 00|00 00 00 05) 	xxblendvd vs47,vs46,vs45,vs44
.*:	(85 ee 6b 3f|3f 6b ee 85) 
.*:	(05 00 00 07|07 00 00 05) 	xxpermx vs43,vs42,vs41,vs40,7
.*:	(89 6a 4a 0f|0f 4a 6a 89) 
