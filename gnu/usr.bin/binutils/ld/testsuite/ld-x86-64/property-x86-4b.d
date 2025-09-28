#source: property-x86-4a.s
#source: property-x86-4b.s
#as: --64 -defsym __64_bit__=1
#ld: -m elf_x86_64 --gc-sections --entry=main
#readelf: -S --wide

#failif
#...
 +\[ [0-9]\] .debug_info.*
#...
