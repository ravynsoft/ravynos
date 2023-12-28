# Source file used to test the mul macro.
	
foo:
	mul	r4,r5,r6
	muli	r4,r5,0
	muli	r4,r5,1
	muli	r4,r5,-0x8000
	muli	r4,r5,0x7fff
	muli	r4,r5,undefined_symbol
	muli	r4,r5,defined_symbol
	mulxss	r4,r5,r6
	mulxsu	r4,r5,r6
	mulxuu  r4,r5,r6
.data
.set defined_symbol, 0x4040
