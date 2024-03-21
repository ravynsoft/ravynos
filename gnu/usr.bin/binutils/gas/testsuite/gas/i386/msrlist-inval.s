# Check Illegal MSRLIST instructions

	.allow_index_reg
	.text
_start:
	rdmsrlist		 #MSRLIST
	wrmsrlist		 #MSRLIST
