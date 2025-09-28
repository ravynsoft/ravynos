# Test for PRU pmem relocations

	.global byte_sym
	.global short_sym
	.global long_sym

	.set byte_sym, 0xFA
	.set short_sym, 0xFACE
	.set long_sym, 0xDEADBEEF

	.text
	.global _text_label
	nop
_text_label:
	nop
