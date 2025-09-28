#name: AVR local symbol size adjustment across alignment boundary
#as: -mmcu=avrxmega2 -mlink-relax
#ld:  -mavrxmega2 --relax
#source: pr21404-6.s
#nm: -n -S
#target: avr-*-*

#...
00000000 00000006 t main
00000000 00000004 t size_after_align
00000000 00000004 t size_before_align
#...
00000002 00000002 t nonzero_sym_after_align
00000002 00000004 t nonzero_sym_after_end
00000002 00000002 t nonzero_sym_before_align
#...
