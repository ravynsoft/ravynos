#source: ./fmul.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 72 30 00 00 00 80          	fmul	#0x80000000, r0
   7:	fd 72 3f 00 00 00 80          	fmul	#0x80000000, r15
   e:	fd 72 30 ff ff ff ff          	fmul	#-1, r0
  15:	fd 72 3f ff ff ff ff          	fmul	#-1, r15
  1c:	fc 8f 00                      	fmul	r0, r0
  1f:	fc 8f 0f                      	fmul	r0, r15
  22:	fc 8f f0                      	fmul	r15, r0
  25:	fc 8f ff                      	fmul	r15, r15
  28:	fc 8c 00                      	fmul	\[r0\]\.l, r0
  2b:	fc 8c 0f                      	fmul	\[r0\]\.l, r15
  2e:	fc 8c f0                      	fmul	\[r15\]\.l, r0
  31:	fc 8c ff                      	fmul	\[r15\]\.l, r15
  34:	fc 8d 00 3f                   	fmul	252\[r0\]\.l, r0
  38:	fc 8d 0f 3f                   	fmul	252\[r0\]\.l, r15
  3c:	fc 8d f0 3f                   	fmul	252\[r15\]\.l, r0
  40:	fc 8d ff 3f                   	fmul	252\[r15\]\.l, r15
  44:	fc 8e 00 ff 3f                	fmul	65532\[r0\]\.l, r0
  49:	fc 8e 0f ff 3f                	fmul	65532\[r0\]\.l, r15
  4e:	fc 8e f0 ff 3f                	fmul	65532\[r15\]\.l, r0
  53:	fc 8e ff ff 3f                	fmul	65532\[r15\]\.l, r15
  58:	ff b0 00                      	fmul	r0, r0, r0
  5b:	ff bf 00                      	fmul	r0, r0, r15
  5e:	ff b0 0f                      	fmul	r0, r15, r0
  61:	ff bf 0f                      	fmul	r0, r15, r15
  64:	ff b0 f0                      	fmul	r15, r0, r0
  67:	ff bf f0                      	fmul	r15, r0, r15
  6a:	ff b0 ff                      	fmul	r15, r15, r0
  6d:	ff bf ff                      	fmul	r15, r15, r15
