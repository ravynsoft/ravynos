# Check 64bit UINTR instructions.

	.text
_start:
	uiret
	testui
	clui
	stui
	senduipi %rax
	senduipi %r10
