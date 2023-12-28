# Test ldm/stm/ldma/stma
        .text
        ldmsh	[%g0+%g1], %g2
        ldmsh	[%g1], %g3
        ldmsh	[%g1+102], %g3
        ldmsh	[102+%g1], %g3
        ldmsh	[102], %g3
        ldmuh	[%g0+%g1], %g2
        ldmuh	[%g1], %g3
        ldmuh	[%g1+102], %g3
        ldmuh	[102+%g1], %g3
        ldmuh	[102], %g3
        ldmsw	[%g0+%g1], %g2
        ldmsw	[%g1], %g3
        ldmsw	[%g1+102], %g3
        ldmsw	[102+%g1], %g3
        ldmsw	[102], %g3
        ldmuw	[%g0+%g1], %g2
        ldmuw	[%g1], %g3
        ldmuw	[%g1+102], %g3
        ldmuw	[102+%g1], %g3
        ldmuw	[102], %g3
        ldmx	[%g0+%g1], %g2
        ldmx	[%g1], %g3
        ldmx	[%g1+102], %g3
        ldmx	[102+%g1], %g3
        ldmx	[102], %g3
        ldmux	[%g0+%g1], %g2
        ldmux	[%g1], %g3
        ldmux	[%g1+102], %g3
        ldmux	[102+%g1], %g3
        ldmux	[102], %g3
        ldmsha	[%g1+%g2] %asi, %g3
        ldmsha	[%g1] %asi, %g2
        ldmuha	[%g1+%g2] %asi, %g3
        ldmuha	[%g1] %asi, %g2
        ldmswa	[%g1+%g2] %asi, %g3
        ldmswa	[%g1] %asi, %g2
        ldmuwa	[%g1+%g2] %asi, %g3
        ldmuwa	[%g1] %asi, %g2
        ldmxa	[%g1+%g2] %asi, %g3
        ldmxa	[%g1] %asi, %g2
	stmh	%g2, [%g0+%g1]
        stmh	%g3, [%g1]
        stmh	%g3, [%g1+102]
        stmh	%g3, [102+%g1]
        stmh	%g3, [102]
        stmw	%g2, [%g0+%g1]
        stmw	%g3, [%g1]
        stmw	%g3, [%g1+102]
        stmw	%g3, [102+%g1]
        stmw	%g3, [102]
        stmx	%g2, [%g0+%g1]
        stmx	%g3, [%g1]
        stmx	%g3, [%g1+102]
        stmx	%g3, [102+%g1]
        stmx	%g3, [102]
        stmha	%g2, [%g0+%g1] %asi
        stmha   %g3, [%g1] %asi
        stmwa	%g2, [%g0+%g1] %asi
        stmwa	%g3, [%g1] %asi
        stmxa	%g2, [%g0+%g1] %asi
        stmxa	%g3, [%g1] %asi
