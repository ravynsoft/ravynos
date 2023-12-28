#source: default-script.s
#ld: --default-script default-script.t -defsym _START=0x800
#nm: -n

#...
0*800 . _START
#...
0*800 T text
#pass
