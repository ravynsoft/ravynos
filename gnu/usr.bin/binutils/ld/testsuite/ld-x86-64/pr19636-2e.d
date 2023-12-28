#source: pr19636-2.s
#as: --64 -mrelax-relocations=no
#ld: -shared -m elf_x86_64 --no-dynamic-linker
#readelf : -r --wide --dyn-syms

Relocation section '\.rela\.dyn' at offset [0x0-9a-f]+ contains 2 entries:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +0+ +func1 \+ 0
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_GLOB_DAT +0+ +func2 \+ 0

Relocation section '\.rela\.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Symbol's Value +Symbol's Name \+ Addend
[0-9a-f]+ +[0-9a-f]+ +R_X86_64_JUMP_SLOT +0+ +func3 \+ 0

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +WEAK +DEFAULT +UND +func[0-9]?
#pass
