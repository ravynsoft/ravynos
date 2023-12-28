# Check 32bit unsupported NOTRACK prefix

	.allow_index_reg
	.text
_start:
	notrack call foo
	notrack jmp foo

	fs notrack call *%eax
	notrack fs call *%eax

	.intel_syntax noprefix
	fs notrack call eax
	notrack fs call eax

	.p2align	4,0
