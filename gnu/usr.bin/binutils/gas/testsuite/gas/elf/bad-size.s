	.text
_test_nop:
	.byte 0
	.section .text.entry.continue, "xa"
	.byte 0
	.size _test_nop, .-_test_nop
