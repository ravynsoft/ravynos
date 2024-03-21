# Diagnostic tests for ON instructions
        .text
        onadd	%f1, %f8, %f16		! Invalid frs1
        onsub	%f8, %f17, %f24 	! Invalid frs2
        onmul	%f32, %f24, %f17 	! Invalid frd
        ondiv	%f32, %f24, %f1		! Likewise.
