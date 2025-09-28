# extb test

	extb r0,r1
	extb fp,sp

	extb r0,0
	extb r1,-1
	extb 0,r2
	extb r4,255
	extb r6,-256

	extb r8,256
	extb r9,-257
	extb r11,0x42424242

	extb r0,foo

	extb.f r0,r1
	extb.f r2,1
	extb.f 0,r4
	extb.f r5,512
