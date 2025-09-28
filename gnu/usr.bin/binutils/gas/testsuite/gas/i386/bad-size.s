	.text
_test_nop:
	nop
	.section .text.entry.continue, "xa"
	nop
	.size _test_nop, .-_test_nop
