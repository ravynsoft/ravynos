.text
.align 0
bar:
        mrrc2    13,   0, r7, r0, cr4
        mcrr2    p14,  0, r7, r0, cr5
        mrrc2    15,  15, r7, r0, cr15
        mcrr2    p14, 15, r7, r0, cr14
