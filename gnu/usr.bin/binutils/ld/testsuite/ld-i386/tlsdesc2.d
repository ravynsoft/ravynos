#source: tlsdesc.s
#as: --32
#ld: -melf_i386 -shared -z now
#readelf: -d --wide

#...
.*\(PLTRELSZ\).*
.*\(PLTREL\).*
.*\(JMPREL\).*
#pass
