#source: pr19617.s
#ld: --dynamic-list-data --no-dynamic-linker
#readelf : --dyn-syms --wide
#target: *-*-linux* *-*-gnu* *-*-solaris* arm*-*-uclinuxfdpiceabi

#failif
#...
 +[0-9]+: +[a-f0-9]+ +0 +FUNC +GLOBAL +DEFAULT +[0-9]+ +start
#...
