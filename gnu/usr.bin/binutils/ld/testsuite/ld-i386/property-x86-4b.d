#source: property-x86-4a.s
#source: property-x86-4b.s
#as: --32
#ld: -m elf_i386 --gc-sections --entry=main
#readelf: -S --wide

#failif
#...
 +\[ [0-9]\] .debug_info.*
#...
