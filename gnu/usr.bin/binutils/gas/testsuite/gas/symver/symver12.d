#readelf: -sW
#name: symver symver12

#...
 +[0-9]+: +0+d +1 +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo
 +[0-9]+: +0+d +1 +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@VERS_2
 +[0-9]+: +0+d +1 +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@VERS_1
 +[0-9]+: +0+d +1 +FUNC +GLOBAL +DEFAULT +[0-9]+ +foo@@VERS_2
#pass
