#source: sizeof.s
#ld: -r
#readelf: -sW

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size Type +Bind +Vis +Ndx Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND +
#...
 +[0-9]+: +[a-f0-9]+ +0 +(NOTYPE|OBJECT) +GLOBAL +DEFAULT +UND +___?stop_scnfoo
#...
 +[0-9]+: +[a-f0-9]+ +0 +(NOTYPE|OBJECT) +GLOBAL +DEFAULT +UND +.sizeof.scnfoo
#pass
