# name: jmpi2 - csky
#as: -mcpu=ck610
#readelf: -r --wide


Relocation section '.rela.text\' at offset .* contains 1 entry:
#...
.*R_CKCORE_ADDR32[ \t]+00000000[ \t]+\.text \+[ \t]+[0-9a-f]+
#pass
