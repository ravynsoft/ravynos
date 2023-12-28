        .syntax unified
        .cpu cortex-a9
        .text
        .thumb
        strd    r10,r11,[r10], #4	@ Unpredictable
        strd    r10,r11,[r11], #4	@ Ditto
        strd    r4,r6,[r4, #4]!		@ Ditto
        strd    r4,r6,[r6, #4]!		@ Ditto

        ldrd    r4,r6,[r4, #4]!		@ Ditto
        ldrd    r4,r6,[r6, #4]!		@ Ditto
        ldrd    r10,r11,[r10], #4	@ Ditto
        ldrd    r10,r11,[r11], #4	@ Ditto
