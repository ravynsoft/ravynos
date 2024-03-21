# Source file used to test the flushda instruction.
.text
.set nobreak
foo:
	flushda	12(r2)
	
