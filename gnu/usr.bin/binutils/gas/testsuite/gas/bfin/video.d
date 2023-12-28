#objdump: -dr
#name: video
.*: +file format .*
Disassembly of section .text:

[0-9a-f]+ <align>:
[ 0-9a-f]+:	0d c6 15 0e 	R7 = ALIGN8 \(R5, R2\);
[ 0-9a-f]+:	0d c6 08 4a 	R5 = ALIGN16 \(R0, R1\);
[ 0-9a-f]+:	0d c6 05 84 	R2 = ALIGN24 \(R5, R0\);

[0-9a-f]+ <disalgnexcpt>:
[ 0-9a-f]+:	12 c4 00 c0 	DISALGNEXCPT;

[0-9a-f]+ <byteop3p>:
[ 0-9a-f]+:	17 c4 02 0a 	R5 = BYTEOP3P \(R1:0, R3:2\) \(LO\);
[ 0-9a-f]+:	37 c4 02 00 	R0 = BYTEOP3P \(R1:0, R3:2\) \(HI\);
[ 0-9a-f]+:	17 c4 02 22 	R1 = BYTEOP3P \(R1:0, R3:2\) \(LO, R\);
[ 0-9a-f]+:	37 c4 02 24 	R2 = BYTEOP3P \(R1:0, R3:2\) \(HI, R\);

[0-9a-f]+ <dual16>:
[ 0-9a-f]+:	0c c4 7f 45 	R5 = A1.L \+ A1.H, R2 = A0.L \+ A0.H;

[0-9a-f]+ <byteop16p>:
[ 0-9a-f]+:	15 c4 82 06 	\(R2, R3\) = BYTEOP16P \(R1:0, R3:2\);
[ 0-9a-f]+:	15 c4 82 21 	\(R6, R0\) = BYTEOP16P \(R1:0, R3:2\) \(R\);

[0-9a-f]+ <byteop1p>:
[ 0-9a-f]+:	14 c4 02 0e 	R7 = BYTEOP1P \(R1:0, R3:2\);
[ 0-9a-f]+:	14 c4 02 44 	R2 = BYTEOP1P \(R1:0, R3:2\) \(T\);
[ 0-9a-f]+:	14 c4 02 26 	R3 = BYTEOP1P \(R1:0, R3:2\) \(R\);
[ 0-9a-f]+:	14 c4 02 6e 	R7 = BYTEOP1P \(R1:0, R3:2\) \(T, R\);

[0-9a-f]+ <byteop2p>:
[ 0-9a-f]+:	16 c4 02 00 	R0 = BYTEOP2P \(R1:0, R3:2\) \(RNDL\);
[ 0-9a-f]+:	36 c4 02 02 	R1 = BYTEOP2P \(R1:0, R3:2\) \(RNDH\);
[ 0-9a-f]+:	16 c4 02 44 	R2 = BYTEOP2P \(R1:0, R3:2\) \(TL\);
[ 0-9a-f]+:	36 c4 02 46 	R3 = BYTEOP2P \(R1:0, R3:2\) \(TH\);
[ 0-9a-f]+:	16 c4 02 28 	R4 = BYTEOP2P \(R1:0, R3:2\) \(RNDL, R\);
[ 0-9a-f]+:	36 c4 02 2a 	R5 = BYTEOP2P \(R1:0, R3:2\) \(RNDH, R\);
[ 0-9a-f]+:	16 c4 02 6c 	R6 = BYTEOP2P \(R1:0, R3:2\) \(TL, R\);
[ 0-9a-f]+:	36 c4 02 6e 	R7 = BYTEOP2P \(R1:0, R3:2\) \(TH, R\);

[0-9a-f]+ <bytepack>:
[ 0-9a-f]+:	18 c4 03 0a 	R5 = BYTEPACK \(R0, R3\);

[0-9a-f]+ <byteop16m>:
[ 0-9a-f]+:	15 c4 82 45 	\(R6, R2\) = BYTEOP16M \(R1:0, R3:2\);
[ 0-9a-f]+:	15 c4 02 6a 	\(R0, R5\) = BYTEOP16M \(R1:0, R3:2\) \(R\);

[0-9a-f]+ <saa>:
[ 0-9a-f]+:	12 c4 02 00 	SAA \(R1:0, R3:2\);
[ 0-9a-f]+:	12 c4 02 20 	SAA \(R1:0, R3:2\) \(R\);

[0-9a-f]+ <byteunpack>:
[ 0-9a-f]+:	18 c4 c0 45 	\(R7, R2\) = BYTEUNPACK R1:0;
[ 0-9a-f]+:	18 c4 90 69 	\(R6, R4\) = BYTEUNPACK R3:2 \(R\);
