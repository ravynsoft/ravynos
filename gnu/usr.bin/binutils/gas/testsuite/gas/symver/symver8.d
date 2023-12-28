#readelf: -sW
#name: symver symver8

#...
 +[0-9]+: +0+ +1 +OBJECT +LOCAL +DEFAULT +[0-9]+ +foo
#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@version1
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@@version2
#pass
