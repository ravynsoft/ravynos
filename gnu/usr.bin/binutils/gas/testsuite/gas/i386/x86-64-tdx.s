# Check TDX instructions.

	.text
_start:
	tdcall
	seamret
	seamops
	seamcall
