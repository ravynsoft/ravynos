#source: ./fcmp.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 72 10 00 00 00 80          	fcmp	#0x80000000, r0
   7:	fd 72 1f 00 00 00 80          	fcmp	#0x80000000, r15
   e:	fd 72 10 ff ff ff ff          	fcmp	#-1, r0
  15:	fd 72 1f ff ff ff ff          	fcmp	#-1, r15
  1c:	fc 87 00                      	fcmp	r0, r0
  1f:	fc 87 0f                      	fcmp	r0, r15
  22:	fc 87 f0                      	fcmp	r15, r0
  25:	fc 87 ff                      	fcmp	r15, r15
  28:	fc 84 00                      	fcmp	\[r0\]\.l, r0
  2b:	fc 84 0f                      	fcmp	\[r0\]\.l, r15
  2e:	fc 84 f0                      	fcmp	\[r15\]\.l, r0
  31:	fc 84 ff                      	fcmp	\[r15\]\.l, r15
  34:	fc 85 00 3f                   	fcmp	252\[r0\]\.l, r0
  38:	fc 85 0f 3f                   	fcmp	252\[r0\]\.l, r15
  3c:	fc 85 f0 3f                   	fcmp	252\[r15\]\.l, r0
  40:	fc 85 ff 3f                   	fcmp	252\[r15\]\.l, r15
  44:	fc 86 00 ff 3f                	fcmp	65532\[r0\]\.l, r0
  49:	fc 86 0f ff 3f                	fcmp	65532\[r0\]\.l, r15
  4e:	fc 86 f0 ff 3f                	fcmp	65532\[r15\]\.l, r0
  53:	fc 86 ff ff 3f                	fcmp	65532\[r15\]\.l, r15
