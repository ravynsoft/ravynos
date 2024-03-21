# rrc test

	rrc r0,r1
	rrc fp,sp

	rrc r0,0
	rrc r1,-1
	rrc 0,r2
	rrc r4,255
	rrc r6,-256

	rrc r8,256
	rrc r9,-257
	rrc r11,0x42424242

	rrc r0,foo

	rrc.f r0,r1
	rrc.f r2,1
	rrc.f 0,r4
	rrc.f r5,512
