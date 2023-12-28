#ld: -T assign-loc.t
#source: data.s
#nm: -n

#...
0+0100 A HEAP_SIZE
#...
0+2000 [AB] _start
#...
0+2100 [AB] _end
#pass
