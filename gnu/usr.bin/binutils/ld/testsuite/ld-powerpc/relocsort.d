#source: relocsort.s
#as: -a64
#ld: -r -melf64ppc
#readelf: -r --wide

.* contains 23 entries:
.*
0+0 +[0-9a-f]+ R_PPC64_GOT_TLSGD16 +0+0 x \+ 0
0+0 +[0-9a-f]+ R_PPC64_NONE +0
0+0 +[0-9a-f]+ R_PPC64_ADDR32 +1
0+0 +[0-9a-f]+ R_PPC64_ADDR24 +2
0+0 +[0-9a-f]+ R_PPC64_ADDR16 +3
0+0 +[0-9a-f]+ R_PPC64_ADDR16_LO +4
0+0 +[0-9a-f]+ R_PPC64_ADDR16_HI +5
0+0 +[0-9a-f]+ R_PPC64_ADDR16_HA +6
0+0 +[0-9a-f]+ R_PPC64_ADDR14 +7
0+0 +[0-9a-f]+ R_PPC64_ADDR14_BRTAKEN +8
0+0 +[0-9a-f]+ R_PPC64_ADDR14_BRNTAKEN +9
0+4 +[0-9a-f]+ R_PPC64_REL24 +0+0 __tls_get_addr \+ 0
0+4 +[0-9a-f]+ R_PPC64_TLSGD +0+0 x \+ 0
0+8 +[0-9a-f]+ R_PPC64_REL24 +a
0+8 +[0-9a-f]+ R_PPC64_REL14 +b
0+8 +[0-9a-f]+ R_PPC64_REL14_BRTAKEN +c
0+8 +[0-9a-f]+ R_PPC64_REL14_BRNTAKEN +d
0+8 +[0-9a-f]+ R_PPC64_GOT16 +e
0+8 +[0-9a-f]+ R_PPC64_GOT16_LO +f
0+8 +[0-9a-f]+ R_PPC64_GOT16_HI +10
0+8 +[0-9a-f]+ R_PPC64_GOT16_HA +11
0+8 +[0-9a-f]+ R_PPC64_NONE +12
0+8 +[0-9a-f]+ R_PPC64_COPY +13
