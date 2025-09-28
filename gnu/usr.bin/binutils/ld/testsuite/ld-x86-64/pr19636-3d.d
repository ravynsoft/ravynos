#source: pr19636-3.s
#as: --64
#ld: -E --defsym foobar=0x100 -m elf_x86_64
#readelf : --dyn-syms --wide

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
#...
 +[0-9]+: +0+100 +0 +NOTYPE +GLOBAL +DEFAULT +ABS +foobar
#pass
