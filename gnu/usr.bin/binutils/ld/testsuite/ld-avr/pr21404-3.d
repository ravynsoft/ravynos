#name: AVR symbol value adjustment with non zero symbol value
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: pr21404-3.s
#nm: -n -S
#target: avr-*-*

#...
00000006 T nonzero_sym
#...
