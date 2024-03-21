# Check 64bit FRED instructions

	.allow_index_reg
	.text
_start:
	erets		 #FRED
	eretu		 #FRED

.intel_syntax noprefix
	erets		 #FRED
	eretu		 #FRED
