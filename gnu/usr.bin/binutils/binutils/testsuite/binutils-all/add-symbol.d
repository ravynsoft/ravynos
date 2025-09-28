#PROG: objcopy
#name: objcopy add-symbol
#source: symbols.s
#objcopy: --add-symbol NEW=0x1234 --add-symbol NEW_DATA=.data:0x4321,local
#objdump: --syms
#section_subst: no

.*: +file format .*

SYMBOL TABLE:
#...
(0+04321 l[ 	]+.data[ 	]+0+00 NEW_DATA|0+01234 g[ 	]+\*ABS\*[ 	]+0+00 NEW)
#...
(0+01234 g[ 	]+\*ABS\*[ 	]+0+00 NEW|0+04321 l[ 	]+.data[ 	]+0+00 NEW_DATA)
#pass
