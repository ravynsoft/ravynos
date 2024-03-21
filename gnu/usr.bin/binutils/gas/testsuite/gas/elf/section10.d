#readelf: -N --wide
#name: numeric section flags and types
# The RX port annoyingly reorders the sections so that they do not match the sequence expected below.
#xfail: rx-*-*

#...
[ 	]*\[.*\][ 	]+.text
[ 	]*PROGBITS.*
[ 	]*\[.*4000006\]: ALLOC, EXEC, OS \(.*4000000\)
#...
[ 	]*\[.*\][ 	]+sec1
[ 	]*PROGBITS.*
[ 	]*\[.*6000000\]: OS \(.*6000000\)
[ 	]*\[.*\][ 	]+sec2
[ 	]*PROGBITS.*
[ 	]*\[0+00086\]: ALLOC, EXEC, LINK ORDER
#...
[ 	]*\[.*\][ 	]+sec3
[ 	]*PROGBITS.*
[ 	]*\[.*fedff030\]: MERGE, STRINGS,.* EXCLUDE, OS \(.*ed00000\), PROC \(.*[3467]0000000\), UNKNOWN \(0+0ff000\)
#...
[ 	]*\[.*\][ 	]+sec4
[ 	]*LOOS\+0x11[ 	].*
[ 	]*\[0+06\]: ALLOC, EXEC
#...
[ 	]*\[.*\][ 	]+sec5
[ 	]*LOUSER\+0x9[ 	].*
[ 	]*\[.*fedf0000\]:.* EXCLUDE, OS \(.*ed00000\), PROC \(.*[3467]0000000\), UNKNOWN \(.*f0000\)
[ 	]*\[.*\][ 	]+.data.foo
[ 	]*LOUSER\+0x7f000000[ 	].*
[ 	]*\[0+003\]: WRITE, ALLOC
[ 	]*\[.*\][ 	]+sec6
[ 	]*0000162e: <unknown>[ 	].*
[ 	]*\[.*120004\]: EXEC, OS \(.*100000\), UNKNOWN \(.*20000\)
#pass
