# Source file used to test the and, andhi and andi instructions
	
foo:
	and	r4,r4,r4
	andi	r4,r4,0x7fff
	andi	r4,r4,0x8000
	andi	r4,r4,0xffff
	andi	r4,r4,0x0
	andhi	r4,r4,0x7fff
	andhi	r4,r4,0x8000
	andhi	r4,r4,0xffff
	andhi	r4,r4,0x0
	
