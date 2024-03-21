# sexb test

	sexb r0,r1
	sexb fp,sp

	sexb r0,0
	sexb r1,-1
	sexb 0,r2
	sexb r4,255
	sexb r6,-256

	sexb r8,256
	sexb r9,-257
	sexb r11,0x42424242

	sexb r0,foo

	sexb.f r0,r1
	sexb.f r2,1
	sexb.f 0,r4
	sexb.f r5,512
