#source: pr19636-3.s
#as: --32
#ld: -E -m elf_i386
#readelf : --dyn-syms --wide

#failif
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func
#...
