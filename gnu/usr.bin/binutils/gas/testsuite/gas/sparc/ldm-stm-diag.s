# Diagnostic tests for ldm/stm/ldma/stma
        .text
        ldmsh	[%g1+0x401], %g2	! Overflow in simm10 constant.
        stmh	%g2, [%g1+0x401]	! Likewise.
