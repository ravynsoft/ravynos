# name: jsriv2_3 - csky
#as: -mcpu=ck807
#readelf: -r --wide


Relocation section '.rela.text\' at offset .* contains 1 entry:
#...
00000008.*R_CKCORE_ADDR32[ \t]+00000000[ \t]+\.text \+[ \t]+[0-9a-f]+
#pass
