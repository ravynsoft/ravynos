target:
        ror     a0, a1, a2
        rol     a0, a1, a2
        rori    a0, a1, 2
        rorw    a0, a1, a2
        rolw    a0, a1, a2
        roriw   a0, a1, 2
        andn    a0, a1, a2
        orn     a0, a1, a2
        xnor    a0, a1, a2
        pack    a0, a1, a2
        packh   a0, a1, a2
        packw   a0, a1, a2
        brev8   a0, a0
        rev8    a0, a0
        clmul   a0, a1, a2
        clmulh  a0, a1, a2
        xperm4  a0, a1, a2
        xperm8  a0, a1, a2
        aes64ds     a0, a1, a2
        aes64dsm    a0, a1, a2
        aes64im     a0, a0
        aes64ks1i   a0, a1, 4
        aes64ks2    a0, a1, a2
        aes64es     a0, a1, a2
        aes64esm    a0, a1, a2
        sha256sig0  a0, a0
        sha256sig1  a0, a0
        sha256sum0  a0, a0
        sha256sum1  a0, a0
        sha512sig0  a0, a0
        sha512sig1  a0, a0
        sha512sum0  a0, a0
        sha512sum1  a0, a0
        sm4ed   a0, a1, a2, 2
        sm4ks   a0, a1, a2, 2
        sm3p0   a0, a0
        sm3p1   a0, a0
