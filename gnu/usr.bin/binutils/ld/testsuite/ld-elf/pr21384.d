#ld: -T pr21384.ld --dynamic-list $srcdir/$subdir/pr21384.dl --no-dynamic-linker
#readelf : --dyn-syms --wide
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi

Symbol table '\.dynsym' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +GLOBAL +DEFAULT +[0-9]+ +edata
#...
 +[0-9]+: +[a-f0-9]+ +0 +NOTYPE +GLOBAL +DEFAULT +[0-9]+ +fdata
#pass
