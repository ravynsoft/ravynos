# name: csky - nolrw
#as: -mcpu=ck810 -mnolrw -W
#readelf: -r --wide


Relocation section '.rela.text\' at offset .* contains 4 entries:
#...
00000020.*R_CKCORE_ADDR_HI16\s*00000030\s*L1\s*\+\s*0
00000024.*R_CKCORE_ADDR_LO16\s*00000030\s*L1\s*\+\s*0
00000028.*R_CKCORE_ADDR_HI16\s*00000030\s*L1\s*\+\s*0
0000002c.*R_CKCORE_ADDR_LO16\s*00000030\s*L1\s*\+\s*0
#pass
