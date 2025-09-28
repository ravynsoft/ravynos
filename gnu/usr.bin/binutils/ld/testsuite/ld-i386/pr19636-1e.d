#source: pr19636-1.s
#as: --32 -mrelax-relocations=no
#ld: -shared -m elf_i386 --no-dynamic-linker
#readelf : -r --wide --dyn-syms

Relocation section '\.rel\.dyn' at offset [0x0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_GLOB_DAT +0+ +func1
[0-9a-f]+ +[0-9a-f]+ +R_386_GLOB_DAT +0+ +func2

Relocation section '\.rel\.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym. Value +Symbol's Name
[0-9a-f]+ +[0-9a-f]+ +R_386_JUMP_SLOT +0+ +func3

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func[0-9]?
#pass
