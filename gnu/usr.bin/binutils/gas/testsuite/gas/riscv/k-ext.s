target:
        ror     a0, a1, a2
        rol     a0, a1, a2
        rori    a0, a1, 2
        andn    a0, a1, a2
        orn     a0, a1, a2
        xnor    a0, a1, a2
        pack    a0, a1, a2
        packh   a0, a1, a2
        brev8   a0, a0
        rev8    a0, a0
        zip     a0, a0
        unzip   a0, a0
        clmul   a0, a1, a2
        clmulh  a0, a1, a2
        xperm4  a0, a1, a2
        xperm8  a0, a1, a2
        aes32dsi    a0, a1, a2, 2
        aes32dsmi   a0, a1, a2, 2
        aes32esi    a0, a1, a2, 2
        aes32esmi   a0, a1, a2, 2
        sha256sig0  a0, a0
        sha256sig1  a0, a0
        sha256sum0  a0, a0
        sha256sum1  a0, a0
        sha512sig0h    a0, a1, a2
        sha512sig0l    a0, a1, a2
        sha512sig1h    a0, a1, a2
        sha512sig1l    a0, a1, a2
        sha512sum0r    a0, a1, a2
        sha512sum1r    a0, a1, a2
        sm4ed   a0, a1, a2, 2
        sm4ks   a0, a1, a2, 2
        sm3p0   a0, a0
        sm3p1   a0, a0
