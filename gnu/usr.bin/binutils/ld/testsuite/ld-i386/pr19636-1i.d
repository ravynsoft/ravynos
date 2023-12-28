#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -E -m elf_i386 --no-dynamic-linker
#readelf : --wide --dyn-syms

#failif
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func[0-9]?
#...
