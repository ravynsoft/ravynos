Relocation section '\.rel(a|)\.plt' at offset 0x[0-9a-f]+ contains 1 entry:
 +Offset +Info +Type +Sym.* Value +Sym.* Name( \+ Addend|)
[a-f0-9]+ +[0-9a-f]+ +R_.*_JUMP_SLOT +[a-f0-9]+ +foo( \+ 0|)

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
 +[0-9]+: +[a-f0-9]+ +0 +FUNC +GLOBAL +DEFAULT +UND +foo
