#as: --32
#ld: -melf_i386 -shared
#readelf: -r --wide -s

There are no relocations in this file.
#...
Symbol table '.symtab' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
[ \t]+[a-f0-9]+: [a-f0-9]+ +1 IFUNC +GLOBAL DEFAULT +[a-f0-9]+ +foo
#...
