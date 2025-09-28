# test for call26 relaxation via linker stubs

.globl text0
.section text0, "ax", @progbits
	call func0	# in same section
	call func1	# in nearby section
	call func2a	# in distant section
	jmpi func2b	# also in distant section

func0:
	ret

.section text1, "ax", @progbits
func1:
	nop
	nop
	call func2a	# in distant section
	ret

.section text2, "ax", @progbits
func2a:
	nop
	nop
	nop
	ret
func2b:
	nop
