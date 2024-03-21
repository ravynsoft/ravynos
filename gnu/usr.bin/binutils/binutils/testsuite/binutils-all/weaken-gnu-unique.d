#name: weaken STB_GNU_UNIQUE symbols
#PROG: objcopy
#objcopy: -W foo
#source: unique.s
#readelf: -s

#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +OBJECT +WEAK +DEFAULT +[1-9] foo
