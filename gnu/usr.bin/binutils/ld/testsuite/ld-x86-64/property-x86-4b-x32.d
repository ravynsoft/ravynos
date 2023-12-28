#source: property-x86-4a.s
#source: property-x86-4b.s
#as: --x32
#ld: -m elf32_x86_64 --gc-sections --entry=main
#readelf: -S --wide

#failif
#...
 +\[ [0-9]\] .debug_info.*
#...
