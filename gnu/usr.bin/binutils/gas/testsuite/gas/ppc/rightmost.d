#as: -mpower10
#objdump: -dr -Mpower10

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(7f ef 80 1b|1b 80 ef 7f) 	lxvrbx  vs63,r15,r16
.*:	(7f d1 90 5b|5b 90 d1 7f) 	lxvrhx  vs62,r17,r18
.*:	(7f b3 a0 9b|9b a0 b3 7f) 	lxvrwx  vs61,r19,r20
.*:	(7f 95 b0 db|db b0 95 7f) 	lxvrdx  vs60,r21,r22
.*:	(7c 17 c1 1a|1a c1 17 7c) 	stxvrbx vs0,r23,r24
.*:	(7c 39 d1 5a|5a d1 39 7c) 	stxvrhx vs1,r25,r26
.*:	(7c 5b e1 9a|9a e1 5b 7c) 	stxvrwx vs2,r27,r28
.*:	(7c 7d f1 da|da f1 7d 7c) 	stxvrdx vs3,r29,r30
