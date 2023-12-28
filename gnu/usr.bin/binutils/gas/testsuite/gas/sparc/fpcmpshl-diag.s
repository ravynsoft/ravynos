# Diagnostic tests for FPCP{ULE8,UGT8,EQ8,NE}{8,16,32}SHL instructions.
        .text
        fpcmpule8shl	%f32, %f48, 4, %g1	! Overflow in imm2 operand.
        fpcmpugt8shl	%f33, %f50, 4, %g2	! Invalid frs1.
        fpcmpeq8shl	%f36, %f53, 4, %g3	! Invalid frs2.
        fpcmpne8shl	%f38, %f46, 4, %g4	! Likewise.
        fpcmpule16shl	%f32, %f0, 4, %g1	! Likewise.
