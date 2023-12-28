	.text
	.intel_syntax noprefix
	.arch generic32
fpu:
	.arch .287
	fneni
	fnsetpm

	.arch .no287
	.arch .387
	fneni
	fnsetpm
