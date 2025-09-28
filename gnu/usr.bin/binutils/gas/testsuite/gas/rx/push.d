#source: ./push.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	7e 80                         	push\.b	r0
   2:	7e 8f                         	push\.b	r15
   4:	7e 90                         	push\.w	r0
   6:	7e 9f                         	push\.w	r15
   8:	7e a0                         	push\.l	r0
   a:	7e af                         	push\.l	r15
   c:	f4 08                         	push\.b	\[r0\]
   e:	f4 f8                         	push\.b	\[r15\]
  10:	f5 08 fc                      	push\.b	252\[r0\]
  13:	f5 f8 fc                      	push\.b	252\[r15\]
  16:	f6 08 fc ff                   	push\.b	65532\[r0\]
  1a:	f6 f8 fc ff                   	push\.b	65532\[r15\]
  1e:	f4 09                         	push\.w	\[r0\]
  20:	f4 f9                         	push\.w	\[r15\]
  22:	f5 09 7e                      	push\.w	252\[r0\]
  25:	f5 f9 7e                      	push\.w	252\[r15\]
  28:	f6 09 fe 7f                   	push\.w	65532\[r0\]
  2c:	f6 f9 fe 7f                   	push\.w	65532\[r15\]
  30:	f4 0a                         	push\.l	\[r0\]
  32:	f4 fa                         	push\.l	\[r15\]
  34:	f5 0a 3f                      	push\.l	252\[r0\]
  37:	f5 fa 3f                      	push\.l	252\[r15\]
  3a:	f6 0a ff 3f                   	push\.l	65532\[r0\]
  3e:	f6 fa ff 3f                   	push\.l	65532\[r15\]
