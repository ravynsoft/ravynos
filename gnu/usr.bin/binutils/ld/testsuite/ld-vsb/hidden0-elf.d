#source: define.s
#source: undef.s
#ld: -r
#readelf: -s

Symbol table '.symtab' contains .* entries:
#...
[ 	]*[0-9]+: [0-9a-fA-F]* +0 +OBJECT +GLOBAL +HIDDEN +. hidden
#pass
