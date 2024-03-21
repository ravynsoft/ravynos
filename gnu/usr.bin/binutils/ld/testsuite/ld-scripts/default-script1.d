#source: default-script.s
#ld: -defsym _START=0x800 -T default-script.t
#nm: -n

#...
0*800 . _START
#...
0*800 T text
#pass
