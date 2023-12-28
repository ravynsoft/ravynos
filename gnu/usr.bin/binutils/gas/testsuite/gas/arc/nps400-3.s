        .macro  xldst_test mnem
        \mnem r12, [ 0x0 ]
        \mnem r23, [ 0xffff ]
        \mnem r3, [ 0x57f00000 ]
        \mnem r11, [ 0x57f0ffff ]
        \mnem r20, [ foo ]
        \mnem r1, [ foo + 0x20 ]
        .endm

        .text
        ;; xldb
        xldst_test xldb
        ;; xldw
        xldst_test xldw
        ;; xld
        xldst_test xld
        ;; xstb
        xldst_test xstb
        ;; xstw
        xldst_test xstw
        ;; xst
        xldst_test xst

