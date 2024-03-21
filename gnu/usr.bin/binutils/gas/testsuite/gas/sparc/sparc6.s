# Test SPARC6 instructions
        .text
        dictunpack	%f32, 10, %f34
	fpsll64x	%f32, %f34, %f36
        fpsra64x	%f38, %f40, %f42
        fpsrl64x	%f44, %f48, %f50
        revbitsb	%g1, %g2
        revbytesh	%g3, %g4
        revbytesw	%g5, %g6
        revbytesx	%g2, %g4
        sha3
