#name: AVR .avr.prop, single .org test.
#as: -mmcu=avrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-1.s
#nm: -n
#target: avr-*-*

#...
00000020 T dest
#...
