#source: default-script.s
#ld: -T default-script.t -defsym _START=0x800
#nm: -n

#...
0*800 . _START
#...
0*900 T text
#pass
