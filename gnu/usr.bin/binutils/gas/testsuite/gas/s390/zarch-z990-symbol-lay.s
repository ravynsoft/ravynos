.text
foo:
	lay	%r1,bar
	.org 0x10000
bar:
	.long 42
