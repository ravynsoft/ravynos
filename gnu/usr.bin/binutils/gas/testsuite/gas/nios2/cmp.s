# Source file used to test the compare instructions
foo:
	cmpeq r11,r2,r3
	cmpge r11,r2,r3
	cmpgeu r11,r2,r3
	cmplt r11,r2,r3
	cmpltu r11,r2,r3
	cmpne r11,r2,r3
# test that cmp generates relocations correctly	
	cmpgei r11,r2,value
	cmpgeui r11,r2,value+0x200
	cmplti	r11,r2,value
	cmpltui r11,r2,value+0x200

	cmpgei r11,r2,0x7fff
	cmpgeui r11,r2,0x8000
	cmplti	r11,r2,-0x8000
	cmpltui r11,r2,0xFFFF
.global value	



