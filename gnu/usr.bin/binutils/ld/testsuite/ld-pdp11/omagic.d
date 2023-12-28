#name: pdp11-aout omagic format
# also testing that --omagic is the default
#source: sections.s
#ld:
#nm: -n
#...
0+0 T _start
#...
0+6 D _data
#...
0+8 B _bss
#pass
