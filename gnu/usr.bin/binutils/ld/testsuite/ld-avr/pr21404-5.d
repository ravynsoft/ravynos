#name: AVR local symbol size adjustment with non zero symbol value
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: pr21404-5.s
#nm: -n -S
#target: avr-*-*

#...
00000000 00000004 t _main
00000002 00000002 t _nonzero_sym
#...
