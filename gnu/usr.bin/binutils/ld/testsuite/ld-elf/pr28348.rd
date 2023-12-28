#ld: -shared
#readelf: --dyn-syms --wide
#target: *-*-linux* *-*-gnu* arm*-*-uclinuxfdpiceabi
#xfail: ![check_shared_lib_support]

#...
 +[0-9]+: +[0-9a-f]+ +[0-9a-f]+ +FUNC +WEAK +DEFAULT (\[NOPV\]|) +[0-9]+ +_?foo
#pass
