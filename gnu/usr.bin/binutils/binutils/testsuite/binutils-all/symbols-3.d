#name: weaken 'fo*' but not 'foo', localize foo.
#PROG: objcopy
#objcopy: -w -W \!foo -W fo\* -L foo
#source: symbols.s
#nm: -n

#...
0+ D bar
0+ [VW] foa
0+ [VW] fob
0+ d foo
0+ [VW] foo1
0+ [VW] foo2

