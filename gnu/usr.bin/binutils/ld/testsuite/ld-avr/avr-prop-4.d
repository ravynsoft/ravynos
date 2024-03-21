#name: AVR .avr.prop, realign .align test.
#as: -mmcu=avrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-4.s
#nm: -n
#target: avr-*-*

#...
00000004 T dest
#...
