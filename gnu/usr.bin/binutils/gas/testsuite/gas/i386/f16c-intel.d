#objdump: -dwMintel
#name: i386 F16C (Intel disassembly)
#source: f16c.s

.*: +file format .*


Disassembly of section .text:

0+ <foo>:
[ 	]*[a-f0-9]+:	c4 e2 7d 13 e4       	vcvtph2ps ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 13 21       	vcvtph2ps ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 13 f4       	vcvtph2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 7d 1d e4 02    	vcvtps2ph xmm4,ymm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 7d 1d 21 02    	vcvtps2ph XMMWORD PTR \[ecx\],ymm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 79 1d e4 02    	vcvtps2ph xmm4,xmm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 79 1d 21 02    	vcvtps2ph QWORD PTR \[ecx\],xmm4,0x2
[ 	]*[a-f0-9]+:	c4 e2 7d 13 e4       	vcvtph2ps ymm4,xmm4
[ 	]*[a-f0-9]+:	c4 e2 7d 13 21       	vcvtph2ps ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 7d 13 21       	vcvtph2ps ymm4,XMMWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 13 f4       	vcvtph2ps xmm6,xmm4
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e2 79 13 21       	vcvtph2ps xmm4,QWORD PTR \[ecx\]
[ 	]*[a-f0-9]+:	c4 e3 7d 1d e4 02    	vcvtps2ph xmm4,ymm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 7d 1d 21 02    	vcvtps2ph XMMWORD PTR \[ecx\],ymm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 7d 1d 21 02    	vcvtps2ph XMMWORD PTR \[ecx\],ymm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 79 1d e4 02    	vcvtps2ph xmm4,xmm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 79 1d 21 02    	vcvtps2ph QWORD PTR \[ecx\],xmm4,0x2
[ 	]*[a-f0-9]+:	c4 e3 79 1d 21 02    	vcvtps2ph QWORD PTR \[ecx\],xmm4,0x2
#pass
