#source: loc9.s
#ld: -e Boot -m mmo
#objdump: -str

# Setting file start through the LOC pseudo, see PR 6607, mmo version.

.*:     file format mmo

SYMBOL TABLE:
#...
8000000000000000 g       \.text Boot
#...
Contents of section \.text:
 8000000000000000 f4000000                             .*
