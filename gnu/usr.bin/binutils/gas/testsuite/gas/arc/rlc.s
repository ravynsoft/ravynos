# rlc test

	rlc r0,r1
	rlc fp,sp

	rlc r0,0
	rlc r1,-1
	rlc 0,r2
	rlc r4,255
	rlc r6,-256

	rlc r8,256
	rlc r9,-257
	rlc r11,0x42424242

	rlc r0,foo

	rlc.f r0,r1
	rlc.f r2,1
	rlc.f 0,r4
	rlc.f r5,512
