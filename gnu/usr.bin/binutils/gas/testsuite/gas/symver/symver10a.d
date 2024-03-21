#source: symver10.s
#readelf: -sW
#name: symver symver10a

#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@version1
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@@version2
#pass
