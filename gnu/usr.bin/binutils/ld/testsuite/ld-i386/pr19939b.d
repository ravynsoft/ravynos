#source: pr19939.s
#as: --32
#ld: -melf_i386 -shared -z notext
#readelf: -d --wide

#...
.*\(TEXTREL\).*
#pass
