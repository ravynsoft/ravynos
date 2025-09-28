#as: --32
#ld: --no-dynamic-linker -m elf_i386 -pie
#readelf: -s --wide

#...
Symbol table '.symtab' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
 +[a-f0-9]+: 00000002     0 NOTYPE  LOCAL  DEFAULT  ABS foo
#pass
