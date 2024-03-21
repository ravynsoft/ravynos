#readelf: -sW
#name: symver symver7

#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +HIDDEN +[0-9]+ +foo
#...
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@version1
 +[0-9]+: +0+ +1 +OBJECT +GLOBAL +DEFAULT +[0-9]+ +foo@@version2
#pass
