#as: --32
#ld: -pie -Bsymbolic -E -melf_i386
#readelf: -r --wide --dyn-syms

Relocation section '.rel.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type                Sym. Value  Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_RELATIVE +

Symbol table '.dynsym' contains [0-9]+ entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
#...
[ 	]*[a-f0-9]+: [a-f0-9]+     0 FUNC    GLOBAL DEFAULT    [a-f0-9]+ xyzzy
#...
