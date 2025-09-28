    # Tests for xBPF-specific alu instructions
    .text
    sdiv %r2, 2
    sdiv %r3, -3
    sdiv %r4, 0x7eadbeef
    sdiv %r5, %r2

    smod %r2, 3
    smod %r3, -4
    smod %r4, 0x7eadbeef
    smod %r5, %r2
