#source: pr19636-1.s
#as: --64
#ld: -E -m elf_x86_64
#readelf : --dyn-syms --wide

#failif
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func
#...
