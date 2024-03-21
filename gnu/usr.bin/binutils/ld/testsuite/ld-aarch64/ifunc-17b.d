#source: ifunc-17b.s
#source: ifunc-17a.s
#ld: -static
#readelf: -s --wide
#target: aarch64*-*-*

#...
 +[0-9]+: +[0-9a-f]+ +4 +OBJECT +GLOBAL +DEFAULT +[1-9] foo
#pass
