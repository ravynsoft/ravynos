#as: -mpower8
#objdump: -d -Mpower8
#name: Return-Oriented Programming tests

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(7f e1 a5 a5|a5 a5 e1 7f) 	hashst  r20,-8\(r1\)
.*:	(7f c1 ad a5|a5 ad c1 7f) 	hashst  r21,-16\(r1\)
.*:	(7c 01 b5 a5|a5 b5 01 7c) 	hashst  r22,-256\(r1\)
.*:	(7c 01 bd a4|a4 bd 01 7c) 	hashst  r23,-512\(r1\)
.*:	(7f e1 a5 e5|e5 a5 e1 7f) 	hashchk r20,-8\(r1\)
.*:	(7f c1 ad e5|e5 ad c1 7f) 	hashchk r21,-16\(r1\)
.*:	(7c 01 b5 e5|e5 b5 01 7c) 	hashchk r22,-256\(r1\)
.*:	(7c 01 bd e4|e4 bd 01 7c) 	hashchk r23,-512\(r1\)
.*:	(7f e1 a5 25|25 a5 e1 7f) 	hashstp r20,-8\(r1\)
.*:	(7f c1 ad 25|25 ad c1 7f) 	hashstp r21,-16\(r1\)
.*:	(7c 01 b5 25|25 b5 01 7c) 	hashstp r22,-256\(r1\)
.*:	(7c 01 bd 24|24 bd 01 7c) 	hashstp r23,-512\(r1\)
.*:	(7f e1 a5 65|65 a5 e1 7f) 	hashchkp r20,-8\(r1\)
.*:	(7f c1 ad 65|65 ad c1 7f) 	hashchkp r21,-16\(r1\)
.*:	(7c 01 b5 65|65 b5 01 7c) 	hashchkp r22,-256\(r1\)
.*:	(7c 01 bd 64|64 bd 01 7c) 	hashchkp r23,-512\(r1\)
#pass
