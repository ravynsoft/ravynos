# target: mips*-*-*
# source: plt-mips1.s
# ld: -shared -z now
# readelf: -s

#...
 +[0-9]*: 00000000 +0 +FUNC +GLOBAL +DEFAULT +UND Foo
#...
 +[0-9]*: 00000000 +0 +FUNC +GLOBAL +DEFAULT +UND Foo
#...
