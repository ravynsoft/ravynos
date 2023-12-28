# Test for call26 relaxation via linker stubs.
# This .s file is used with several different linker scripts that vary the
# placement of the sections in the output.
# Section text0 is 32 bytes long and requires at least 2 linker stubs
# (12 bytes each) to reach the call destinations in text2.  Another stub
# may be required to reach func0 if the section is laid out so that it crosses 
# a 256MB memory segment boundary.

.globl text0
.section text0, "ax", @progbits
	call func0	# in same section
	call func2a	# in distant section
	nop
	nop
	nop
	nop
	jmpi func2b	# in distant section

func0:
	ret

.section text2, "ax", @progbits
func2a:
	nop
	nop
	nop
	ret
func2b:
	nop
