# Test SPARC "natural" registers and instructions
        .text
        ba %ncc, 0f
         nop
        ldn [%g1], %g2
        ldna [%g1]#ASI_AIUP, %g2
        stn %g1, [%g2]
        stna %g1, [%g2]#ASI_AIUP
        slln %g1, 10, %g2
        srln %g1, 10, %g2
        sran %g1, 10, %g2
        casn [%g1], %g2, %g3
        casna [%g1]#ASI_AIUP, %g2, %g3
        clrn [%g1]
0:      nop
