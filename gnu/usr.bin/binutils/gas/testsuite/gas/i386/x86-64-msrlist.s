# Check 64bit MSRLIST instructions

	.text
_start:
	rdmsrlist		 #MSRLIST
	wrmsrlist		 #MSRLIST

.intel_syntax noprefix
	rdmsrlist		 #MSRLIST
	wrmsrlist		 #MSRLIST
