#source: dt-relr-1.s
#as: --32
#ld: -shared -melf_i386 $DT_RELR_LDFLAGS
#readelf: -r -s --wide
#target: x86_64-*-linux* i?86-*-linux-gnu i?86-*-gnu*

Relocation section '\.rel\.dyn' at offset [0x0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_32 +0+ +data1
[0-9a-f]+ +[0-9a-f]+ +R_386_32 +0+ +data1

Relocation section '\.rel\.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_JUMP_SLOT +0+ +func1

Relocation section '.relr.dyn' at offset 0x[a-f0-9]+ contains 2 entries:
 +3 offsets
[a-f0-9]+
[a-f0-9]+
[a-f0-9]+

#...
Symbol table '.symtab' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
 +[a-f0-9]+: 00000000     0 NOTYPE  LOCAL  DEFAULT    1 __ehdr_start
#pass
