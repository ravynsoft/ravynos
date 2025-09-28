# Source file to test branch relaxation with the BPOSGE32 and BPOSGE64
# instructions.

	.text
foo:
	b	bar
	bposge32 bar
	bposge64 bar
	bal	bar

	.space	0x20000
bar:
