#source: pr19636-3.s
#as: --64
#ld: -pie --defsym foobar=0x100 -m elf_x86_64
#readelf : --dyn-syms --wide

Symbol table '\.dynsym' contains 1 entry:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
