#source: hidden2.s
#ld: -shared -T hidden2.ld --hash-style=sysv
#readelf: -Ds
# It is also ok to remove this symbol, but we currently make it local.

Symbol table for image contains [0-9]+ entries:
#...
[ 	]*[0-9]+: [0-9a-fA-F]* +0 +OBJECT +LOCAL +DEFAULT .* foo
#pass
