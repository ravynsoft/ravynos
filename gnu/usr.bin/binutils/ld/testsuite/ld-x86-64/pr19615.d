#as: --64
#ld: -pie -Bsymbolic -E -melf_x86_64
#readelf: -r --wide --dyn-syms

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
    Offset             Info             Type               Symbol's Value  Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_RELATIVE +[0-9]+

Symbol table '.dynsym' contains [0-9]+ entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
#...
[ 	]*[a-f0-9]+: [a-f0-9]+     0 FUNC    GLOBAL DEFAULT    [a-f0-9]+ xyzzy
#...
