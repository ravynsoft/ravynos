#as: -mlibresoc
#objdump: -dr -Mlibresoc

.*:     file format .*


Disassembly of section \.text:
0+ <\.text>:
.*:	(27 00 00 58|58 00 00 27) 	svstep. r0,1,0
.*:	(26 00 00 58|58 00 00 26) 	svstep  r0,1,0
.*:	(26 00 e0 5b|5b e0 00 26) 	svstep  r31,1,0
.*:	(26 7e 00 58|58 00 7e 26) 	svstep  r0,64,0
.*:	(66 00 00 58|58 00 00 66) 	svstep  r0,1,1
