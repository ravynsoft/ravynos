#as: -mpower8
#objdump: -dr -Mpower8
#name: Altivec ISA 2.07 instructions

.*

Disassembly of section \.text:

0+00 <start>:
.*:	(10 d1 84 03|03 84 d1 10) 	vabsdub v6,v17,v16
.*:	(12 b2 24 43|43 24 b2 12) 	vabsduh v21,v18,v4
.*:	(13 34 4c 83|83 4c 34 13) 	vabsduw v25,v20,v9
.*:	(10 d1 a6 ad|ad a6 d1 10) 	vpermxor v6,v17,v20,v26
.*:	(13 ba 7f 3c|3c 7f ba 13) 	vaddeuqm v29,v26,v15,v28
.*:	(11 e8 3e 3d|3d 3e e8 11) 	vaddecuq v15,v8,v7,v24
.*:	(10 46 a8 7e|7e a8 46 10) 	vsubeuqm v2,v6,v21,v1
.*:	(13 a6 01 3f|3f 01 a6 13) 	vsubecuq v29,v6,v0,v4
.*:	(11 c9 18 88|88 18 c9 11) 	vmulouw v14,v9,v3
.*:	(13 10 90 89|89 90 10 13) 	vmuluwm v24,v16,v18
.*:	(11 51 88 c0|c0 88 51 11) 	vaddudm v10,v17,v17
.*:	(13 d9 20 c2|c2 20 d9 13) 	vmaxud  v30,v25,v4
.*:	(11 46 e0 c4|c4 e0 46 11) 	vrld    v10,v6,v28
.*:	(13 67 38 c7|c7 38 67 13) 	vcmpequd v27,v7,v7
.*:	(12 d0 c9 00|00 c9 d0 12) 	vadduqm v22,v16,v25
.*:	(10 35 e9 40|40 e9 35 10) 	vaddcuq v1,v21,v29
.*:	(12 8b 99 88|88 99 8b 12) 	vmulosw v20,v11,v19
.*:	(13 13 09 c2|c2 09 13 13) 	vmaxsd  v24,v19,v1
.*:	(11 bb f2 88|88 f2 bb 11) 	vmuleuw v13,v27,v30
.*:	(11 38 8a c2|c2 8a 38 11) 	vminud  v9,v24,v17
.*:	(11 52 e2 c7|c7 e2 52 11) 	vcmpgtud v10,v18,v28
.*:	(10 1d b3 88|88 b3 1d 10) 	vmulesw v0,v29,v22
.*:	(11 bc 0b c2|c2 0b bc 11) 	vminsd  v13,v28,v1
.*:	(11 54 2b c4|c4 2b 54 11) 	vsrad   v10,v20,v5
.*:	(13 75 2b c7|c7 2b 75 13) 	vcmpgtsd v27,v21,v5
.*:	(10 17 f6 01|01 f6 17 10) 	bcdadd\. v0,v23,v30,1
.*:	(13 38 d4 08|08 d4 38 13) 	vpmsumb v25,v24,v26
.*:	(11 04 26 41|41 26 04 11) 	bcdsub\. v8,v4,v4,1
.*:	(12 0e d4 48|48 d4 0e 12) 	vpmsumh v16,v14,v26
.*:	(13 62 d4 4e|4e d4 62 13) 	vpkudum v27,v2,v26
.*:	(10 d7 8c 88|88 8c d7 10) 	vpmsumw v6,v23,v17
.*:	(12 86 cc c8|c8 cc 86 12) 	vpmsumd v20,v6,v25
.*:	(13 76 84 ce|ce 84 76 13) 	vpkudus v27,v22,v16
.*:	(12 b4 94 c0|c0 94 b4 12) 	vsubudm v21,v20,v18
.*:	(12 b4 95 00|00 95 b4 12) 	vsubuqm v21,v20,v18
.*:	(13 bd 35 08|08 35 bd 13) 	vcipher v29,v29,v6
.*:	(10 4d a5 09|09 a5 4d 10) 	vcipherlast v2,v13,v20
.*:	(12 80 95 0c|0c 95 80 12) 	vgbbd   v20,v18
.*:	(12 68 cd 40|40 cd 68 12) 	vsubcuq v19,v8,v25
.*:	(11 3a ed 44|44 ed 3a 11) 	vorc    v9,v26,v29
.*:	(12 94 6d 48|48 6d 94 12) 	vncipher v20,v20,v13
.*:	(11 e5 dd 49|49 dd e5 11) 	vncipherlast v15,v5,v27
.*:	(10 73 35 4c|4c 35 73 10) 	vbpermq v3,v19,v6
.*:	(13 c4 e5 4e|4e e5 c4 13) 	vpksdus v30,v4,v28
.*:	(10 04 75 84|84 75 04 10) 	vnand   v0,v4,v14
.*:	(12 28 ed c4|c4 ed 28 12) 	vsld    v17,v8,v29
.*:	(13 b4 05 c8|c8 05 b4 13) 	vsbox   v29,v20
.*:	(11 67 5d ce|ce 5d 67 11) 	vpksdss v11,v7,v11
.*:	(10 73 84 c7|c7 84 73 10) 	vcmpequd\. v3,v19,v16
.*:	(12 40 8e 4e|4e 8e 40 12) 	vupkhsw v18,v17
.*:	(13 a8 6e 82|82 6e a8 13) 	vshasigmaw v29,v8,0,13
.*:	(12 fc d6 84|84 d6 fc 12) 	veqv    v23,v28,v26
.*:	(13 a0 17 8c|8c 17 a0 13) 	vmrgew  v29,v0,v2
.*:	(13 a0 16 8c|8c 16 a0 13) 	vmrgow  v29,v0,v2
.*:	(13 73 06 c2|c2 06 73 13) 	vshasigmad v27,v19,0,0
.*:	(12 9c e6 c4|c4 e6 9c 12) 	vsrd    v20,v28,v28
.*:	(12 40 ae ce|ce ae 40 12) 	vupklsw v18,v21
.*:	(13 c0 3f 02|02 3f c0 13) 	vclzb   v30,v7
.*:	(13 a0 af 03|03 af a0 13) 	vpopcntb v29,v21
.*:	(13 20 af 42|42 af 20 13) 	vclzh   v25,v21
.*:	(12 00 f7 43|43 f7 00 12) 	vpopcnth v16,v30
.*:	(13 80 1f 82|82 1f 80 13) 	vclzw   v28,v3
.*:	(11 40 4f 83|83 4f 40 11) 	vpopcntw v10,v9
.*:	(12 c0 4f c2|c2 4f c0 12) 	vclzd   v22,v9
.*:	(11 e0 f7 c3|c3 f7 e0 11) 	vpopcntd v15,v30
.*:	(10 5f 36 c7|c7 36 5f 10) 	vcmpgtud\. v2,v31,v6
.*:	(12 8f 17 c7|c7 17 8f 12) 	vcmpgtsd\. v20,v15,v2
#pass
