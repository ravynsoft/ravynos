#as: -mpower10
#objdump: -dr -Mpower10
#name: mask manipulation operations

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(10 10 0e 42|42 0e 10 10) 	mtvsrbm v0,r1
.*:	(10 51 1e 42|42 1e 51 10) 	mtvsrhm v2,r3
.*:	(10 92 2e 42|42 2e 92 10) 	mtvsrwm v4,r5
.*:	(10 d3 3e 42|42 3e d3 10) 	mtvsrdm v6,r7
.*:	(11 14 4e 42|42 4e 14 11) 	mtvsrqm v8,r9
.*:	(11 5a 12 14|14 12 5a 11) 	mtvsrbmi v10,4660
.*:	(11 60 66 42|42 66 60 11) 	vexpandbm v11,v12
.*:	(11 a1 76 42|42 76 a1 11) 	vexpandhm v13,v14
.*:	(11 e2 86 42|42 86 e2 11) 	vexpandwm v15,v16
.*:	(12 23 96 42|42 96 23 12) 	vexpanddm v17,v18
.*:	(12 64 a6 42|42 a6 64 12) 	vexpandqm v19,v20
.*:	(12 a8 b6 42|42 b6 a8 12) 	vextractbm r21,v22
.*:	(12 e9 c6 42|42 c6 e9 12) 	vextracthm r23,v24
.*:	(13 2a d6 42|42 d6 2a 13) 	vextractwm r25,v26
.*:	(13 6b e6 42|42 e6 6b 13) 	vextractdm r27,v28
.*:	(13 ac f6 42|42 f6 ac 13) 	vextractqm r29,v30
.*:	(13 f8 06 42|42 06 f8 13) 	vcntmbb r31,v0,0
.*:	(13 db 0e 42|42 0e db 13) 	vcntmbh r30,v1,1
.*:	(13 bd 16 42|42 16 bd 13) 	vcntmbw r29,v2,1
.*:	(13 9e 1e 42|42 1e 9e 13) 	vcntmbd r28,v3,0
