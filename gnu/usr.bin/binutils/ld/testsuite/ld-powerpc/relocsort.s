# Deliberately create out-of-order relocs to test that ld -r sorts them.
 .text
 .long 0, 0, 0
 .reloc 4, R_PPC64_REL24, __tls_get_addr
 .reloc 4, R_PPC64_TLSGD, x
 .reloc 0, R_PPC64_GOT_TLSGD16, x
 .reloc 0, R_PPC64_NONE, 0
 .reloc 0, R_PPC64_ADDR32, 1
 .reloc 0, R_PPC64_ADDR24, 2
 .reloc 8, R_PPC64_REL24, 10
 .reloc 8, R_PPC64_REL14, 11
 .reloc 8, R_PPC64_REL14_BRTAKEN, 12
 .reloc 0, R_PPC64_ADDR16, 3
 .reloc 0, R_PPC64_ADDR16_LO, 4
 .reloc 0, R_PPC64_ADDR16_HI, 5
 .reloc 0, R_PPC64_ADDR16_HA, 6
 .reloc 0, R_PPC64_ADDR14, 7
 .reloc 8, R_PPC64_REL14_BRNTAKEN, 13
 .reloc 8, R_PPC64_GOT16, 14
 .reloc 8, R_PPC64_GOT16_LO, 15
 .reloc 8, R_PPC64_GOT16_HI, 16
 .reloc 8, R_PPC64_GOT16_HA, 17
 .reloc 8, R_PPC64_NONE, 18
 .reloc 8, R_PPC64_COPY, 19
 .reloc 0, R_PPC64_ADDR14_BRTAKEN, 8
 .reloc 0, R_PPC64_ADDR14_BRNTAKEN, 9
