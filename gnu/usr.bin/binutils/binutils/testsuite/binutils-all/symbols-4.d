#name: weaken '*' but not 'foo' or 'bar'
#PROG: objcopy
#objcopy: -w -W \!foo -W \!bar -W \*
#source: symbols.s
#nm: -n

#...
0+ D bar
0+ [VW] foa
0+ [VW] fob
0+ D foo
0+ [VW] foo1
0+ [VW] foo2

