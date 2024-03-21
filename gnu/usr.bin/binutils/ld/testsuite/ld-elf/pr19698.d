#ld: -shared $srcdir/$subdir/pr19698.t
#readelf : --dyn-syms --wide
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support] 

Symbol table '\.dynsym' contains [0-9]+ entries:
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@VERS.1
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@@VERS.2
#pass
