#source: symver10.s
#readelf: -sW
#name: symver symver10b

#failif
#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo
#pass
