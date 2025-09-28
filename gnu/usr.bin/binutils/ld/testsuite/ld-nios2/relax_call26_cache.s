# test for call26 relaxation via linker stubs
#
# The purpose of this test is to ensure that, when section text0 straddles
# a 256MB memory segment boundary with calls to the same function on either
# side, the stub caching doesn't get confused and incorrectly use a stub
# on the wrong side.

.globl text0
.section text0, "ax", @progbits
	call func2a	# in distant section
	call func2a	# in distant section
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	call func2a	# in distant section
	call func2a	# in distant section

.section text2, "ax", @progbits
.globl func2a
func2a:
	ret
