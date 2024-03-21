#name: AVR symbol size adjustment with non zero symbol value
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: pr21404-1.s
#nm: -n -S
#target: avr-*-*

#...
00000000 00000004 T main
#...
00000002 00000002 T nonzero_sym
#...
