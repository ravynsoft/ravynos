#source: ./fdiv.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 72 40 00 00 00 80          	fdiv	#0x80000000, r0
   7:	fd 72 4f 00 00 00 80          	fdiv	#0x80000000, r15
   e:	fd 72 40 ff ff ff ff          	fdiv	#-1, r0
  15:	fd 72 4f ff ff ff ff          	fdiv	#-1, r15
  1c:	fc 93 00                      	fdiv	r0, r0
  1f:	fc 93 0f                      	fdiv	r0, r15
  22:	fc 93 f0                      	fdiv	r15, r0
  25:	fc 93 ff                      	fdiv	r15, r15
  28:	fc 90 00                      	fdiv	\[r0\]\.l, r0
  2b:	fc 90 0f                      	fdiv	\[r0\]\.l, r15
  2e:	fc 90 f0                      	fdiv	\[r15\]\.l, r0
  31:	fc 90 ff                      	fdiv	\[r15\]\.l, r15
  34:	fc 91 00 3f                   	fdiv	252\[r0\]\.l, r0
  38:	fc 91 0f 3f                   	fdiv	252\[r0\]\.l, r15
  3c:	fc 91 f0 3f                   	fdiv	252\[r15\]\.l, r0
  40:	fc 91 ff 3f                   	fdiv	252\[r15\]\.l, r15
  44:	fc 92 00 ff 3f                	fdiv	65532\[r0\]\.l, r0
  49:	fc 92 0f ff 3f                	fdiv	65532\[r0\]\.l, r15
  4e:	fc 92 f0 ff 3f                	fdiv	65532\[r15\]\.l, r0
  53:	fc 92 ff ff 3f                	fdiv	65532\[r15\]\.l, r15
