	.text
	.section .init0, "x"
	.global _start
_start:

	# U16_PMEMIMM
	ldi r0, %pmem(_text_label)
	ldi r0, %pmem(_text_label + 8)

# Try 32/16_PMEM
	.4byte %pmem(_text_label)
	.2byte %pmem(_text_label)
	.2byte %pmem(_text_label + 4)
