#name: AVR .avr.prop, single .align test.
#as: -mmcu=avrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-3.s
#nm: -n
#target: avr-*-*

#...
00000008 T dest
#...
