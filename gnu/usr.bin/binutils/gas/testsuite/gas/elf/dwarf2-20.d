#as: -gdwarf-3
#readelf: -wr
#name: DWARF2_20: debug ranges ignore non-code sections
# The mn10200 target has a pointer size of 3, but it does not use segment selectors.  This confuses DWARF and readelf will complain.
#xfail: mn102*-*
# score-elf, tic6x-elf and xtensa-elf need special handling to support .nop 16
#xfail: score-* tic6x-* xtensa-*

Contents of the .debug_aranges section:

[ 	]+Length:[ 	]+(16|28|44)
[ 	]+Version:.*
[ 	]+Offset into .debug_info:[ 	]+(0x)?0
[ 	]+Pointer Size:[ 	]+(2|4|8)
[ 	]+Segment Size:[ 	]+0

[ 	]+Address[ 	]+Length
[ 	]+0+000 0+010 ?
[ 	]+0+000 0+000 ?
#pass
