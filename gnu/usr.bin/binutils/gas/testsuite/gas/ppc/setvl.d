#as: -mlibresoc
#objdump: -dr -Mlibresoc

.*:     file format .*


Disassembly of section \.text:
0+ <\.text>:
.*:	(37 00 00 58|58 00 00 37) 	setvl.  r0,r0,1,0,0,0
.*:	(36 00 00 58|58 00 00 36) 	setvl   r0,r0,1,0,0,0
.*:	(36 00 e0 5b|5b e0 00 36) 	setvl   r31,r0,1,0,0,0
.*:	(36 00 1f 58|58 1f 00 36) 	setvl   r0,r31,1,0,0,0
.*:	(36 7e 00 58|58 00 7e 36) 	setvl   r0,r0,64,0,0,0
.*:	(76 00 00 58|58 00 00 76) 	setvl   r0,r0,1,1,0,0
.*:	(b6 00 00 58|58 00 00 b6) 	setvl   r0,r0,1,0,1,0
.*:	(36 01 00 58|58 00 01 36) 	setvl   r0,r0,1,0,0,1
