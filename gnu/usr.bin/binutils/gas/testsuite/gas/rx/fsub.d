#source: ./fsub.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 72 00 00 00 00 80          	fsub	#0x80000000, r0
   7:	fd 72 0f 00 00 00 80          	fsub	#0x80000000, r15
   e:	fd 72 00 ff ff ff ff          	fsub	#-1, r0
  15:	fd 72 0f ff ff ff ff          	fsub	#-1, r15
  1c:	fc 83 00                      	fsub	r0, r0
  1f:	fc 83 0f                      	fsub	r0, r15
  22:	fc 83 f0                      	fsub	r15, r0
  25:	fc 83 ff                      	fsub	r15, r15
  28:	fc 80 00                      	fsub	\[r0\]\.l, r0
  2b:	fc 80 0f                      	fsub	\[r0\]\.l, r15
  2e:	fc 80 f0                      	fsub	\[r15\]\.l, r0
  31:	fc 80 ff                      	fsub	\[r15\]\.l, r15
  34:	fc 81 00 3f                   	fsub	252\[r0\]\.l, r0
  38:	fc 81 0f 3f                   	fsub	252\[r0\]\.l, r15
  3c:	fc 81 f0 3f                   	fsub	252\[r15\]\.l, r0
  40:	fc 81 ff 3f                   	fsub	252\[r15\]\.l, r15
  44:	fc 82 00 ff 3f                	fsub	65532\[r0\]\.l, r0
  49:	fc 82 0f ff 3f                	fsub	65532\[r0\]\.l, r15
  4e:	fc 82 f0 ff 3f                	fsub	65532\[r15\]\.l, r0
  53:	fc 82 ff ff 3f                	fsub	65532\[r15\]\.l, r15
  58:	ff 80 00                      	fsub	r0, r0, r0
  5b:	ff 8f 00                      	fsub	r0, r0, r15
  5e:	ff 80 0f                      	fsub	r0, r15, r0
  61:	ff 8f 0f                      	fsub	r0, r15, r15
  64:	ff 80 f0                      	fsub	r15, r0, r0
  67:	ff 8f f0                      	fsub	r15, r0, r15
  6a:	ff 80 ff                      	fsub	r15, r15, r0
  6d:	ff 8f ff                      	fsub	r15, r15, r15
