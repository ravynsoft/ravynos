.text
.align 0
bar:
        mrrc    13,   0, r7, r0, cr4
        mcrr    p14,  0, r7, r0, cr5
        mrrc    15,  15, r7, r0, cr15
        mcrr    p14, 15, r7, r0, cr14
