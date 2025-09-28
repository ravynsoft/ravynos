#as: --64 --elf-stt-common=yes
#ld: -shared -Bsymbolic-functions -melf_x86_64
#readelf: -r --wide --dyn-syms

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +[0-9a-f]+ +foobar \+ 0

Symbol table '.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
#...
 +[0-9]+: +[0-9a-f]+ +30 +OBJECT +GLOBAL +DEFAULT +[0-9]+ foobar
#pass
