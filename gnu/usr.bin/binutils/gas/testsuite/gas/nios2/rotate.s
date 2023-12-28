# test Nios II rotate instructions

.text
foo:
	rol	r4,r4,r4
	roli	r4,r4,31
	ror	r4,r4,r4
	sll	r4,r4,r4
	slli	r4,r4,24
	sra	r4,r4,r4
	srai	r4,r4,10
	srl	r4,r4,r4
	srli	r4,r4,5
