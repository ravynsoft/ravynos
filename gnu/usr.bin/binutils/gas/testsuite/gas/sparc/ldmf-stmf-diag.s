# Diagnostic tests for ldmf/stmf/ldmfa/stmfs
        .text
        ldmfs	[%g1+0x401], %f1	! Overflow in simm10 constant.
        ldmfs   [0x401], %f1		! Likewise.
        ldmfd	[%g1+0x401], %f2	! Likewise.
        ldmfd	[0x401], %f2		! Likewise.
        ldmfd	[%g1+%g2], %f1  	! Invalid frd.
        stmfs	%f1, [%g1+0x401]	! Overflow in simm10 constant.
        stmfs	%f1, [0x401]	        ! Likewise.
        stmfd	%f2, [%g1+0x401]	! Likewise.
        stmfd	%f2, [0x401]		! Likewise.
        stmfd	%f1, [%g1+%g2]		! Invalid frd.
