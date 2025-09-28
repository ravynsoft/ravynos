#source: pr19636-1.s
#as: --64
#ld: -shared -Bsymbolic -m elf_x86_64 -z notext
#readelf : -r --wide --dyn-syms

Relocation section '\.rela?\..*' at offset 0x[0-9a-f]+ contains [0-9]+ entr(y|ies):
#...
[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+R_.*[ \t]+[0-9a-f]+[ \t]+func.*
#...
Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func
#...
