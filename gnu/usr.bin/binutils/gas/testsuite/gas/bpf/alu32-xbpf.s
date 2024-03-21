    # Tests for xBPF-specific alu instructions
    .text
    sdiv32 %r2, 2
    sdiv32 %r3, -3
    sdiv32 %r4, 0x7eadbeef
    sdiv32 %r5, %r2

    smod32 %r2, 3
    smod32 %r3, -4
    smod32 %r4, 0x7eadbeef
    smod32 %r5, %r2
