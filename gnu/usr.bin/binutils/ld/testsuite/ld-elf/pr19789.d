#ld: -r -E
#readelf: -s --wide
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi

Symbol table '\.symtab' contains [0-9]+ entries:
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@@VERS.2
#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@VERS.1
#pass
