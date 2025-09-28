#source: ./rtsd.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	67 00                         	rtsd	#0
   2:	67 ff                         	rtsd	#0x3fc
   4:	3f 18 00                      	rtsd	#0, r1-r8
   7:	3f 1e 00                      	rtsd	#0, r1-r14
   a:	3f 78 00                      	rtsd	#0, r7-r8
   d:	3f 7e 00                      	rtsd	#0, r7-r14
  10:	3f 18 ff                      	rtsd	#0x3fc, r1-r8
  13:	3f 1e ff                      	rtsd	#0x3fc, r1-r14
  16:	3f 78 ff                      	rtsd	#0x3fc, r7-r8
  19:	3f 7e ff                      	rtsd	#0x3fc, r7-r14
