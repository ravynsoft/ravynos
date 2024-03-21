#as: --32
#ld: -shared -melf_i386 $NO_DT_RELR_LDFLAGS
#readelf: -r -s --wide

Relocation section '.rel.dyn' at offset 0x[a-f0-9]+ contains 1 entry:
 Offset     Info    Type                Sym. Value  Symbol's Name
0+[a-f0-9]+  00000008 R_386_RELATIVE        

#...
Symbol table '.symtab' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
 +[a-f0-9]+: 00000000     0 NOTYPE  LOCAL  DEFAULT    1 __ehdr_start
#pass
