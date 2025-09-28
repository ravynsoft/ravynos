# Tests for the LDDW instruction
        .text
        lddw	%r3, 1
        lddw	%r4, 0xdeadbeef
        lddw	%r5, 0x1122334455667788
        lddw	%r6, -2
