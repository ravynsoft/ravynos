#ld: -d -r
#readelf: -s

#...
.*: [0-9a-f]+ +4 +OBJECT +GLOBAL +DEFAULT +[0-9]+ foo
#pass
