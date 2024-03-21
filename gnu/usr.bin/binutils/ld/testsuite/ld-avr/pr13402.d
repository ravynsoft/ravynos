#name: AVR fix broken sync between debug_line and code addresses
#as: -mmcu=avrxmega2 -mlink-relax -gdwarf-2
#ld:  -mavrxmega2 --relax
#source: pr13402.s
#objdump: -S
#target: avr-*-*

#...
main:
call a
call b
   0:	02 d0       	rcall	.+4      	; 0x6 <_etext>
#...
call c
   2:	01 d0       	rcall	.+2      	; 0x6 <_etext>
#...
   4:	00 d0       	rcall	.+0      	; 0x6 <_etext>
#...
