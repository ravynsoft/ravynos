# Test ldmf/stmf/ldmfa/stmfa
        .text
        ldmfs	[%g0+%g1], %f1
        ldmfs	[%g1],	%f1
        ldmfs	[%g1+102], %f1
        ldmfs	[102+%g1], %f1
        ldmfs	[102], %f1
        ldmfd	[%g0+%g1], %f32
        ldmfd	[%g1],	%f32
        ldmfd	[%g1+102], %f32
        ldmfd	[102+%g1], %f32
        ldmfd	[102], %f32
        ldmfsa	[%g0+%g1] %asi, %f1
        ldmfsa	[%g1] %asi, %f1
        ldmfda	[%g0+%g1] %asi, %f32
        ldmfda	[%g1] %asi, %f32
        stmfs	%f1, [%g0+%g1]
        stmfs	%f1, [%g1]
        stmfs	%f1, [%g1+102]
        stmfs	%f1, [102+%g1]
        stmfs	%f1, [102]
        stmfd	%f32, [%g0+%g1]
        stmfd	%f32, [%g1]
        stmfd	%f32, [%g1+102]
        stmfd	%f32, [102+%g1]
        stmfd	%f32, [102]
        stmfsa	%f1, [%g0+%g1] %asi
        stmfsa	%f1, [%g1] %asi
        stmfda	%f32, [%g0+%g1] %asi
        stmfda	%f32, [%g1] %asi

