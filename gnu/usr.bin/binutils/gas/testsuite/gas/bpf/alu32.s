# Tests for the ALU eBPF instructions
        .text
        add32	%r2, 666
        add32	%r3, -666
        add32	%r4, 0x7eadbeef
        add32	%r5, %r6
        sub32	%r2, 666
        sub32	%r3, -666
        sub32	%r4, 0x7eadbeef
        sub32	%r5, %r6
        mul32	%r2, 666
        mul32	%r3, -666
        mul32	%r4, 0x7eadbeef
        mul32	%r5, %r6
        div32	%r2, 666
        div32	%r3, -666
        div32	%r4, 0x7eadbeef
        div32	%r5, %r6
        or32	%r2, 666
        or32	%r3, -666
        or32	%r4, 0x7eadbeef
        or32	%r5, %r6
        and32	%r2, 666
        and32	%r3, -666
        and32	%r4, 0x7eadbeef
        and32	%r5, %r6
        lsh32	%r2, 666
        lsh32	%r3, -666
        lsh32	%r4, 0x7eadbeef
        lsh32	%r5, %r6
        rsh32	%r2, 666
        rsh32	%r3, -666
        rsh32	%r4, 0x7eadbeef
        rsh32	%r5, %r6
        mod32	%r2, 666
        mod32	%r3, -666
        mod32	%r4, 0x7eadbeef
        mod32	%r5, %r6
        xor32	%r2, 666
        xor32	%r3, -666
        xor32	%r4, 0x7eadbeef
        xor32	%r5, %r6
        mov32	%r2, 666
        mov32	%r3, -666
        mov32	%r4, 0x7eadbeef
        mov32	%r5, %r6
        arsh32	%r2, 666
        arsh32	%r3, -666
        arsh32	%r4, 0x7eadbeef
        arsh32	%r5, %r6
        neg32	%r2
	endle	%r9,16
        endle	%r8,32
        endle	%r7,64
        endbe	%r6,16
        endbe	%r5,32
        endbe	%r4,64
