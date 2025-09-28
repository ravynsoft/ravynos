#source: loc10.s
#ld: -m mmo --defsym __.MMIX.start..text=0x8000000000000000
#objdump: -str

# Setting file start through the special symbol, mmo version.

.*:     file format mmo
#...
Contents of section \.text:
 8000000000000000 f4000000                             .*
