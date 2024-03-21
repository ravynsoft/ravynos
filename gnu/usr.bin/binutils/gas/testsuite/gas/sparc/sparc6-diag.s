# Test error detection and reporting in SPARC6 instructions.
        .text
        dictunpack	%f32, 100, %f34 ! Too big immediate.
        dictunpack	%f1, 10, %f32	! rs1 is not a double fp-register
        dictunpack	%32, 10, %f5	! rd is not a double fp-register
	fpsll64x	%f31, %f34, %f36 ! rs1 is not a double fp-register
        fpsra64x	%f32, %f33, %f36 ! rs2 is not a double fp-register
        fpsrl64x	%f32, %f34, %f1	 ! rd is not a double fp-register
