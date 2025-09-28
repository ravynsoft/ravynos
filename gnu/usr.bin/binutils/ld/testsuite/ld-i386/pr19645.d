#as: --32 --elf-stt-common=yes
#ld: -shared -Bsymbolic-functions -melf_i386
#readelf: -r --wide --dyn-syms

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_GLOB_DAT +[0-9a-f]+ +foobar

Symbol table '.dynsym' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
 +[0-9]+: +[0-9a-f]+ +30 +OBJECT +GLOBAL +DEFAULT +[0-9]+ foobar
#pass
