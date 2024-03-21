#name: localize 'fo*' but not 'foo'
#PROG: objcopy
#objcopy: -w -L \!foo -L fo\*
#source: symbols.s
#nm: -n

#...
0+ D bar
0+ d foa
0+ d fob
0+ D foo
0+ d foo1
0+ d foo2

