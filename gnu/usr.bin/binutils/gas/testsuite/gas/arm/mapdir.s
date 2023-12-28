# Test that .arm / .thumb do not cause mapping symbols to be
# generated.  This could lead to duplicate mapping symbols at
# the same address.

	.section .fini_array
	.thumb
	.align	2
	.type	__do_global_dtors_aux_fini_array_entry, %object
__do_global_dtors_aux_fini_array_entry:
	.word   __do_global_dtors_aux

	.section .code,"ax",%progbits
	.thumb
	.arm
	nop

# .bss should not automatically emit $d.
	.bss

# Make sure that mapping symbols are placed in the correct section.
	.thumb
	.section .tcode,"ax",%progbits
	nop
