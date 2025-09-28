# Source file used to test the andci and andchi instructions
	
foo:
	andci	r4,r4,0x7fff
	andci	r4,r4,0x8000
	andci	r4,r4,0xffff
	andci	r4,r4,0x0
	andchi	r4,r4,0x7fff
	andchi	r4,r4,0x8000
	andchi	r4,r4,0xffff
	andchi	r4,r4,0x0
	
