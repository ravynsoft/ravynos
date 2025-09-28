#name: Symbol flags copy
#ld: -T attributes.ld
#objdump: -t
#xfail: [is_generic]

#...
0+0000000 g     F .text	0+0000000 __start
#...
0+0000000 g     F .text	0+0000000 start
#pass
