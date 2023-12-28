#source: tlsdesc.s
#as: --64
#ld: -melf_x86_64 -shared -z now
#readelf: -d --wide

#...
.*\(PLTRELSZ\).*
.*\(PLTREL\).*
.*\(JMPREL\).*
#pass
