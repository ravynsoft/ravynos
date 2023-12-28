# this checks that aarch64 RELA relocs are ignoring existing section
# content of the relocated place
       .text
       .global _start
_start:
       ret

       .data
       .p2align 4
l:     .long   0x11111111, 0x22222222
q:     .quad   0x4444444433333333

       .reloc l, BFD_RELOC_64, q+42
       .reloc q, BFD_RELOC_64, l+84
