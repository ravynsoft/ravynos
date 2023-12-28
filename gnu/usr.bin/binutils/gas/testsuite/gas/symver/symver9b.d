#source: symver9.s
#readelf: -sW
#name: symver symver9b

#failif
#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo
#pass
