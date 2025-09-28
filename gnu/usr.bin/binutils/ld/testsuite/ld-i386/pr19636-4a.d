#source: pr19636-4.s
#as: --32
#ld: -pie --defsym foobar=0x100 -m elf_i386
#readelf : --dyn-syms --wide

Symbol table '\.dynsym' contains 1 entry:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
