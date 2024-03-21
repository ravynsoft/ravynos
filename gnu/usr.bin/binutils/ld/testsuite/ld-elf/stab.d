#source: start.s
#as: -gstabs
#readelf: -S --wide
#ld:
#notarget: "ia64-*-*" "alpha*"

# Disabled on alpha because the entry point may be above 4GB but the stabs
# value only 32 bits.

#...
.* \.stab +PROGBITS +0+ [0-9a-f]+ [0-9a-f]+ [0-9a-f]+ +[1-9][0-9]* +0.*
#...
.* \.stabstr +STRTAB +0+ [0-9a-f]+ [0-9a-f]+ 00 +0 +0.*
#...
