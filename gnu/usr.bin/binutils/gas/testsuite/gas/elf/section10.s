	# Test numeric values for the section's flags field.
	.section sec1, "0x06000000"
	.word 1

	# Make sure that a numeric value can be mixed with alpha values.
	.section sec2, "a128x"
	.word 2

	# Make sure that specifying further arguments to .sections is still supported
	.section sec3, "0xfedff000MS", %progbits, 32
	.word 3

	# Make sure that extra flags can be set for well known sections as well.
	.section .text, "0x04000006"
	.word 4

	# Test numeric values for the section's type field.
	.section sec4, "ax", %0x60000011
	.word 5

	# Test both together, with a quoted type value.
	.section sec5, "0xfedf0000", "0x80000009"
	.word 6

	# Test that declaring an extended version of a known special section works.
	.section .data.foo, "aw", %0xff000000
	.word 7

	# Check that .pushsection works as well.
	.pushsection sec6, 2, "0x120004", %5678
	.word 8

	.popsection

	# FIXME: We ought to check setting 64-bit flag values for 64-bit ELF targets...
