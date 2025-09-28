#source: ./fadd.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 72 20 00 00 00 80          	fadd	#0x80000000, r0
   7:	fd 72 2f 00 00 00 80          	fadd	#0x80000000, r15
   e:	fd 72 20 ff ff ff ff          	fadd	#-1, r0
  15:	fd 72 2f ff ff ff ff          	fadd	#-1, r15
  1c:	fc 8b 00                      	fadd	r0, r0
  1f:	fc 8b 0f                      	fadd	r0, r15
  22:	fc 8b f0                      	fadd	r15, r0
  25:	fc 8b ff                      	fadd	r15, r15
  28:	fc 88 00                      	fadd	\[r0\]\.l, r0
  2b:	fc 88 0f                      	fadd	\[r0\]\.l, r15
  2e:	fc 88 f0                      	fadd	\[r15\]\.l, r0
  31:	fc 88 ff                      	fadd	\[r15\]\.l, r15
  34:	fc 89 00 3f                   	fadd	252\[r0\]\.l, r0
  38:	fc 89 0f 3f                   	fadd	252\[r0\]\.l, r15
  3c:	fc 89 f0 3f                   	fadd	252\[r15\]\.l, r0
  40:	fc 89 ff 3f                   	fadd	252\[r15\]\.l, r15
  44:	fc 8a 00 ff 3f                	fadd	65532\[r0\]\.l, r0
  49:	fc 8a 0f ff 3f                	fadd	65532\[r0\]\.l, r15
  4e:	fc 8a f0 ff 3f                	fadd	65532\[r15\]\.l, r0
  53:	fc 8a ff ff 3f                	fadd	65532\[r15\]\.l, r15
  58:	ff a0 00                      	fadd	r0, r0, r0
  5b:	ff af 00                      	fadd	r0, r0, r15
  5e:	ff a0 0f                      	fadd	r0, r15, r0
  61:	ff af 0f                      	fadd	r0, r15, r15
  64:	ff a0 f0                      	fadd	r15, r0, r0
  67:	ff af f0                      	fadd	r15, r0, r15
  6a:	ff a0 ff                      	fadd	r15, r15, r0
  6d:	ff af ff                      	fadd	r15, r15, r15
