#source: pr19617.s
#ld: -E --no-dynamic-linker --hash-style=sysv
#readelf : --dyn-syms --wide
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi
#xfail: h8300-*-*

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
#...
 +[0-9]+: +[a-f0-9]+ +0 +FUNC +GLOBAL +DEFAULT +[0-9]+ +start
#...
 +[0-9]+: +[a-f0-9]+ +0 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +bar
#pass
