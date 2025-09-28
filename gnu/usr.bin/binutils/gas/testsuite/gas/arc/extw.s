# extw test

	extw r0,r1
	extw fp,sp

	extw r0,0
	extw r1,-1
	extw 0,r2
	extw r4,255
	extw r6,-256

	extw r8,256
	extw r9,-257
	extw r11,0x42424242

	extw r0,foo

	extw.f r0,r1
	extw.f r2,1
	extw.f 0,r4
	extw.f r5,512
