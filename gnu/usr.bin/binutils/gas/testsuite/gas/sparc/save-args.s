! Test several forms of save argument
	.text
foo:
	save
	save	%sp, -96, %sp
	save	-96, %sp, %sp
