# Test error conditions in CBCOND instructions
        .text
        cwbe	%o1, +32,1f ! Overflow in the simm5 field.
        cwbe	%o1, -17,1f ! Likewise.
1:      nop
