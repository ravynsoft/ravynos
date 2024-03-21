#name: STB_GNU_UNIQUE with -Bsymbolic
#as: --64
#ld: -melf_x86_64 -shared -Bsymbolic
#readelf: -rs --wide

#...
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +[0-9a-f]+ bar \+ 0
#...
 +[0-9]+: +[0-9a-f]+ +8 +OBJECT +UNIQUE +DEFAULT +[0-9]+ bar
#pass
