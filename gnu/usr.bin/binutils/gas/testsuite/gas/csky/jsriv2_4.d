# name: jsriv2_4 - csky
#as: -mcpu=ck810
#readelf: -r --wide


Relocation section '.rela.text\' at offset .* contains 1 entry:
#...
0000000c.*R_CKCORE_ADDR32[ \t]+00000000[ \t]+\.text \+[ \t]+[0-9a-f]+
#pass
