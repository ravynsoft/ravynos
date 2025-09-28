# Test to check for the proper bump warning due to the ASI load.
        .text
        lda	[%g0] #ASI_P, %g1
