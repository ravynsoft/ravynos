# sexw test

	sexw r0,r1
	sexw fp,sp

	sexw r0,0
	sexw r1,-1
	sexw 0,r2
	sexw r4,255
	sexw r6,-256

	sexw r8,256
	sexw r9,-257
	sexw r11,0x42424242

	sexw r0,foo

	sexw.f r0,r1
	sexw.f r2,1
	sexw.f 0,r4
	sexw.f r5,512
