; Test C64x+ L, S or D-unit compact instruction formats
        .text
        nop
        .align 16
        nop
        .align 16
lsdmvto:
        .short 0x0006
        .short 0x000f
        .short 0x0016
        .short 0x0017
        .short 0x000e
        .short 0x0007
        .short 0x0006
        .short 0x100f
        .short 0x1016
        .short 0x1017
        .short 0x100e
        .short 0x1007
        .short 0x1006
        .short 0x100f
        .word  0xefe00000 | 0x0000
lsdmvfr:
        .short 0x0046
        .short 0x004f
        .short 0x0056
        .short 0x0057
        .short 0x004e
        .short 0x0047
        .short 0x0046
        .short 0x104f
        .short 0x1056
        .short 0x1057
        .short 0x104e
        .short 0x1047
        .short 0x1046
        .short 0x104f
        .word  0xefe00000 | 0x0000
lsdx1c:
        .short 0x0866
        .short 0x4967
        .short 0x8ae6
        .short 0xcbe7
        .short 0x886e
        .short 0x496f
        .short 0x0aee
        .short 0x6bef
        .short 0xa876
        .short 0xe977
        .short 0xaaf6
        .short 0x6bf7
        .short 0x2866
        .short 0x6967
        .word  0xefe00000 | 0x0000
lsdx1:
        .short 0x1866
        .short 0x1867
        .short 0x1866
        .short 0x3867
        .short 0x3866
        .short 0x3877
        .short 0x3876
        .short 0xb877
        .short 0xb876
        .short 0xb86f
        .short 0xf86e
        .short 0xf86f
        .short 0xf86e
        .short 0xf86f
        .word  0xefe00000 | 0x0000

