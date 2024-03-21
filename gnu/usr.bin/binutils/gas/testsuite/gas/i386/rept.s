        .data
        .global _pad_data
_pad_data:
        .rept (0x40000*210) 
        .byte 0
        .endr
