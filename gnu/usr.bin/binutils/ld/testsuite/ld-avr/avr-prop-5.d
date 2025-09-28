#name: AVR .avr.prop, single .align proper sym val test.
#as: -mmcu=avrxmega2 -mlink-relax
#ld: -mavrxmega2 --relax
#source: avr-prop-5.s
#objdump: -S
#target: avr-*-*

#...
   0:	00 d0\s+rcall\s+\.\+0\s+; 0x2 <dest>
#...